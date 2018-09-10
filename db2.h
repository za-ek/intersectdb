/**
 * @brief intersect2_createDb
 * @param dir       Path to storage directory
 * @param db_name   Database name
 * @param dicSize   Element count of the dictionary
 * @return
 */
int intersect2_createDb(char*dir, char*db_name, int16_t dicSize);
/**
 * @brief intersect2_inc    Increment intersection of two IDs
 * @param e1
 * @param e2
 */
void intersect2_inc(int16_t e1, int16_t e2);
/**
 * @brief intersect2_get    Return value of intersection of two elements
 * @param e1
 * @param e2
 * @return
 */
int intersect2_get(int16_t e1, int16_t e2);
