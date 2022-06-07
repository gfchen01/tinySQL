//
// Created by luke on 22-6-2.
//

#include "share/config.h"
#include "API/interface.h"
#include "fstream"
#include <iostream>
#include <filesystem>

std::string create_table_query = "CREATE TABLE student "
                "(id char(8), "
                "name char(10), "
                "age int, "
                "score double, "
                "primary key (id));";

std::string drop_query = "DROP TABLE student;";

std::string select_query = "SELECT student.name "
                      "FROM student "
                      "WHERE student.name = 'John' "
                      "and student.age > 30 "
                      "and student.teacher = 'Cathy';";

std::string delete_query = "DELETE FROM students WHERE age between 20 and 30;";

std::string create_index_query = "CREATE INDEX name_id on student(name);";

std::string insert_query = "INSERT INTO student(id, name, age)"
                      "VALUES('12789', 'Newman', 20);";

std::string update_query = "update student set name = 'John' where name = 'Cathy';";

void build_tiny_sql_folders(){
    if (!std::filesystem::is_directory(PATH::DATA_PATH)){
        std::filesystem::create_directory(PATH::DATA_PATH);
    }
    if (!std::filesystem::is_directory(PATH::RECORD_PATH)){
        std::filesystem::create_directory(PATH::RECORD_PATH);
    }
    if (!std::filesystem::is_directory(PATH::INDEX_PATH)){
        std::filesystem::create_directory(PATH::INDEX_PATH);
    }
    if (!std::filesystem::is_directory(PATH::CATALOG_PATH)){
        std::filesystem::create_directory(PATH::CATALOG_PATH);
    }
}

int main()
{
    std::ifstream ifs;
    ifs.open("QueryInsert2.txt", std::fstream::in);

//    std::ofstream ofs;
//    ofs.open("Output.txt", std::fstream::out);
    build_tiny_sql_folders();
    Interface anInterface(std::cin, std::cout);
    anInterface.run();

//    ifs.close();
}