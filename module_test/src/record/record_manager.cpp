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
void RecordManager::CreateTableFile(std::string table_name) {
    table_name = PATH::RECORD_PATH + table_name;
    FILE* f = fopen(table_name.c_str() , "w");
    fclose(f);
}

//输入：表名
//输出：void
//功能：删除表文件
//异常：无异常处理（由catalog manager处理）
void RecordManager::DropTableFile(std::string table_name) {
    table_name = PATH::RECORD_PATH + table_name;
    remove(table_name.c_str());
}

//输入：表名，一个元组
//输出：void
//功能：向对应表中插入一条记录
//异常：如果元组类型不匹配，抛出tuple_type_conflict异常。如果
//主键冲突，抛出primary_key_conflict异常。如果unique属性冲突，
//抛出unique_conflict异常。如果表不存在，抛出table_not_exist异常。
void RecordManager::InsertRecord(std::string table_name , Tuple& tuple) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    // TODO : Change to factory mode
    CatalogManager catalog_manager(_bfm);
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    std::vector<Data> v = tuple.getData();
    //检测插入的元组的各个属性是否合法
    for (int i = 0;i < v.size();i++) {
        if (v[i].type != attr.type[i])
            throw DB_TUPLE_TYPE_CONFLICT;
    }
    Table table = SelectRecord(tmp_name);
    std::vector<Tuple>& tuples = table.getTuple();
    //检测是否存在主键冲突
    if (attr.primary_Key >= 0) {
        if (isConflict(tuples , v , attr.primary_Key))
            throw DB_PRIMARY_KEY_CONFLICT;
    }
    //检测是否存在unqiue冲突
    for (int i = 0;i < attr.num;i++) {
        if (attr.unique[i]) {
            if (isConflict(tuples , v , i))
                throw DB_UNIQUE_CONFLICT;
        }
    }

    //异常检测完成


    int block_num = getBlockNum(table_name);
    //处理表文件大小为0的特殊情况
    if (block_num <= 0)
        block_num = 1;
    //获取表的最后一块的句柄
    char* p = _bfm->getPage(table_name , block_num - 1);
    record* r = new record;
    int block_id;
    r = reinterpret_cast<record*> (p);
    if( p[0] == '\0')
    {
        block_id = block_num - 1;
        r->tuple_num = 1;
        r->tuple[r->tuple_num - 1] = tuple;
        r->record_length += 2 * sizeof(int) + getTupleLength(tuple);
        int page_id = _bfm->getPageId(table_name , block_num - 1);
        _bfm->modifyPage(page_id);
    }
    else if(r->record_length + getTupleLength(tuple) <= PAGESIZE){
        block_id = block_num - 1;
        r->tuple_num++;
        r->tuple[r->tuple_num - 1] = tuple;
        r->record_length += getTupleLength(tuple);
        int page_id = _bfm->getPageId(table_name , block_num - 1);
        _bfm->modifyPage(page_id);
    }
    else{
        block_id = block_num;
        p = _bfm->getPage(table_name , block_num);
        r = reinterpret_cast<record*> (p);
        r->tuple_num = 1;
        r->tuple[r->tuple_num - 1] = tuple;
        r->record_length += 2 * sizeof(int) + getTupleLength(tuple);
        int page_id = _bfm->getPageId(table_name , block_num);
        _bfm->modifyPage(page_id);
    }
    p = reinterpret_cast<char*> (r);
    IndexManager index_manager(_bfm);
    for(int i = 0; i < attr.num; i++)
    {
        if(attr.index[i] == true)
        {
            std::string attr_name = attr.name[i];
            std::string index_name = catalog_manager.getIndexName(tmp_name, attr_name);
            std::vector<Data> d = tuple.getData();
            index_manager.InsertId(index_name , d[i] , block_id);
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
    CatalogManager catalog_manager(_bfm);
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    //获取文件所占块的数量
    // int block_num = getFileSize(table_name) / PAGESIZE;
    // 改为
    int block_num = getBlockNum(table_name);
    //表文件大小为0时直接返回
    if (block_num <= 0)
        return 0;
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    IndexManager index_manager(_bfm);
    int count = 0;
    //遍历所有块
    for (int i = 0;i < block_num;i++) {
        char* p = _bfm->getPage(table_name , i);
        record* r = new record;
        r = reinterpret_cast<record*> (p);
        for(int k = 0; k < r->record_length; k++)
        {
            r->tuple[i].setDeleted();
            for(int j = 0; j < attr.num; j++)
            {
                if(attr.index[j] == true){
                    std::string attr_name = attr.name[j];
                    std::string index_name = catalog_manager.getIndexName(tmp_name, attr_name);
                    std::vector<Data> d = r->tuple[k].getData();
                    index_manager.DeleteId(index_name , d[j]);
                }
            }
            count++;
        }

        //将块写回表文件
        int page_id = _bfm->getPageId(table_name , i);
        _bfm->modifyPage(page_id);
    }
    return count;
}

//输入：表名，目标属性，一个Where类型的对象
//输出：int(删除的记录数)
//功能：删除对应表中所有目标属性值满足Where条件的记录
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
//如果Where条件中的两个数据类型不匹配，抛出data_type_conflict异常。
int RecordManager::DeleteRecord(std::string table_name , std::string target_attr , Where where) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    CatalogManager catalog_manager(_bfm);
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    int index = -1;
    bool flag = false;
    //获取目标属性对应的编号
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == target_attr) {
            index = i;
            if (attr.index[i] == true)
                flag = true;
            break;
        }
    }
    //目标属性不存在，抛出异常
    if (index == -1) {
        throw DB_ATTRIBUTE_NOT_EXIST;
    }
        //where条件中的两个数据的类型不匹配，抛出异常
    else if (attr.type[index] != where.data.type) {
        throw DB_TYPE_ERR;
    }

    //异常处理完成

    int count = 0;
    //如果目标属性上有索引
    if (flag == true && where.relation_operator != Operator::NE) {
        std::vector<Index_t> block_ids;
        //通过索引获取满足条件的记录所在的块号
        searchWithIndex(tmp_name , target_attr , where , block_ids);
        for (int i = 0;i < block_ids.size();i++) {
            count += conditionDeleteInBlock(tmp_name , block_ids[i] , attr , index , where);
        }
    }
    else {
        //获取文件所占块的数量
        // int block_num = getFileSize(table_name) / PAGESIZE;
        // 改为
        int block_num = getBlockNum(table_name);
        //文件大小为0，直接返回
        if (block_num <= 0)
            return 0;
        //遍历所有的块
        for (int i = 0;i < block_num;i++) {
            count += conditionDeleteInBlock(tmp_name , i , attr , index , where);
        }
    }
    return count;
}

