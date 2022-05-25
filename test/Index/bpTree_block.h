#ifndef BPTREE_BLOCK_H
#define BPTREE_BLOCK_H

#include "buffer_manager.h"
#include "config.h"

#define KEY_VALUE_T_DECLARE template<typename key_t, typename value_t>
#define MAPPING_T std::pair<key_t, value_t>

#define LEAF_HEADER_SIZE 24
#define INTERNAL_HEADER_SIZE 20
#define MAX_LEAF_SIZE ((BLOCK_SIZE - LEAF_HEADER_SIZE) / sizeof(std::pair<key_t, value_t>))
#define MAX_INTERNAL_SIZE ((BLOCK_SIZE - INTERNAL_HEADER_SIZE) / sizeof(std::pair<key_t, value_t))
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
    blockType_t _blockType;
    size_t _size;
    size_t _max_size;
    blockId_t parent_page_id;
    blockId_t page_id;
};

KEY_VALUE_T_DECLARE
struct bpTree_Leaf : public bpTree_Block<key_t, value_t>{
    blockId_t next_page_id;
    MAPPING_T _k_rowid_pair[0];
};

KEY_VALUE_T_DECLARE
struct bpTree_Internal : public bpTree_Block<key_t, value_t>{
public:
    size_t biSearch(key_t key){
        size_t left = 0;
        size_t right = bpTree_Block<key_t, value_t>::_size;
    }
private:
    MAPPING_T _k_child_pair[0];
};



#endif