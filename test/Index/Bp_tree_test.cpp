#include "bpTree.h"
#include <iostream>
#include <random>

using namespace std;

int main()
{
    Bp_tree<int, int> T(5);

    for(int i=10; i<50; i++)
    {
        T.Insert(i, i);
    }
    for (int i=9; i>0; --i){
        T.Insert(i, i);
    }
    for (int i = -30; i<0; ++i){
        T.Insert(i, i);
    }

    // T.PrintTree();

    auto node = T.FindNode(35);
    for (auto key : node->key_field){
        cout << key << " " ;
    }
}