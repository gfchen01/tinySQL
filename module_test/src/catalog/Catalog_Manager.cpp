#include <iostream>
#include "catalog/catalog.h"


CatalogManager::CatalogManager(){
    //Create a file that contains all table information
    pageId_t all_table_pId;
    if (!_bfm->isFileExists(ALL_TABLE_PATH)){
        _bfm->createEmptyFile(ALL_TABLE_PATH);
    }
    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0, all_table_pId);
    _bfm->modifyPage(all_table_pId);
    std::string str_buff = buffer;
    std::string star = "*";
    std::string::size_type idx;
    idx = str_buff.find(star);
    char* str;
    if(idx == std::string::npos)
        str = "*";
    else
        str = "";
    strcat(buffer,str);
    _bfm->flushPage(all_table_pId, ALL_TABLE_PATH, 0);
    // buffer save

    std::vector<std::string> VecTableName = getTableName();
    for(auto iter = VecTableName.begin(); iter != VecTableName.end(); iter++){
        auto index_cat = getIndex(*iter);
        for (size_t i = 0; i < index_cat.number; ++i){
            indexName2tableName.emplace(index_cat.index_name[i], *iter);
        }
    }
}

CatalogManager::CatalogManager(BufferManager *bfm):_bfm(bfm){
    //創建一個紀錄有多少table的file
    pageId_t p_Id;
    if (!_bfm->isFileExists(ALL_TABLE_PATH)){
        _bfm->createEmptyFile(ALL_TABLE_PATH);
    }
    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0, p_Id);
    _bfm->modifyPage(p_Id);
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

    //map table -> index
    std::vector<std::string> VecTableName = getTableName();
    for(auto iter = VecTableName.begin(); iter != VecTableName.end(); iter++){
        auto index_cat = getIndex(*iter);
        for (size_t i = 0; i < index_cat.number; ++i){
            indexName2tableName.emplace(index_cat.index_name[i], *iter);
        }
    }
}

bool CatalogManager::existTable(std::string table_name){
    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0);
    std::string str_buffer = buffer;

    //獲取table_name in All_TableNames
    int number = 0, a = -1, i = 0;
    std::string table[100];
    while(buffer[i]!='*'){
        if(buffer[i]=='@'){
            table[number++] = str_buffer.substr(a+1, i-a-1);
            a = i;
        }
        i++;
    }
    //沒有任何table_name存在
    if(number == 0){
        return false;
    }
    table[number] = str_buffer.substr(a+1,i-a-1);
    return true;
}

bool CatalogManager::existAttribute(std::string table_name, std::string attr_name){
    int pageID;
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name, 0);
    std::string str_buffer = buffer;
    Attribute attr = getAttribute(table_name);
    for (int i = 0; i<attr.num; i++){
        if(attr_name == attr.name[i])
            return true;
    }

    return false;
}

std::vector<std::string> CatalogManager::getTableName(){
    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0);
    std::string str_buffer = buffer;
    int a = -1, i = 0;
    std::vector<std::string> table;
    while(buffer[i]!='*'){
        if(buffer[i]=='@'){
            if (i == 0) a = 0;
            else{
                table.push_back( str_buffer.substr(a+1, i-a-1));
                a = i;
            }
        }
        i++;
    }
    if ((i - a - 1) > 0){
        table.push_back(str_buffer.substr(a + 1,i-a-1));
    }
    return table;
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
        throw DB_TABLE_ALREADY_EXIST;
    }
//    for (int i=0;i<attr.num;i++){
//        attr.unique[i] = false;
//    }

    std::string str = "";
//    Table table = Table(table_name,attr);
    std::string filename = PATH::CATALOG_PATH + table_name;

    pageId_t buffer_pId;
