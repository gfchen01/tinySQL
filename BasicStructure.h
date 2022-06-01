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
    BASE_SQL_ValType type;       
    int int_data;
    float f_data;
    std::string str_data;
};

//used to confirm the attribute, max32
struct Attribute{
    int num;         // number of property
    int type[32];       // -1:int, 0:float, 1~255 string
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

    void showTable(string table_name);
};


#endif