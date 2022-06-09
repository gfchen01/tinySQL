#include "index/Index.h"
#include <cstdio>

namespace fs = std::filesystem;

IndexManager::IndexManager(BufferManager* bfm): _bfm(bfm), _index_tree(bfm) {}

void IndexManager::CreateIndex(const std::string& indexName) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    _bfm->createEmptyFile(fileName);

    pageId_t header_id;
    char* raw = _bfm->getPage(fileName, 0, header_id);
    _bfm->modifyPage(header_id);
    bpTree_Block* b = reinterpret_cast<bpTree_Block*>(raw);

    b->init(0, INVALID_BLOCK_ID);
    _bfm->flushPage(header_id);
}

void IndexManager::DropIndex(const std::string &indexName) {
    std::string file_path = PATH::INDEX_PATH + indexName;
    _bfm->removeFile(file_path);
}

void IndexManager::InsertId(const std::string &indexName, const Data &Key, const Index_t &rec_ptr) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    // TODO : Be more efficient by using same tree for multiple times
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName);
    }
    _index_tree.Insert(Key, rec_ptr);
}

void IndexManager::UpdateId(const std::string &indexName, const Data &key, const Index_t &new_rec_ptr) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName); // May throw here
    }

    _index_tree.UpdateValue(key, new_rec_ptr); // May throw here
}

void IndexManager::UpdateKey(const std::string &indexName, const Data &key, const Data &new_key) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName); // May throw here
    }

    _index_tree.UpdateKey(key, new_key); // May throw here
}

bool IndexManager::CheckExistance(const std::string &indexName, const Data &key, Index_t &id) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName); // TODO : Flush all pages when init.
    }
    bool exists = false;
    try {
        exists = _index_tree.FindValue(key, id);
    }
    catch(db_err_t& db_err){
        if (db_err == DB_BPTREE_EMPTY) exists = false; // When the index file is empty
        else throw db_err;
    }
    return exists;
}

bool IndexManager::FindId(const std::string &indexName, const Data &key, Index_t &result) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName); // May throw here
    }
    return _index_tree.FindValue(key, result); // May throw here
}

bool IndexManager::FindId(const std::string &indexName, const Data &lower_key, const Data &upper_key, std::vector<Index_t> &result) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName); // May throw here
    }
    return _index_tree.FindRange(lower_key, upper_key, result);
}

void IndexManager::DeleteId(const std::string &indexName, const Data &key) {
    std::string fileName = PATH::INDEX_PATH + indexName;
    if (_index_tree.getName() != fileName){
        _index_tree.InitRoot(fileName);
    }
    _index_tree.Delete(key);
}

// db_err_t IndexManager::CreateIndex()