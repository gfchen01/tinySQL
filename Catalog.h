#ifndef CATALOG_H
#define CATALOG_H

#include "BasicStructure.h"

class CatalogManager{
public:

    CatalogManager();
    /**
     * @brief Create a Table object
     * 
     * @param table_name 表的名字
     * @param attr 屬性
     * @param index 索引
     * @param primary Primary Key的位置
     */
    void CreateTable(string table_name, Attribute attr,Index index, int primary);

   /**
    * @brief Update Index
    * 
    * @param table_name 通过table_name來找對應的table
    * @param attr_name  通过attr_name來找對應的attribute
    * @param index_name  通过index_name來防止命名重複
    */
    void UpdateIndex(string table_name,string attr_name,string index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來刪除表格
     */
    void DropTable(string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name查找該表
     * @param index_name 通过index_name來刪除index
     */
    void DropIndex(string table_name,string index_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表是否存在
     * @return true 
     * @return false 
     */
    bool existTable(string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过表的名字來判斷表內的屬性是否存在 
     * @return true 
     * @return false 
     */
    bool existAttribute(string table_name, string attr_name);

    /**
     * @brief Get the Attribute object
     * 
     * @param table_name 通过表的名字來獲取屬性
     * @return Attribute 
     */
    Attribute getAttribute(string table_name);

    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前的table
     * @param attr_name 通过attr_name來獲取當前的attribute
     * @param index_name 通过index_name來獲取attribute
     * @return string 
     */
    string Index2Attr(string table_name, string attr_name,string index_name);
    
    /**
     * @brief 
     * 
     * @param table_name 通过table_name來獲取當前表格的信息 
     */
    void ShowTable(string table_name);

private:
    
    string getTableName(string name, int start);
    
    int getTableLocate(string table_name, int block_num);

    Index getIndex(string table_name);
    
    int getBlockSize(string table_name);

};

#endif
