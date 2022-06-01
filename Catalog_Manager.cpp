#include "catalog.h"
#include <iostream>
#include "buffer manager.h"
using namespace std;
#define Catalog_Manager_PATH "./catalog_file"

Table::Table(string table_name,Attribute attr){
    this->table_name = table_name;
    this->attr = attr;
    this->index.number = 0;
}

int CatalogManager::getBlockSize(string table_name){
    
}

bool CatalogManager::existTable(string table_name){
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    string str_buffer = buffer;
    
    //獲取table_name in All_TableNames
    int number,a,i = 0;
    string table[10000];
    while(buffer[i]!='*'){
        if(buffer[i]=='@'){
            table[number] = str_buffer.substr(a+1, i-a-1);
            number++;
            a = i;
        }
        i++;
    }
    //沒有任何table_name存在
    if(number=0){
        return false;
    }
    table[number] = str_buffer.substr(a+1,i-a-1);
    
    
}

bool CatalogManager::existAttribute(string table_name, string attr_name){
    return false;
}

string CatalogManager::getTableName(string table_name, int start){
    string str="";
    int i;
    if (table_name=="") return table_name;
   for (i = 0; table_name[start+i+5]!=' '; i++ ){
       
   }
    str = table_name.substr(start+5,i);
    i = start + 5 + i;
    return table_name;
}

void CatalogManager::CreateTable(string table_name, Attribute attr, Index index, int primary){
    if(existTable(table_name) == true){
        cout<<"Error the table has already exist!!!"<<endl;
        throw;
    }
    for (int i=0;i<attr.num;i++){
        attr.unique[i] = false;    
    }
    attr.unique[primary] = true;
    attr.primary_Key = primary;
    string str="";
    Table table = Table(table_name,attr);
    string filename = table_name;
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(filename, 0);
    
    //信息："@table_name%attr_num attr_info#primary#index_num index_info\n"
    str = "@" + table_name;
    str = str + "%" + to_string(attr.num);
    for (int i=0; i<attr.num; i++){
        str = str + " " + to_string(attr.type[i]) + " " + attr.name[i] + " " + to_string(attr.unique[i]);
    }
    str = str + "#" + to_string(attr.primary_Key);
    str = str + "#" + to_string(index.number);
    for (int i=0; i<index.number; i++){
        str = str + " " + index.index_name[i] +" "+ to_string(index.localation[i]);    
    }
    str = str + "\n";

    char* tmp = (char*)str.data();
    strcat(buffer,tmp);
    // buffer save buffer_manager.modifyPage(filename);


    char* all_buff = buffer_manager.getPage("All_TableNames", 0);
        //buffer = buffer_manager.getPage("All_TableNames", 0);
    string table_info = "@";
    table_info = table_info + table_name + "*";
    // buffer save

    //找到最后一個的位置，然後在最后一個的位置上添加table_name
    /*int i=0;
    while(buffer[i]!='*'){
        i++;
    }
    i++;*/
    string str_buffer = all_buff;
    str_buffer = str_buffer.substr(0,str_buffer.size()-1);
    char* tmp_tb = (char*)table_info.data();
    all_buff  = strcat((char*)str_buffer.data(),tmp_tb);
    
    //buffer save buffer_manager.modifyPage(filename);


}

CatalogManager::CatalogManager(){
    BufferManager buffer_manager = BufferManager();
    //創建一個紀錄有多少table的file
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    string str_buff = buffer;
    string star = "*";
    string::size_type idx;
    idx = str_buff.find(star);
    char* str;
    if(idx == string::npos)
        str = "*";
    else
        str = "";
    buffer = strcat(buffer,str);    
    // buffer save
}

