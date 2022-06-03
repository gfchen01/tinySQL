#include <iostream>
#include "catalog/catalog.h"
#include "buffer/buffer_manager.h"

#define Catalog_Manager_PATH "./catalog_file"

Table::Table(std::string table_name,Attribute attr){
    this->table_name = table_name;
    this->attr = attr;
    this->index.number = 0;
}

bool CatalogManager::existTable(std::string table_name){
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    std::string str_buffer = buffer;
    
    //獲取table_name in All_TableNames
    int number,a,i = 0;
    std::string table[10000];
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

bool CatalogManager::existAttribute(std::string table_name, std::string attr_name){
    BufferManager buffer_manager = BufferManager();
    int pageID;
    char* buffer = buffer_manager.getPage(table_name, 0);
    std::string str_buffer = buffer;
    Attribute attr = getAttribute(table_name);
    for (int i = 0; i<attr.num; i++){
        if(attr_name == attr.name[i])
            return true;
    }
    
    return false;
}

std::string CatalogManager::getTableName(std::string table_name, int start){
    std::string str="";
    int i;
    if (table_name=="") return table_name;
   for (i = 0; table_name[start+i+5]!=' '; i++ ){
       
   }
    str = table_name.substr(start+5,i);
    i = start + 5 + i;
    return table_name;
}

void CatalogManager::CreateTable(std::string table_name, Attribute attr){
    Index index;
    if(attr.primary_Key>=0){
        index.number = 1;
        index.index_name[0] = table_name + "_" + attr.name[attr.primary_Key];
        index.location[0] = attr.primary_Key; 
    }
    else {
        index.number = 0;
    }
    
    if(existTable(table_name) == true){
        std::cout<<"Error the table has already exist!!!"<<std::endl;
        throw;
    }
    for (int i=0;i<attr.num;i++){
        attr.unique[i] = false;    
    }

    std::string str="";
    Table table = Table(table_name,attr);
    std::string filename = table_name;
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(filename, 0);
    
    //信息："@table_name%attr_num attr_info#primary#index_num index_info\n"
    str = "@" + table_name;
    str = str + "%" + std::to_string(attr.num);
    for (int i=0; i<attr.num; i++){
        str = str + " " + std::to_string(attr.type[i]) + " " + attr.name[i] + " " + std::to_string(attr.unique[i]);
    }
    str = str + "#" + std::to_string(attr.primary_Key);
    str = str + "#" + std::to_string(index.number);
    for (int i=0; i<index.number; i++){
        str = str + " " + index.index_name[i] +" "+ std::to_string(index.location[i]);    
    }
    str = str + "\n";

    char* tmp = (char*)str.data();
    strcat(buffer,tmp);
    // buffer save buffer_manager.modifyPage(filename);


    char* all_buff = buffer_manager.getPage("All_TableNames", 0);
        //buffer = buffer_manager.getPage("All_TableNames", 0);
    std::string table_info = "@";
    table_info = table_info + table_name + "*";
    // buffer save

    //找到最后一個的位置，然後在最后一個的位置上添加table_name
    /*int i=0;
    while(buffer[i]!='*'){
        i++;
    }
    i++;*/
    std::string str_buffer = all_buff;
    str_buffer = str_buffer.substr(0,str_buffer.size()-1);
    char* tmp_tb = (char*)table_info.data();
    all_buff  = strcat((char*)str_buffer.data(),tmp_tb);
    
    //buffer_manager.modifyPage(filename);


}

CatalogManager::CatalogManager(){
    BufferManager buffer_manager = BufferManager();
    //創建一個紀錄有多少table的file
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    std::string str_buff = buffer;
    std::string star = "*";
    std::string::size_type idx;
    idx = str_buff.find(star);
    char* str;
    if(idx == std::string::npos)
        str = "*";
    else
        str = "";
    buffer = strcat(buffer,str);    
    // buffer save
}

void CatalogManager::DropTable(std::string table_name){
    BufferManager buffer_manager = BufferManager();
    if(existTable(table_name) == false){
        throw;
    }
    // 一開始打開All_Table 找對應的table_name 然後delete
    char* buffer = buffer_manager.getPage("All_TableNames", 0);
    std::string str_buffer = buffer;
    
    //獲取table_name in All_TableNames
    int number,a,i = 0;
    std::string table[10000];
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

void CatalogManager::UpdateIndex(std::string table_name, std::string attr_name, std::string index_name){
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
        if(attr.name[index_record.location[i]] == attr_name){
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
            index_record.location[index_record.number] = i;
        }
    }
    index_record.number++;
    DropTable(table_name);
    CreateTable(table_name,attr);
   
   /*
    // 用參數table_name 和buffer_Manager要 相應的block (getPage)
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(table_name, 0 );
    // 用Table* a = (Table *)buffer 拿取buffer信息
    Table* table = (Table*)buffer;
    // 對a內的index修改, 要用pinPage
    */
}

void CatalogManager::DropIndex(std::string table_name, std::string index_name){
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
    index_record.location[number] = index_record.location[theLast];

    DropTable(table_name);
    CreateTable(table_name, attr);
}

Index CatalogManager::getIndex(std::string table_name){
    if(existTable(table_name) == false){
        throw;
    }
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(table_name,0);
    Index index_record;
    std::string table_info = buffer;
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
    std::string indexNum;
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
        index_record.location[i] = atoi(table_info.substr(0, current).c_str());
        table_info = table_info.substr(current+1);
    }
    return index_record;
    
}

