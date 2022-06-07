#include "share/data_t.h"
//新增数据
void DiskTuple::serializeFromMemory(const MemoryTuple &in_tuple) {
    assert(!in_tuple.empty());
    _total_len = in_tuple.size();
    memcpy(cell, in_tuple.data(), sizeof(Data) * in_tuple.size());
}

MemoryTuple&& DiskTuple::deserializeToMemory(const std::vector<int>& pos) {
    MemoryTuple inmemory_tuple;
    if (pos.empty()){
        inmemory_tuple.resize(_total_len);
        memcpy(inmemory_tuple.data(), cell, getBytes());
    }
    else{
        for (auto p : pos){
            inmemory_tuple.emplace_back(cell[p]);
        }
    }
    return std::move(inmemory_tuple);
}

//
//void DiskTuple::addData(Data &dat){
//    assert(_current_len < _total_len);
//    cell[_current_len++] = dat;
//}

bool DiskTuple::isDeleted() {
    return isDeleted_;
}

void DiskTuple::setDeleted() {
    isDeleted_ = true;
}

//得到元组中的数据
std::vector<Data>&& DiskTuple::getData() const{
    std::vector<Data> data_vec;
    for(db_size_t i = 0; i< _total_len; ++i){
        data_vec.emplace_back(cell[i]);
    }
    return std::move(data_vec);
}

//DiskTuple* TupleFactory::makeTuple(db_size_t tuple_len) {
//    auto* t = (DiskTuple*)malloc(sizeof(DiskTuple) + tuple_len * sizeof(Data));
//    t->_total_len = tuple_len;
//    t->_current_len = 0;
//    t->isDeleted_ = false;
//    return t;
//}
//
//DiskTuple* TupleFactory::makeTuple(DiskTuple *t) {
//    auto* this_t = (DiskTuple*)malloc(sizeof(DiskTuple) + t->_total_len * sizeof(Data));
//    memcpy((void*)this_t, (void*)t, t->getBytes());
//    return this_t;
//}
//
//DiskTuple* TupleFactory::makeTuple(std::vector<Data> &dat_vec) {
//    auto* t = (DiskTuple*)malloc(sizeof(DiskTuple) + dat_vec.size() * sizeof(Data));
//    t->_total_len = t->_current_len = dat_vec.size();
//    for (db_size_t i = 0; i < t->_total_len; ++i){
//        t->cell[i] = dat_vec.at(i);
//    }
//    return t;
//}