/**
 * @brief z_trim        Remove leading and trailing spaces from string
 * @param str
 * @return
 */
char *z_trim(char *str);
/**
 * @brief z_err         Shows error
 * @param err
 */
void z_err(char*err);
/**
 * @brief z_mkdir       Recursively create directory
 * @param path
 * @return
 */
int z_mkdir(char*path);
/**
 * @brief z_dirExist    Check if directory exists (return 1 if exist)
 * @param path
 * @return
 */
int z_dirExist(char*path);