//输入：表名
//输出：Table类型对象
//功能：返回整张表
//异常：如果表不存在，抛出table_not_exist异常
Table RecordManager::SelectRecord(std::string table_name , std::string result_table_name) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    CatalogManager catalog_manager(_bfm);
    record* r = new record;
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    int block_num = getBlockNum(table_name);
    //处理文件大小为0的特殊情况
    if (block_num <= 0)
        block_num = 1;
    //获取表的属性
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    //构建table类的实例
    Table table(result_table_name , attr);
    std::vector<Tuple>& v = table.getTuple();
    for(int i = 0; i < block_num; i++)
    {
        char* p = _bfm->getPage(table_name , i);

        r = reinterpret_cast<record*> (p);
        for(int j = 0; j < r->tuple_num; j++)
        {
            if(r->tuple[j].isDeleted() == false)
                v.push_back(r->tuple[j]);
        }
    }
    return table;
}

//输入：表名，目标属性，一个Where类型的对象
//输出：Table类型对象
//功能：返回包含所有目标属性满足Where条件的记录的表
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
//如果Where条件中的两个数据类型不匹配，抛出data_type_conflict异常。
Table RecordManager::SelectRecord(std::string table_name , std::string target_attr , Where where , std::string result_table_name) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    CatalogManager catalog_manager(_bfm);
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    int index = -1;
    bool flag = false;
    //获取目标属性的编号
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == where.attr_name) {
            index = i;
            if (attr.index[i] == true)
                flag = true;
            break;
        }
    }
    //目标属性不存在，抛出异常
    if (index == -1) {
        throw DB_ATTRIBUTE_NOT_EXIST;
    }
        //where条件中的两个数据的类型不匹配，抛出异常
    else if (attr.type[index] != where.data.type) {
        throw DB_TYPE_ERR;
    }

    //异常检测完成

    //构建table
    Table table(result_table_name , attr);
    IndexManager index_manager(_bfm);
    std::vector<Tuple>& v = table.getTuple();
    if (flag == true && where.relation_operator != Operator::NE) {
        std::vector<Index_t> block_ids;
        //使用索引获取满足条件的记录所在块号
        searchWithIndex(tmp_name , target_attr , where , block_ids);
        for (int i = 0; i < block_ids.size();i++) {
            conditionSelectInBlock(tmp_name , block_ids[i] , attr , index , where , v);
        }
    }
    else {
        int block_num = getBlockNum(table_name);
        //处理文件大小为0的特殊情况
        if (block_num <= 0)
            block_num = 1;
        //遍历所有块
        for (int i = 0;i < block_num;i++) {
            conditionSelectInBlock(tmp_name , i , attr , index , where , v);
        }
    }
    return table;
}

