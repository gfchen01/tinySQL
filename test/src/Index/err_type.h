#ifndef ERR_TYPE_H
#define ERR_TYPE_H

/**
 * @brief The error types for miniSQL.
 * 
 */
enum db_err_t{
    DB_SUCCESS = 0,
    DB_FAILED,
    DB_TABLE_ALREADY_EXIST,
    DB_TABLE_NOT_EXIST,
    DB_INDEX_ALREADY_EXIST,
    DB_INDEX_NOT_FOUND,
    DB_COLUMN_NAME_NOT_EXIST,
    DB_KEY_NOT_FOUND,
    DB_FILE_NOT_FOUND
};

#endif