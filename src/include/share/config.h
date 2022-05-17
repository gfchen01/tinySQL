#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace{
    std::string DATA_PATH = "./DB_DATAFILES/";
    std::string INDEX_PATH = "./DB_DATAFILES/INDEX/";
    std::string RECORD_PATH = "./DB_DATAFILES/RECORDS/";
    std::string SCHEMA_PATH = "./DB_DATAFILES/SCHEMAS/";
}

#define Index_t uint32_t

#endif