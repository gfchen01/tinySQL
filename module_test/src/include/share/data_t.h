#ifndef DATA_T_H
#define DATA_T_H

#include <string>
#include <map>
#include <vector>
#include <cstring>

#include "share/err_type.h"
#include "hsql/SQLParser.h"

enum struct Operator {
    GT,GE,LT,LE,EQ,NE
}; ///< Comparasion

const std::map<hsql::OperatorType, Operator> op_map
        {{hsql::OperatorType::kOpGreater, Operator::GT},
         {hsql::OperatorType::kOpGreaterEq, Operator::GE},
         {hsql::OperatorType::kOpLess, Operator::LT},
         {hsql::OperatorType::kOpLessEq, Operator::LE},
         {hsql::OperatorType::kOpEquals, Operator::EQ},
         {hsql::OperatorType::kOpNotEquals, Operator::NE}};

enum struct BASE_SQL_ValType
{INT,
    FLOAT,
    STRING}; ///< MiniSQL supported data types.

const std::map<hsql::DataType, BASE_SQL_ValType> type_map
        {{hsql::DataType::INT, BASE_SQL_ValType::INT},
         {hsql::DataType::FLOAT, BASE_SQL_ValType::FLOAT},
         {hsql::DataType::CHAR, BASE_SQL_ValType::STRING},
         {hsql::DataType::DOUBLE, BASE_SQL_ValType::FLOAT},
         {hsql::DataType::VARCHAR, BASE_SQL_ValType::STRING},
         {hsql::DataType::DECIMAL, BASE_SQL_ValType::FLOAT}};

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

    Data(){}

    Data(hsql::Expr* expr){
        switch (expr->type) {
            case hsql::kExprLiteralInt :{
                type = BASE_SQL_ValType::INT;
                data_meta.i_data = expr->ival;
                break;
            }
            case hsql::kExprLiteralFloat :{
                type = BASE_SQL_ValType::FLOAT;
                data_meta.f_data = expr->fval;
                break;
            }
            case hsql::kExprLiteralString :{
                type = BASE_SQL_ValType::STRING;
                strcpy(data_meta.s_data, expr->name);
                break;
            }
            default:{
                throw DB_FAILED;
            }
        }
    }

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

    friend std::ostream& operator<<(std::ostream& os, const Data& data){
        switch(data.type){
            case BASE_SQL_ValType::INT:{
                os << data.data_meta.i_data << ' | ';
                break;
            }
            case BASE_SQL_ValType::FLOAT:{
                os << data.data_meta.f_data << ' | ';
                break;
            }
            case BASE_SQL_ValType::STRING:{
                os << data.data_meta.s_data << ' | ';
                break;
            }
        }
        return os;
    }
};

struct Where{
    Data data;
    Operator relation_operator;
    std::string attr_name; ///< The attribute to perform select on
};
//used to confirm the attribute, max32
/**
 * @brief The attributes for a schema
 *
 * Define the attributes of a schema.
 */
struct Attribute{
    int num;         ///< number of property
    BASE_SQL_ValType type[32];
    std::string name[32];    ///< property name
    bool unique[32] = {false};    ///<  uniqure or not
    bool index[32] = {false};     ///< index exist or not
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
    bool isDeleted_;
public:
    Tuple() : isDeleted_(false) {};
    Tuple(const Tuple &tuple);  //拷贝元组
    void addData(Data data);  //新增元组
    std::vector<Data> &getData();  //返回数据
    int getSize(){
        return (int)data.size();
    }  //返回元组的数据数量
    bool isDeleted();
    void setDeleted();
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
    Table(const Table &table);
    std::string getTableName();
    Attribute getAttr();
    std::vector<Tuple>& getTuple();
    Index getIndex();
    void showTable(std::string table_name);
    size_t getTupleBytes(){
        return attr.num * sizeof(Data);
    }
};

#endif