//输入：表名，目标属性名
//输出：void
//功能：对表中已经存在的记录建立索引
//异常：如果表不存在，抛出table_not_exist异常。如果属性不存在，抛出attribute_not_exist异常。
void RecordManager::CreateIndex(IndexManager& index_manager , std::string table_name , std::string target_attr) {
    std::string tmp_name = table_name;
    table_name = PATH::RECORD_PATH + table_name;
    CatalogManager catalog_manager(_bfm);
    //检测表是否存在
    if (!catalog_manager.existTable(tmp_name)) {
        throw DB_TABLE_NOT_EXIST;
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
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
    std::string index_name = catalog_manager.getIndexName(tmp_name, target_attr);
    //遍历所有块
    record* r = new record;
    for(int i = 0; i < block_num; i++)
    {
        char* p = _bfm->getPage(table_name , i);
        r = reinterpret_cast<record*>(p);
        for(int j = 0; j < r->tuple_num; j++)
        {
            if(r->tuple[j].isDeleted() == false){
                std::vector<Data> v = r->tuple[j].getData();
                index_manager.InsertId(index_name , v[index] , i);
            }
        }
    }
}



//获取文件大小
int RecordManager::getBlockNum(std::string table_name) {
    char* p;
    int block_num = -1;
    do {
        p = _bfm->getPage(table_name , block_num + 1);
        block_num++;
    } while(p[0] != '\0');
    return block_num;
}
int RecordManager::getTupleLength(Tuple tuple)
{
    return tuple.getSize() + sizeof(bool);
}
//判断插入的记录是否和其他记录冲突
bool RecordManager::isConflict(std::vector<Tuple>& tuples , std::vector<Data>& v , int index) {
    for (int i = 0; i < tuples.size(); i++) {
        if (tuples[i].isDeleted() == true)
            continue;
        std::vector<Data> d = tuples[i].getData();
        switch(v[index].type) {
            case BASE_SQL_ValType::INT:{
                if (v[index].data_meta.i_data == d[index].data_meta.i_data)
                    return true;
            };break;
            case BASE_SQL_ValType::FLOAT:{
                if (v[index].data_meta.f_data == d[index].data_meta.f_data)
                    return true;
            };break;
            default:{
                if (v[index].data_meta.s_data == d[index].data_meta.s_data)
                    return true;
            };
        }
    }
    return false;
}

//带索引查找
void RecordManager::searchWithIndex(std::string table_name , std::string target_attr , Where where , std::vector<Index_t>& block_ids) {
    IndexManager index_manager(_bfm);
    CatalogManager catalog_manager(_bfm);
    Data tmp_data;
    Index_t blockID;
    std::string index_name = catalog_manager.getIndexName(table_name, target_attr);
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
        index_manager.FindId(index_name , tmp_data , where.data , block_ids);
        if(where.relation_operator == Operator::LE){
            index_manager.FindId(index_name, where.data, blockID);
            block_ids.push_back(blockID);
        }
    }

    else if (where.relation_operator == Operator::GT|| where.relation_operator == Operator::GE) {
        if (where.data.type == BASE_SQL_ValType::INT) {
            tmp_data.type = BASE_SQL_ValType::INT;
            tmp_data.data_meta.i_data = INF;
        }
        else if (where.data.type == BASE_SQL_ValType::FLOAT) {
            tmp_data.type = BASE_SQL_ValType::FLOAT;
            tmp_data.data_meta.i_data = INF;
        }
        else {
            tmp_data.type = BASE_SQL_ValType::STRING;
            strcpy(tmp_data.data_meta.s_data, "~~~~~~~~");
        }
        index_manager.FindId(index_name , where.data , tmp_data , block_ids);
        if(where.relation_operator == Operator::GE){
            index_manager.FindId(index_name, where.data, blockID);
            block_ids.push_back(blockID);
        }
    }
    else{
        index_manager.FindId(index_name, where.data, blockID);
        block_ids.push_back(blockID);
    }
}

