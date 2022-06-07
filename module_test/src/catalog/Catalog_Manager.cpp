#include <iostream>
#include "catalog/catalog.h"

CatalogManager::CatalogManager(BufferManager *bfm):_bfm(bfm){
    //創建一個紀錄有多少table的file
    pageId_t p_Id;
    if (!_bfm->FileExists(ALL_TABLE_PATH)){
        _bfm->createEmptyFile(ALL_TABLE_PATH);
    }

    char* buffer = _bfm->getPage(ALL_TABLE_PATH, 0, p_Id);
    _bfm->modifyPage(p_Id);
    std::string str_buffer = buffer;
    std::string star = "*";
    std::string::size_type idx;
    idx = str_buffer.find(star);
    if(idx == std::string::npos) // Empty file
    {
        strcat(buffer, "*");
    }

    int a = -1, i = 0;
    while(buffer[i]!='*'){
        if(buffer[i]=='@'){
            if (i == 0) {
                a = i;
                ++i;
                continue;
            }
            tableNames.emplace(str_buffer.substr(a + 1, i - a - 1));
            a = i;
        }
        ++i;
    }
    if (i != 0)
        tableNames.emplace(str_buffer.substr(a + 1, i - a - 1));

    _bfm->flushPage(p_Id);
    // buffer save

    //map table -> has_index
    for(const auto& name : tableNames){
        auto index_cat = getIndex(name);
        for (size_t j = 0; j < index_cat.number; ++j){
            indexName2tableName.emplace(index_cat.index_name[j], name);
        }
    }
}

CatalogManager::~CatalogManager(){
    // Serialize all table info to the file
    std::string allTableInfo;
    for (auto name : tableNames){
        allTableInfo += "@" + name;
    }
    allTableInfo += "*";

    pageId_t alltable_pId;
    char *allTableFile = _bfm->getPage(ALL_TABLE_PATH, 0, alltable_pId);
    _bfm->modifyPage(alltable_pId);

    strcpy(allTableFile, allTableInfo.c_str());
}

bool CatalogManager::existTable(const std::string& table_name){
    auto iter = tableNames.find(table_name);
    return (iter != tableNames.end());
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

void CatalogManager::rewriteAttribute(const std::string &table_name, const Attribute &attr, const Index &index)
{
    std::string str;
    pageId_t buffer_pId;
    // Just overwrite even if the table file exists.
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name, 0, buffer_pId);
    _bfm->modifyPage(buffer_pId);
    str = buffer;

    //信息："@table_name%attr_num attr_info#primary#index_num index_info\n"
    std::string attr_index_info_str;
    attr_index_info_str = std::to_string(attr.num);
    for (int i=0; i<attr.num; i++){
        attr_index_info_str += " " + std::to_string((int)attr.type[i]) + " " + attr.name[i] + " " + std::to_string(attr.is_unique[i]);
    }
    attr_index_info_str += "#" + std::to_string(attr.primary_Key);

    //Index info
    attr_index_info_str += "#" + std::to_string(index.number);
    for (int i=0; i < index.number; i++){
        attr_index_info_str +=" " + index.index_name[i] +" "+ std::to_string(index.location[i]);
    }
    attr_index_info_str += "\n";

    auto attr_index_start = str.find_first_of('%');
    attr_index_start += 1;
    str.replace(attr_index_start, (str.size() - attr_index_start), attr_index_info_str);

    strcpy(buffer,str.c_str());
}

void CatalogManager::CreateTable(std::string table_name, Attribute &attr){
    Index index;
    if(attr.primary_Key >= 0){
        index.number = 1;
        index.index_name[0] = table_name + "_" + attr.name[attr.primary_Key];
        index.location[0] = attr.primary_Key;

        attr.has_index[attr.primary_Key] = true;

        // Create the has_index for primary key
        // TODO : Switch to factory mode
        IndexManager id_manager(_bfm);
        id_manager.CreateIndex(index.index_name[0]);
    }
    else {
        index.number = 0;
    }

    if(existTable(table_name)){
        std::cout<<"Error the table has already exist!!!"<<std::endl;
        throw DB_TABLE_ALREADY_EXIST;
    }
    else{
        tableNames.emplace(table_name);
    }

    std::string str;
    pageId_t buffer_pId;
    // Just overwrite even if the table file exists.
    _bfm->createEmptyFile(PATH::CATALOG_PATH + table_name);
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name, 0, buffer_pId);
    _bfm->modifyPage(buffer_pId);

    //信息："@table_name%attr_num attr_info#primary#index_num index_info\n"
    str = "@" + table_name;
    str = str + "%" + std::to_string(attr.num);
    for (int i=0; i<attr.num; i++){
        str += " " + std::to_string((int)attr.type[i]) + " " + attr.name[i] + " " + std::to_string(attr.is_unique[i]);
    }
    str = str + "#" + std::to_string(attr.primary_Key);
    //Index info
    str = str + "#" + std::to_string(index.number);
    for (int i=0; i < index.number; i++){
        str +=" " + index.index_name[i] +" "+ std::to_string(index.location[i]);
    }
    str += "\n";

    strcpy(buffer,str.c_str());
}

