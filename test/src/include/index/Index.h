#ifndef INDEX_H
#define INDEX_H

#include "share/data_t.h"
#include "share/config.h"
#include "share/err_type.h"
#include "catalog/catalog.h"
#include "index/bpTree_disk.h"
#include "buffer/buffer_manager.h"

class IndexManager{
public:
    IndexManager(BufferManager* bfm);

    /**
     * @brief Create an Index of an attr.
     * Actually creates an index file with a head block.
     * File name convention: ${INDEX_PATH}/{table name}_{attribute name}
     *
     * @param indexName the name of index as a string.
     * @throw DB_FILE_NOT_FOUND
     */
    void CreateIndex(const std::string& indexName);

    /**
     * @brief Drop an Index of an attr.
     * 
     * @param tableName 
     * @param attr
     * @throw DB_FILE_NOT_FOUND
     */
    void DropIndex(const std::string& indexName);

    /**
     * @brief Update the pointer of a record in the index file.
     *
     * @param indexName Name of index.
     * @param key Key of the record.
     * @param rec_ptr Pointer of the record.
     * @throw DB_KEY_NOT_FOUND If key doesn't exist
     */
    void UpdateId(const std::string &indexName, const Data &key, const Index_t &new_rec_ptr);

    void UpdateKey(const std::string &indexName, const Data &key, const Data &new_key);

    /**
     * @brief Delete an index of a certain key.
     * 
     * @param tableName the name of table (file) as a string.
     * @param key the key that user wish to delete
     * @throw DB_KEY_NOT_FOUND
     */
    void DeleteId(const std::string& indexName, const Data &key);

    /**
     * @brief Find the Index based on the key (data)
     * 
     * @param tableName 
     * @param key[in] The data to search for, as a key.
     * @param result[out] The container for result
     * @throw DB_KEY_NOT_FOUND
     */
    bool FindId(const std::string& indexName, const Data &key, Index_t &result);

    bool FindId(const std::string& indexName, const Data &lower_key, const Data &upper_key, std::vector<Index_t>& result);

    /**
     * @brief Insert a key into the index table
     * The user should make sure that the file exists. Or he should use TRY!
     *
     * @param tableName
     * @param attr[in] the attribute of the Key that user wish to insert on
     * @param key The key of the data. Here simply insert the value of the data itself.
     * @param rec_ptr record pointer. The protocal index of a sinlge record.
     * @throw DB_KEY_NOT_FOUND
     */
    void InsertId(const std::string& indexName, const Data &Key, const Index_t &rec_ptr);

//private:
    std::string _indexName;
    BufferManager* _bfm;
    Bp_tree<Data, Index_t> _index_tree;
};


#endif