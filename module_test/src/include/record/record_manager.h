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
#include "share/data_t.h"
#include "share/config.h"
#include "index/Index.h"
#include "catalog/catalog.h"
#include "share/err_type.h"
#include "buffer/buffer_manager.h"

//extern BufferManager buffer_manager;
/**
 * @brief RecordManager类
 *
 */
struct record{
    int tuple_num;
    Tuple tuple[1000];
    int record_length;
};

template <typename T>
bool judge(T a , T b , Where relation) {
    switch(relation.relation_operator) {
        case Operator::LT:{
            if (a < b)
                return true;
            else
                return false;
        };break;
        case Operator::LE:{
            if (a <= b)
                return true;
            else
                return false;
        };break;
        case Operator::EQ:{
            if (a == b)
                return true;
            else
                return false;
        };break;
        case Operator::GE:{
            if (a >= b)
                return true;
            else
                return false;
        };break;
        case Operator::GT:{
            if (a > b)
                return true;
            else
                return false;
        };break;
        case Operator::NE:{
            if (a != b)
                return true;
            else
                return false;
        };break;
    }
}

class RecordManager {
public:
    RecordManager(BufferManager *bfm):_bfm(bfm){}
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
    std::vector<Tuple> SelectRecord(std::string table_name);
    std::vector<Tuple> SelectRecord(std::string table_name , std::vector<std::string> target_attr);
    /**
     * @brief 返回包含所有目标属性满足Where条件的记录的表
     *
     * @param table_name 表的名字
     * @param target_attr 投影属性
     * @param where where条件，内含属性
     * @param result_table_name 返回的表的名字
     * @return Table 返回表
     */
    std::vector<Tuple> SelectRecord(std::string table_name , Where where);
    std::vector<Tuple> SelectRecord(std::string table_name , std::vector<std::string> target_attr , Where where);
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
    int getBlockNum(std::string &table_name);
    //insertRecord的辅助函数
    int getTupleLength(Tuple tuple);
    //判断插入的记录是否和其他记录冲突
    bool isConflict(std::vector<Tuple>& tuples , std::vector<Data>& v , int index);
    //带索引查找
    void searchWithIndex(std::string &table_name , std::string &target_attr , Where where , std::vector<Index_t>& block_ids);
    //在块中进行条件删除
    int conditionDeleteInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where);
    //在块中进行条件查询
    void conditionSelectInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where , std::vector<Tuple>& v);

    BufferManager *_bfm;
};

#endif