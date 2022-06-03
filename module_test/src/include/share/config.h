#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>

namespace PATH {
    const std::string DATA_PATH = "./DB_DATAFILES/";
    const std::string INDEX_PATH = "./DB_DATAFILES/INDEX/";
    const std::string RECORD_PATH = "./DB_DATAFILES/RECORDS/";
    const std::string SCHEMA_PATH = "./DB_DATAFILES/SCHEMAS/";
}

#define Index_t uint32_t
#define pageId_t int32_t
#define blockId_t int32_t
#define db_size_t uint32_t

#define INVALID_PAGE_ID -1

#define PAGESIZE 4096 /**< 每一页的大小为4KB */
#define BLOCKSIZE PAGESIZE /**< 文件每一页的大小和PAGESIZE要保持相同 */
#define MAXFRAMESIZE 10 /**< 最大页数为100 */

#endif