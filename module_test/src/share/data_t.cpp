#include "share/data_t.h"

Tuple::Tuple(const Tuple &tuple){
    for(int index = 0; index < tuple.data.size(); index++)
    {
        this->data.push_back(tuple.data[index]);
    }
}

//新增数据
void Tuple::addData(Data data){
    this->data.push_back(data);
}

bool Tuple::isDeleted() {
    return isDeleted_;
}

void Tuple::setDeleted() {
    isDeleted_ = true;
}

//得到元组中的数据
std::vector<Data> Tuple::getData() const{
    return this->data;
}


Table::Table(std::string table_name, Attribute attr){
    this->table_name = table_name;
    this->attr=attr;
    //this->index.num=0;
}

//table的构造函数，拷贝用
Table::Table(const Table &table){
    this->attr = table.attr;
    this->index = table.index;
    this->table_name = table.table_name;
    for(int index = 0; index < tuple.size(); index++)
        this->tuple.push_back(table.tuple[index]);
}


//返回一些private的值
std::string Table::getTableName(){
    return table_name;
}
Attribute Table::getAttr(){
    return attr;
}
std::vector<Tuple>& Table::getTuple(){
    return tuple;
}
Index Table::getIndex(){
    return index;
}