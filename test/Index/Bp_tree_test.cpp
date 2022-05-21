#include "bpTree.h"
#include <iostream>
#include <random>
#include <vector>

using namespace std;

int main()
{
    Bp_tree<int, int> T(10);

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
    
    for (int i = 10; i < 50; i += 2){
        T.Delete(i);
    }

    vector<int> value_ret;
    bool success = T.FindRange(-15, 37, value_ret);

    for (auto value : value_ret){
        cout << value << " " ;
    }
}