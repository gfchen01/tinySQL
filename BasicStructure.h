#ifndef BASIC_STRUCT
#define BASIC_STRUCT

#include <string>
#include <map>
#include <vector>
using namespace std;


enum struct Operator {
    GT,GE,LT,LE,EQ,NE
};
typedef unsigned SQL_ValType;
enum struct BASE_SQL_ValType {INT,FLOAT,STRING};
enum struct S_ATTRIBUTE {Null,PrimaryKey,Unique};

// use to store information
// type int:-1 float:0 stirng:1-255
struct Data{
    int type;       
    int int_data;
    float f_data;
    string str_data;
};

//used to confirm the attribute, max32
struct Attribute{
    int num;         // number of property
    string name[32];    // property name
    bool unique[32];    //  uniqure or not
    bool index[32];     // index exist or not
    int primary_Key;    //-1 not exist, 1-32 exist and the place where the primary key are
};

// Index Manager, at most 10
struct Index{
    int number;             //number of index
    int localation[10];     // where it is in Attribute
    string index_name[10];  // index name
};

class Tuple{
public:
    vector<Data> data;
private:  
};


class Table{
private:
    string table_name;
    vector<Tuple> tuple;
    Index index;
public:
    Attribute attr;
    Table(){};
    Table(string table_name,Attribute attr);
    Table(const Table &index);

    int setIndex(int index,string index_name);  //插入索引，输入要建立索引的Attribute的编号，以及索引的名字，成功返回1失败返回0
    int dropIndex(string index_name);  //删除索引，输入建立的索引的名字，成功返回1失败返回0

    void showTable(string table_name);
    
    
};


#endif