//
// Created by luke on 22-6-3.
//

#include "API/interface.h"
#include "share/data_t.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>

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
        default:{
            _os << "DB ERROR!\n";
        }
    }
}

void Interface::run() {
    std::string query;
    hsql::SQLParserResult result;
    const hsql::SQLStatement* statement;

    int i = 0;

    while(true){
        query.clear();
        std::getline(_is, query, ';');

        if (_is.eof()){
            return;
        }

        hsql::SQLParser::parse(query, &result);

        if (result.isValid() && result.size() > 0){
            for (size_t k = 0; k < result.size(); ++k){
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

                        std::vector<Tuple> res; // Result container
                        // TODO: Call executor
                        try{
                            executor->selectRecord(tableName, attr_names, where_clauses, res);
                        }
                        catch (db_err_t &db_err){
                            showErrMsg(db_err);
                        }
                        serialOutput(res);
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
                            Tuple row;
                            for(auto val : *(insert->values)){
                                row.cell.emplace_back(val);
                            }
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
                    case hsql::kStmtUpdate :{
                        const auto* update = dynamic_cast<const hsql::UpdateStatement*>(statement);
                        std::string tableName = update->table->name;

                        std::vector<Where> where_dat;
                        parseWhere(update->where, where_dat);
                        //TODO : Add update content
//                        executor->updateRecord(tableName, where_dat);
                        break;
                    }
                }
            }
            _os << ">> Success." << std::endl;
        }
        else {
            for (auto character : query){
                if (character != ' '){
                    _os << result.errorMsg() << std::endl;
                    break;
                }
            }
        }
        result.reset();
    }
}


void Interface::serialOutput(std::vector<Tuple> &tuples) {
    for (const auto& tuple : tuples){
        for (auto dat : tuple.cell){
            _os << ">> " << dat;
        }
        _os << std::endl;
    }
    _os.flush();
}

void Interface::serialOutput(std::vector<Tuple> &tuples, std::vector<std::string> &attr_names){
    for(auto i : attr_names){
         _os << std::setw(6) <<"|" << std::setw(12) << i;
    }
    _os<< std::endl;
    for(auto i: attr_names){
        _os <<"------------------";
    }
    _os<<std::endl;
    //
    for(auto tuple : tuples){
        for(auto dat : tuple.cell){
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
}