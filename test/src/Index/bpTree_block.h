#ifndef BPTREE_BLOCK_H
#define BPTREE_BLOCK_H

#include "buffer_manager.h"
#include "config.h"

#define KEY_VALUE_T_DECLARE template<typename key_t, typename value_t>
#define MAPPING_T std::pair<key_t, value_t>

#define LEAF_HEADER_SIZE 24
#define INTERNAL_HEADER_SIZE 20
#define MAX_LEAF_SIZE ((BLOCKSIZE - LEAF_HEADER_SIZE) / sizeof(std::pair<key_t, value_t>) - 1)
#define MAX_INTERNAL_SIZE ((BLOCKSIZE - INTERNAL_HEADER_SIZE) / sizeof(std::pair<key_t, value_t>) - 1)
#define INVALID_BLOCK_ID -1

enum blockType_t{
    INVALID_BLOCK,
    LEAF_BLOCK,
    INTERNAL_BLOCK
};

/**
 * Both internal and leaf blocks are inherited from this.
 *
 * It actually serves as a header part for each B+ tree page and
 * contains information shared by both leaf block and internal block.
 *
 * Header format (size in byte, 20 bytes in total):
 * ----------------------------------------------------------------------------
 * | BlockType (4) | CurrentSize (4) | MaxSize (4) |
 * ----------------------------------------------------------------------------
 * | ParentBlockId (4) | BlockId(4) |
 * ----------------------------------------------------------------------------
 */
KEY_VALUE_T_DECLARE
struct bpTree_Block{
    /**
     * Init a block, neither leaf nor internal node.
     * Usually occurs when a table exists but has no content
     * @param myBId
     * @param parentId
     */
    void init(blockId_t myBId, blockId_t parentId){
        _blockType = INVALID_BLOCK;
        _parent_block_id = parentId;
        _block_id = myBId;
    }

public:
    bool isLeaf() const{
        return _blockType == LEAF_BLOCK;
    }

    blockType_t _blockType;
    blockId_t _parent_block_id;
    blockId_t _block_id;
};

KEY_VALUE_T_DECLARE
struct bpTree_Leaf : public bpTree_Block<key_t, value_t>{
    /**
     * Init a leaf node.
     * @param myBId block id for this node
     * @param parentId block id for its parent
     * @param next_block_id (right) sibling block id
     */
    void init(blockId_t myBId, blockId_t parentId, blockId_t next_block_id){
        bpTree_Block<key_t, value_t>::init(myBId, parentId);
        bpTree_Block<key_t, value_t>::_blockType = LEAF_BLOCK;
        _next_block_id = next_block_id;
        _size = 0;
        _max_size = MAX_LEAF_SIZE;
    }

    /**
     * The smallest key that is greater or equal to the given key.
     * If the given key is greater than all the key, return the position after the last element.
     *
     * @param key The given key
     * @return
     */
    size_t leaf_biSearch(key_t key){
        size_t left = 0; // Every pair is used.
        size_t right = bpTree_Block<key_t, value_t>::_size - 1;
        size_t mid;
        while(left <= right){
            mid = (left + right) / 2;
            if (_k_rowid_pair[mid].first < key){
                left = mid + 1;
            }
            else if (_k_rowid_pair[mid].first == key){
                return mid;
            }
            else{
                right = mid - 1;
            }
        }
        return left;
    }

    size_t _size;
    size_t _max_size;
    blockId_t _next_block_id;
    MAPPING_T _k_rowid_pair[MAX_LEAF_SIZE];
};

KEY_VALUE_T_DECLARE
struct bpTree_Internal : public bpTree_Block<key_t, value_t>{
public:
    /**
     * Init an internal node.
     * @param myBId block id for this node
     * @param parentId block id for its parent. INVALID_BLOCK_ID for no parent.
     */
    void init(blockId_t myBId, blockId_t parentId){
        bpTree_Block<key_t, value_t>::init(myBId, parentId);
        bpTree_Block<key_t, value_t>::_blockType = INTERNAL_BLOCK;

        _size = 0;
        _max_size = MAX_INTERNAL_SIZE;
    }

    /**
     * Return the position of the greatest key that is smaller or equal to the given key.
     * @param key The given key
     * @return
     */
    size_t internal_biSearch(key_t key){
        size_t left = 1; // The first _k_child_pair doesn't contain a valid key.
        size_t right = bpTree_Block<key_t, value_t>::_size - 1;
        size_t mid;
        while(left <= right){
            mid = (left + right + 1) / 2;
            if (_k_child_pair[mid].first < key){
                left = mid + 1;
            }
            else if (_k_child_pair[mid].first == key){
                return mid;
            }
            else{
                right = mid - 1;
            }
        }
        return right;
    }

    size_t _size;
    size_t _max_size;
    MAPPING_T _k_child_pair[MAX_INTERNAL_SIZE];
};



#endif