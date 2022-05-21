/**
 * @file bpTree.h
 * @author CHEN Guofei (simonsatzju@gmail.com)
 * @brief A Bplus tree designed for 2022 MiniSQL project
 * @version 0.1
 * @date 2022-05-21
 * 
 * @copyright Copyright (c) 2022
 *
 * Type declarations:
 * There 3 things in a Bp_node: key, value and ptr
 * <Key> is the corresponding attribute value to build index on.
 * <Value> only occurs in leaf nodes. They are aligned with key,
 * and points to the record in a table.
 * <ptr> Is the pointer to Bp_nodes. Users should never change them.
 *
 * For leaf nodes, there is only one element in ptr_field, which points
 * to next leaf node. It's designed for range search.
 *
 */
#ifndef BPTREE_H
#define BPTREE_H

#include <vector>
#include <iostream>
#include <cassert>

#define KEY_VALUE_T_DECLARE template<typename key_t, typename value_t>
#define BP_NODE_T Bp_node<key_t, value_t>
#define BP_TREE_T Bp_tree<key_t, value_t>

KEY_VALUE_T_DECLARE
struct Bp_node{
    bool isLeaf = false;

    std::vector<key_t> key_field; ///< the field for keys
    std::vector<value_t> value_field; ///< the field for values
    std::vector<Bp_node*> ptr_field; ///< the field for pointers

    BP_NODE_T* parent = nullptr;

    /**
     * Find the position of a key in the node.
     * @param key
     * @return The index of key in key_field; -1 if failed
     */
    size_t FindPos_at_Node (key_t key) const{
        for (size_t i = 0; i < key_field.size(); ++i){
            if (key_field.at(i) == key){
                return i;
            }
        }
        return -1; // Convert to unsigned int max.
    }
};

KEY_VALUE_T_DECLARE
class Bp_tree{
public:
    Bp_tree(size_t _degree = 4):degree(_degree){};

    /**
     * @brief Basic insertion to the tree.
     *
     * @param key The key for the val.
     * @param val The val (or normally so-called "pointer") to the record.
     * @return true Insertion success
     * @return false Insertion fail
     */
    bool Insert(key_t key, value_t val);

    /**
     * @brief Basic deletion from the tree.
     *
     * Note that normally it is not used by users.
     * Users should call more general functions, like update val, etc.
     *
     * @param key The key for a certain record.
     * @return true Deletion success.
     * @return false Insertion success.
     */
    bool Delete(key_t key);

    /**
     * @brief Find the values by key (normally, we may say "pointers" for "values")
     *
     * @param key The search key.
     * @param result [out] The search result. Push them back in the result.
     * @return true Search success
     * @return false
     */
    bool FindValue(key_t key, value_t& result);

    /**
     * @brief Find the value(pointer) based on the given lower and upper key
     *
     * @param lower_key
     * @param upper_key
     * @param result [out]
     * @return true Search success
     * @return false Search failure
     */
    bool FindRange(key_t lower_key, key_t upper_key, std::vector<value_t>& result);

    /**
     *
     * @param former_key
     * @param new_key
     * @return
     */
    bool UpdateIndex(key_t former_key, key_t new_key);

    /**
     * @brief Print the tree. For debug purpose
     *
     */
    void PrintTree(){
        PrintTreeRecurse(root, 0);
    }

private:
    size_t degree;
    BP_NODE_T* root = nullptr;


    /**
     * @brief Find the leaf node where the key-value stays
     *
     * @param key Search key
     * @return BP_NODE_T* Result
     */
    BP_NODE_T* FindNode(key_t key);

    /**
     * @brief Simply insert in a leaf node.
     *
     * @param key
     * @param val
     * @param leaf [in] The leaf node to insert the value
     * @return true insertion success
     * @return false insertion failure
     */
    bool Insert_in_Leaf(key_t key, value_t val, BP_NODE_T* leaf);

