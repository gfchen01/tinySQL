#include "API/exec_engine.h"

void Exec_Engine::createTable(std::string &table_name, Attribute &attr){
    record_manager.CreateTableFile(table_name);
    catalog_manager.CreateTable(table_name, attr);
}
void Exec_Engine::dropTable(std::string &table_name){
    record_manager.DropTableFile(table_name);
    catalog_manager.DropTable(table_name);
}

void Exec_Engine::createIndex(std::string &table_name, std::string &index_name, std::string &attr_name){
    catalog_manager.UpdateIndex(table_name,attr_name,index_name);

    index_manager.CreateIndex(index_name);
    record_manager.CreateIndex(index_manager, table_name, attr_name);
}

void Exec_Engine::dropIndex(std::string &table_name, std::string &index_name){
    index_manager.DropIndex(index_name);
    catalog_manager.DropIndex(table_name, index_name);

}


void Exec_Engine::deleteRecord(std::string &table_name, std::vector<Where>& data){
    for(std::vector<Where>::iterator it=data.begin();it!=data.end();it++){
        if((*it).attr_name == ""){
            record_manager.DeleteRecord(table_name);
        }
        else{
            record_manager.DeleteRecord(table_name, (*it).attr_name, *it);
        }
    }


}

void Exec_Engine::selectRecord(std::string &table_name, std::vector<std::string>& attr_names, std::vector<Where>& data, std::vector<Tuple>& result){
    if(attr_names.size() == 0){
        auto table = record_manager.SelectRecord(table_name);
    }
    else if(attr_names.size() == 1){
        auto table = record_manager.SelectRecord(table_name, attr_names[0], data[0]);
    }

}

void Exec_Engine::insertRecord(std::string &table_name, Tuple &row){
    record_manager.InsertRecord(table_name, row);
}