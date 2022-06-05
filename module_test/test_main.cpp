//
// Created by luke on 22-6-2.
//

#include "hsql/SQLParser.h"
#include "catalog/catalog.h"
#include "API/interface.h"
#include <ios>
#include <iostream>
// #include "hsql/SQLParserResult.h"
using namespace std;

string create_table_query = "CREATE TABLE student "
                "(id char(8), "
                "name char(10), "
                "age int, "
                "score double, "
                "primary key (id));";

string drop_query = "DROP TABLE student;";

string select_query = "SELECT student.name "
                      "FROM student "
                      "WHERE student.name = 'John' "
                      "and student.age > 30 "
                      "and student.teacher = 'Cathy';";

string delete_query = "DELETE FROM students WHERE age between 20 and 30";

string create_index_query = "CREATE INDEX name_id on student(name)";

string insert_query = "INSERT INTO student(id, name, age)"
                      "VALUES('12789', 'Newman', 20)";

string update_query = "update student set name = 'John' where name = 'Cathy'";

int main()
{
    // Basic Usage Example
    const std::string query = update_query;
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
        else if(statement->isType(hsql::kStmtInsert)) {
            const auto* insert = static_cast<const hsql::InsertStatement*>(statement);
            std::cout << 1;
        }
        else if(statement->isType(hsql::kStmtDelete)){
            const auto* del = static_cast<const hsql::DeleteStatement*>(statement);
            std::cout << 1;
        }
        else if(statement->isType(hsql::kStmtDrop)){
            const auto* drop = static_cast<const hsql::DropStatement*>(statement);
            std::cout << 1;
        }
        else if(statement->isType(hsql::kStmtUpdate)){
            const auto* update = static_cast<const hsql::UpdateStatement*>(statement);
            std::cout << 1;
        }
    }
    std::cout << endl;
    Interface anInterface;
    anInterface.run();

//    CatalogManager c_manager;
//    c_manager.CreateTable("Student",)


}