    /**
     * @brief Insert in a parent. May split. Recursive call.
     *
     * @param key the smallest search key in split (the right one when splitting)
     * @param prev_leaf left split node, whose pointer is in its parent->ptr_field
     * @param split right split node. The new node.
     * @return true Insert success
     * @return false Insert failure
     */
    bool Insert_in_Parent(key_t key, BP_NODE_T* prev_leaf, BP_NODE_T* split);

    /**
     * @brief Delete in the parent based on child pointer.
     *
     * @param to_delete_child The child to be removed
     * @return true Delete success
     */
    bool Delete_in_Parent(BP_NODE_T* to_delete_child);

    void PrintTreeRecurse(BP_NODE_T* root, int depth){ //Recursive print (debug)
        assert(root != nullptr);

        std::cout << "Depth:" << depth <<" Keys: \n";
        for (auto item : root->key_field){
            std::cout << item << ',' ;
        }
        std::cout << std::endl;

        if(!root->isLeaf){
            for (BP_NODE_T* bp_n_ptr : root->ptr_field){
                PrintTreeRecurse(bp_n_ptr, depth + 1);
            }
        }
    }
};

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

    return C;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Parent(key_t key, BP_NODE_T* prev_leaf, BP_NODE_T* split){
    if (prev_leaf == root){
        root = new BP_NODE_T;

        root->key_field.push_back(key);
        root->ptr_field.push_back(prev_leaf);
        root->ptr_field.push_back(split);

        prev_leaf->parent = root;
        split->parent = root;
        return true;
    }
    else{
        BP_NODE_T* parent = prev_leaf->parent;
        if (parent->ptr_field.size() < degree){ //Enough space for the split node
            size_t i;
            for (i = 0; i < parent->ptr_field.size(); ++i){ //TODO: May change it to bisearch
                if(parent->ptr_field.at(i) == prev_leaf){
                    // Insertion happens "before" the given iterator
                    parent->ptr_field.insert(parent->ptr_field.begin() + i + 1, split);
                    parent->key_field.insert(parent->key_field.begin() + i, key);
                    split->parent = parent;
                    break;
                }
            }
            assert(i < parent->ptr_field.size());
            return true;
        }
        else{ //Not enough space. Split parent.
            BP_NODE_T* split_parent = new BP_NODE_T;
            /// Set type.
            split_parent->isLeaf = false;

            /// Insert the current key and pointer temporarily
            size_t i;
            for (i = 0; i < parent->ptr_field.size(); ++i){ //TODO: May change it to bisearch
                if(parent->ptr_field.at(i) == prev_leaf){
                    parent->ptr_field.insert(parent->ptr_field.begin() + i + 1, split);
                    split->parent = parent;
                    parent->key_field.insert(parent->key_field.begin() + i, key);
                    break;
                }
            }

            // Move keys and ptrs.
            i = (degree + 1) / 2;
            key_t right_smallest_key = parent->key_field.at(i - 1);

            // Abandon a key here (which is the key for their parent),
            // so specially run a loop here.
            split_parent->ptr_field.push_back(parent->ptr_field.at(i));
            split_parent->ptr_field.back()->parent = split_parent;
            parent->ptr_field.erase(parent->ptr_field.begin() + i);
            parent->key_field.erase(parent->key_field.begin() + i - 1);

            while(i < parent->ptr_field.size()){
                split_parent->ptr_field.push_back(parent->ptr_field.at(i));
                split_parent->ptr_field.back()->parent = split_parent;
                split_parent->key_field.push_back(parent->key_field.at(i - 1));

                parent->ptr_field.erase(parent->ptr_field.begin() + i);
                parent->key_field.erase(parent->key_field.begin() + i - 1);
            }

            return Insert_in_Parent(right_smallest_key, parent, split_parent);
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Leaf(key_t key, value_t val, BP_NODE_T* leaf){
    assert(leaf->isLeaf);

    size_t i = 0;
    while(i < leaf->key_field.size() && leaf->key_field.at(i) < key){
        ++i;
    }

    leaf->key_field.insert(leaf->key_field.begin() + i, key);
    leaf->value_field.insert(leaf->value_field.begin() + i, val);

    return true;
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

            /// Insert in leaf first
            Insert_in_Leaf(key, val, leaf);

            /// Copy values and keys
            size_t i = degree / 2; // The position right after the (degree/2)th element

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

            Insert_in_Parent(split->key_field.front(), leaf, split);
        }
        return true;
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Delete_in_Parent(BP_NODE_T* to_delete_child){
    BP_NODE_T* parent = to_delete_child->parent;

    // Delete the pointer to the child from its parent, and its "separation key"
    // Caller need to make sure that the abandoned child is the latter one in merging
    for (size_t i = 0; i < parent->ptr_field.size(); ++i){
        if (parent->ptr_field.at(i) == to_delete_child){
            parent->ptr_field.erase(parent->ptr_field.begin() + i);
            parent->key_field.erase(parent->key_field.begin() + i - 1);
            break;
        }
    }

    if (parent == root){
        if (parent->ptr_field.size() == 1){
            root = parent->ptr_field.front();
            delete parent;
            return true;
        }
    }
    else if (parent->key_field.size() < (degree + 1) / 2){ // (int)(degree + 1)/2 is equivalent to degree/2 's upper integer
        BP_NODE_T* sibling;
        BP_NODE_T* grand_parent = parent->parent;
        key_t sep_key_in_grandparent;
        size_t i;
        bool is_predecessor;
        for (i = 0; i < grand_parent->ptr_field.size(); ++i){
            if (grand_parent->ptr_field.at(i) == parent) break;
        }

        assert(i < grand_parent->ptr_field.size());

        if (i < grand_parent->ptr_field.size() - 1){ // Not the last child
            sibling = grand_parent->ptr_field.at(i + 1);
            sep_key_in_grandparent = grand_parent->key_field.at(i);
            is_predecessor = true;
        }
        else{
            sibling = grand_parent->ptr_field.at(i - 1);
            sep_key_in_grandparent = grand_parent->key_field.at(i - 1);
            is_predecessor = false;
        }

        // At most n - 1 keys in non-leaves
        if (parent->key_field.size() + sibling->key_field.size() < degree){ // Merge leaf and sibling
            if (is_predecessor) { // Parent is the predecessor
                auto temp = parent;
                parent = sibling;
                sibling = temp;
            }
            // Move the content in leaf to sibling, and set the child->parent pointer
            sibling->key_field.push_back(sep_key_in_grandparent);
            sibling->ptr_field.push_back(parent->ptr_field.front());
            sibling->ptr_field.back()->parent = sibling;
            for (size_t j = 0; j < parent->key_field.size(); ++j){
                sibling->key_field.push_back(parent->key_field.at(j));
                sibling->ptr_field.push_back(parent->ptr_field.at(j + 1));
                sibling->ptr_field.back()->parent = sibling;
            }

            Delete_in_Parent(parent);

            delete parent;

            return true;
        }
        else{ // Redistribute the keys and pointers
            if (is_predecessor){
                parent->key_field.push_back(sep_key_in_grandparent);
                parent->ptr_field.push_back(sibling->ptr_field.front());
                parent->ptr_field.back()->parent = parent;

                grand_parent->key_field.at(i) = sibling->key_field.front();

                sibling->key_field.erase(sibling->key_field.begin());
                sibling->ptr_field.erase(sibling->ptr_field.begin());
            }
            else{
                parent->key_field.insert(parent->key_field.begin(), sep_key_in_grandparent);
                parent->ptr_field.insert(parent->ptr_field.begin(), sibling->ptr_field.back());
                parent->ptr_field.front()->parent = parent;

                grand_parent->key_field.at(i - 1) = sibling->key_field.back();

                sibling->key_field.pop_back();
                sibling->ptr_field.pop_back();
            }

            return true;
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Delete(key_t key) {
    BP_NODE_T* leaf = FindNode(key);

    // Delete the key and value from leaf
    for (size_t i = 0; i < leaf->key_field.size(); ++i){
        if (leaf->key_field.at(i) == key){
            leaf->key_field.erase(leaf->key_field.begin() + i);
            leaf->value_field.erase(leaf->value_field.begin() + i);
            break;
        }
    }

    if (leaf == root) return true;
        // Violate capacity constraint for leaf and non-root node
    else if (leaf->key_field.size() < degree / 2){ // (int)degree/2 is equivalent to (degree - 1)/2's upper integer
        BP_NODE_T* sibling;
        BP_NODE_T* parent = leaf->parent;
        key_t sep_key_in_parent;
        size_t i;
        bool is_predecessor;
        for (i = 0; i < parent->ptr_field.size(); ++i){
            if (parent->ptr_field.at(i) == leaf) break;
        }
        assert(i < parent->ptr_field.size());
        if (i < parent->ptr_field.size() - 1){ // Not the last element
            sibling = parent->ptr_field.at(i + 1);
            is_predecessor = true;
            sep_key_in_parent = parent->key_field.at(i);
        }
        else {
            sibling = parent->ptr_field.at(i - 1);
            is_predecessor = false;
            sep_key_in_parent = parent->key_field.at(i - 1);
        }

        // At most n - 1 keys in leaves.
        if (sibling->key_field.size() + leaf->key_field.size() < degree){ // Merge leaf and sibling
            if (is_predecessor){ // leaf is the predecessor of sibling
                // Exchange the variables for convenience here
                auto temp = leaf;
                leaf = sibling;
                sibling = temp;
            }

            // Move the content in leaf to sibling
            if (!leaf->ptr_field.empty()){
                sibling->ptr_field.front() = leaf->ptr_field.front();
            }
            for (size_t j = 0; j < leaf->key_field.size(); ++j){
                sibling->key_field.push_back(leaf->key_field.at(j));
                sibling->value_field.push_back(leaf->value_field.at(j));
            }

            Delete_in_Parent(leaf);

            delete leaf;

            return true;
        }
        else{ // Redistribution between two leaves
            if (is_predecessor){ // leaf is the predecessor of sibling
                leaf->key_field.push_back(sibling->key_field.front());
                leaf->value_field.push_back(sibling->value_field.front());

                sibling->key_field.erase(sibling->key_field.begin());
                sibling->value_field.erase(sibling->value_field.begin());

                parent->key_field.at(i) = sibling->key_field.front(); // The separation key
            }
            else{ // leaf is the successor of sibling
                leaf->key_field.insert(leaf->key_field.begin(), sibling->key_field.back());
                leaf->value_field.insert(leaf->value_field.begin(), sibling->value_field.back());

                sibling->key_field.pop_back();
                sibling->value_field.pop_back();

                parent->key_field.at(i - 1) = leaf->key_field.front(); // The separation key
            }

            return true;
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::FindValue(key_t key, value_t& result) {
    BP_NODE_T& node = *(FindNode(key));
    size_t id = node.FindPos_at_Node(key);
    if (id != -1){
        result = node.value_field.at(id);
        return true;
    }
    else return false;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::FindRange(key_t lower_key, key_t upper_key, std::vector<value_t>& result) {
    const BP_NODE_T* lower_node = FindNode(lower_key);
    const BP_NODE_T* upper_node = FindNode(upper_key);

    size_t start_id = lower_node->FindPos_at_Node(lower_key);
    size_t end_id = upper_node->FindPos_at_Node(upper_key);

    if (start_id == -1 || end_id == -1){
        return false;
    }
    else{
        if (lower_node == upper_node){
            for (size_t i = start_id; i <= end_id; ++i){
                result.push_back(lower_node->value_field.at(i));
            }
        }
        else{
            for (size_t i = start_id; i < lower_node->value_field.size(); ++i){
                result.push_back(lower_node->value_field.at(i));
            }
            const BP_NODE_T* next = lower_node->ptr_field.front();
            while(next != upper_node){
                for (auto value : next->value_field){
                    result.push_back(value);
                }
                next = next->ptr_field.front();
            }
            for (size_t i = 0; i <= end_id; ++i) {
                result.push_back(upper_node->value_field.at(i));
            }
        }
        return true;
    }
}

#endif