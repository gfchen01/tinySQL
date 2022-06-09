#ifndef BPTREE_DISK_H
#define BPTREE_DISK_H

#include <vector>
#include <iostream>
#include <cassert>
#include <cstring>

#include "index/bpTree_block.h"
#include "buffer/buffer_manager.h"
#include "share/err_type.h"


#define BP_TREE_T Bp_tree<key_t, value_t>
#define BP_TREE_BLOCK_T bpTree_Block
#define BP_TREE_LEAF_T bpTree_Leaf<key_t, value_t>
#define BP_TREE_INTERNAL_T bpTree_Internal<key_t, blockId_t>

// TODO : Check if every pin is eliminated after use.

KEY_VALUE_T_DECLARE
class Bp_tree{
//    friend class IndexManager;
public:
    Bp_tree(BufferManager* buf_manager):_bfm(buf_manager){};

    /**
     * Init 1.indexName of the bpTree 2.root block id 3.total block count.
     * Must be called before every use !
     * @param indexFileName
     */
    void InitRoot(std::string indexFileName);

    const std::string& getName() const {
        return _index_fname;
    }

    /**
     * @brief Basic insertion to the tree.
     *
     * @param key The key for the val.
     * @param val The val (or normally so-called "pointer") to the record.
     * @return true Insertion success
     * @return false Insertion fail
     */
    bool Insert(const key_t &key, const value_t &val);

    /**
     * @brief Basic deletion from the tree.
     *
     * Users should call more general functions, like update val, etc.
     *
     * @param key The key for a certain record.
     * @return true Deletion success.
     * @return false Insertion success.
     */
    bool Delete(const key_t &key);

    /**
     *
     * @param key
     * @param deleted [out] The deleted value.
     * @return
     */
    bool Delete(const key_t &key, value_t &deleted);

    /**
     * @brief Find the values by key (normally, we may say "pointers" for "values")
     *
     * @param key The search key.
     * @param result [out] The search result. Push them back in the result.
     * @return true Search success
     * @return false
     */
    bool FindValue(const key_t &key, value_t &result) const;

    /**
     * @brief Find the value(pointer) based on the given lower and upper key
     *
     * @param lower_key [in]
     * @param upper_key [in]
     * @param result [out]
     * @return true Search success
     * @return false Search failure
     */
    bool FindRange(const key_t& lower_key, const key_t& upper_key, std::vector<value_t>& result) const;

    /** @brief Update a certain key.
     *
     * @param former_key
     * @param new_key
     * @return
     */
    bool UpdateKey(const key_t &former_key, const key_t &new_key);

    /**
     * @brief Update a certain value
     * @param key
     * @param new_val
     * @return
     */
    bool UpdateValue(const key_t &key, const value_t &new_val);

private:
    blockId_t _root_id = INVALID_BLOCK_ID;
    std::string _index_fname; ///< The full name of the has_index file name.
    db_size_t _file_block_count; ///< The block number of the has_index file.
//    BP_TREE_BLOCK_T* header_ptr;
//    pageId_t header_pageId; ///< The header page id in the buffer pool. The page is pinned.

    BufferManager* _bfm;

    blockId_t FindRootBlockId(BP_TREE_BLOCK_T* block);

    /**
     * @brief Find the pointer leaf node where the key-value stays.
     * NOTE: return the pointer because the corresponding block will be loaded in the disk.
     * And the block will be pinned. So remember to unpin the page.
     *
     * @param key Search key
     * @return BP_TREE_LEAF_T* Result
     */
    BP_TREE_LEAF_T* FindNode(const key_t &key, pageId_t &leaf_pageId) const;

    /**
     * @brief Simply insert in a leaf node.
     *
     * @param key
     * @param val
     * @param leaf [in] The leaf node to insert the value
     * @return true insertion success
     * @return false insertion failure
     */
    bool Insert_in_Leaf(const key_t &key, const value_t &val, BP_TREE_LEAF_T* leaf);

    /**
     * @brief Insert in a parent. May split. Recursive call.
     *
     * @param key the smallest search key in split (the right one when splitting)
     * @param prev_leaf[in] left split node, whose pointer is in its parent->ptr_field
     * @param split[in] right split node. The new node.
     * @return true for success
     */
    bool Insert_in_Parent(const key_t &key, BP_TREE_BLOCK_T* prev_leaf, BP_TREE_BLOCK_T* split);

