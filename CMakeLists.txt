cmake_minimum_required(VERSION 3.22)
project(MiniSQL)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src/include)
include_directories(src/include/API)
include_directories(src/include/catalog)
include_directories(src/include/index)
include_directories(src/include/share)
include_directories(src/index)
#include_directories(test/Index)

add_executable(MiniSQL
        src/include/API/exec_engine.h
        src/include/API/parser.h
        src/include/catalog/catalog.h
        src/include/index/Index.h
        src/include/share/config.h
        src/include/share/data_t.h
        src/include/share/err_type.h
        src/index/bpTree.h
        src/index/index.cpp
        test/Index/Bp_tree_test.cpp
        test/Index/bpTree.h)
