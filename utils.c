#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"

/**
 * https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 *
 * @brief z_trim
 * @param str
 * @return
 */
char *z_trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}


void z_err (char*err)
{
    printf(err);
}

/**
 * @brief z_dirExist
 * @param path
 * @return 1 - exist, 2 - no, 3 - failed to check
 */
int z_dirExist(char*path)
{
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else if (ENOENT == errno) {
        return 2;
    } else {
        return 3;
    }
}
