#include "API/exec_engine.h"

void Exec_Engine::createTable(std::string &table_name, Attribute &attr){
    catalog_manager.CreateTable(table_name, attr); // Must run this first
    record_manager.CreateTableFile(table_name);
}

void Exec_Engine::dropTable(std::string &table_name){
    catalog_manager.DropTable(table_name); // Must run this first
    record_manager.DropTableFile(table_name);
}

void Exec_Engine::createIndex(std::string &table_name, std::string &index_name, std::string &attr_name){
    catalog_manager.UpdateIndex(table_name,attr_name,index_name);
    index_manager.CreateIndex(index_name);
    record_manager.CreateIndex(table_name, attr_name);
}

void Exec_Engine::dropIndex(std::string &index_name){
    std::string table_name = catalog_manager.indexName2table(index_name); // May throw here
    catalog_manager.DropIndex(table_name, index_name); // Must run this first
    index_manager.DropIndex(index_name);
}


void Exec_Engine::deleteRecord(std::string &table_name, std::vector<Where>& wheres){
    record_manager.DeleteRecord(table_name, wheres);
}

void Exec_Engine::selectRecord(std::string &table_name, std::vector<std::string>& attr_names, std::vector<Where>& wheres, std::vector<MemoryTuple>& result){
    if(attr_names.empty()){
        if (wheres.empty()){
            result = record_manager.SelectRecord(table_name);
        }
        else{
            result = record_manager.SelectRecord(table_name, wheres);
        }
    }
    else {
        if (wheres.empty()){
            result = record_manager.SelectRecord(table_name, attr_names);
        }
        else{
            result = record_manager.SelectRecord(table_name, attr_names, wheres);
        }
    }
}

Attribute Exec_Engine::getTableAttributes(const std::string &table_name) {
    return catalog_manager.getAttribute(table_name);
}

void Exec_Engine::insertRecord(std::string &table_name, MemoryTuple &row){
    record_manager.InsertRecord(table_name, row);
}