//在块中进行条件删除
int RecordManager::conditionDeleteInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where) {
    //获取当前块的句柄
    table_name = PATH::RECORD_PATH + table_name;//新增
    char* p = _bfm->getPage(table_name , block_id);
    record* r = new record;
    r = reinterpret_cast<record*> (p);
    int count = 0;
    //将当前块写回文件
    for(int i = 0; i < r->tuple_num; i++)
    {
        std::vector<Data> d = r->tuple[i].getData();
        switch(attr.type[index]){
            case BASE_SQL_ValType::INT:{
                if(judge(d[index].data_meta.i_data, where.data.data_meta.i_data, where) == true){
                    r->tuple[i].setDeleted();
                    count++;
                }
            };break;
            case BASE_SQL_ValType::FLOAT:{
                if(judge(d[index].data_meta.f_data, where.data.data_meta.f_data, where) == true){
                    r->tuple[i].setDeleted();
                    count++;
                }
            }break;
            default:{
                if(judge(d[index].data_meta.s_data, where.data.data_meta.s_data, where) == true){
                    r->tuple[i].setDeleted();
                    count++;
                }
            }
        }
    }
    int page_id = _bfm->getPageId(table_name , block_id);
    _bfm->modifyPage(page_id);
    return count;
}

//在块中进行条件查询
void RecordManager::conditionSelectInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where , std::vector<Tuple>& v) {
    //获取当前块的句柄
    table_name = PATH::RECORD_PATH + table_name;//新增
    char* p = _bfm->getPage(table_name , block_id);
    record* r = new record;
    r = reinterpret_cast<record*>(p);
    for(int i = 0; i < r->tuple_num; i++)
    {
        if(r->tuple[i].isDeleted() == false)
        {
            std::vector<Data> d = r->tuple[i].getData();
            switch(attr.type[index]){
                case BASE_SQL_ValType::INT:{
                    if(judge(d[index].data_meta.i_data, where.data.data_meta.i_data, where) == true){
                        v.push_back(r->tuple[i]);
                    }
                };break;
                case BASE_SQL_ValType::FLOAT:{
                    if(judge(d[index].data_meta.f_data, where.data.data_meta.f_data, where) == true){
                        v.push_back(r->tuple[i]);
                    }
                }break;
                default:{
                    if(judge(d[index].data_meta.s_data, where.data.data_meta.s_data, where) == true){
                        v.push_back(r->tuple[i]);
                    }
                }
            }
        }
    }
}
