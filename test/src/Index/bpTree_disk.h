#ifndef BPTREE_DISK_H
#define BPTREE_DISK_H

#include <vector>
#include <iostream>
#include <cassert>
#include "bpTree_block.h"
#include "buffer_manager.h"
#include "err_type.h"
#include <cstring>

#define BP_TREE_T Bp_tree<key_t, value_t>
#define BP_TREE_BLOCK_T bpTree_Block<key_t, value_t>
#define BP_TREE_LEAF_T bpTree_Leaf<key_t, value_t>
#define BP_TREE_INTERNAL_T bpTree_Internal<key_t, blockId_t>

// TODO : Check if every pin is eliminated after use.

KEY_VALUE_T_DECLARE
class Bp_tree{
public:
    Bp_tree(BufferManager* buf_manager):_bfm(buf_manager){};

    /**
     * Must be called before every use !
     * @param indexFileName
     */
    void InitRoot(std::string indexFileName);

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

private:
    blockId_t _root_id = INVALID_BLOCK_ID;
    std::string _index_fname;
    size_t _file_block_count;
//    BP_TREE_BLOCK_T* header_ptr;
//    pageId_t header_pageId; ///< The header page id in the buffer pool. The page is pinned.

    BufferManager* _bfm;

    /**
     * @brief Find the pointer leaf node where the key-value stays.
     * NOTE: return the pointer because the corresponding block will be loaded in the disk.
     *
     * @param key Search key
     * @return BP_TREE_LEAF_T* Result
     */
    BP_TREE_LEAF_T* FindNode(key_t key, pageId_t leaf_pageId);

    /**
     * @brief Simply insert in a leaf node.
     *
     * @param key
     * @param val
     * @param leaf [in] The leaf node to insert the value
     * @return true insertion success
     * @return false insertion failure
     */
    bool Insert_in_Leaf(key_t key, value_t val, BP_TREE_LEAF_T* leaf);

    /**
     * @brief Insert in a parent. May split. Recursive call.
     *
     * @param key the smallest search key in split (the right one when splitting)
     * @param prev_leaf[in] left split node, whose pointer is in its parent->ptr_field
     * @param split[in] right split node. The new node.
     * @return true for success
     */
    bool Insert_in_Parent(key_t key, BP_TREE_BLOCK_T* prev_leaf, BP_TREE_BLOCK_T* split);

    void loadPointer(BP_TREE_LEAF_T* leaf, pageId_t& leaf_pageId, blockId_t b_id){
        char* raw = _bfm->getPage(PATH::INDEX_PATH + _index_fname, b_id, leaf_pageId);
        leaf = reinterpret_cast<BP_TREE_LEAF_T*>(raw);
    }

    void loadPointer(BP_TREE_INTERNAL_T* internal, pageId_t& internal_pageId, blockId_t b_id){
        char* raw = _bfm->getPage(PATH::INDEX_PATH + _index_fname, b_id, internal_pageId);
        internal = reinterpret_cast<BP_TREE_LEAF_T*>(raw);
    }

    void loadPointer(BP_TREE_BLOCK_T* block, blockId_t b_id){
        char* raw = _bfm->getPage(PATH::INDEX_PATH + _index_fname, b_id);
        block = reinterpret_cast<BP_TREE_BLOCK_T*>(raw);
    }

    void setChildrenParent(BP_TREE_INTERNAL_T* internal){
        for(size_t i = 0; i < internal->_size; ++i){
            BP_TREE_BLOCK_T* child;
            loadPointer(child, internal->_k_child_pair[i].second);
            child->_parent_block_id = internal->_block_id;
        }
    }

    /**
     * @brief Delete in the parent based on child pointer.
     *
     * @param to_delete_child The child to be removed
     * @return true Delete success
     */
    template<typename Node>
    bool Delete_in_Parent(Node* to_delete_child);
};

KEY_VALUE_T_DECLARE
void BP_TREE_T::


