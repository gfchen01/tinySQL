/**
 * @file buffer_manager.cc
 * @author lmz (3190104724@zju.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-05-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "buffer_manager.h"

/**
 * @brief Construct a new Page:: Page object
 * 
 */
Page::Page() {
    initialize();
}
/**
 * @brief 初始化
 * 
 */
void Page::initialize() {
    file_name_ = "";
    block_id_ = -1;
    pin_count_ = -1;
    dirty_ = false;
    avaliable_ = true;
    timeb t;
    ftime(&t);
    last_access_time = t.time * 1000 + t.millitm;
    for (int i = 0;i < PAGESIZE;i++) 
        buffer_[i] = '\0';
}


inline void Page::setFileName(std::string file_name) {
    file_name_ = file_name;
}

inline std::string Page::getFileName() {
    return file_name_;
}

inline void Page::setBlockId(int block_id) {
    block_id_ = block_id;
}

inline int Page::getBlockId() {
    return block_id_;
}

inline void Page::setPinCount(int pin_count) {
    pin_count_ = pin_count;
}

inline int Page::getPinCount() {
    return pin_count_;
}

inline void Page::setDirty(bool dirty) {
    dirty_ = dirty;
}

inline bool Page::getDirty() {
    return dirty_;
}

inline void Page::setAvaliable(bool avaliable) {
    avaliable_ = avaliable;
}

inline bool Page::getAvaliable() {
    return avaliable_;
}
inline void Page::setTime(){
    timeb t;
    ftime(&t);
    last_access_time = t.time * 1000 + t.millitm;
}
inline long long Page::getTime(){
    return last_access_time;
}
inline char* Page::getBuffer() {
    return buffer_;
}

/**
 * @brief Construct a new Buffer Manager:: Buffer Manager object
 * 
 */
BufferManager::BufferManager() {
    initialize(MAXFRAMESIZE);
}

BufferManager::BufferManager(int frame_size) {
    initialize(frame_size);
}

/**
 * @brief Destroy the Buffer Manager:: Buffer Manager object
 * 
 */
BufferManager::~BufferManager() {
    for (int i = 0;i < frame_size_;i++) {
        std::string file_name;
        int block_id;
        file_name = Frames[i].getFileName();
        block_id = Frames[i].getBlockId();
        flushPage(i , file_name , block_id);
    }
}


void BufferManager::initialize(int frame_size) {
    Frames = new Page[frame_size];//在堆上分配内存
    frame_size_ = frame_size;
    current_position_ = 0;
}

/**
 * @brief 获取一页
 * 
 * @param file_name 文件名
 * @param block_id  块ID
 * @return char* 一个页的句柄
 */

char* BufferManager::getPage(std::string file_name , int block_id){
    int page_id = getPageId(file_name , block_id);
    if (page_id == -1) {
        page_id = getEmptyPageId();
        loadDiskBlock(page_id , file_name , block_id);
    }
    std::cout << "$" << page_id << "$" << Frames[page_id].getTime() << std::endl;
    Frames[page_id].setTime();
    return Frames[page_id].getBuffer();    
}
/**
 * @brief 标记页是否被修改
 * 
 * @param page_id 页ID
 */
void BufferManager::modifyPage(int page_id) {
    Frames[page_id].setDirty(true);
}
/**
 * @brief 钉住一页
 * 
 * @param page_id 页ID
 */
void BufferManager::pinPage(int page_id) {
    int pin_count = Frames[page_id].getPinCount();
    Frames[page_id].setPinCount(pin_count + 1);
}
/**
 * @brief 解钉一次
 * 
 * @param page_id 页ID
 * @return int 该页未被钉住返回-1，否则返回0
 */
int BufferManager::unpinPage(int page_id) {
    int pin_count = Frames[page_id].getPinCount();
    if (pin_count <= 0) 
        return -1;
    else
        Frames[page_id].setPinCount(pin_count - 1);
    return 0;
}

/**
 * @brief 核心函数之一。内存和磁盘交互的接口。
 * 
 * @param page_id 页ID
 * @param file_name 文件名
 * @param block_id 块ID
 * @return int 成功返回0，否则返回-1
 */
int BufferManager::loadDiskBlock(int page_id , std::string file_name , int block_id) {
    // 初始化一个页
    Frames[page_id].initialize();
    // 打开磁盘文件
    FILE* f = fopen(file_name.c_str() , "r");
    // 打开失败返回-1
    if (f == NULL)
        return -1;
    // 将文件指针定位到对应位置
    fseek(f , PAGESIZE * block_id , SEEK_SET);
    // 获取页的句柄
    char* buffer = Frames[page_id].getBuffer();
    // 读取对应磁盘块到内存页
    fread(buffer , PAGESIZE , 1 , f);
    // 关闭文件
    fclose(f);
    // 对新载入的页进行相应设置
    Frames[page_id].setFileName(file_name);
    Frames[page_id].setBlockId(block_id);
    Frames[page_id].setPinCount(1);
    Frames[page_id].setDirty(false);
    //Frames[page_id].setRef(true);
    //Frames[page_id].setTime();
    Frames[page_id].setAvaliable(false);
    return 0;
}

/**
 * @brief 核心函数之一。内存和磁盘交互的接口。
 * 
 * @param page_id 块ID
 * @param file_name 文件名
 * @param block_id 页ID
 * @return int 成功返回0，否则返回-1
 */
int BufferManager::flushPage(int page_id , std::string file_name , int block_id) {
    // 打开文件
    FILE* f = fopen(file_name.c_str() , "r+");
    // 其实这里有写多余，因为打开一个文件读总是能成功。
    if (f == NULL)
        return -1; 
    // 将文件指针定位到对应位置
    fseek(f , PAGESIZE * block_id , SEEK_SET);
    // 获取页的句柄
    char* buffer = Frames[page_id].getBuffer();
    // 将内存页的内容写入磁盘块
    fwrite(buffer , PAGESIZE , 1 , f);
    // 关闭文件
    fclose(f);
    return 0;
}

/**
 * @brief 简单遍历获取页号
 * 
 * @param file_name 文件名
 * @param block_id 块ID
 * @return int 找到返回对应页ID，否则返回-1
 */
int BufferManager::getPageId(std::string file_name , int block_id) {
    for (int i = 0;i < frame_size_;i++) {
        std::string tmp_file_name = Frames[i].getFileName();
        int tmp_block_id = Frames[i].getBlockId();
        if (tmp_file_name == file_name && tmp_block_id == block_id)
            return i;
    }
    return -1;
}

/**
 * @brief 寻找一个闲置的页
 * 
 * @return int 返回闲置页ID
 */
int BufferManager::getEmptyPageId(){
    for (int i = 0;i < frame_size_;i++) {
        if (Frames[i].getAvaliable() == true)
            return i;
    }
    while(1){
        timeb t;
        ftime(&t);
        long long now = t.time * 1000 + t.millitm + 1;
        for (int i = 0; i < frame_size_; i++){
            if(Frames[i].getPinCount() == 0 && Frames[i].getTime() < now){
                current_position_ = i;
                now = Frames[i].getTime();
                //std::cout << "@";
            }
        }
        if (Frames[current_position_].getDirty() == true) {
                std::string file_name = Frames[current_position_].getFileName();
                int block_id = Frames[current_position_].getBlockId();
                flushPage(current_position_ , file_name , block_id);
            }
        // 删除页
        Frames[current_position_].initialize();
        // 返回页号
        return current_position_;
    }
}
