/**
 * @file buffer manager.h
 * @author lmz (3190104724@zju.edu.cn)
 * @brief 本部分负责缓冲区的管理，包括缓冲区与磁盘的交互以及页面替换等。（具体参数待修改，接口应该不会改动）
 * @version 0.1
 * @date 2022-05-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_ 1

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include "const.h"

/**
 * @brief Page类。磁盘文件中的每一块对应内存中的一个页（page)
 * 
 */
class Page {
    public:
        Page();
        void initialize();
        void setFileName(std::string file_name);
        std::string getFileName();
        void setBlockId(int block_id);
        int getBlockId();
        void setPinCount(int pin_count);
        int getPinCount();
        void setDirty(bool dirty);
        bool getDirty();
        void setRef(bool ref);
        bool getRef();
        void setAvaliable(bool avaliable);
        bool getAvaliable();
        char* getBuffer();
    private:
        char buffer_[PAGESIZE]; /**< 每一页都是一个大小为PAGESIZE字节的数组 */
        std::string file_name_; /**< 页所对应的文件名 */
        int block_id_; /**< 页在所在文件中的块号(磁盘中通常叫块) */
        int pin_count_; /**< 记录被钉住的次数。被钉住的意思就是不可以被替换 */
        bool dirty_; /**< dirty记录页是否被修改 */
        bool ref_; /**< ref变量用于时钟替换策略 */
        bool avaliable_; /**< avaliable标示页是否可以被使用(即将磁盘块load进该页) */
};

/**
 * @brief BufferManager类。对外提供操作缓冲区的接口。
 * 
 */
class BufferManager {
    public: 
        BufferManager();
        BufferManager(int frame_size);
        ~BufferManager();
        char* getPage(std::string file_name , int block_id); /**< 通过页号得到页的句柄(一个页的头地址) */
        void modifyPage(int page_id); /**< 标记page_id所对应的页已经被修改 */
        void pinPage(int page_id); /**< 钉住一个页 */
        int unpinPage(int page_id); /**< 解除一个页的钉住状态(需要注意的是一个页可能被多次钉住，该函数只能解除一次),如果对应页的pin_count_为0，则返回-1 */
        int flushPage(int page_id , std::string file_name , int block_id); /**< 将对应内存页写入对应文件的对应块。 */
        int getPageId(std::string file_name , int block_id); /**< 获取对应文件的对应块在内存中的页号，没有找到返回-1 */
    private:
        Page* Frames; /**< 缓冲池，实际上就是一个元素为Page的数组，实际内存空间将分配在堆上 */
        int frame_size_; /**< 记录总页数 */
        int current_position_; /**< 时钟替换策略需要用到的变量 */
        void initialize(int frame_size); /**< 实际初始化函数 */
        int getEmptyPageId(); /**< 获取一个闲置的页的页号 */
        int loadDiskBlock(int page_id , std::string file_name , int block_id); /**< 将对应文件的对应块载入对应内存页，对于文件不存在返回-1，否则返回0 */
};

#endif
