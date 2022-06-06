//
// Created by luke on 22-6-2.
//

#include "hsql/SQLParser.h"
#include "catalog/catalog.h"
#include "API/interface.h"
#include <ios>
#include <iostream>
#include <fstream>
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

string delete_query = "DELETE FROM students WHERE age between 20 and 30;";

string create_index_query = "CREATE INDEX name_id on student(name);";

string insert_query = "INSERT INTO student(id, name, age)"
                      "VALUES('12789', 'Newman', 20);";

string update_query = "update student set name = 'John' where name = 'Cathy';";

int main()
{
//    std::ifstream ifs;
//    ifs.open("QueryInsert2.txt", std::fstream::in);

//    std::ofstream ofs;
//    ofs.open("Output.txt", std::fstream::out);

    Interface anInterface(std::cin, std::cout);
    anInterface.run();
//    ifs.close();
}