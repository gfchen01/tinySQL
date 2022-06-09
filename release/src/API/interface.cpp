//
// Created by luke on 22-6-3.
//

#include "API/interface.h"
#include "share/data_t.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <strstream>
#include <cassert>
#include <iomanip>

/**
 * This sort is actually a filter
 *
 * @param attr_names
 * @param attr
 */
void sortAttrNames(std::vector<std::string> &attr_names, Attribute& attr){
    if (attr_names.empty()) return;
    std::vector<std::string> sorted;
    for (int i = 0; i < attr.num; ++i){
        int j;
        for (j = 0; j < attr_names.size(); ++j){
            if (attr_names.at(j) == attr.name[i]){
                sorted.emplace_back(attr_names.at(j));
                break;
            }
        }
        if (j == attr_names.size()){
            throw DB_COLUMN_NAME_NOT_EXIST; // Don't allow attr_names not in the correct range
        }
    }
    attr_names = sorted;
}

void Interface::parseWhere(hsql::Expr *Clause, std::vector<Where> &where_vec){
    // For where clause, if one side is kExprOperator, then the other side has to be kExprOperator
    if (Clause == nullptr) return;
    if (Clause->expr->type == hsql::kExprOperator){
        assert(Clause->expr2->type == hsql::kExprOperator);
        parseWhere(Clause->expr, where_vec);
        parseWhere(Clause->expr2, where_vec);
    }
    else{
        assert(Clause->expr->type == hsql::kExprColumnRef);

        Where w;
        w.attr_name = Clause->expr->name;
        w.relation_operator = op_map.at(Clause->opType);
        w.data = Data(Clause->expr2);

        where_vec.push_back(w);
    }
}

void Interface::showErrMsg(db_err_t &dbErr) {
    switch (dbErr) {
        case DB_KEY_NOT_FOUND:{
            _os << "Table doesn't have the given attribute or value\n";
            break;
        }
        case DB_FILE_NOT_FOUND:{
            _os << "File doesn't exist\n";
            break;
        }
        case DB_TABLE_ALREADY_EXIST:{
            _os << "Table already exist\n";
            break;

        }
        case DB_COLUMN_NAME_NOT_EXIST:{
            _os << "Table attribute not exist\n";
            break;

        }
        case DB_INDEX_ALREADY_EXIST:{
            _os << "Index already exist on given attribute or has_index name already exist\n";
            break;

        }
        case DB_INDEX_NOT_FOUND:{
            _os << "Index doesn't exist\n" ;
            break;

        }
        case DB_PRIMARY_KEY_CONFLICT:{
            _os << "DB PRIMARY CONFLICT!\n";
            break;
        }
        case DB_TABLE_NOT_EXIST:{
            _os << "Table doesn't exist!\n";
        }
        default:{
            _os << "DB ERROR: " << ERR_STR_MAP.at(dbErr) << std::endl;
        }
    }
}

void Interface::readFromFile(std::string &query) {
    std::string file_path;
    _os << "Input file name(relative path, end with ENTER): > ";
    _os.flush();

    _is.clear();
    _is >> file_path;
//    std::getline(_is, file_path);

    std::ifstream ifs;
    ifs.open(file_path, std::fstream::in);
    if (!ifs.is_open()){
        _os << "File doesn't exist." << std::endl;
        _os.flush();
        return;
    }

    std::ostrstream str_buf;
    char ch;
    while(str_buf && ifs.get(ch)){
        str_buf.put(ch);
    }
    query = str_buf.str();
    return;
}


