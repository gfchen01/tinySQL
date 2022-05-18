#ifndef BPTREE_H
#define BPTREE_H

#include <vector>

template<typename key_t, typename value_t>
struct Bp_node{
    bool isLeaf;

    std::vector<key_t> key_field; ///< the field for keys
    std::vector<value_t> value_field; ///< the field for values
    std::vector<Bp_node*> ptr_field; ///< the field for pointers

    Bp_node<key_t, value_t>* parent;
    
};

template<typename key_t, typename value_t>
class Bp_tree{
public:
    Bp_tree(size_t _d):degree(_d){};
    
    bool Insert(key_t key, value_t val);

    bool Delete(key_t key);
    
    bool FindValue(key_t key, value_t& result);
    
    bool FindNode(key_t key, Bp_node* node);

private:
    size_t degree;
    Bp_node<key_t, value_t>* root = nullptr;

    /**
     * @brief Simply insert in a leaf node.
     * 
     * @param key 
     * @param val 
     * @param leaf 
     * @return true insertion success
     * @return false insertion failure
     */
    bool Insert_in_Leaf(key_t key, value_t val, Bp_node<key_t, value_t>* leaf){
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
     * @param key 
     * @param child 
     * @param parent 
     * @return true 
     * @return false 
     */
    bool Insert_in_Parent(key_t key, Bp_node<key_t, value_t>* split1, Bp_node<key_t, value_t>* split2);
};

template<typename key_t, typename value_t>
bool Bp_tree<key_t, value_t>::Insert(key_t key, value_t val){
    if (root == nullptr){
        root = new Bp_node<key_t, value_t>;
        root->isLeaf = true;
        root->key_field.push_back(key);
        root->value_field.push_back(val);
        return true;
    }
    else{
        Bp_node<key_t, value_t>* leaf; 
        if (!FindNode(key, leaf)){
            return false;
        }

        if (leaf->key_field.size() < degree - 1){ // Still have space
            Insert_in_Leaf(key, val, leaf);
        }
        else{
            assert(leaf->key_field.size() == degree - 1);

            Bp_node<key_t, value_t>* split1 = new Bp_node<key_t, value_t>;
            Bp_node<key_t, value_t>* split2 = new Bp_node<key_t, value_t>;
            
            /// Set type
            split1->isLeaf = true;
            split2->isLeaf = true;
            
            /// Copy values and keys
            for (size_t i=0; i < (degree - 1) / 2; ++i){
                split1->key_field.push_back(leaf->key_field.at(i));
                split1->value_field.push_back(leaf->value_field.at(i));
                
                split2->key_field.push_back(leaf->key_field.at(i + (degree - 1) / 2));
                split2->key_field.push_back(leaf->value_field.at(i + (degree - 1) / 2));
            }
            
            if (i * 2 < (degree - 1)){
                split2->key_field.push_back(leaf->key_field.at(degree - 2)); //Note the index.
                split2->value_field.push_back(leaf->value_field.at(degree - 2));
            }

            /// Set pointer for next leaf node (For range search)
            split1->ptr_field.push_back(split2);
            split2->ptr_field.push_back(leaf->ptr_field.front());

            /// Set parent
            split1->parent = split2->parent = leaf->parent

            delete leaf;

            // Insert_in_Parent()
        }

    }
}

template<typename key_t, typename value_t>
bool Bp_tree<key_t, value_t>::Insert_in_Parent(key_t key, Bp_node<key_t, value_t>* split1, Bp_node<key_t, value_t>* split2){

}

#endif