void CatalogManager::DropTable(std::string table_name){
    if(!existTable(table_name)){
        throw;
    }
    tableNames.erase(table_name);
    // Only delete when destruct
}

void CatalogManager::UpdateIndex(const std::string& table_name, const std::string& attr_name, const std::string& index_name){
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
            attr.has_index[i] = true;
        }
    }
    ++index_record.number;
    rewriteAttribute(table_name, attr, index_record);
//    DropTable(table_name);
//    CreateTable(table_name,attr);

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
    if(!existTable(table_name)){
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
        throw DB_INDEX_NOT_FOUND;
    }
    //index存在 delete it
    index_record.number = index_record.number - 1;
    int theLast = index_record.number;
    index_record.index_name[number] = index_record.index_name[theLast];
    index_record.location[number] = index_record.location[theLast];

    rewriteAttribute(table_name, attr, index_record);
//    DropTable(table_name);
//    CreateTable(table_name, attr);
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
    table_info.erase(current + 1);
//    table_info = table_info.substr(current);
    current = 0;
    while(table_info[current]!=' '){
        current++;
    }
    std::string indexNum;
    indexNum = table_info.substr(0,current);
    index_record.number = atoi(indexNum.c_str());
    table_info.erase(current + 1);
//    table_info = table_info.substr(current+1);
    if(index_record.number>10){
        throw;
    }
    for(int i = 0; i<index_record.number; i++){
        current = 0;
        while(table_info[current]!=' '){
            current++;
        }
        index_record.index_name[i] = table_info.substr(0,current);
        table_info.erase(current + 1);
//        table_info = table_info.substr(current+1);
        current = 0;
        while(table_info[current]!=' '){
            if(table_info[current]=='\n'){
                break;
            }
            current++;
        }
        index_record.location[i] = atoi(table_info.substr(0, current).c_str());
        table_info.erase(current + 1);
        table_info = table_info.substr(current+1);
    }
    return index_record;
}

Attribute CatalogManager::getAttribute(const std::string& table_name){
    if(!existTable(table_name)){
        throw;
    }
    pageId_t p_id;
    char* buffer = _bfm->getPage(PATH::CATALOG_PATH + table_name,0, p_id);
    _bfm->pinPage(p_id);

    Attribute attr_record;
    std::string attr_info(buffer);
    int current = 0;
    while(attr_info[current] != '%'){
        current++;
    }
//    std::string str = attr_info.substr(current, 1);
    ++current;
    attr_info.erase(0, current);
//    attr_info = attr_info.substr(current);
    attr_record.num = atoi(attr_info.substr(0, 1).c_str());

    attr_info.erase(0, 2);
//    attr_info = attr_info.substr(2); // Go over the attr_num field.
    for(int i = 0; i < attr_record.num; i++){
        for(int j = 0; j<2; j++){
            current = 0 ;
            while(attr_info[current] != ' '){
                ++current;
            }
            if(j == 0)
                attr_record.type[i] = (BASE_SQL_ValType)atoi(attr_info.substr(0,current).c_str());
            else
                attr_record.name[i] = attr_info.substr(0,current);
            attr_info.erase(0, current+1);
//            auto temp = attr_info.substr(current+1, attr_info.size());
//            attr_info = temp;
        }
        current = 0;
        while(attr_info[current]!=' '){
            if(attr_info[current]=='#'){
                break;
            }
            ++current;
        }
        if(attr_info.substr(0,current) == "1")
            attr_record.is_unique[i] = true;
        else
            attr_record.is_unique[i] = false;

        attr_info.erase(0, current+1);
//        auto temp = attr_info.substr(current+1);
//        attr_info = temp;
    }

    // primary key info
    current = 0;
    while(attr_info[current]!='#'){
        current++;
    }
    attr_record.primary_Key = atoi(attr_info.substr(0,current).c_str());

    // has_index info
    Index index_record = getIndex(table_name);
    for(int i=0; i<index_record.number; i++)
        attr_record.has_index[index_record.location[i]] = true;

    _bfm->unpinPage(p_id);

    return attr_record;
}

std::string CatalogManager::Index2Attr(const std::string& table_name, const std::string& attr_name, const std::string& index_name){
    if(!existTable(table_name)){
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

void CatalogManager::ShowTable(const std::string& table_name){
    if(!existTable(table_name)){
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
        std::cout << "Attr_unique" << attr_record.is_unique[i] << std::endl;
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

std::string CatalogManager::getIndexName(const std::string& table_name, const std::string& attr_name){
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