void Interface::run() {
    std::string query;
    hsql::SQLParserResult result;
    const hsql::SQLStatement* statement;

    _os << "TinySQL started.\nInput below.\n";
    _os.flush();

    int loop_counter = 0;
    while(true){
        query.clear();
        std::getline(_is, query, ';');

        if (query.front() == '\n') query.erase(0, 1);
        if (query.back() == '\n') query.erase(query.size() - 1, 1);

        if (query == "READ FILE"){
            readFromFile(query);
        }
        else if (_is.eof()){
            break;
        }

        hsql::SQLParser::parse(query, &result);

        if (result.isValid() && result.size() > 0){
            for (size_t k = 0; k < result.size(); ++k){
                ++loop_counter;
                statement = result.getStatement(k);
                switch (statement->type()) {
                    case hsql::kStmtSelect:{
                        const auto* select = dynamic_cast<const hsql::SelectStatement*>(statement);

                        std::string tableName = select->fromTable->getName();

                        std::vector<std::string> attr_names;
                        for (auto sel : *(select->selectList)){
                            if (sel->isType(hsql::kExprStar)) continue;
                            attr_names.emplace_back(sel->name);
                        }

                        std::vector<Where> where_clauses;
                        parseWhere(select->whereClause, where_clauses);

                        Attribute tableAttr;
                        try{
                            tableAttr = executor->getTableAttributes(tableName);
                            sortAttrNames(attr_names, tableAttr);
                        }
                        catch(db_err_t &db_err){ // Table may not exist
                            showErrMsg(db_err);
                        }

                        std::vector<MemoryTuple> res; // Result container
                        // TODO: Call executor
                        try{
                            executor->selectRecord(tableName, attr_names, where_clauses, res);
                        }
                        catch (db_err_t &db_err){
                            showErrMsg(db_err);
                        }
//                        executor->selectRecord(tableName, attr_names, where_clauses, res);

                        if (attr_names.empty()) {
                            for(int i = 0; i < tableAttr.num; ++i){
                                attr_names.emplace_back(tableAttr.name[i]);
                            }
                        }
                        serialOutput(res, attr_names);
                        break;
                    }
                    case hsql::kStmtCreate :{
                        const auto* create = dynamic_cast<const hsql::CreateStatement*>(statement);
                        if (create->type == hsql::kCreateTable){ // Only handle create table
                            std::string tableName = create->tableName;
                            Attribute attr;
                            int i = 0;
                            for (auto stmt : *(create->columns)){
                                attr.name[i] = stmt->name;
                                attr.type[i] = type_map.at(stmt->type.data_type);
                                ++i;
                            }
                            attr.num = i;
                            attr.primary_Key = -1;
                            int j;
                            for (auto cons : *(create->tableConstraints)){
                                for (j = 0; j < i; ++j){
                                    if (cons->type == hsql::ConstraintType::PrimaryKey){
                                        if (attr.name[j] == std::string(cons->columnNames->at(0))){
                                            attr.primary_Key = j;
                                            attr.is_unique[j] = true;
                                            break;
                                        }
                                    }
                                }
                            }
//                            executor->createTable(tableName, attr);
                            try{
                                executor->createTable(tableName, attr);
                            }
                            catch (db_err_t &db_err){
                                showErrMsg(db_err);
                            }
                            break;
                        }
                        else if(create->type == hsql::kCreateIndex){
                            std::string tableName = create->tableName;
                            std::string indexName = create->indexName;
                            std::string attrName;
                            for(auto col : *(create->indexColumns)){
                                attrName = col;
//                                executor->createIndex(tableName, indexName, attrName);

                                try{
                                    executor->createIndex(tableName, indexName, attrName);
                                }
                                catch (db_err_t &db_err){
                                    showErrMsg(db_err);
                                }
                            }
                        }
                        else{
                            _os << "Invalid operation" << std::endl;
                        }
                        break;

                    }
                    case hsql::kStmtInsert :{
                        const auto* insert = dynamic_cast<const hsql::InsertStatement*>(statement);
                        // TODO : Add feature that a insert can specify the attribute
                        if (insert->select == nullptr){
                            std::string tableName = insert->tableName;
                            MemoryTuple row;
                            for(auto val : *(insert->values)){
                                row.emplace_back(val);
                            }
//                            executor->insertRecord(tableName, row);
                            try{
                                executor->insertRecord(tableName, row);
                            }
                            catch (db_err_t &db_err){
                                showErrMsg(db_err);
                            }
//                            executor->insertRecord(tableName, row);
                        }
                        else{
                            //TODO : Add select
                            throw DB_FAILED;
                        }
                        break;

                    }
                    case hsql::kStmtDelete :{
                        const auto* del = dynamic_cast<const hsql::DeleteStatement*>(statement);
                        std::string tableName = del->tableName;
                        //Assume the most simple delete, that the expr represents the common where.
                        std::vector<Where> where_clause_dat;
                        parseWhere(del->expr, where_clause_dat);
                        try{
                            executor->deleteRecord(tableName, where_clause_dat);
                        }
                        catch (db_err_t &db_err){
                            showErrMsg(db_err);
                        }
//                        executor->deleteRecord(tableName, where_clause_dat);
                        break;

                    }
                    case hsql::kStmtDrop :{
                        const auto* drop = dynamic_cast<const hsql::DropStatement*>(statement);
                        if (drop->type == hsql::kDropTable){
                            std::string tableName(drop->name);
                            executor->dropTable(tableName);
                        }
                        else if(drop->type == hsql::kDropIndex){
                            std::string indexName(drop->indexName);
                            try{
                                executor->dropIndex(indexName);

                            }
                            catch (db_err_t &db_err){
                                showErrMsg(db_err);
                            }
                        }
                        break;
                    }
//                    case hsql::kStmtUpdate :{
//                        const auto* update = dynamic_cast<const hsql::UpdateStatement*>(statement);
//                        std::string tableName = update->table->name;
//
//                        std::vector<Where> where_dat;
//                        parseWhere(update->where, where_dat);
//                        //TODO : Add update content
////                        executor->updateRecord(tableName, where_dat);
//                        break;
//                    }
                    case hsql::kStmtShow :{
                        const auto* show = dynamic_cast<const hsql::ShowStatement*>(statement);
                        executor->showTables();
                        break;
                    }
                    default :{
                        _os << ">> Unsupported valid SQL command. May support later. \n";
                    }
                }
                _os << ">>> Success." << std::endl;
            }
            if (result.size() > 1){
                _os << "---------------------------------------------\n";
                _os << ">>> File query successfully executed!\n";
            }
        }
        else {
            for (auto character : query){
                if (character != ' '){
                    _os <<">>" << result.errorMsg() << std::endl;
                    break;
                }
            }
        }
        result.reset();
    }
    _os << ">>Execute: " << loop_counter << " queries." << std::endl;
    _os << ">>Goodbye." << std::endl;
}


void Interface::serialOutput(std::vector<MemoryTuple> &tuples) {
    for (const auto& tuple : tuples){
        for (auto dat : tuple){
            _os << ">> " << dat;
        }
        _os << std::endl;
    }
    _os.flush();
}

void Interface::serialOutput(std::vector<MemoryTuple> &tuples, std::vector<std::string> &attr_names){
    for(const auto& i : attr_names){
         _os << std::setw(6) <<"|" << std::setw(12) << i;
    }
    _os<< std::endl;
//    for(auto i: attr_names){
//    }

    _os <<"------------------";

    _os<<std::endl;
    //
    for(const auto& tuple : tuples){
        for(auto dat : tuple){
            if(dat.type==(BASE_SQL_ValType::INT)){
                _os <<std::setw(6)<<"|"<<std::setw(12)<<dat.data_meta.i_data;
            }
            else if(dat.type==BASE_SQL_ValType::FLOAT){
                _os <<std::setw(6)<<"|"<<std::setw(12)<<dat.data_meta.f_data;
            }
            else
                _os <<std::setw(6)<<"|"<<std::setw(12)<<dat.data_meta.s_data;
        }
        _os<<std::endl;
    }
    _os << "    Output " << tuples.size() << " records." << std::endl;
}