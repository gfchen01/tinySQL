/**
 * @authors Junhao Qu, Guofei Chen
 * @email <*> <3190105552@zju.edu.cn>
 *
 * One serious problem :
 * Assume all the schema files and ALL_TABLE_INFO don't exceed one page.
 * This could corrupt when DB grows VERY big (e.g., more than 1e3 tables)
 *
 * File structure:
 * There are two kinds of files: AllTableName file, and table schema file
 * Only one AllTableName file. Stores all table names. Only update when destruct.
 * One table schema file for one table. Stores the schema information.
 */
#ifndef CATALOG_H
#define CATALOG_H

#include "share/data_t.h"
#include <string>
#include "index/Index.h"
#include "buffer/buffer_manager.h"
#include <set>


#define ALL_TABLE_PATH PATH::CATALOG_PATH+"All_TableNames"

class CatalogManager{
public:
    CatalogManager(BufferManager *bfm);

    ~CatalogManager();
    /**
     * @brief Create a Table object
     * 
     * @param table_name 表的名字
     * @param attr 屬性
     * @param index 索引
     * @param primary Primary Key的位置
     */
    void CreateTable(const std::string& table_name, Attribute &attr);

    /**
     * @brief Update Index
     *
     * @param table_name 通过table_name來找對應的table
     * @param attr_name  通过attr_name來找對應的attribute
     * @param index_name  通过index_name來防止命名重複
     */
    void UpdateIndex(const std::string& table_name,const std::string& attr_name,const std::string& index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來刪除表格
     */
    void DropTable(const std::string& table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name查找該表
     * @param index_name 通过index_name來刪除index
     */
    void DropIndex(const std::string& table_name,const std::string& index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表是否存在
     * @return true 
     * @return false 
     */
    bool existTable(const std::string& table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表內的屬性是否存在 
     * @return true 
     * @return false 
     */
    bool existAttribute(const std::string& table_name, const std::string& attr_name);

    /**
     * @brief Get the Attribute object
     * 
     * @param table_name 通过表的名字來獲取屬性
     * @return Attribute 
     */
    Attribute &&getAttribute(const std::string& table_name);

    Index getIndex(const std::string& table_name);

    std::string getIndexName(const std::string& table_name, const std::string& attr_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前的table
     * @param attr_name 通过attr_name來獲取當前的attribute
     * @param index_name 通过index_name來獲取attribute
     * @return std::string 
     */
    std::string Index2Attr(const std::string& table_name, const std::string& attr_name,const std::string& index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前表格的信息 
     */
    void ShowTable(const std::string& table_name);

    std::string indexName2table(std::string &index_name) const{
        auto iter = indexName2tableName.find(index_name);
        if (iter == indexName2tableName.end()) throw DB_INDEX_NOT_FOUND;
        return iter->second;
    }

private:
    std::map<std::string, std::string> indexName2tableName;

    /**
     * @brief Rewrite the attribute and has_index information of a table
     * @param table_name
     * @param attr
     * @param index
     */
    void rewriteAttribute(const std::string &table_name, const Attribute &attr, const Index &index);

private:
    BufferManager *_bfm;
    std::set<std::string> tableNames; ///< The names of all tables.
};

#endif