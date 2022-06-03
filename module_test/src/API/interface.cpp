//
// Created by luke on 22-6-3.
//

#include "API/interface.h"
#include "share/data_t.h"
#include <iostream>

BASE_SQL_ValType convertType(hsql::ColumnType t){
    switch (t.data_type) {
        case hsql::DataType::CHAR :{
            return BASE_SQL_ValType::STRING;
        }
        case hsql::DataType::FLOAT :
        case hsql::DataType::DOUBLE :{
            return BASE_SQL_ValType::FLOAT;
        }
        case hsql::DataType::INT :{
            return BASE_SQL_ValType::INT;
        }
    }
}

void Interface::run() {
    std::string query;
    hsql::SQLParserResult result;
    const hsql::SQLStatement* statement;

    while(true){
        query.clear();
        std::cin >> query;
        hsql::SQLParser::parse(query, &result);

        if (result.isValid() && result.size() > 0){
            for (size_t i = 0; i < result.size(); ++i){
                statement = result.getStatement(i);
                switch (statement->type()) {
                    case hsql::kStmtSelect:{
                        const auto* select = static_cast<const hsql::SelectStatement*>(statement);
                        Where w;
                        std::string tableName = select->fromTable->getName();
                        w.attr_name = select->whereClause->expr->getName();
                        Data d(select->whereClause->expr->expr2);
                        w.data = d; // TODO : increase efficiency
                        // TODO: Call executor
                        break;
                    }
                    case hsql::kStmtCreate :{
                        const auto* create = static_cast<const hsql::CreateStatement*>(statement);
                        if (create->type == hsql::kCreateTable){
                            std::string tableName = create->tableName;
                            Attribute attr;
                            size_t i = 0;
                            for (auto stmt : *(create->columns)){
                                attr.name[i] = stmt->name;
                                attr.type[i] = convertType(stmt->type);
                                ++i;
                            }
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

                        }
                        else{
                            std::cout << "Invalid operation" << std::endl;
                        }
                    }
                }
            }
        }
    }
}