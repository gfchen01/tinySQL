#ifndef API_H
#define API_H

#include "BasicStructure.h"
#include <tuple>
#include <vector>
#include <string>

using std::tuple;
using std::vector;
using std::string;



bool CreateTable(string table_name,const vector<tuple<string,SQL_ValType,S_ATTRIBUTE>>);
bool DropTable();
bool CreateIndex();
bool DropIndex();
vector<Tuple> Select_Record(string table_name,vector<Condition> condition);


size_t Insert();
size_t Delete();






#endif