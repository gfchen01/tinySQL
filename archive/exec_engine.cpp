//
// Created by luke on 22-6-3.
//

#include "API/exec_engine.h"

Executor::Executor() {
    bufferManager = new BufferManager();
    catalogManager = new CatalogManager();
    indexManager = new IndexManager(bufferManager);
}
