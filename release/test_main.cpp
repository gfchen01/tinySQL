//
// Created by luke on 22-6-2.
//

#include "share/config.h"
#include "API/interface.h"
#include "fstream"
#include <iostream>
#include <filesystem>

void build_tiny_sql_folders(){
    if (!std::filesystem::is_directory(PATH::DATA_PATH)){
        std::filesystem::create_directory(PATH::DATA_PATH);
    }
    if (!std::filesystem::is_directory(PATH::RECORD_PATH)){
        std::filesystem::create_directory(PATH::RECORD_PATH);
    }
    if (!std::filesystem::is_directory(PATH::INDEX_PATH)){
        std::filesystem::create_directory(PATH::INDEX_PATH);
    }
    if (!std::filesystem::is_directory(PATH::CATALOG_PATH)){
        std::filesystem::create_directory(PATH::CATALOG_PATH);
    }
}

int main()
{
    build_tiny_sql_folders();
    Interface anInterface(std::cin, std::cout);
    anInterface.run();
}