#include "share/data_t.h"

//Tuple::Tuple(const Tuple &tuple){
//    for(int has_index = 0; has_index < tuple.cell.size(); has_index++)
//    {
//        this->cell.push_back(tuple.cell[has_index]);
//    }
//}

Tuple::Tuple(std::vector<Data> &dat_vec){
    this =
}


//新增数据
void Tuple::addData(Data &dat){
    assert(_current_len < _total_len);
    cell[_current_len++] = dat;
}

bool Tuple::isDeleted() {
    return isDeleted_;
}

void Tuple::setDeleted() {
    isDeleted_ = true;
}

//得到元组中的数据
std::vector<Data>&& Tuple::getData() const{
    std::vector<Data> data_vec;
    for(db_size_t i = 0; i< _total_len; ++i){
        data_vec.emplace_back(cell[i]);
    }
    return std::move(data_vec);
}


//Table::Table(std::string table_name, Attribute attr){
//    this->table_name = table_name;
//    this->attr=attr;
//    //this->has_index.num=0;
//}

////table的构造函数，拷贝用
//Table::Table(const Table &table){
//    this->attr = table.attr;
//    this->has_index = table.has_index;
//    this->table_name = table.table_name;
//    for(int has_index = 0; has_index < tuple.size(); has_index++)
//        this->tuple.push_back(table.tuple[has_index]);
//}
//
//
////返回一些private的值
//std::string Table::getTableName(){
//    return table_name;
//}
//Attribute Table::getAttr(){
//    return attr;
//}
//std::vector<Tuple>& Table::getTuple(){
//    return tuple;
//}
//Index Table::getIndex(){
//    return has_index;
//}