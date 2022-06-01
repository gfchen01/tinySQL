#include <iostream>
#include <random>
#include <vector>
#include "bpTree_block.h"
#include "bpTree_disk.h"

using namespace std;

void createBptreeFile(std::string fileName, BufferManager& bfm){
    fileName += PATH::INDEX_PATH;
    FILE* f = fopen(fileName.c_str(), "w+"); // Create the file

    pageId_t header_id;
    char* raw = bfm.getPage(fileName, 0, header_id);
    bfm.modifyPage(header_id);
    bpTree_Block* b = reinterpret_cast<bpTree_Block*>(raw);

    b->init(0, INVALID_BLOCK_ID);
}

int main() 
{
    BufferManager bfm;
    string fileName = "testBptreeFile";
    createBptreeFile(fileName, bfm);

    Bp_tree<int, int> bpTree(&bfm);
    bpTree.InitRoot(fileName);

    bpTree.Insert(1, 2);
    // cout << sizeof(x1) << "," << sizeof(x2) << ", " << sizeof(x3) << endl;
    // cout << sizeof(blockId_t);
}