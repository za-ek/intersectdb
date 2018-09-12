#include <stdio.h>
#ifndef DB2_STRUCT_HEADER
#define DB2_STRUCT_HEADER 1
struct db2 {
    unsigned int dic_size;
    unsigned int block_size;
    char*path;
    char*name;
    int8_t opened;
    FILE*fp;
};

#define DB_OPEN_INFO "rb"
#define DB_OPEN_READ "rb"

#endif

/**
 * @brief storage_path
 */
extern char storage_path[255];

/**
 * @brief intersect2_createDb
 * @param dir       Path to storage directory
 * @param db_name   Database name
 * @param dicSize   Element count of the dictionary
 * @return
 */
int intersect2_createDb(char*db_name, unsigned int dicSize);
/**
 * @brief intersect2_inc    Increment intersection of two IDs
 * @param e1
 * @param e2
 */
int intersect2_inc(char*db_name, int e1, int e2, int n);
/**
 * @brief intersect2_get    Return value of intersection of two elements
 * @param e1
 * @param e2
 * @return
 */
int intersect2_get(char*db_name, int e1, int e2);

unsigned int * intersect2_fetch(char*db_name, unsigned int el);

struct db2 getDb(char*db_name, char*mode);

unsigned int intersect2_offset(struct db2 db, int el1, int el2);

void closeDb(struct db2 db);

char*getDbPath(char*db_name);
