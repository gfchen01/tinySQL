/**
 * @file record_manager.h
 * @author lmz (3190104724@zju.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-05-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _RECORD_MANAGER_H_
#define _RECORD_MANAGER_H_
#define INF 1000000

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "share/data_t.h"
#include "share/config.h"
#include "index/Index.h"
#include "catalog/catalog.h"
#include "share/err_type.h"
#include "buffer/buffer_manager.h"
#include "share/singleton.h"


/**
 * The page def of a record.
 * Never explicit initialize.
 */

struct record_page{
    int tuple_num;
    db_size_t record_bytes;
    DiskTuple tuples[];

    DiskTuple& tuple_at(int offset){
        if (offset == 0) return tuples[0];
        else{
            if (offset >= tuple_num){
                throw;
            }
            else{
                char* start = (char*)tuples;
                start = start + offset * tuples[0].getBytes();
                return *((DiskTuple*)start);
            }
        }
    }

    ///< Safety purpose
    template<class T>
    DiskTuple& operator[](T offset) = delete;
};

//bool judge(const Data& a , const Data& b , Operator relation);

std::vector<Index_t> Union(const std::vector<Index_t>& a, const std::vector<Index_t>& b);

/**
 * @brief RecordManager类
 *
 */
class RecordManager {
public:
    RecordManager(BufferManager *bfm, CatalogManager *clm, IndexManager *idm): buffer_manager(bfm), catalog_manager(clm), index_manager(idm){}
    /**
     * @brief Create a Table File object
     *
     * @param table_name 表的名字
     */
    void CreateTableFile(const std::string& table_name);
    /**
     * @brief Drop a Table File object
     *
     * @param table_name 表的名字
     */
    void DropTableFile(const std::string& table_name);
    /**
     * @brief 向对应表中插入一条记录
     *
     * @param table_name 表的名字
     * @param tuple 元组
     */
    void InsertRecord(std::string table_name , const MemoryTuple& tuple);
    /**
     * @brief 删除对应表中所有记录（不删除表文件）
     *
     * @param table_name 表的名字
     * @return int 删除的记录数
     */
    int DeleteRecord(std::string table_name);
    /**
     * @brief 删除对应表中所有目标属性值满足Where条件的记录
     *
     * @param table_path 表的名字
     * @param target_attr 目标属性
     * @param where where条件
     * @return int 删除的记录数
     */
    int DeleteRecord(std::string table_path, std::vector<Where> where);

    /**
     * @brief 返回整张表
     *
     * @param table_name 表的名字
     * @param result_table_name 返回的表的名字
     * @return Table 返回表
     */
    std::vector<MemoryTuple> SelectRecord(std::string table_name);
    std::vector<MemoryTuple> SelectRecord(const std::string& table_path , const std::vector<std::string>& target_attr);
    /**
     * @brief 返回包含所有目标属性满足Where条件的记录的表
     *
     * @param table_path 表的名字
     * @param target_attr 投影属性
     * @param where where条件，内含属性
     * @param result_table_name 返回的表的名字
     * @return Table 返回表
     */
    std::vector<MemoryTuple> SelectRecord(std::string table_path , std::vector<Where> where);
    std::vector<MemoryTuple> SelectRecord(std::string table_name , const std::vector<std::string>& target_attr , std::vector<Where> where);
    /**
     * @brief Create a Index on an attribute that has already exists
     *
     * @param index_manager
     * @param table_name 表的名字
     * @param target_attr 目标属性
     * @throw 如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
     */
    void CreateIndex(std::string table_name , const std::string& target_attr);
private:
    //获取文件大小
    static int getBlockNum(std::string &table_name);
    //insertRecord的辅助函数
//    static int getTupleLength(DiskTuple tuples);
    //判断插入的记录是否和其他记录冲突
    bool isConflict(const MemoryTuple &v, const std::string& tableName, int check_index);
    //带索引查找
    void searchWithIndex(std::string &table_name , std::string &target_attr , const Where& where , std::vector<Index_t>& record_ids);

//    void deleteInIndex(std::string &table_path, std::string &target_attr, std::ve)
    //在块中进行条件删除
    int conditionDeleteInBlock(const std::string& table_name , const std::vector<Index_t>& record_id);
    void DeleteInBlock(std::string table_name , int block_id , const Attribute& attr , int index , Where where, std::vector<Index_t>& record_ids);
    //在块中进行条件查询
    void conditionSelectInBlock(std::string table_name , const std::vector<Index_t>& record_id , std::vector<MemoryTuple>& v);
    void SelectInBlock(std::string table_name , int block_id , const Attribute& attr , int index , Where where , std::vector<Index_t>& record_ids);
    BufferManager *buffer_manager;
    CatalogManager *catalog_manager;
    IndexManager *index_manager;
};

#endif