    /**
     * @brief Delete in the parent based on child pointer.
     *  Use many pointers because they are loaded and pinned. This will reduce cost
     * @param to_delete_child The child to be removed
     * @param sibling The sibling to the child. (Always the left sibling)
     * @param parent Parent to the children
     * @return true Delete success
     */
    bool Delete_in_Parent(BP_TREE_BLOCK_T* parent, db_size_t pos_sep);

    void loadPointer(BP_TREE_LEAF_T*& leaf, pageId_t& leaf_pageId, blockId_t b_id) const{
        char* raw = _bfm->getPage(_index_fname, b_id, leaf_pageId);
        leaf = reinterpret_cast<BP_TREE_LEAF_T*>(raw);
    }

    void loadPointer(BP_TREE_INTERNAL_T*& internal, pageId_t& internal_pageId, blockId_t b_id) const{
        char* raw = _bfm->getPage(_index_fname, b_id, internal_pageId);
        internal = reinterpret_cast<BP_TREE_INTERNAL_T*>(raw);
    }

    void loadPointer(BP_TREE_BLOCK_T*& block, blockId_t b_id) const{
        char* raw = _bfm->getPage(_index_fname, b_id);
        block = reinterpret_cast<BP_TREE_BLOCK_T*>(raw);
    }

    void loadPointer(BP_TREE_BLOCK_T*& block, pageId_t& pageId, blockId_t b_id) const {
        char* raw = _bfm->getPage(_index_fname, b_id, pageId);
        block = reinterpret_cast<BP_TREE_BLOCK_T*>(raw);
    }

    void setChildrenParent(BP_TREE_INTERNAL_T* internal){
        for(db_size_t i = 0; i < internal->_size; ++i){
            BP_TREE_BLOCK_T* child;
            loadPointer(child, internal->_k_child_pair[i].second);
            child->_parent_block_id = internal->_block_id;
        }
    }

    std::pair<key_t, value_t>& getKeyRowidPair(const key_t &key);
};

KEY_VALUE_T_DECLARE
void BP_TREE_T::InitRoot(std::string indexFileName) {
    _index_fname = indexFileName;

    pageId_t headPageId;
    BP_TREE_BLOCK_T* headBlock;
    loadPointer(headBlock, headPageId, 0); // May throw here
    _bfm->pinPage(headPageId);

    // Set _root_id to avoid expensive operations later;
    // and set the initial total block count of the file
    if (headBlock->_blockType == INVALID_BLOCK){ // Happens when no content except the header is in the file
        _root_id = INVALID_BLOCK_ID;
    }
    else{
        _root_id = FindRootBlockId(headBlock);
    }
    // Upper bound of the integer: fileSize / PAGESIZE
    _file_block_count = (_bfm->getFileSize(indexFileName) + PAGESIZE - 1) / PAGESIZE;
    _bfm->unpinPage(headPageId);
}

