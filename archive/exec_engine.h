/**
 * @file exec_engine.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef EXEC_ENGINE_H
#define EXEC_ENGINE_H

#include "buffer/buffer_manager.h"
#include "catalog/catalog.h"
#include "index/Index.h"

class Executor{
public:
    Executor();
    void execSelect(std::string tableName, );

private:
    BufferManager *bufferManager;
    CatalogManager *catalogManager;
    IndexManager *indexManager;
};

#endif