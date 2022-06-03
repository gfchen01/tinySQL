#ifndef DATA_T_H
#define DATA_T_H

#include <string>
#include <map>
#include <vector>
#include <cstring>

#include "share/err_type.h"

enum struct Operator {
    GT,GE,LT,LE,EQ,NE
}; ///< Comparasion

typedef unsigned SQL_ValType;

enum struct BASE_SQL_ValType
        {INT,
         FLOAT,
         STRING}; ///< MiniSQL supported data types.

enum struct S_ATTRIBUTE
        {Null,
         PrimaryKey,
         Unique}; ///< Special attribute types.

;

/**
 * @brief Basic data element in a tuple.
 * 
 * types: int:-1 float:0 string:1-255
 */
struct Data{
    BASE_SQL_ValType type;
    union {
        int i_data;
        float f_data;
        char s_data[20];
    } data_meta;

    friend bool operator<(const Data& d_l, const Data& d_r){
        if (d_l.type != d_r.type) throw DB_OPERANDS_MISMATCH;

        switch (d_l.type) {
            case BASE_SQL_ValType::STRING:{
                if(strcmp(d_l.data_meta.s_data, d_r.data_meta.s_data) < 0){
                    return true;
                }
                else {
                    return false;
                }
            }
            case BASE_SQL_ValType::INT:{
                return d_l.data_meta.i_data < d_r.data_meta.i_data;
            }
            case BASE_SQL_ValType::FLOAT:{
                return d_l.data_meta.f_data < d_r.data_meta.f_data;
            }
            default:{
                throw DB_TYPE_ERR;
            }
        }
    }

    friend bool operator==(const Data& d_l, const Data& d_r){
        if (d_l.type != d_r.type){
            throw DB_OPERANDS_MISMATCH;
        }
        switch (d_l.type) {
            case BASE_SQL_ValType::STRING:{
                if(strcmp(d_l.data_meta.s_data, d_r.data_meta.s_data) == 0){
                    return true;
                }
                else {
                    return false;
                }
            }
            case BASE_SQL_ValType::INT:{
                return d_l.data_meta.i_data == d_r.data_meta.i_data;
            }
            case BASE_SQL_ValType::FLOAT:{
                return d_l.data_meta.f_data == d_r.data_meta.f_data;
            }
            default:{
                throw DB_TYPE_ERR;
            }
        }
    }

    friend bool operator!=(const Data& d_l, const Data& d_r){
        return !(d_l == d_r);
    }
};
struct Where{
    Data data;
    Operator relation_operator;
    std::string attr_name;
};
//used to confirm the attribute, max32
/**
 * @brief The attributes for a schema
 * 
 * Define the attributes of a schema.
 */
struct Attribute{
    int num;         ///< number of property
    int type[32];
    std::string name[32];    ///< property name
    bool unique[32];    ///<  uniqure or not
    bool index[32];     ///< index exist or not
    int primary_Key;    ///< -1 not exist, 1-32 exist and the place where the primary key are
};

// 这个是不是应该交给Index Manager来定义？
/**
 * @brief Index desciption for a table.
 * 
 */
struct Index{
    int number;             ///< number of indexes
    int location[10];     ///< where it is in Attribute
    std::string index_name[10];  ///< index name
};

/**
 * @brief One row in a table
 * 
 * Tuple is a row in a table.
 */
class Tuple{
public:
    std::vector<Data> data;
private:  
};


class Table{
private:
    std::string table_name;
    std::vector<Tuple> tuple;
    Index index;
public:
    Attribute attr;
    Table(){};
    Table(std::string table_name, Attribute attr);
    Table(const Table &index);

// 这个真的必要吗？是不是应该交给API来做？SetIndex应该是一个事务才对。
/***********
 * 
    int setIndex(int index, std::string index_name);  //插入索引，输入要建立索引的Attribute的编号，以及索引的名字，成功返回1失败返回0
    int dropIndex(std::string index_name);  //删除索引，输入建立的索引的名字，成功返回1失败返回0
***********/
    void showTable(std::string table_name);

    
};


#endif
