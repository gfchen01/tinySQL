#include <iostream>
#include <random>
#include <vector>
#include "index/bpTree_block.h"
#include "index/bpTree_disk.h"

#include <chrono>
#include <random>
#include <set>

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
    bfm.flushPage(header_id, fileName, 0);
}

int main() 
{
    BufferManager bfm;
    string fileName = "testBptreeFile";
    createBptreeFile(fileName, bfm);

    Bp_tree<int, int> bpTree(&bfm);
    bpTree.InitRoot(fileName);

    size_t len = 1e4;
//    bpTree.Insert(-1, -1);
//    bpTree.Delete(-1);

    int value;
    try{
        bpTree.FindValue(10000, value);
    }
    catch (db_err_t& err){
        cout << err << endl;
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> distribution(0,len);

    set<int> randKeys;

    for (size_t i = 0; i < len; ++i){
        int rand_key = distribution(generator);
        if (randKeys.find(rand_key) != randKeys.end()){
            continue;
        }
//        cout << rand_key << endl;
        randKeys.insert(rand_key);
        bpTree.Insert(rand_key, rand_key);
    }

    for (auto item : randKeys){
        cout << item << endl;
    }

    int i = 0;
    for (auto item : randKeys){
        int res;
        if(!bpTree.FindValue(item, res)){
            cout << "Error, debug" << endl;
        }
        try{
            bpTree.Delete(item);
        }
        catch (db_err_t& err){
            cout << err;
        }
        if (bpTree.FindValue(item, res)){
            cout << "Error, debug2" << endl;
            bpTree.Delete(item);
        }
        ++i;
    }

    std::cout << "HAHAHA" << std::endl;
    // cout << sizeof(x1) << "," << sizeof(x2) << ", " << sizeof(x3) << endl;
    // cout << sizeof(blockId_t);
}