KEY_VALUE_T_DECLARE
BP_TREE_LEAF_T* BP_TREE_T::FindNode(key_t key, pageId_t leaf_pageId){
    pageId_t temp_pageId;
    char* temp_ptr = _bfm->getPage(PATH::INDEX_PATH + _index_fname, _root_id, temp_pageId);
    _bfm->pinPage(temp_pageId);

    BP_TREE_LEAF_T* root = reinterpret_cast<BP_TREE_LEAF_T>(temp_ptr);
    BP_TREE_INTERNAL_T* internal_ptr;

    if(!root->isLeaf()){
        internal_ptr = reinterpret_cast<BP_TREE_INTERNAL_T>(root);
        while(!internal_ptr->isLeaf()){
            size_t pos = internal_ptr->internal_biSearch(key); //The greatest key pos that is smaller or equal to the key.
            blockId_t child_b_id = internal_ptr->_k_child_pair[pos].second;
            if (temp_pageId != -1){
                _bfm->unpinPage(temp_pageId);
            }
            temp_ptr = _bfm->getPage(PATH::INDEX_PATH + _index_fname, child_b_id, temp_pageId);
            _bfm->pinPage(temp_pageId);
            internal_ptr = reinterpret_cast<BP_TREE_INTERNAL_T>(temp_ptr);
        }
    }
    else if (root->_blockType == INVALID_BLOCK){
        return nullptr;
    }

    // Make sure the page that internal_ptr pointing at is pinned!!!
    // internal_ptr is actually leaf pointer here
    return reinterpret_cast<BP_TREE_LEAF_T>(internal_ptr);
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Parent(key_t key, BP_TREE_BLOCK_T* prev_leaf, BP_TREE_BLOCK_T* split){
    if (prev_leaf->_block_id == _root_id){
        pageId_t newRoot_pageId;
        BP_TREE_INTERNAL_T* newRoot;
        loadPointer(newRoot, newRoot_pageId, _file_block_count);
        _bfm->modifyPage(newRoot_pageId);
        // TODO : Add pin page and not pin page for concurrency. Assume the page is not likely to be changed here.
        _file_block_count += 1;

        newRoot->init(_file_block_count - 1, INVALID_BLOCK_ID);
        newRoot->_size = 2;
        key_t trash;
        newRoot->_k_child_pair[0] = std::make_pair(trash, prev_leaf->_block_id);
        newRoot->_k_child_pair[1] = std::make_pair(key, split->_block_id);

        // CAUTION : Remember to reset the _parent_block_id
        prev_leaf->_parent_block_id = newRoot->_block_id;
        split->_parent_block_id = newRoot->_block_id;
        return true;
    }
    else{
        bool ret = false;

        BP_TREE_INTERNAL_T* parent = prev_leaf->parent;
        pageId_t parent_pageId;
        loadPointer(parent, parent_pageId, prev_leaf->_parent_block_id);
        _bfm->pinPage(parent_pageId);
        _bfm->modifyPage(parent_pageId);

        // Directly insert the current key and pointer temporarily
        size_t pos = parent->internal_biSearch(key); // This position should exactly be the separator of prev_leaf and its sibling
        pos += 1; // The correct starting point to move.
        size_t move_len = parent->_size - pos;

        memmove(parent->_k_child_pair + pos + 1, parent->_k_child_pair + pos,  move_len * sizeof(parent->_k_child_pair[0]));
        parent->_size += 1;
        parent->_k_child_pair[pos] = std::make_pair(key, split);
        split->_parent_block_id = parent->_block_id;
        prev_leaf->_parent_block_id = parent->_block_id;

        if (parent->_size > parent->_max_size){ // Not enough space. Split parent.
            assert(parent->_size == parent->_max_size + 1);

            BP_TREE_INTERNAL_T* split_parent;
            pageId_t split_pageId;

            loadPointer(split_parent, split_pageId, _file_block_count); // Assign a new block the node
            _bfm->pinPage(split_pageId);
            _bfm->modifyPage(split_pageId);

            _file_block_count += 1;
            // CAUTION : remember to init the newly assigned block.
            split_parent->init(_file_block_count, parent->_parent_block_id);

            // Move keys and pointers to split.
            size_t mid = (parent->_max_size + 1) / 2;
            key_t right_smallest_key = parent->_k_child_pair[mid].first;
            size_t move_len_split = (parent->_max_size + 1) - mid;

            // Because the first key is not used, we in fact abandoned the first key in split node.
            memcpy(split_parent, parent + mid, move_len_split * sizeof(parent->_k_child_pair[0]));
            split_parent->_size = move_len_split;
            parent->_size = mid;

            // CAUTION: Always remember to reset its children's parent
            setChildrenParent(split_parent);

            ret = Insert_in_Parent(right_smallest_key, parent, split_parent);
            _bfm->unpinPage(split_pageId);
            return ret;
        }
        else{
            _bfm->unpinPage(parent_pageId);
            return true;
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Leaf(key_t key, value_t val, BP_TREE_LEAF_T* leaf){
    assert(leaf->isLeaf());
//    assert(leaf->_size < leaf->_max_size);

    size_t pos = leaf->leaf_biSearch(key);

    // TODO : Change this to throw
    if (leaf->_k_rowid_pair[pos].first == key) return false;

    // Insert the key-val pair. The space should be pre-allocated at the initialization of node pages.
    memmove(leaf->_k_rowid_pair + pos + 1, leaf->_k_rowid_pair + pos, sizeof(leaf->_k_rowid_pair) * (leaf->_size - pos));
    leaf->_k_rowid_pair[pos] = std::make_pair(key, val);
    leaf->_size += 1;

    return true;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert(key_t key, value_t val){
    if (_root_id == INVALID_BLOCK_ID){ // Create a new root at block 0 of B+ tree file.
        _root_id = 0;
        pageId_t root_pageId;
        char* root_page;
        try{
            root_page = _bfm->getPage(PATH::INDEX_PATH + _index_fname, _root_id, root_pageId);
        }
        catch(db_err_t& db_err){
            throw db_err;
        }
        _bfm->pinPage(root_pageId);
        BP_TREE_LEAF_T* root = reinterpret_cast<BP_TREE_LEAF_T*>(root_page);

        _bfm->modifyPage(root_pageId);
        root->init(0, INVALID_BLOCK_ID, INVALID_BLOCK_ID);

        Insert_in_Leaf(key, val, root);
        _bfm->unpinPage(root_pageId);
        return true;
    }
    else{
        pageId_t leaf_pageId;
        BP_TREE_LEAF_T* leaf = FindNode(key, leaf_pageId); // The page is already pinned in the function.
        if (leaf == nullptr){
            return false;
        }

        if (leaf->_size() < leaf->_max_size){ // Still have space
            Insert_in_Leaf(key, val, leaf);
        }
        else{
            assert(leaf->_size == leaf->_max_size);

            pageId_t split_pageId;
            char* split_ptr;
            try{
                split_ptr = _bfm->getPage(PATH::INDEX_PATH + _index_fname, _file_block_count, split_pageId);
            }
            catch(db_err_t db_err){
                throw db_err;
            }

            _bfm->pinPage(split_pageId);
            BP_TREE_LEAF_T* split = reinterpret_cast<BP_TREE_LEAF_T*>(split_ptr);
            _file_block_count += 1;


            _bfm->modifyPage(split_pageId);
            _bfm->modifyPage(leaf_pageId);
            // Set the block_id, root_id, sibling, parent
            split->init(_file_block_count - 1, leaf->_parent_block_id, INVALID_BLOCK_ID);
            leaf->_next_block_id = split->_block_id;

            // Insert in leaf first
            Insert_in_Leaf(key, val, leaf);

            /// Copy values and keys, and set the **size**
            // The position right after the (max_size + 1)/2 th element. In this way, both split nodes contains more than max_size/2 elements
            size_t i = (leaf->_max_size + 1) / 2;
            size_t copy_len = (leaf->_max_size + 1) - i;

            memcpy(split->_k_rowid_pair, leaf->_k_rowid_pair + i, sizeof(leaf->_k_rowid_pair[0]) * copy_len);
            leaf->_size = i;
            split->_size = copy_len;

            Insert_in_Parent(split->_k_rowid_pair[0].first, leaf, split);

            _bfm->unpinPage(split_pageId);
            _bfm->unpinPage(leaf_pageId);
        }
        return true;
    }
}

//KEY_VALUE_T_DECLARE
//bool BP_TREE_T::Delete_in_Parent(BP_NODE_T* to_delete_child){
//    BP_NODE_T* parent = to_delete_child->parent;
//
//    // Delete the pointer to the child from its parent, and its "separation key"
//    // Caller need to make sure that the abandoned child is the latter one in merging
//    for (size_t i = 0; i < parent->ptr_field.size(); ++i){
//        if (parent->ptr_field.at(i) == to_delete_child){
//            parent->ptr_field.erase(parent->ptr_field.begin() + i);
//            parent->key_field.erase(parent->key_field.begin() + i - 1);
//            break;
//        }
//    }
//
//    if (parent == root){
//        if (parent->ptr_field.size() == 1){
//            root = parent->ptr_field.front();
//            delete parent;
//            return true;
//        }
//    }
//    else if (parent->key_field.size() < (degree + 1) / 2){ // (int)(degree + 1)/2 is equivalent to degree/2 's upper integer
//        BP_NODE_T* sibling;
//        BP_NODE_T* grand_parent = parent->parent;
//        key_t sep_key_in_grandparent;
//        size_t i;
//        bool is_predecessor;
//        for (i = 0; i < grand_parent->ptr_field.size(); ++i){
//            if (grand_parent->ptr_field.at(i) == parent) break;
//        }
//
//        assert(i < grand_parent->ptr_field.size());
//
//        if (i < grand_parent->ptr_field.size() - 1){ // Not the last child
//            sibling = grand_parent->ptr_field.at(i + 1);
//            sep_key_in_grandparent = grand_parent->key_field.at(i);
//            is_predecessor = true;
//        }
//        else{
//            sibling = grand_parent->ptr_field.at(i - 1);
//            sep_key_in_grandparent = grand_parent->key_field.at(i - 1);
//            is_predecessor = false;
//        }
//
//        // At most n - 1 keys in non-leaves
//        if (parent->key_field.size() + sibling->key_field.size() < degree){ // Merge leaf and sibling
//            if (is_predecessor) { // Parent is the predecessor
//                auto temp = parent;
//                parent = sibling;
//                sibling = temp;
//            }
//            // Move the content in leaf to sibling, and set the child->parent pointer
//            sibling->key_field.push_back(sep_key_in_grandparent);
//            sibling->ptr_field.push_back(parent->ptr_field.front());
//            sibling->ptr_field.back()->parent = sibling;
//            for (size_t j = 0; j < parent->key_field.size(); ++j){
//                sibling->key_field.push_back(parent->key_field.at(j));
//                sibling->ptr_field.push_back(parent->ptr_field.at(j + 1));
//                sibling->ptr_field.back()->parent = sibling;
//            }
//
//            Delete_in_Parent(parent);
//
//            delete parent;
//
//            return true;
//        }
//        else{ // Redistribute the keys and pointers
//            if (is_predecessor){
//                parent->key_field.push_back(sep_key_in_grandparent);
//                parent->ptr_field.push_back(sibling->ptr_field.front());
//                parent->ptr_field.back()->parent = parent;
//
//                grand_parent->key_field.at(i) = sibling->key_field.front();
//
//                sibling->key_field.erase(sibling->key_field.begin());
//                sibling->ptr_field.erase(sibling->ptr_field.begin());
//            }
//            else{
//                parent->key_field.insert(parent->key_field.begin(), sep_key_in_grandparent);
//                parent->ptr_field.insert(parent->ptr_field.begin(), sibling->ptr_field.back());
//                parent->ptr_field.front()->parent = parent;
//
//                grand_parent->key_field.at(i - 1) = sibling->key_field.back();
//
//                sibling->key_field.pop_back();
//                sibling->ptr_field.pop_back();
//            }
//
//            return true;
//        }
//    }
//}
//
//KEY_VALUE_T_DECLARE
//bool BP_TREE_T::Delete(key_t key) {
//    BP_NODE_T* leaf = FindNode(key);
//
//    // Delete the key and value from leaf
//    for (size_t i = 0; i < leaf->key_field.size(); ++i){
//        if (leaf->key_field.at(i) == key){
//            leaf->key_field.erase(leaf->key_field.begin() + i);
//            leaf->value_field.erase(leaf->value_field.begin() + i);
//            break;
//        }
//    }
//
//    if (leaf == root) return true;
//        // Violate capacity constraint for leaf and non-root node
//    else if (leaf->key_field.size() < degree / 2){ // (int)degree/2 is equivalent to (degree - 1)/2's upper integer
//        BP_NODE_T* sibling;
//        BP_NODE_T* parent = leaf->parent;
//        key_t sep_key_in_parent;
//        size_t i;
//        bool is_predecessor;
//        for (i = 0; i < parent->ptr_field.size(); ++i){
//            if (parent->ptr_field.at(i) == leaf) break;
//        }
//        assert(i < parent->ptr_field.size());
//        if (i < parent->ptr_field.size() - 1){ // Not the last element
//            sibling = parent->ptr_field.at(i + 1);
//            is_predecessor = true;
//            sep_key_in_parent = parent->key_field.at(i);
//        }
//        else {
//            sibling = parent->ptr_field.at(i - 1);
//            is_predecessor = false;
//            sep_key_in_parent = parent->key_field.at(i - 1);
//        }
//
//        // At most n - 1 keys in leaves.
//        if (sibling->key_field.size() + leaf->key_field.size() < degree){ // Merge leaf and sibling
//            if (is_predecessor){ // leaf is the predecessor of sibling
//                // Exchange the variables for convenience here
//                auto temp = leaf;
//                leaf = sibling;
//                sibling = temp;
//            }
//
//            // Move the content in leaf to sibling
//            if (!leaf->ptr_field.empty()){
//                sibling->ptr_field.front() = leaf->ptr_field.front();
//            }
//            for (size_t j = 0; j < leaf->key_field.size(); ++j){
//                sibling->key_field.push_back(leaf->key_field.at(j));
//                sibling->value_field.push_back(leaf->value_field.at(j));
//            }
//
//            Delete_in_Parent(leaf);
//
//            delete leaf;
//        }
//        else{ // Redistribution between two leaves
//            if (is_predecessor){ // leaf is the predecessor of sibling
//                leaf->key_field.push_back(sibling->key_field.front());
//                leaf->value_field.push_back(sibling->value_field.front());
//
//                sibling->key_field.erase(sibling->key_field.begin());
//                sibling->value_field.erase(sibling->value_field.begin());
//
//                parent->key_field.at(i) = sibling->key_field.front(); // The separation key
//            }
//            else{ // leaf is the successor of sibling
//                leaf->key_field.insert(leaf->key_field.begin(), sibling->key_field.back());
//                leaf->value_field.insert(leaf->value_field.begin(), sibling->value_field.back());
//
//                sibling->key_field.pop_back();
//                sibling->value_field.pop_back();
//
//                parent->key_field.at(i - 1) = leaf->key_field.front(); // The separation key
//            }
//        }
//        return true;
//    }
//}
//
//KEY_VALUE_T_DECLARE
//bool BP_TREE_T::FindValue(key_t key, value_t& result) {
//    BP_NODE_T& node = *(FindNode(key));
//    size_t id = node.FindPos_at_Node(key);
//    if (id != -1){
//        result = node.value_field.at(id);
//        return true;
//    }
//    else return false;
//}
//
//KEY_VALUE_T_DECLARE
//bool BP_TREE_T::FindRange(key_t lower_key, key_t upper_key, std::vector<value_t>& result) {
//    const BP_NODE_T* lower_node = FindNode(lower_key);
//    const BP_NODE_T* upper_node = FindNode(upper_key);
//
//    size_t start_id = lower_node->FindPos_at_Node(lower_key);
//    size_t end_id = upper_node->FindPos_at_Node(upper_key);
//
//    if (start_id == -1 || end_id == -1){
//        return false;
//    }
//    else{
//        if (lower_node == upper_node){
//            for (size_t i = start_id; i <= end_id; ++i){
//                result.push_back(lower_node->value_field.at(i));
//            }
//        }
//        else{
//            for (size_t i = start_id; i < lower_node->value_field.size(); ++i){
//                result.push_back(lower_node->value_field.at(i));
//            }
//            const BP_NODE_T* next = lower_node->ptr_field.front();
//            while(next != upper_node){
//                for (auto value : next->value_field){
//                    result.push_back(value);
//                }
//                next = next->ptr_field.front();
//            }
//            for (size_t i = 0; i <= end_id; ++i) {
//                result.push_back(upper_node->value_field.at(i));
//            }
//        }
//        return true;
//    }
//}

#endif