//
// Created by luke on 22-6-2.
//

#include "hsql/SQLParser.h"
#include "catalog/catalog.h"
#include <iostream>
// #include "hsql/SQLParserResult.h"
using namespace std;

string create_query = "CREATE TABLE student "
                "(id char(8), "
                "name char(10), "
                "age int, "
//                "score numeric(2, 2), "
                "primary key (id));";

string drop_query = "DROP TABLE student";

string select_query = "SELECT student.name "
                      "FROM student "
                      "WHERE student.name = 'John'";

string delete_query = "DELETE FROM student"
                      "WHERE student.age > 21";

int main()
{
    // Basic Usage Example
    const std::string query = create_query;
    hsql::SQLParserResult result;
    hsql::SQLParser::parse(query, &result);

    if (result.isValid() && result.size() > 0) {
        const hsql::SQLStatement* statement = result.getStatement(0);
        if (statement->isType(hsql::kStmtSelect)) {
            const auto* select = static_cast<const hsql::SelectStatement*>(statement);
            std::cout << 1;
            /* ... */
        }
        else if (statement->isType(hsql::kStmtCreate)) {
            const auto* create = static_cast<const hsql::CreateStatement*>(statement);
            std::cout << 1;
        }
    }
//    CatalogManager c_manager;
//    c_manager.CreateTable("Student",)


}