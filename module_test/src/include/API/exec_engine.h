/**
 * @file exec_engine.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef EXEC_ENGINE_H
#define EXEC_ENGINE_H

#include "share/data_t.h"
#include "hsql/SQLParser.h"


class Exec_Engine{

    void createTable(std::string &table_name, Attribute &attr);
    void dropTable(std::string &table_name);
    void selectRecord(std::string &table_name, std::vector<std::string&> attr_names, std::vector<Where&> data, std::vector<Tuple>& result); //recordManager
    void insertRecord(std::string &table_name, std::vector<Tuple>& row);
    void createIndex(std::string &table_name, std::string &index_name, std::string &attr_name); //index_manager
    void dropIndex(std::string &table_name, std::string &index_name);
    void deleteRecord(std::string &table_name, std::vector<Where&> data);
    void updateRecord(std::string &table_name, std::vector<Where&> data);

};





#endif