void CatalogManager::DropTable(string table_name){
    BufferManager buffer_manager = BufferManager();
    if(existTable(table_name) == false){
        throw;
    }
    // 一開始打開All_Table 找對應的table_name 然後delete
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    string str_buffer = buffer;
    
    //獲取table_name in All_TableNames
    int number,a,i = 0;
    string table[10000];
    while(buffer[i]!='*'){
        if(buffer[i]=='@'){
            table[number] = str_buffer.substr(a+1, i-a-1);
            number++;
            a = i;
        }
        i++;
    }
    table[number] = str_buffer.substr(a+1,i-a-1);
    
    //在數組中找出table的位置，刪掉
    for(int i = 1; i <= number; i++){
        if(table_name == table[i]){
            int index = str_buffer.find(table_name);
            str_buffer.erase(index-1, table_name.size() + 1);
            break;
        }
    }

    
    // 用buffer_manager來刪除對應的file_name


}

void CatalogManager::UpdateIndex(string table_name, string attr_name, string index_name){
    Index index_record = getIndex(table_name);
    Attribute attr = getAttribute(table_name);
    if(existTable(table_name)==false){
        throw;
    }
    if(existAttribute(table_name,attr_name) == false){
        throw;
    }
    if(index_record.number>=10){
        throw;
    }
    for(int i=0; i<index_record.number; i++){
        if(attr.name[index_record.localation[i]] == attr_name){
            throw;
        }
        if(index_record.index_name[i] == index_name){
            throw;
        }
    }
    //上述無任何的異常后，新增index
    index_record.index_name[index_record.number] = index_name;
    for (int i = 0; i<attr.num; i++){
        if(attr_name == attr.name[i]){
            //新的index在attribute的那个位置
            index_record.localation[index_record.number] = i;
        }
    }
    index_record.number++;
    DropTable(table_name);
    CreateTable(table_name,attr,index_record,attr.primary_Key);
   
   /*
    // 用參數table_name 和buffer_Manager要 相應的block (getPage)
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(table_name, 0 );
    // 用Table* a = (Table *)buffer 拿取buffer信息
    Table* table = (Table*)buffer;
    // 對a內的index修改, 要用pinPage
    */
}

void CatalogManager::DropIndex(string table_name, string index_name){
    Index index_record = getIndex(table_name);
    Attribute attr = getAttribute(table_name);
    if(existTable(table_name)==false){
        throw;
    }
    //index位置
    int number = -1;
    for (int i = 0; i<index_record.number; i++){
        if(index_record.index_name[i] == index_name){
            number = i;
            break;
        }
    }
    //index不存在
    if(number == -1){
        throw;
    }
    //index存在 delete it
    index_record.number = index_record.number - 1;
    int theLast = index_record.number;
    index_record.index_name[number] = index_record.index_name[theLast];
    index_record.localation[number] = index_record.localation[theLast];

    DropTable(table_name);
    CreateTable(table_name, attr, index_record, attr.primary_Key);
}

Index CatalogManager::getIndex(string table_name){
    if(existTable(table_name) == false){
        throw;
    }
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(table_name,0);
    Index index_record;
    string table_info = buffer;
    int current = 0;
    for(int i = 0; i<2; i++){
        while(table_info[i]!='#'){
            current++;
        }
        current++;
    }
    table_info = table_info.substr(current);
    current = 0;
    while(table_info[current]!=' '){
        current++;
    }
    string indexNum;
    indexNum = table_info.substr(0,current);
    index_record.number = atoi(indexNum.c_str());
    table_info = table_info.substr(current+1);
    if(index_record.number>10){
        throw;
    }
    int info_num = 0;
    for(int i = 0; i<index_record.number; i++){
        current = 0;
        while(table_info[current]!=' '){
            current++;
        }
        index_record.index_name[i] = table_info.substr(0,current);
        table_info = table_info.substr(current+1);
        current = 0;
        while(table_info[current]!=' '){
            if(table_info[current]=='\n'){
                break;
            }
            current++;
        }
        index_record.localation[i] = atoi(table_info.substr(0, current).c_str());
        table_info = table_info.substr(current+1);
    }
    return index_record;
    
}

Attribute CatalogManager::getAttribute(string table_name){
    
}