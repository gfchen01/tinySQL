#ifndef CATALOG_H
#define CATALOG_H

#include "share/data_t.h"
#include <string>
#include "buffer/buffer_manager.h"

#define ALL_TABLE_PATH PATH::CATALOG_PATH+"All_TableNames"

class CatalogManager{
public:
    CatalogManager(BufferManager *bfm);

    CatalogManager();
    /**
     * @brief Create a Table object
     * 
     * @param table_name 表的名字
     * @param attr 屬性
     * @param index 索引
     * @param primary Primary Key的位置
     */
    void CreateTable(std::string table_name, Attribute attr);

    /**
     * @brief Update Index
     *
     * @param table_name 通过table_name來找對應的table
     * @param attr_name  通过attr_name來找對應的attribute
     * @param index_name  通过index_name來防止命名重複
     */
    void UpdateIndex(std::string table_name,std::string attr_name,std::string index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來刪除表格
     */
    void DropTable(std::string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name查找該表
     * @param index_name 通过index_name來刪除index
     */
    void DropIndex(std::string table_name,std::string index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表是否存在
     * @return true 
     * @return false 
     */
    bool existTable(std::string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表內的屬性是否存在 
     * @return true 
     * @return false 
     */
    bool existAttribute(std::string table_name, std::string attr_name);

    /**
     * @brief Get the Attribute object
     * 
     * @param table_name 通过表的名字來獲取屬性
     * @return Attribute 
     */
    Attribute getAttribute(std::string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前的table
     * @param attr_name 通过attr_name來獲取當前的attribute
     * @param index_name 通过index_name來獲取attribute
     * @return std::string 
     */
    std::string Index2Attr(std::string table_name, std::string attr_name,std::string index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前表格的信息 
     */
    void ShowTable(std::string table_name);

    std::string getIndexName(std::string table_name, std::string attr_name);

    std::string indexName2table(std::string &index_name) const{
        auto iter = indexName2tableName.find(index_name);
        if (iter == indexName2tableName.end()) throw DB_INDEX_NOT_FOUND;
        return iter->second;
    }

private:

    std::vector<std::string> getTableName();

    Index getIndex(std::string table_name);

    BufferManager *_bfm;

    std::map<std::string, std::string> indexName2tableName;
};

#endif