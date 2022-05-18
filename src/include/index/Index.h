#ifndef INDEX_H
#define INDEX_H

#include "data_t.h"
#include "config.h"
#include "err_type.h"
#include "catalog.h"

struct IndexHead{
    std::string indexName;
    std::string tableName;
    std::string attrName;
};

class IndexManager{
public:
    IndexManager();

    /**
     * @brief Create an Index of an attr.
     * 
     * @param tableName the name of table (file) as a string.
     * @param attr the name of the attrbute that user wish to build index on
     * @return db_err_t the handling result
     */
    db_err_t CreateIndex(const std::string tableName, const Attribute& attr);

    /**
     * @brief Drop an Index of an attr.
     * 
     * @param tableName 
     * @param attr 
     * @return db_err_t 
     */
    db_err_t DropIndex(const std::string tableName, const Attribute& attr);

    /**
     * @brief Delete an index of a certain key.
     * 
     * @param tableName the name of table (file) as a string.
     * @param key the key that user wish to delete
     * @return db_err_t the handling result
     */
    db_err_t DeleteIndex(const std::string tableName, const Data key);

    /**
     * @brief Find the Index based on the key (data)
     * 
     * @param tableName 
     * @param key The data to search for, as a key.
     * @param result[out] The container for result
     * @return db_err_t 
     */
    db_err_t FindId(const std::string tableName, const Data key, std::vector<Index_t>& result);

    /**
     * @brief Insert a key into the index table
     * 
     * @param tableName 
     * @param key The data to search for, as a key.
     * @return db_err_t 
     */
    db_err_t InsertIndex(const std::string tableName, const Data Key);

private:
    std::string indexName;

};

#endif