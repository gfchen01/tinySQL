//
// Created by luke on 22-6-3.
//

#include "API/interface.h"
#include "share/data_t.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <cassert>

void parseWhere(hsql::Expr *Clause, std::vector<Where> &where_vec){
    // For where clause, if one side is kExprOperator, then the other side has to be kExprOperator
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

void Interface::run(std::ifstream *s) {
    std::string query;
    hsql::SQLParserResult result;
    const hsql::SQLStatement* statement;

    while(true){
        query.clear();
        std::getline(*s, query);

        if (s->eof()){
            return;
        }

        hsql::SQLParser::parse(query, &result);

        if (result.isValid() && result.size() > 0){
            for (size_t i = 0; i < result.size(); ++i){
                statement = result.getStatement(i);
                switch (statement->type()) {
                    case hsql::kStmtSelect:{
                        const auto* select = static_cast<const hsql::SelectStatement*>(statement);

                        std::string tableName = select->fromTable->getName();

                        std::vector<std::string> attr_names;
                        for (auto sel : *(select->selectList)){
                            attr_names.push_back(sel->name);
                        }

                        std::vector<Where> where_clauses;
                        parseWhere(select->whereClause, where_clauses);

                        std::vector<Tuple> res; // Result container
                        // TODO: Call executor
                        executor->selectRecord(tableName, attr_names, where_clauses, res);
                        break;
                    }
                    case hsql::kStmtCreate :{
                        const auto* create = static_cast<const hsql::CreateStatement*>(statement);
                        if (create->type == hsql::kCreateTable){ // Only handle create table
                            std::string tableName = create->tableName;
                            Attribute attr;
                            size_t i = 0;
                            for (auto stmt : *(create->columns)){
                                attr.name[i] = stmt->name;
                                attr.type[i] = type_map.at(stmt->type.data_type);
                                ++i;
                            }
                            attr.num = i;
                            size_t j;
                            for (auto cons : *(create->tableConstraints)){
                                for (j = 0; j < i; ++j){
                                    if (cons->type == hsql::ConstraintType::PrimaryKey){
                                        if (attr.name[j] == std::string(cons->columnNames->at(0))){
                                            attr.primary_Key = j;
                                            attr.unique[j] = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            executor->createTable(tableName, attr);
                            break;
                        }
                        else if(create->type == hsql::kCreateIndex){
                            std::string tableName = create->tableName;
                            std::string indexName = create->indexName;
                            std::string attrName;
                            for(auto col : *(create->indexColumns)){
                                attrName = col;
                                executor->createIndex(tableName, indexName, attrName);
                            }
                        }
                        else{
                            std::cout << "Invalid operation" << std::endl;
                        }
                        break;

                    }
                    case hsql::kStmtInsert :{
                        const auto* insert = static_cast<const hsql::InsertStatement*>(statement);
                        // TODO : Add feature that a insert can specify the attribute
                        if (insert->select == NULL){
                            std::string tableName = insert->tableName;
                            Tuple row;
                            for(auto val : *(insert->values)){
                                row.data.push_back(Data(val));
                            }
                            executor->insertRecord(tableName, row);
                        }
                        else{
                            //TODO : Add select
                            throw DB_FAILED;
                        }
                        break;

                    }
                    case hsql::kStmtDelete :{
                        const auto* del = static_cast<const hsql::DeleteStatement*>(statement);
                        std::string tableName = del->tableName;
                        //Assume the most simple delete, that the expr represents the common where.
                        std::vector<Where> where_clause_dat;
                        parseWhere(del->expr, where_clause_dat);
                        executor->deleteRecord(tableName, where_clause_dat);
                        break;

                    }
                    case hsql::kStmtDrop :{
                        const auto* drop = static_cast<const hsql::DropStatement*>(statement);
                        if (drop->type == hsql::kDropTable){
                            std::string tableName(drop->name);
                            executor->dropTable(tableName);
                        }
                        else if(drop->type == hsql::kDropIndex){
                            std::string indexName(drop->indexName);
//                            executor->dropIndex(indexName);
                        }
                        break;
                    }
                    case hsql::kStmtUpdate :{
                        const auto* update = static_cast<const hsql::UpdateStatement*>(statement);
                        std::string tableName = update->table->name;

                        std::vector<Where> where_dat;
                        parseWhere(update->where, where_dat);
                        //TODO : Add update content
//                        executor->updateRecord(tableName, where_dat);
                        break;
                    }
                }
            }
        }
        else {
            std::cout << "INVALID QUERY" << std::endl;
        }
        result.reset();
    }
}