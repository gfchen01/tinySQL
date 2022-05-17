#ifndef INDEX_H
#define INDEX_H

#include "data_t.h"
#include "config.h"
#include "err_type.h"
#include "catalog.h"


class IndexManager{
public:
    IndexManager();

    /**
     * @brief Create a Index object
     * 
     * @param TableName the name of table (file) as a string.
     * @param attr the name of the attrbute that user wish to build index on
     * @return db_err_t the handling result
     */
    db_err_t CreateIndex(std::string TableName, const Attribute& attr);

    /**
     * @brief Delete the index of an attribute.
     * 
     * @param tableName the name of table (file) as a string.
     * @param attr the name of the attrbute that user wish to delete index on
     * @return db_err_t the handling result
     */
    db_err_t DeleteIndex(std::string tableName, const Attribute& attr);

    /**
     * @brief Find the Index based on the key (data)
     * 
     * @param tableName 
     * @param key The data to search for, as a key.
     * @param result[out] The container for result
     * @return db_err_t 
     */
    db_err_t FindId(std::string tableName, Data key, std::vector<Index_t> result);

    /**
     * @brief Insert a key into the index table
     * 
     * @param tableName 
     * @param key The data to search for, as a key.
     * @return db_err_t 
     */
    db_err_t InsertIndex(std::string tableName, Data Key);

private:
    std::string indexName;
};

#endif