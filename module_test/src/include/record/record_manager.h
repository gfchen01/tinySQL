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
#define _RECORD_MANAGER_H_ 1
#define INF 1000000

#include <cstdio> 
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "data_t.h"
#include "config.h"
#include "Index.h"
#include "catalog.h"
#include "err_type.h"
//#include "basic.h"
//#include "index_manager.h"
//#include "catalog_manager.h"
#include "buffer_manager.h"
#include "exception.h"
#include "const.h"
//#include "template_function.h"

extern BufferManager buffer_manager;
/**
 * @brief RecordManager类
 * 
 */
class RecordManager {
    public:
        /**
         * @brief Create a Table File object
         * 
         * @param table_name 表的名字
         */
        void CreateTableFile(std::string table_name);   
        /**
         * @brief Drop a Table File object
         * 
         * @param table_name 表的名字
         */
        void DropTableFile(std::string table_name);     
        /**
         * @brief 向对应表中插入一条记录
         * 
         * @param table_name 表的名字
         * @param tuple 元组
         */
        void InsertRecord(std::string table_name , Tuple& tuple); 
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
         * @param table_name 表的名字
         * @param target_attr 目标属性
         * @param where where条件
         * @return int 删除的记录数
         */
        int DeleteRecord(std::string table_name , std::string target_attr , Where where);
        /**
         * @brief 返回整张表
         * 
         * @param table_name 表的名字
         * @param result_table_name 返回的表的名字
         * @return Table 返回表
         */
        Table SelectRecord(std::string table_name , std::string result_table_name = "tmp_table");
        /**
         * @brief 返回包含所有目标属性满足Where条件的记录的表
         * 
         * @param table_name 表的名字
         * @param target_attr 目标属性
         * @param where where条件
         * @param result_table_name 返回的表的名字
         * @return Table 返回表
         */
        Table SelectRecord(std::string table_name , std::string target_attr , Where where , std::string result_table_name = "tmp_table");
        /**
         * @brief Create a Index object
         * 
         * @param index_manager 
         * @param table_name 表的名字
         * @param target_attr 目标属性
         */
        void CreateIndex(IndexManager& index_manager , std::string table_name , std::string target_attr);
    private:
        //获取文件大小
        int getBlockNum(std::string table_name);
        //insertRecord的辅助函数
        void insertRecord1(char* p , int offset , int len , const std::vector<Data>& v);
        //deleteRecord的辅助函数
        char* deleteRecord1(char* p);
        //从内存中读取一个tuple
        Tuple readTuple(const char* p , Attribute attr);
        //获取一个tuple的长度
        int getTupleLength(char* p);
        //判断插入的记录是否和其他记录冲突
        bool isConflict(std::vector<Tuple>& tuples , std::vector<Data>& v , int index);
        //带索引查找
        void searchWithIndex(std::string table_name , std::string target_attr , Where where , std::vector<int>& block_ids);
        //在块中进行条件删除
        int conditionDeleteInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where);
        //在块中进行条件查询
        void conditionSelectInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where , std::vector<Tuple>& v);
};

#endif