KEY_VALUE_T_DECLARE
blockId_t BP_TREE_T::FindRootBlockId(bpTree_Block *block) {
    assert(block->_blockType != INVALID_BLOCK);

    while (block->_parent_block_id != INVALID_BLOCK_ID){
        loadPointer(block, block->_parent_block_id);
    }

    return block->_block_id;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::FindValue(const key_t &key, value_t &result) const {
    pageId_t p_Id;
    BP_TREE_LEAF_T* n = FindNode(key, p_Id);

    if (n == nullptr) throw DB_BPTREE_EMPTY;

    db_size_t pos = n->leaf_biSearch(key);
    if (pos < n->_size){
        result = n->_k_rowid_pair[pos].second;
    }
    if (pos < n->_size && n->_k_rowid_pair[pos].first == key){
        _bfm->unpinPage(p_Id);
        return true;
    }
    else {
        _bfm->unpinPage(p_Id);
        return false;
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::FindRange(const key_t &lower_key, const key_t &upper_key, std::vector<value_t> &result) const {
    assert(lower_key < upper_key);

    pageId_t lkey_pId;
    BP_TREE_LEAF_T* lkey_leaf = FindNode(lower_key, lkey_pId);

    pageId_t rkey_pId;
    BP_TREE_LEAF_T* rkey_leaf = FindNode(upper_key, rkey_pId);

    if (rkey_leaf == nullptr || lkey_leaf == nullptr) throw DB_BPTREE_EMPTY;

    db_size_t l_pos = lkey_leaf->leaf_biSearch(lower_key);
//    if (lkey_leaf->_k_rowid_pair[l_pos].first != lower_key) return false;
    db_size_t r_pos = rkey_leaf->leaf_biSearch(upper_key);

    if (r_pos >= rkey_leaf->_size || rkey_leaf->_k_rowid_pair[r_pos].first != upper_key) {
        if(r_pos != 0) --r_pos;
        else{
            std::cout << "BP Tree Error! -- Contact CGF" << std::endl;
        }
    }

    if (lkey_leaf->_block_id == rkey_leaf->_block_id){
        for (db_size_t pos = l_pos; pos <= r_pos; ++pos){
            result.push_back(lkey_leaf->_k_rowid_pair[pos].second);
        }
    }
    else{
        for (db_size_t pos = l_pos; pos < lkey_leaf->_size; ++pos){
            result.push_back(lkey_leaf->_k_rowid_pair[pos].second);
        }
        BP_TREE_LEAF_T* next_node;
        pageId_t next_pId;
        while(lkey_leaf->_next_block_id != rkey_leaf->_block_id){
            loadPointer(next_node, next_pId, lkey_leaf->_next_block_id);
            _bfm->unpinPage(lkey_pId);
            lkey_leaf = next_node;
            lkey_pId = next_pId;

            for (db_size_t pos = 0; pos < lkey_leaf->_size; ++pos){
                result.push_back(lkey_leaf->_k_rowid_pair[pos].second);
            }
        }

        _bfm->unpinPage(lkey_pId);

        for (db_size_t pos = 0; pos <= r_pos; ++pos){
            result.push_back(lkey_leaf->_k_rowid_pair[pos].second);
        }
        _bfm->unpinPage(rkey_pId);
    }
    return true;
}

KEY_VALUE_T_DECLARE
BP_TREE_LEAF_T* BP_TREE_T::FindNode(const key_t &key, pageId_t& leaf_pageId) const{
    pageId_t temp_pageId;
    BP_TREE_BLOCK_T* root;
    if (_root_id == INVALID_BLOCK_ID){
        return nullptr;
    }
    loadPointer(root, temp_pageId, _root_id);

    BP_TREE_INTERNAL_T* internal_ptr;
    if (root->_blockType == INVALID_BLOCK){
        return nullptr;
    }
    else if(root->isLeaf()){
        internal_ptr = (BP_TREE_INTERNAL_T*)root; // Simply init. Make no sense.
    }
    else{
        internal_ptr = static_cast<BP_TREE_INTERNAL_T*>(root);
        while(!internal_ptr->isLeaf()){
            //The greatest key pos that is smaller or equal to the key.
            db_size_t pos = internal_ptr->internal_biSearch(key);
            blockId_t child_b_id = internal_ptr->_k_child_pair[pos].second;
            loadPointer(internal_ptr, temp_pageId, child_b_id);
        }
    }

    // Make sure the page that internal_ptr pointing at is pinned!!!
    leaf_pageId = temp_pageId;
    _bfm->pinPage(leaf_pageId);
    // internal_ptr is actually leaf pointer here.
    return reinterpret_cast<BP_TREE_LEAF_T*>(internal_ptr);
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Parent(const key_t &key, BP_TREE_BLOCK_T* prev_leaf, BP_TREE_BLOCK_T* split){
    if (prev_leaf->_block_id == _root_id){
        pageId_t newRoot_pageId;
        BP_TREE_INTERNAL_T* newRoot;
        loadPointer(newRoot, newRoot_pageId, _file_block_count);
        _bfm->modifyPage(newRoot_pageId);
        // TODO : Add pin page and not pin page for concurrency. Assume the page is not likely to be changed here.
        _file_block_count += 1;

        _root_id = _file_block_count - 1;
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
        bool ret;

        BP_TREE_INTERNAL_T* parent;
        pageId_t parent_pageId;
        loadPointer(parent, parent_pageId, prev_leaf->_parent_block_id);

        if(parent->_blockType != INTERNAL_BLOCK){
            loadPointer(parent, parent_pageId, prev_leaf->_parent_block_id);
        }

        _bfm->pinPage(parent_pageId);
        _bfm->modifyPage(parent_pageId);

        // Directly insert the current key and pointer temporarily
        db_size_t pos = parent->internal_biSearch(key); // This position should exactly be the separator of prev_leaf and its sibling
        pos += 1; // The correct starting point to move.
        db_size_t move_len = parent->_size - pos;

        memmove(parent->_k_child_pair + pos + 1, parent->_k_child_pair + pos,  move_len * sizeof(parent->_k_child_pair[0]));
        parent->_size += 1;
        parent->_k_child_pair[pos] = std::make_pair(key, split->_block_id);
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
            split_parent->init(_file_block_count - 1, parent->_parent_block_id);

            // Move keys and pointers to split.
            db_size_t mid = (parent->_max_size + 1) / 2;
            key_t right_smallest_key = parent->_k_child_pair[mid].first;
            db_size_t move_len_split = (parent->_max_size + 1) - mid;

            // Because the first key is not used, we in fact abandoned the first key in split node.
            memcpy(split_parent->_k_child_pair, parent->_k_child_pair + mid, move_len_split * sizeof(parent->_k_child_pair[0]));
            split_parent->_size = move_len_split;
            parent->_size = mid;

            // CAUTION: Always remember to reset its children's parent
            setChildrenParent(split_parent);

            ret = Insert_in_Parent(right_smallest_key, parent, split_parent);
            _bfm->unpinPage(split_pageId);
            _bfm->unpinPage(parent_pageId);
            return ret;
        }
        else{
            _bfm->unpinPage(parent_pageId);
            return true;
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert_in_Leaf(const key_t &key, const value_t &val, BP_TREE_LEAF_T* leaf){
    assert(leaf->isLeaf());
//    assert(leaf->_size < leaf->_max_size);

    db_size_t pos = leaf->leaf_biSearch(key);

    // TODO : Change this to throw
    // Only allow is_unique key
    if (pos < leaf->_size /*&& leaf->_size > 0*/ && leaf->_k_rowid_pair[pos].first == key) return false;

    // Insert the key-val pair. The space should be pre-allocated at the initialization of node pages.
    memmove((void*)(leaf->_k_rowid_pair + pos + 1), (void*)(leaf->_k_rowid_pair + pos), sizeof(leaf->_k_rowid_pair[0]) * (leaf->_size - pos));
    leaf->_k_rowid_pair[pos] = std::make_pair(key, val);
    leaf->_size += 1;

    return true;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Insert(const key_t &key, const value_t &val){
    if (_root_id == INVALID_BLOCK_ID){ // Create a new root at block 0 of B+ tree file.
        _root_id = 0;
        pageId_t root_pageId;
        BP_TREE_LEAF_T* root;
        loadPointer(root, root_pageId, 0);

        _bfm->pinPage(root_pageId);
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

        _bfm->modifyPage(leaf_pageId);

        if (leaf->_size < leaf->_max_size){ // Still have space
            Insert_in_Leaf(key, val, leaf);
            _bfm->unpinPage(leaf_pageId);
        }
        else{
            assert(leaf->_size == leaf->_max_size);

            BP_TREE_BLOCK_T* p;
            BP_TREE_INTERNAL_T* p2;
            pageId_t trash;
            if(leaf->_block_id != _root_id){
                loadPointer(p, leaf->_parent_block_id);
                assert(p->_blockType == INTERNAL_BLOCK);

                loadPointer(p2, trash, leaf->_parent_block_id);
                assert(p2->_blockType == INTERNAL_BLOCK);
            }

            pageId_t split_pageId;
            BP_TREE_LEAF_T* split;
            loadPointer(split, split_pageId, _file_block_count);
            _file_block_count += 1;
            _bfm->pinPage(split_pageId);

            _bfm->modifyPage(split_pageId);
            // Set the block_id, root_id, sibling, parent
            split->init(_file_block_count - 1, leaf->_parent_block_id, leaf->_next_block_id);
            leaf->_next_block_id = split->_block_id;

            // Insert in leaf first
            Insert_in_Leaf(key, val, leaf);

            /// Copy values and keys, and set the **size**
            // The position right after the (max_size + 1)/2 th element. In this way, both split nodes contains more than max_size/2 elements
            db_size_t i = (leaf->_max_size + 1) / 2;
            db_size_t copy_len = (leaf->_max_size + 1) - i;

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

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Delete_in_Parent(BP_TREE_BLOCK_T* _parent, db_size_t pos_sep){
    // Caller has set the modifyPage flag for these pointers!
    auto parent = static_cast<BP_TREE_INTERNAL_T*>(_parent);

    // Delete the pointer to the child from its parent, and its "separation key"
    // Caller need to make sure that the abandoned child is the latter one in merging
    memmove(parent->_k_child_pair + pos_sep, parent->_k_child_pair + pos_sep + 1,
            (parent->_size - (pos_sep + 1)) * sizeof(parent->_k_child_pair[0]));
    parent->_size -= 1;

    if (parent->_block_id == _root_id){
        if (parent->_size == 1){
            _root_id = parent->_k_child_pair[0].second;
            // TODO : Check the lazy delete (reorganize file)
            parent->_blockType = INVALID_BLOCK;
//            delete parent;
        }
        return true;
    }
    else if (parent->_size < (parent->_max_size + 1) / 2){ // (int)(degree + 1)/2 is equivalent to degree/2 's upper integer
        BP_TREE_INTERNAL_T* grandparent;
        pageId_t grandparent_pageId;

        BP_TREE_INTERNAL_T* parent_sibling;
        pageId_t parentSib_pageId;

        loadPointer(grandparent, grandparent_pageId, parent->_parent_block_id);
        _bfm->pinPage(grandparent_pageId);

        assert(grandparent->_blockType == INTERNAL_BLOCK); // Avoid invalid_block debug

        key_t sep_key_in_grandparent;
        db_size_t pos_sep_grand = grandparent->internal_biSearch(parent->_k_child_pair[0].first);
        bool is_predecessor;

        assert(grandparent->_k_child_pair[pos_sep_grand].second == parent->_block_id);

        if (pos_sep_grand == grandparent->_size - 1){ // The last child
            loadPointer(parent_sibling, parentSib_pageId, grandparent->_k_child_pair[pos_sep_grand - 1].second);
            is_predecessor = false;
            sep_key_in_grandparent = grandparent->_k_child_pair[pos_sep_grand].first;

            assert(grandparent->_k_child_pair[pos_sep_grand].first == parent->_k_child_pair[0].first);
        }
        else{
            pos_sep_grand += 1;
            loadPointer(parent_sibling, parentSib_pageId, grandparent->_k_child_pair[pos_sep_grand].second);
            is_predecessor = true;
            sep_key_in_grandparent = grandparent->_k_child_pair[pos_sep_grand].first;

            assert(grandparent->_k_child_pair[pos_sep_grand].first == parent_sibling->_k_child_pair[0].first);
        }
        _bfm->pinPage(parentSib_pageId);

        // At most n - 1 keys in non-leaves
        if (parent->_size + parent_sibling->_size < parent->_max_size){ // Merge leaf and sibling
            if (is_predecessor) { // Parent is the predecessor
                auto temp = parent;
                parent = parent_sibling;
                parent_sibling = temp;
            }
            // Move the content in parent to sibling, and
            memcpy(parent_sibling->_k_child_pair + parent_sibling->_size, parent->_k_child_pair,
                   parent->_size * sizeof(parent->_k_child_pair[0]));
            parent_sibling->_size += parent->_size;
            parent->_size = 0;

            // set the child->parent pointer
            for(db_size_t i = parent_sibling->_size - parent->_size; i < parent_sibling->_size; ++i){
                BP_TREE_BLOCK_T* child;
                loadPointer(child, parent_sibling->_k_child_pair[i].second);
                child->_parent_block_id = parent_sibling->_block_id;
            }
            _bfm->unpinPage(parentSib_pageId);
            Delete_in_Parent(grandparent, pos_sep_grand);
            // Lazy delete
            parent->_blockType = INVALID_BLOCK;
//            delete parent;
            return true;
        }
        else{ // Redistribute the keys and pointers
            if (is_predecessor){
                parent->_k_child_pair[parent->_size] = parent_sibling->_k_child_pair[0];
                parent->_size += 1;
                parent_sibling->_size -= 1;
                memmove(parent_sibling->_k_child_pair, parent_sibling->_k_child_pair + 1,
                        parent_sibling->_size * sizeof(parent->_k_child_pair[0]));

                // Set the moved child's parent
                BP_TREE_BLOCK_T* child;
                loadPointer(child, parent->_k_child_pair[parent->_size - 1].second);
                child->_parent_block_id = parent_sibling->_block_id;

                grandparent->_k_child_pair[pos_sep_grand].first = parent_sibling->_k_child_pair[0].first;
            }
            else{
                memmove(parent->_k_child_pair + 1, parent->_k_child_pair,
                        parent->_size * sizeof(parent->_k_child_pair[0]));
                parent->_k_child_pair[0] = parent_sibling->_k_child_pair[parent_sibling->_size - 1];
                parent_sibling->_size -= 1;
                parent->_size += 1;

                // Set the moved child's parent
                BP_TREE_BLOCK_T* child;
                loadPointer(child, parent->_k_child_pair[0].second);
                child->_parent_block_id = parent->_block_id;

                grandparent->_k_child_pair[pos_sep_grand].first = parent->_k_child_pair[0].first;
            }
            _bfm->unpinPage(parentSib_pageId);
            return true;
        }
    }
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::Delete(const key_t &key) {
    pageId_t leaf_pageId;
    BP_TREE_LEAF_T* leaf = FindNode(key, leaf_pageId); // The page should be already pinned
    if (leaf == nullptr){ // Usually happens when tree is empty
        throw DB_KEY_NOT_FOUND;
    }

    _bfm->modifyPage(leaf_pageId);

    // Delete the key and value from leaf
    db_size_t pos = leaf->leaf_biSearch(key);
    if(leaf->_k_rowid_pair[pos].first != key){ // Delete must match
        throw DB_KEY_NOT_FOUND;
    }

    // Remove the key and value
    memmove(leaf->_k_rowid_pair + pos, leaf->_k_rowid_pair + pos + 1,
            sizeof(leaf->_k_rowid_pair[0]) * (leaf->_size - (pos + 1)));
    leaf->_size -= 1;

    if (leaf->_block_id == _root_id) {
        if (leaf->_size == 0){ // Empty tree
            leaf->_blockType = INVALID_BLOCK;
            _root_id = INVALID_BLOCK_ID;
        }
        return true;
    }
    // Violate capacity constraint for leaf and non-root node
    else if (leaf->_size < (leaf->_max_size + 1) / 2){ // The upper bound of _max_size / 2
        // Make sure of the safety.
        assert(leaf->_size == (leaf->_max_size + 1) / 2 - 1);

        BP_TREE_LEAF_T *sibling;
        pageId_t sibling_pageId;
        BP_TREE_INTERNAL_T *parent;
        pageId_t parent_pageId;

        loadPointer(parent, parent_pageId, leaf->_parent_block_id);
        _bfm->pinPage(parent_pageId);
        // We have to know whether to choose the sibling forward or backward.
        // I choose the sibling backward, unless leaf is the tail child of its parent
        key_t sep_key_in_parent;
        bool is_predecessor;
        db_size_t pos_sep = parent->internal_biSearch(leaf->_k_rowid_pair[0].first); ///< the separation position
        assert(parent->_k_child_pair[pos_sep].second == leaf->_block_id);

        if (pos_sep == parent->_size - 1){ // The tail child.
            loadPointer(sibling, sibling_pageId, parent->_k_child_pair[pos_sep - 1].second);
            sep_key_in_parent = parent->_k_child_pair[pos_sep].first;
            // For safety and debug purpose.
            assert(parent->_k_child_pair[pos_sep].first == leaf->_k_rowid_pair[0].first);
            is_predecessor = false;
        }
        else{
            pos_sep += 1;
            loadPointer(sibling, sibling_pageId, parent->_k_child_pair[pos_sep].second);
            sep_key_in_parent = parent->_k_child_pair[pos_sep].first;

            assert(parent->_k_child_pair[pos_sep].first == sibling->_k_rowid_pair[0].first);
            is_predecessor = true;
        }
        _bfm->pinPage(sibling_pageId);

        // At most n - 1 keys in leaves.
        if (sibling->_size + leaf->_size < sibling->_max_size){ // Merge leaf and sibling
            _bfm->modifyPage(sibling_pageId);
            _bfm->modifyPage(leaf_pageId);
            _bfm->modifyPage(parent_pageId);
            // Make sure leaf is the latter one
            if (is_predecessor){
                auto temp1 = leaf;
                leaf = sibling;
                sibling = temp1;
            }

            // Move the content in leaf to sibling
            // 1. move the sibling pointer to the next node
            sibling->_next_block_id = leaf->_next_block_id;
            // 2. move the _k_rowid_pair
            memcpy(sibling->_k_rowid_pair + sibling->_size, leaf->_k_rowid_pair,
                   leaf->_size * sizeof(leaf->_k_rowid_pair[0]));
            sibling->_size += leaf->_size;
            leaf->_size = 0;

            _bfm->unpinPage(sibling_pageId);
            _bfm->unpinPage(leaf_pageId);

            Delete_in_Parent(parent, pos_sep);
            // Lazy delete here. Forget about leaf block_id.
            leaf->_blockType = INVALID_BLOCK;
//            delete leaf;
        }
        else{ // Redistribution between two leaves
            if (is_predecessor){ // leaf is the predecessor of sibling
                leaf->_k_rowid_pair[leaf->_size] = sibling->_k_rowid_pair[0];
                leaf->_size += 1;
                sibling->_size -= 1;
                memmove(sibling->_k_rowid_pair, sibling->_k_rowid_pair + 1,
                        sibling->_size * sizeof(sibling->_k_rowid_pair[0]));
                // The separation key
                parent->_k_child_pair[pos_sep].first = sibling->_k_rowid_pair[0].first;
            }
            else{ // leaf is the successor of sibling
                memmove(leaf->_k_rowid_pair + 1, leaf->_k_rowid_pair,
                        leaf->_size * sizeof(sibling->_k_rowid_pair[0]));
                leaf->_k_rowid_pair[0] = sibling->_k_rowid_pair[sibling->_size - 1];
                leaf->_size += 1;
                sibling->_size -= 1;

                parent->_k_child_pair[pos_sep].first = leaf->_k_rowid_pair[0].first;
            }

            _bfm->unpinPage(sibling_pageId);
            _bfm->unpinPage(leaf_pageId);
        }
        // TODO : Check if it is necessary to optimize the pin and unpin
        _bfm->unpinPage(parent_pageId);

        return true;
    }
}

//KEY_VALUE_T_DECLARE
//std::pair<key_t, value_t>& BP_TREE_T::getKeyRowidPair(const key_t &key) {
//
//    FindNode(key, )
//}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::UpdateKey(const key_t &former_key, const key_t &new_key) {
    pageId_t key_pageId;
    BP_TREE_LEAF_T* n = FindNode(former_key, key_pageId);
    db_size_t pos = n->leaf_biSearch(former_key);
    if (n->_k_rowid_pair[pos].first != former_key) throw DB_KEY_NOT_FOUND;
    value_t val = n->_k_rowid_pair[pos].second;
    _bfm->unpinPage(key_pageId);

    bool success_flg;
    // TODO : Make it more efficient. Redundant search
    success_flg = Delete(former_key);
    if (success_flg){
        success_flg = Insert(new_key, val);
    }
    return success_flg;
}

KEY_VALUE_T_DECLARE
bool BP_TREE_T::UpdateValue(const key_t &key, const value_t &new_val) {
    pageId_t key_pageId;
    BP_TREE_LEAF_T* n = FindNode(key, key_pageId);
    db_size_t pos = n->leaf_biSearch(key);
    if (n->_k_rowid_pair[pos].first != key) throw DB_KEY_NOT_FOUND;
    n->_k_rowid_pair[pos].second = new_val;
    _bfm->unpinPage(key_pageId);
    return true;
}

//KEY_VALUE_T_DECLARE
//bool BP_TREE_T::FindRange(key_t lower_key, key_t upper_key, std::vector<value_t>& result) {
//    const BP_NODE_T* lower_node = FindNode(lower_key);
//    const BP_NODE_T* upper_node = FindNode(upper_key);
//
//    db_size_t start_id = lower_node->FindPos_at_Node(lower_key);
//    db_size_t end_id = upper_node->FindPos_at_Node(upper_key);
//
//    if (start_id == -1 || end_id == -1){
//        return false;
//    }
//    else{
//        if (lower_node == upper_node){
//            for (db_size_t i = start_id; i <= end_id; ++i){
//                result.push_back(lower_node->value_field.at(i));
//            }
//        }
//        else{
//            for (db_size_t i = start_id; i < lower_node->value_field.size(); ++i){
//                result.push_back(lower_node->value_field.at(i));
//            }
//            const BP_NODE_T* next = lower_node->ptr_field.front();
//            while(next != upper_node){
//                for (auto value : next->value_field){
//                    result.push_back(value);
//                }
//                next = next->ptr_field.front();
//            }
//            for (db_size_t i = 0; i <= end_id; ++i) {
//                result.push_back(upper_node->value_field.at(i));
//            }
//        }
//        return true;
//    }
//}

#endif