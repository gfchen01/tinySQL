#include "bpTree.h"
#include <iostream>
#include <random>
#include <vector>
#include "bpTree_block.h"

using namespace std;

int main()
{
    bpTree_Block<int, int> x1;
    bpTree_Leaf<int, int> x2;
    bpTree_Internal<int, int> x3;
    
    x2._size = 3;
    auto ptr_x2 = &x2;
    bpTree_Leaf<int, int>* x4 = reinterpret_cast<bpTree_Leaf<int, int>*>(ptr_x2);

    std::pair<int, int> x[0];

    cout << x4->_size;
    // cout << sizeof(x1) << "," << sizeof(x2) << ", " << sizeof(x3) << endl;
    // cout << sizeof(blockId_t);
}