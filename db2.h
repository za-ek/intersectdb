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
int intersect2_createDb(char*db_name, int16_t dicSize);
/**
 * @brief intersect2_inc    Increment intersection of two IDs
 * @param e1
 * @param e2
 */
int intersect2_inc(char*db_name, int e1, int e2);
/**
 * @brief intersect2_get    Return value of intersection of two elements
 * @param e1
 * @param e2
 * @return
 */
int intersect2_get(char*db_name, int e1, int e2);
