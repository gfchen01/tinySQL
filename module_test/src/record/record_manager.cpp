/**
 * @file record_manager.cc
 * @author lmz (3190104724@zju.edu.cn)
 * @brief
 * @version 0.1
 * @date 2022-05-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "record/record_manager.h"

//输入：表名
//输出：void
//功能：建立表文件
//异常：无异常处理（由catalog manager处理）
void RecordManager::CreateTableFile(const std::string& table_name) {
    auto table_path = PATH::RECORD_PATH + table_name;
    buffer_manager->createEmptyFile(table_path);
}

//输入：表名
//输出：void
//功能：删除表文件
//异常：无异常处理（由catalog manager处理）
void RecordManager::DropTableFile(const std::string& table_name) {
    std::string table_path = PATH::RECORD_PATH + table_name;
    buffer_manager->removeFile(table_path);
}

//输入：表名，一个元组
//输出：void
//功能：向对应表中插入一条记录
//异常：如果元组类型不匹配，抛出tuple_type_conflict异常。如果
//主键冲突，抛出primary_key_conflict异常。如果unique属性冲突，
//抛出unique_conflict异常。如果表不存在，抛出table_not_exist异常。
void RecordManager::InsertRecord(std::string table_name , const MemoryTuple& tuple) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    // TODO : Change to factory mode
//    CatalogManager catalog_manager(buffer_manager);
    //检测表是否存在
    if (!catalog_manager->existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager->getAttribute(tmp_name);
    //检测插入的元组的各个属性是否合法
    for (int i = 0; i < tuple.size(); i++) {
        if (tuple[i].type != attr.type[i])
            throw DB_TUPLE_TYPE_CONFLICT;
    }

//    std::vector<MemoryTuple> tuples = SelectRecord(tmp_name);

    //检测是否存在主键冲突
    if (attr.primary_Key >= 0) {
        if (isConflict(tuple, tmp_name, attr.primary_Key))
            throw DB_PRIMARY_KEY_CONFLICT;
    }
    //检测是否存在unique冲突
    for (int i = 0;i < attr.num;i++) {
        if (attr.is_unique[i]) {
            if (isConflict(tuple, tmp_name, i))
                throw DB_UNIQUE_CONFLICT;
        }
    }

    //异常检测完成

    int block_num = getBlockNum(table_name);
//    //处理表文件大小为0的特殊情况
//    if (block_num <= 0)
//        block_num = 1;

    //获取表的最后一块的句柄
    pageId_t page_id;
    char* p = buffer_manager->getPage(table_name , block_num - 1, page_id);
    record_page* r_page;
    int block_id;
    r_page = reinterpret_cast<record_page*> (p);
    if( p[0] == '\0') // Nothing in the page, i.e., empty file.
    {
        block_id = block_num - 1;
//        r_page->tuples[r_page->tuple_num - 1] = tuples;
//        memcpy(r_page->tuples + r_page->tuple_num, tuple, tuple_bytes);
        r_page->tuples[0].serializeFromMemory(tuple);

        r_page->tuple_num = 1;
        r_page->record_bytes = sizeof(record_page) + r_page->tuples[0].getBytes();
        buffer_manager->modifyPage(page_id);
    }
    else if(r_page->record_bytes + r_page->tuples[0].getBytes() <= PAGESIZE){
        block_id = block_num - 1;
//        memcpy(r_page->tuples + r_page->tuple_num, tuple, tuple_bytes);
        r_page->tuples[r_page->tuple_num].serializeFromMemory(tuple);
        r_page->tuple_num += 1;
        r_page->record_bytes += r_page->tuples[0].getBytes();
        buffer_manager->modifyPage(page_id);
    }
    else{ // Create a new page
        block_id = block_num;
        pageId_t new_pId;
        p = buffer_manager->getPage(table_name , block_num, new_pId);
        r_page = reinterpret_cast<record_page*> (p);

        r_page->tuples[0].serializeFromMemory(tuple);

        r_page->tuple_num = 1;
        r_page->record_bytes = sizeof(record_page) + r_page->tuples[0].getBytes();
//        memcpy(r_page->tuples + r_page->tuple_num - 1, tuple, tuple_bytes);
        buffer_manager->modifyPage(page_id);
    }

    for(int i = 0; i < attr.num; i++)
    {
        if(attr.has_index[i])
        {
            std::string attr_name = attr.name[i];
            std::string index_name = catalog_manager->getIndexName(tmp_name, attr_name);
            Data d = tuple.at(i);
            index_manager->InsertId(index_name , d, block_id * 1000 + r_page->tuple_num - 1);
        }
    }
}

// 输入：表名
// 输出：int(删除的记录数)
// 功能：删除对应表中所有记录（不删除表文件）
// 异常：如果表不存在，抛出table_not_exist异常
int RecordManager::DeleteRecord(std::string table_name) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    //获取文件所占块的数量
    // int block_num = getFileSize(table_name) / PAGESIZE;
    // 改为
    int block_num = getBlockNum(table_name);
    //表文件大小为0时直接返回
    if (block_num <= 0)
        return 0;
    Attribute attr = catalog_manager->getAttribute(tmp_name);
    int count = 0;
    //遍历所有块
    for (int i = 0;i < block_num;i++) {
        char* p = buffer_manager->getPage(table_name , i);
        record_page* r;
        r = reinterpret_cast<record_page*> (p);
        for(int k = 0; k < r->record_bytes; k++)
        {
            r->tuples[i].setDeleted();
            for(int j = 0; j < attr.num; j++)
            {
                if(attr.has_index[j]){
                    std::string attr_name = attr.name[j];
                    std::string index_name = catalog_manager->getIndexName(tmp_name, attr_name);
                    std::vector<Data> d = r->tuples[k].getData();
                    index_manager->DeleteId(index_name , d[j]);
                }
            }
            count++;
        }

        //将块写回表文件
        int page_id = buffer_manager->getPageId(table_name , i);
        buffer_manager->modifyPage(page_id);
    }
    return count;
}

//输入：表名，目标属性，一个Where类型的对象
//输出：int(删除的记录数)
//功能：删除对应表中所有目标属性值满足Where条件的记录
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
//如果Where条件中的两个数据类型不匹配，抛出data_type_conflict异常。
int RecordManager::DeleteRecord(std::string table_name , std::vector<Where> where) {
    std::string table_path = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(table_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager->getAttribute(table_name);
    int index = -1;
    bool flag = false;
    std::vector<Index_t> result_record_id;
    //获取目标属性对应的编号
    for(int j = 0; j < where.size(); j++)
    {
        for (int i = 0;i < attr.num;i++) {
            if (attr.name[i] == where[j].attr_name) {
                index = i;
                if (attr.has_index[i])
                    flag = true;
                break;
            }
        }
        //目标属性不存在，抛出异常
        if (index == -1) {
            throw DB_ATTRIBUTE_NOT_EXIST;
        }
            //where条件中的两个数据的类型不匹配，抛出异常
        else if (attr.type[index] != where[j].data.type) {
            throw DB_TYPE_ERR;
        }

        //异常处理完成

        if(j == 0){
            if (flag && where[j].relation_operator != Operator::NE) {
                //通过索引获取满足条件的记录所在的块号
                searchWithIndex(table_name , where[j].attr_name , where[j] , result_record_id);
            }
            else {
                int block_num = getBlockNum(table_path);
                //文件大小为0，直接返回
                if (block_num <= 0)
                    return 0;
                //遍历所有的块
                for (int i = 0;i < block_num;i++) {
                    DeleteInBlock(table_name , i , attr , index , where[j] , result_record_id);
                }
            }
        }
        else{
            std::vector<Index_t> tmp_record_id;
            if (flag && where[j].relation_operator != Operator::NE) {
                //通过索引获取满足条件的记录所在的块号
                searchWithIndex(table_name , where[j].attr_name , where[j] , tmp_record_id);
            }
            else {
                int block_num = getBlockNum(table_path);
                //文件大小为0，直接返回
                if (block_num <= 0)
                    return 0;
                //遍历所有的块
                for (int i = 0;i < block_num;i++) {
                    DeleteInBlock(table_name , i , attr , index , where[j] , tmp_record_id);
                }
            }
            result_record_id = Union(result_record_id, tmp_record_id);
            tmp_record_id.resize(0);
        }
    }
    return conditionDeleteInBlock(table_name, result_record_id);
}


//输入：表名
//输出：Table类型对象
//功能：返回整张表
//异常：如果表不存在，抛出table_not_exist异常
std::vector<MemoryTuple> RecordManager::SelectRecord(std::string table_name) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    int block_num = getBlockNum(table_name);
    //获取表的属性
    std::vector<MemoryTuple> result{};
    for(int i = 0; i < block_num; i++)
    {
        char* p = buffer_manager->getPage(table_name , i);

        record_page* r_page = reinterpret_cast<record_page*> (p);
        for(int j = 0; j < r_page->tuple_num; j++)
        {
            if(!r_page->tuples[j].isDeleted())
                result.emplace_back(r_page->tuples[j].deserializeToMemory());
        }
    }
    return result;
}

//TODO : 检查Attribute的存在性.
std::vector<MemoryTuple> RecordManager::SelectRecord(const std::string& table_name, const std::vector<std::string>& target_attr){
    std::string table_path = PATH::RECORD_PATH + table_path;
    //record* r = new record;
    //检测表是否存在
    if (!catalog_manager->existTable(table_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    int block_num = getBlockNum(table_path);
    //处理文件大小为0的特殊情况
    if (block_num <= 0)
        block_num = 1;
    //获取表的属性
    Attribute attr = catalog_manager->getAttribute(table_name);
    std::vector<int> position;
    for(auto & i : target_attr)
    {
        for(int j = 0; j < attr.num; j++)
        {
            if(attr.name[j] == i)
            {
                position.push_back(j);
            }
        }
    }

    if(position.empty()) throw DB_COLUMN_NAME_NOT_EXIST;

    std::vector<MemoryTuple> result{};
    for(int i = 0; i < block_num; i++)
    {
        char* p = buffer_manager->getPage(table_path , i);

        record_page* r = reinterpret_cast<record_page*> (p);
        for(int j = 0; j < r->tuple_num; j++)
        {
            if(!r->tuples[j].isDeleted())
                result.emplace_back(r->tuples[j].deserializeToMemory(position));
        }
    }

    return result;
}
//输入：表名，目标属性，一个Where类型的对象
//输出：Table类型对象
//功能：返回包含所有目标属性满足Where条件的记录的表
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
//如果Where条件中的两个数据类型不匹配，抛出data_type_conflict异常。
std::vector<MemoryTuple> RecordManager::SelectRecord(std::string table_name , std::vector<Where> where){
    std::string table_path = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(table_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager->getAttribute(table_name);
    int index = -1;
    bool exist_index = false;
    std::vector<Index_t> result_record_id;
    for(int j = 0; j < where.size(); j++)
    {
        for (int i = 0;i < attr.num;i++) {
            if (attr.name[i] == where[j].attr_name) {
                index = i;
                if (attr.has_index[i])
                    exist_index = true;
                break;
            }
        }
        if (index == -1) {
            throw DB_ATTRIBUTE_NOT_EXIST;
        }
            //where条件中的两个数据的类型不匹配，抛出异常
        else if (attr.type[index] != where[j].data.type) {
            throw DB_TYPE_ERR;
        }
        // Return tuples length should equal attribute number of the table.
        MemoryTuple tmp_tuple(attr.num);
        std::vector<MemoryTuple> tmp_tuples;
        std::vector<Index_t> tmp_record_id;
        if(j == 0){
            if (exist_index && where[j].relation_operator != Operator::NE) {
                std::vector<Index_t> record_ids;
                //使用索引获取满足条件的记录所在块号
                searchWithIndex(table_name , where[j].attr_name , where[j] , result_record_id);
            }
            else {
                int block_num = getBlockNum(table_path);
                //处理文件大小为0的特殊情况
                if (block_num <= 0)
                    block_num = 1;
                //遍历所有块
                for (int i = 0;i < block_num;i++) {
                    SelectInBlock(table_name , i , attr , index , where[j] , result_record_id);
                }
            }
        }
        else{
            if (exist_index && where[j].relation_operator != Operator::NE) {
                std::vector<Index_t> record_ids;
                //使用索引获取满足条件的记录所在块号
                searchWithIndex(table_name , where[j].attr_name , where[j] , tmp_record_id);
            }
            else {
                int block_num = getBlockNum(table_path);
                //处理文件大小为0的特殊情况
                if (block_num <= 0)
                    block_num = 1;
                //遍历所有块
                for (int i = 0;i < block_num;i++) {
                    SelectInBlock(table_name , i , attr , index , where[j] , tmp_record_id);
                }
            }
            result_record_id = Union(result_record_id, tmp_record_id);
            tmp_record_id.resize(0);
        }
    }
    std::vector<MemoryTuple> result;
    conditionSelectInBlock(table_name , result_record_id, result);
    return result;
}

//TODO : 加入对target_attr是否存在的判断；或Catalog应直接提供target_attr对应的下标，以防止重复访问
std::vector<MemoryTuple> RecordManager::SelectRecord(std::string table_name , const std::vector<std::string>& target_attr , std::vector<Where> where) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager->getAttribute(tmp_name);
    int index = -1;
    bool flag = false;

    std::vector<Index_t> result_record_id;
    for(int j = 0; j < where.size(); j++)
    {
        for (int i = 0;i < attr.num;i++) {
            if (attr.name[i] == where[j].attr_name) {
                index = i;
                if (attr.has_index[i])
                    flag = true;
                break;
            }
        }
        if (index == -1) {
            throw DB_ATTRIBUTE_NOT_EXIST;
        }
            //where条件中的两个数据的类型不匹配，抛出异常
        else if (attr.type[index] != where[j].data.type) {
            throw DB_TYPE_ERR;
        }
        std::vector<Index_t> tmp_record_id;
        if(j == 0){
            if (flag && where[j].relation_operator != Operator::NE) {
                std::vector<Index_t> record_ids;
                //使用索引获取满足条件的记录所在块号
                searchWithIndex(tmp_name , where[j].attr_name , where[j] , result_record_id);
            }
            else {
                int block_num = getBlockNum(table_name);
                //处理文件大小为0的特殊情况
                if (block_num <= 0)
                    block_num = 1;
                //遍历所有块
                for (int i = 0;i < block_num;i++) {
                    SelectInBlock(tmp_name , i , attr , index , where[j] , result_record_id);
                }
            }
        }
        else{
            if (flag && where[j].relation_operator != Operator::NE) {
                std::vector<Index_t> record_ids;
                //使用索引获取满足条件的记录所在块号
                searchWithIndex(tmp_name , where[j].attr_name , where[j] , tmp_record_id);
            }
            else {
                int block_num = getBlockNum(table_name);
                //处理文件大小为0的特殊情况
                if (block_num <= 0)
                    block_num = 1;
                //遍历所有块
                for (int i = 0;i < block_num;i++) {
                    SelectInBlock(tmp_name , i , attr , index , where[j] , tmp_record_id);
                }
            }
            result_record_id = Union(result_record_id, tmp_record_id);
            tmp_record_id.resize(0);
        }
    }

    std::vector<MemoryTuple> v;
    conditionSelectInBlock(tmp_name , result_record_id, v);
    std::vector<int> position;

    for(auto & i : target_attr)
    {
        for(int j = 0; j < attr.num; j++)
        {
            if(attr.name[j] == i)
            {
                position.push_back(j);
            }
        }
    }
    std::vector<MemoryTuple> result;
//    std::vector<Data> tmp{};
    for(auto & i : v)
    {
        MemoryTuple tmp_Tuple(target_attr.size());
        for(int j : position)
        {
            tmp_Tuple.emplace_back(i.at(j));
        }
        result.push_back(tmp_Tuple);
    }
    return result;
}
//输入：表名，目标属性名
//输出：void
//功能：对表中已经存在的记录建立索引
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
void RecordManager::CreateIndex(std::string table_name , const std::string& target_attr) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    //检测表是否存在
    if (!catalog_manager->existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager->getAttribute(tmp_name);
    int index = -1;
    //获取目标属性的编号
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == target_attr) {
            index = i;
            break;
        }
    }
    //目标属性不存在，抛出异常
    if (index == -1) {
        throw DB_ATTRIBUTE_NOT_EXIST;
    }
    //异常检测完成

    //获取文件所占的块的数量
    // int block_num = getFileSize(table_name) / PAGESIZE;
    // 改为
    int block_num = getBlockNum(table_name);
    //处理文件大小为0的特殊情况
    if (block_num <= 0)
        block_num = 1;
    //获取表的属性
    std::string index_name = catalog_manager->getIndexName(tmp_name, target_attr);
    //遍历所有块
    record_page* r;
    for(int i = 0; i < block_num; i++)
    {
        char* p = buffer_manager->getPage(table_name , i);
        r = reinterpret_cast<record_page*>(p);
        for(int j = 0; j < r->tuple_num; j++)
        {
            if(!r->tuples[j].isDeleted()){
                std::vector<Data> v = r->tuples[j].getData();
                index_manager->InsertId(index_name , v[index] , i * 1000 + j);
            }
        }
    }
}

//获取文件大小
int RecordManager::getBlockNum(std::string &table_fname) {
    size_t f_size = BufferManager::getFileSize(table_fname);
    int block_num = (int)((f_size + BLOCKSIZE - 1) / BLOCKSIZE) + 1;
    return block_num;
}

//int RecordManager::getTupleLength(DiskTuple tuples)
//{
//    return (int)(sizeof(Data) * tuples.getSize() + sizeof(bool));
//}

//判断插入的记录是否和其他记录冲突
// CGF: 加速 and 防止返回的表过大而内存装不下
bool RecordManager::isConflict(const MemoryTuple & v, const std::string &table_name , int check_index) {
    std::string table_fname = PATH::RECORD_PATH + table_name;
    int block_num = getBlockNum(table_fname);
    //获取表的属性
    for(int i = 0; i < block_num; i++)
    {
        char* p = buffer_manager->getPage(table_fname , i);
        record_page* r_page = reinterpret_cast<record_page*> (p);
        for(int j = 0; j < r_page->tuple_num; j++)
        {
            if(!r_page->tuples[j].isDeleted() && (r_page->tuples[j].cell[check_index] == v.at(check_index))){
                return true;
            }
        }
    }
    return false;
}

void RecordManager::DeleteInBlock(std::string table_name , int block_id , const Attribute& attr , int index , Where where, std::vector<Index_t>& record_ids){
    //获取当前块的句柄
    table_name = PATH::RECORD_PATH + table_name;//新增
    char* p = buffer_manager->getPage(table_name , block_id);
    record_page* r;
    r = reinterpret_cast<record_page*> (p);
    //将当前块写回文件
    for(int i = 0; i < r->tuple_num; i++)
    {
        std::vector<Data> d = r->tuples[i].getData();
        switch(attr.type[index]){
            case BASE_SQL_ValType::INT:{
                if(judge(d[index].data_meta.i_data, where.data.data_meta.i_data, where) == true){
                    record_ids.push_back(block_id * 1000 + i);
                }
            };break;
            case BASE_SQL_ValType::FLOAT:{
                if(judge(d[index].data_meta.f_data, where.data.data_meta.f_data, where) == true){
                    r->tuples[i].setDeleted();
                    record_ids.push_back(block_id * 1000 + i);
                }
            }break;
            default:{
                if(judge(d[index].data_meta.s_data, where.data.data_meta.s_data, where) == true){
                    r->tuples[i].setDeleted();
                    record_ids.push_back(block_id * 1000 + i);
                }
            }
        }
    }
}

void RecordManager::SelectInBlock(std::string table_name , int block_id , const Attribute& attr , int index , Where where , std::vector<Index_t>& record_ids){
    table_name = PATH::RECORD_PATH + table_name;//新增
    char* p = buffer_manager->getPage(table_name , block_id);
    record_page* r;
    r = reinterpret_cast<record_page*>(p);
    for(int i = 0; i < r->tuple_num; i++)
    {
        if(!r->tuples[i].isDeleted())
        {
            std::vector<Data> d = r->tuples[i].getData();
            switch(attr.type[index]){
                case BASE_SQL_ValType::INT:{
                    if(judge(d[index].data_meta.i_data, where.data.data_meta.i_data, where) == true){
                        record_ids.push_back(block_id * 1000 + i);
                    }
                }break;
                case BASE_SQL_ValType::FLOAT:{
                    if(judge(d[index].data_meta.f_data, where.data.data_meta.f_data, where) == true){
                        record_ids.push_back(block_id * 1000 + i);
                    }
                }break;
                default:{
                    if(judge(d[index].data_meta.s_data, where.data.data_meta.s_data, where) == true){
                        record_ids.push_back(block_id * 1000 + i);
                    }
                }
            }
        }
    }
}
void RecordManager::searchWithIndex(std::string &table_name , std::string &target_attr , const Where& where , std::vector<Index_t>& record_ids) {
    Data tmp_data;
    Index_t recordID;
    std::string index_name = catalog_manager->getIndexName(table_name, target_attr);
    if (where.relation_operator == Operator::LT || where.relation_operator == Operator::LE) {
        if (where.data.type == BASE_SQL_ValType::INT) {
            tmp_data.type = BASE_SQL_ValType::INT;
            tmp_data.data_meta.i_data = -INF;
        }
        else if (where.data.type == BASE_SQL_ValType::FLOAT) {
            tmp_data.type = BASE_SQL_ValType::FLOAT;
            tmp_data.data_meta.f_data = -INF;
        }
        else {
            tmp_data.type = BASE_SQL_ValType::STRING;
            strcpy(tmp_data.data_meta.s_data, "");
        }
        index_manager->FindId(index_name , tmp_data , where.data , record_ids);
        if(where.relation_operator == Operator::LE){
            index_manager->FindId(index_name, where.data, recordID);
            record_ids.push_back(recordID);
        }
    }

    else if (where.relation_operator == Operator::GT|| where.relation_operator == Operator::GE) {
        if (where.data.type == BASE_SQL_ValType::INT) {
            tmp_data.type = BASE_SQL_ValType::INT;
            tmp_data.data_meta.i_data = INF;
        }
        else if (where.data.type == BASE_SQL_ValType::FLOAT) {
            tmp_data.type = BASE_SQL_ValType::FLOAT;
            tmp_data.data_meta.f_data = INF;
        }
        else {
            tmp_data.type = BASE_SQL_ValType::STRING;
            strcpy(tmp_data.data_meta.s_data, "~~~~~~~~");
        }
        index_manager->FindId(index_name , where.data , tmp_data , record_ids);
        if(where.relation_operator == Operator::GE){
            index_manager->FindId(index_name, where.data, recordID);
            record_ids.push_back(recordID);
        }
    }
    else{
        index_manager->FindId(index_name, where.data, recordID);
        record_ids.push_back(recordID);
    }
}

//在块中进行条件删除
int RecordManager::conditionDeleteInBlock(std::string table_name , const std::vector<Index_t>& record_id) {
    //获取当前块的句柄
    table_name = PATH::RECORD_PATH + table_name;//新增
    for(unsigned int i : record_id)
    {
        int block_id = i / 1000;
        int tuple_id = i % 1000;
        char* p = buffer_manager->getPage(table_name , block_id);
        record_page* r = reinterpret_cast<record_page*> (p);
        r->tuples[tuple_id].setDeleted();
        int page_id = buffer_manager->getPageId(table_name , block_id);
        buffer_manager->modifyPage(page_id);
    }
    return (int)record_id.size();
}

//在块中进行条件查询
void RecordManager::conditionSelectInBlock(std::string table_name , const std::vector<Index_t>& record_id , std::vector<MemoryTuple>& v)
{
    table_name = PATH::RECORD_PATH + table_name;
    for(unsigned int i : record_id)
    {
        int block_id = i / 1000;
        int tuple_id = i % 1000;
        char* p = buffer_manager->getPage(table_name , block_id);
        record_page* r = reinterpret_cast<record_page*>(p);
        v.emplace_back(r->tuples[tuple_id].deserializeToMemory());
    }
}

std::vector<Index_t> Union(const std::vector<Index_t>& a, const std::vector<Index_t>& b){
    std::vector<Index_t> result;
        for(const unsigned int & i : a)
            {
                for(unsigned int j : b)
                {
                    if(i == j){
                    result.push_back(i);
                    }
            }
        }
    return result;
}