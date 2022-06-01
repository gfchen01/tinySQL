#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace PATH {
    std::string DATA_PATH = "./DB_DATAFILES/";
    std::string INDEX_PATH = "./DB_DATAFILES/INDEX/";
    std::string RECORD_PATH = "./DB_DATAFILES/RECORDS/";
    std::string SCHEMA_PATH = "./DB_DATAFILES/SCHEMAS/";
}

#define Index_t uint32_t
#define pageId_t int32_t
#define blockId_t int32_t

#define INVALID_PAGE_ID -1

#define PAGESIZE 4096 /**< 每一页的大小为4KB */
#define BLOCKSIZE PAGESIZE /**< 文件每一页的大小和PAGESIZE要保持相同 */
#define MAXFRAMESIZE 1000 /**< 最大页数为100 */

#endif