Attribute CatalogManager::getAttribute(std::string table_name){
    if(existTable(table_name) == false){
        throw;
    }
    BufferManager buffer_manager = BufferManager();
    char* buffer = buffer_manager.getPage(table_name,0);
    Attribute attr_record;
    std::string attr_info = buffer;
    int current = 0;
    while(attr_info[current] != '%'){
        current++;
    }
    std::string str = attr_info.substr(0, current);
    attr_info = attr_info.substr(current+1);
    attr_record.num = atoi(str.c_str());
    for(int i = 0; i<attr_record.num; i++){
        for(int j = 0; j<2; j++){
            current = 0 ;
            while(attr_info[current] != ' '){
                current++;
            }
            if(j == 0)
                attr_record.type[i] = atoi(attr_info.substr(0,current).c_str());
            else
                attr_record.name[i] = attr_info.substr(0,current);
            attr_info = attr_info.substr(current+1);
        }
        current = 0;
        while(attr_info[current]!=' '){
            if(attr_info[current]=='#'){
                break;
            }
            current++;
        }
        if(attr_info.substr(0,current) == "1")
            attr_record.unique[i] = true;
        else   
            attr_record.unique[i] = false;
        
        attr_info = attr_info.substr(current+1);
    }
    
    // primary key info
    current = 0;
    while(attr_info[current]!='#'){
        current++;
    }
    attr_record.primary_Key = atoi(attr_info.substr(0,current).c_str());
    attr_info = attr_info.substr(current+1);

    // index info
    Index index_record = getIndex(table_name);
    for(int i=0;i<32;i++)
        attr_record.index[i]=false;
    for(int i=0; i<index_record.number; i++)
        attr_record.index[index_record.location[i]]=true;


    return attr_record;
    
}

std::string CatalogManager::Index2Attr(std::string table_name, std::string attr_name, std::string index_name){
    if(existTable(table_name) == false){
        throw;
    }
    Index index_record = getIndex(table_name);
    int found = -1;
    for (int i = 0; i<index_record.number; i++){
        if(index_record.index_name[i] == index_name){
            found = i;
            break;
        }
    }
    if(found == -1){
        throw;
    }
    Attribute attr_record = getAttribute(table_name);
    return attr_record.name[index_record.location[found]];

}

void CatalogManager::ShowTable(std::string table_name){
    if(existTable(table_name) == false){
        throw;
    }
    std::cout<<"Table name:"<<table_name<<std::endl;
    Attribute attr_record=getAttribute(table_name);
    Index index_record=getIndex(table_name);

    std::string attr_type;
    std::cout<<"Attribute number:"<<attr_record.num<<std::endl;
    for(int i = 0; i<attr_record.num; i++){
        switch (attr_record.type[i]){
            case -1:
                attr_type = "int";
                break; 
            case 0:
                attr_type = "float";
                break;
            default:
                attr_type = "std::string";
                break;

        }
        std::cout<<"Attr_type"<<attr_type<<std::endl;
        std::cout<<"Attr_name:"<<attr_record.name[i]<<std::endl;
        std::cout<<"Attr_unique"<<attr_record.unique[i]<<std::endl;
        if(i == attr_record.primary_Key){
            std::cout<<"Primary Key"<<std::endl;
        }
    }
    std::cout<<"Index number:"<<index_record.number<<std::endl;
    for (int i = 0; i<index_record.number; i++){
        std::cout<<"Index name:"<<index_record.index_name[i]<<std::endl;
        std::cout<<"Index location"<<index_record.location[i]<<std::endl;
    }
    
}