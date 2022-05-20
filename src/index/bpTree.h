#ifndef BPTREE_H
#define BPTREE_H

#include <vector>

#define KEY_VALUE_T_DECLARE template<typename key_t, typename value_t>
#define BP_NODE_T Bp_node<key_t, value_t>
#define BP_TREE_T Bp_tree<key_t, value_t>

KEY_VALUE_T_DECLARE
struct Bp_node{
    bool isLeaf;

    std::vector<key_t> key_field; ///< the field for keys
    std::vector<value_t> value_field; ///< the field for values
    std::vector<Bp_node*> ptr_field; ///< the field for pointers

    BP_NODE_T* parent = nullptr;
    
};

KEY_VALUE_T_DECLARE
class Bp_tree{
public:
    Bp_tree(size_t _d):degree(_d){};
    
    bool Insert(key_t key, value_t val);

    bool Delete(key_t key);
    
    /**
     * @brief Find the values by key (normally, we may say "pointers" for "values")
     * 
     * @param key The search key.
     * @param result [out] The output result. Push them back in the result.
     * @return true Search success
     * @return false 
     */
    bool FindValue(key_t key, value_t& result);
    
    /**
     * @brief Find the leaf node where the key-value stays
     * 
     * @param key Search key
     * @return BP_NODE_T* Result
     */
    BP_NODE_T* FindNode(key_t key);

    /**
     * @brief Print the tree. For debug purpose
     * 
     * @param root 
     * @param depth 
     */
    void PrintTree(BP_NODE_T* root, int depth){ //Recursive print (debug)
        assert(root != nullptr);

        std::cout << "Depth:" <<depth <<" Keys: \n";
        for (auto item : root->key_field){
            std::cout << item << ','
        }
        std::cout << std::endl;

        if(!root->isLeaf){
            for (BP_NODE_T* bp_n_ptr : root->ptr_field){
                PrintTree(bp_n_ptr, depth + 1);
            }
        }
    }

private:
    size_t degree;
    BP_NODE_T* root = nullptr;

    /**
     * @brief Simply insert in a leaf node.
     * 
     * @param key 
     * @param val 
     * @param leaf 
     * @return true insertion success
     * @return false insertion failure
     */
    bool Insert_in_Leaf(key_t key, value_t val, BP_NODE_T* leaf){
        assert(leaf->isLeaf);

        while(i < leaf->key_field.size() && key > leaf->key_field.at(i)){
            ++i;
        }
        
        leaf->key_field.insert(leaf->key_field.begin() + i, key);
        leaf->value_field.insert(leaf->value_field.begin() + i, value);
    }

    /**
     * @brief Insert in a parent. May split. Resursive call.
     * 
     * @param key the smallest search key in split (the right one when spliting)
     * @param prev_leaf left splitted node, whose pointer is in its parent field
     * @param split2 right splitted node. The new node.
     * @return true Insert sucess
     * @return false Insert failure
     */
    bool Insert_in_Parent(key_t key, BP_NODE_T* prev_leaf, BP_NODE_T* split);
};

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Parent(key_t key, BP_NODE_T* prev_leaf, BP_NODE_T* split){
    if (prev_leaf == root){
        root = new BP_NODE_T;

        root->key_field.push_back(key);
        root->ptr_field.push_back(prev_leaf);
        root->ptr_field.push_back(split);

        prev_leaf->parent = root;
        split->parent = root;
    }
    else{
        
    }
}


KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert(key_t key, value_t val){
    if (root == nullptr){
        root = new BP_NODE_T;
        root->isLeaf = true;
        root->key_field.push_back(key);
        root->value_field.push_back(val);
        return true;
    }
    else{
        BP_NODE_T* leaf = FindNode(key);
        if (leaf == nullptr){ 
            return false;
        }

        if (leaf->key_field.size() < degree - 1){ // Still have space
            Insert_in_Leaf(key, val, leaf);
        }
        else{
            assert(leaf->key_field.size() == degree - 1);

            BP_NODE_T* split = new BP_NODE_T;
            
            /// Set type
            split->isLeaf = true;
            
            /// Copy values and keys
            size_t i = (degree - 1) / 2;
            while(i < leaf->key_field.size()){
                split->key_field.push_back(leaf->key_field.at(i));
                split->value_field.push_back(leaf->value_field.at(i));

                leaf->key_field.erase(leaf->key_field.begin() + i);
                leaf->value_field.erase(leaf->value_field.begin() + i);
            }

            /// Set pointer for next leaf node (For range search)
            if (leaf->ptr_field.empty()){
                leaf->ptr_field.push_back(split);                
            }
            else{
                split->ptr_field.push_back(leaf->ptr_field.front());
                leaf->ptr_field.front() = split;
            }

            /// Set parent
            split->parent = leaf->parent;

            // Insert_in_Parent()
        }

    }
}

KEY_VALUE_T_DECLARE
BP_NODE_T* BP_TREE_T::FindNode(key_t key){
    BP_NODE_T* C = root;
    assert(C->key_field.size() > 0);
    while(!C->isLeaf){
        size_t i = 0;
        while(i < C->key_field.size() && C->key_field.at(i) < key){
            i += 1;
        }

        // 2 cases : either way, i corresponds to the index of C->ptr_field
        if (i == C->key_field.size()){
            C = C->ptr_field.at(i);
        }
        else if (key == C->key_field.at(i)){
            C = C->ptr_field.at(i+1);
        }
        else{
            C = C->ptr_field.at(i);
        }
    }

    size_t i = 0;
    while(i < C->key_field.size() && C->key_field.at(i) < key){
        i += 1;
    }
    if (i == C->key_field.size()) return nullptr; //No corresponding value
    else {
        return C;
    }
}

#endif