//    if(_bfm->isFileExists(filename)) throw DB_TABLE_ALREADY_EXIST;
//    else _bfm->createEmptyFile(filename);
    _bfm->createEmptyFile(filename);
    char* buffer = _bfm->getPage(filename, 0, buffer_pId);
    _bfm->modifyPage(buffer_pId);

    //信息："@table_name%attr_num attr_info#primary#index_num index_info\n"
    str = "@" + table_name;
    str = str + "%" + std::to_string(attr.num);
    for (int i=0; i<attr.num; i++){
        str = str + " " + std::to_string((int)attr.type[i]) + " " + attr.name[i] + " " + std::to_string(attr.unique[i]);
    }
    str = str + "#" + std::to_string(attr.primary_Key);
    str = str + "#" + std::to_string(index.number);
    for (int i=0; i<index.number; i++){
        str = str + " " + index.index_name[i] +" "+ std::to_string(index.location[i]);
    }
    str = str + "\n";

    char* tmp = (char*)str.data();
    strcat(buffer,tmp);

    pageId_t allbuff_pId;
    char* all_buff = _bfm->getPage(ALL_TABLE_PATH, 0, allbuff_pId);
    _bfm->modifyPage(allbuff_pId);
    //buffer = _bfm->getPage("All_TableNames", 0);
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
    all_buff[str_buffer.size() - 1] = '\0';
    char* tmp_tb = (char*)table_info.data();
    strcat(all_buff,tmp_tb);
}

void CatalogManager::DropTable(std::string table_name){
    if(existTable(table_name) == false){
        throw;
    }
    // 一開始打開All_Table 找對應的table_name 然後delete
    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0);
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
    if(!existTable(table_name)){
        throw DB_TABLE_NOT_EXIST;
    }
    if(!existAttribute(table_name, attr_name)){
        throw DB_COLUMN_NAME_NOT_EXIST;
    }
    if(index_record.number>=10){
        throw DB_FAILED;
    }
    for(int i=0; i<index_record.number; i++){
        if(attr.name[index_record.location[i]] == attr_name){
            throw DB_INDEX_ALREADY_EXIST;
        }
        if(index_record.index_name[i] == index_name){
            throw DB_INDEX_ALREADY_EXIST;
        }
    }
    //上述無任何的異常后，新增index
    index_record.index_name[index_record.number] = index_name;
    for (int i = 0; i<attr.num; i++){
        if(attr_name == attr.name[i]){
            //新的index在attribute的那个位置
            index_record.location[index_record.number] = i;
            attr.index[i] = true;
        }
    }
    index_record.number++;
    DropTable(table_name);
    CreateTable(table_name,attr);
    /*
     // 用參數table_name 和buffer_Manager要 相應的block (getPage)
     char* buffer = _bfm->getPage(table_name, 0 );
     // 用Table* a = (Table *)buffer 拿取buffer信息
     Table* table = (Table*)buffer;
     // 對a內的index修改, 要用pinPage
     */
}

void CatalogManager::DropIndex(std::string table_name, std::string index_name){
    Index index_record = getIndex(table_name);
    Attribute attr = getAttribute(table_name);
    if(existTable(table_name)==false){
        throw DB_TABLE_NOT_EXIST;
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
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name,0);
    Index index_record;
    std::string table_info = buffer;
    int current = 0;
    for(int i = 0; i<2; i++){
        while(table_info[current]!='#'){
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
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name,0);
    Attribute attr_record;
    std::string attr_info = buffer;
    int current = 0;
    while(attr_info[current] != '%'){
        current++;
    }
//    std::string str = attr_info.substr(current, 1);
    ++current;
    attr_info = attr_info.substr(current);
    attr_record.num = atoi(attr_info.substr(0, 1).c_str());
    attr_info = attr_info.substr(2); // Go over the attr_num field.
    for(int i = 0; i < attr_record.num; i++){
        for(int j = 0; j<2; j++){
            current = 0 ;
            while(attr_info[current] != ' '){
                current++;
            }
            if(j == 0)
                attr_record.type[i] = (BASE_SQL_ValType)atoi(attr_info.substr(0,current).c_str());
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
        throw DB_INDEX_NOT_FOUND;
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
            case BASE_SQL_ValType::INT:
                attr_type = "int";
                break;
            case BASE_SQL_ValType::FLOAT:
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

std::string CatalogManager::getIndexName(std::string table_name, std::string attr_name){
    Index index = getIndex(table_name);
    Attribute attr = getAttribute(table_name);
    int location;
    for(int i = 0; i < attr.num; i++)
    {
        if(attr_name == attr.name[i]){
            location = i;
            break;
        }
    }
    for(int j = 0; j < 10; j++)
    {
        if(location == index.location[j])
        {
            return index.index_name[j];
        }
    }
    throw DB_INDEX_NOT_FOUND;
}