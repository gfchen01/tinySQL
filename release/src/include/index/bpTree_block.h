#ifndef BPTREE_BLOCK_H
#define BPTREE_BLOCK_H

#include "buffer/buffer_manager.h"
#include "share/config.h"

#include <cassert>

#define KEY_VALUE_T_DECLARE template<typename key_t, typename value_t>
#define MAPPING_T std::pair<key_t, value_t>

#define LEAF_HEADER_SIZE 24
#define INTERNAL_HEADER_SIZE 24
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

    bool isLeaf() const{
        return _blockType == LEAF_BLOCK;
    }

    blockType_t _blockType;
    blockId_t _parent_block_id;
    blockId_t _block_id;
    db_size_t _size;
    db_size_t _max_size;
    blockId_t _next_block_id;

};

KEY_VALUE_T_DECLARE
struct bpTree_Leaf : public bpTree_Block{
    /**
     * Init a leaf node.
     * @param myBId block id for this node
     * @param parentId block id for its parent
     * @param next_block_id (right) sibling block id
     */
    void init(blockId_t myBId, blockId_t parentId, blockId_t next_block_id){
        bpTree_Block::init(myBId, parentId);
        bpTree_Block::_blockType = LEAF_BLOCK;
        bpTree_Block::_next_block_id = next_block_id;
        bpTree_Block::_size = 0;
        bpTree_Block::_max_size = MAX_LEAF_SIZE;
    }

    /**
     * The smallest key that is greater or equal to the given key.
     * If the given key is greater than all the key, return the position after the last element.
     *
     * @param key The given key
     * @return
     */
    db_size_t leaf_biSearch(key_t key){
        int left = 0; // Every pair is used.
        if (_size == 0)
            return 0;
        int right = _size - 1;
        int mid;
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
        assert(left >= 0);
        return (db_size_t)left;
    }

    MAPPING_T _k_rowid_pair[MAX_LEAF_SIZE + 1]; ///< One extra space for spanning.
};

KEY_VALUE_T_DECLARE
struct bpTree_Internal : public bpTree_Block{
public:
    /**
     * Init an internal node.
     * @param myBId block id for this node
     * @param parentId block id for its parent. INVALID_BLOCK_ID for no parent.
     */
    void init(blockId_t myBId, blockId_t parentId){
        bpTree_Block::init(myBId, parentId);
        bpTree_Block::_blockType = INTERNAL_BLOCK;

        bpTree_Block::_size = 0;
        bpTree_Block::_max_size = MAX_INTERNAL_SIZE;
    }

    /**
     * Return the position of the greatest key that is smaller or equal to the given key.
     * @param key The given key
     * @return
     */
    db_size_t internal_biSearch(key_t key){
        db_size_t left = 1; // The first _k_child_pair doesn't contain a valid key.
        db_size_t right = _size - 1;
        db_size_t mid;
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

    MAPPING_T _k_child_pair[MAX_INTERNAL_SIZE + 1]; ///< One extra space for spanning.
};



#endif