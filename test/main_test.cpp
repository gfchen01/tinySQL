#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <random>
#include <set>

#include "index/Index.h"
#include "buffer/buffer_manager.h"

using namespace std;

void createBptreeFile(std::string fileName, BufferManager& bfm){
    fileName = PATH::INDEX_PATH + fileName;
    FILE* f = fopen(fileName.c_str(), "w"); // Create the file
    fclose(f);

    pageId_t header_id;
    char* raw = bfm.getPage(fileName, 0, header_id);
    bfm.modifyPage(header_id);
    bpTree_Block* b = reinterpret_cast<bpTree_Block*>(raw);

    b->init(0, INVALID_BLOCK_ID);
    bfm.flushPage(header_id);
}

int main()
{
    BufferManager bfm;
    string fileName = "idx.tree";
    createBptreeFile(fileName, bfm);

//    Bp_tree<Data, int> t(&bfm);

//    IndexManager id_manager(&bfm);
//    std::set<Data> dataSet;
//
//    for (int i = 1e6; i > 0; --i){
//        Data key;
//        key.type = BASE_SQL_ValType::INT;
//        key.data_meta.i_data = i;
//        id_manager.InsertId(fileName, key, i);
//    }


//    std::vector<Index_t> result;
//    Data upper;
//    upper.type = BASE_SQL_ValType::INT;
//    upper.data_meta.i_data = 1e4;
//
//    Data lower;
//    lower.type = BASE_SQL_ValType::INT;
//    lower.data_meta.i_data = -300;
//    id_manager.FindId(fileName, lower, upper, result);
//
//    for (auto &i : result){
//        cout << i << endl;
//    }

    Bp_tree<Data, Index_t> t(&bfm);
    t.InitRoot(PATH::INDEX_PATH + fileName);
    for (int i = 1e6; i > 0; --i){
        if (i == 1e5){
            cout <<"Here" << endl;
        }
        Data key;
        key.type = BASE_SQL_ValType::INT;
        key.data_meta.i_data = i;
        t.Insert(key, i);
    }

    std::vector<Index_t> result;
    Data upper;
    upper.type = BASE_SQL_ValType::INT;
    upper.data_meta.i_data = 1e4;

    Data lower;
    lower.type = BASE_SQL_ValType::INT;
    lower.data_meta.i_data = -300;
    t.FindRange(lower, upper, result);

    for (auto &i : result){
        cout << i << endl;
    }

    cout << "here\n";

//    IndexManager id_m(&bfm);
//    Attribute a;
//    a.name = "name";
//
//    try{
//        id_m.CreateIndex("School");
//    }
//    catch (db_err_t& db_err){
//        cout << db_err;
//    }
//    Data data1;
//    data1.type = BASE_SQL_ValType::INT;
//    data1.data_meta.i_data = 3;
//
//    Data data2;
//    data2.type = BASE_SQL_ValType::INT;
//    data2.data_meta.i_data = 90;
//
//    Index_t row_id = 582851;
//    Index_t res;
//
//    id_m.InsertId("School", data1, row_id);
//    id_m.FindId("School", data1, res);
//    cout << res << endl;
//    id_m.UpdateKey("School", data1, data2);
//    cout << id_m.FindId("School", data1, res) <<endl;
//    cout << id_m.FindId("School", data2, res) <<endl;
//    cout << res;
//    id_m.DeleteId("School", data2);
//    id_m.DropIndex("School");
//    Bp_tree<int, int> bpTree(&bfm);
//    bpTree.InitRoot(fileName);

}