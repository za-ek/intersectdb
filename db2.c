#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <common.h>

/*
 * Storing intersections it will use a file that
 * represent intersect table. Such as in table
 * we have repeated values (e.g. #3-#1 has intersections
 * on first and third lines) we need only store
 * the half of that table.
 *
 * The first block will store the point of the intersection cell
 *
 */

int intersect2_createDb(char*dir, char*db_name, int dicSize)
{
    char* path = (char*)malloc((strlen(dir)+1+5+strlen(db_name)) * sizeof(char));

    sprintf(path, "%s%s%s", dir, db_name, ".idb2");

    unsigned int elementCount = (dicSize - 1) * dicSize / 2;

    unsigned int head;
    head = 1 << 24;
    head += dicSize;

    int8_t headSize = sizeof(head);
    int8_t BLOCK_SIZE = 2;

    int64_t total_byte_count = headSize + ((dicSize - 2) + elementCount)*BLOCK_SIZE;

    // Truncate() doesn't fill with zeros an existing file of a smaller size
    if(access( path, F_OK ) != -1) {
        remove(path);
    }
    FILE *fp = fopen(path, "ab+");
    fclose(fp);

    truncate(path, total_byte_count);
    if(errno) {
        char *err = "";
        sprintf(err, "There was an error creating DB \"%s\"\n", strerror(errno));
        z_err(err);
    }

    FILE*f = fopen(path, "rb+");
    fseek(f, 0, SEEK_SET);
    fwrite(&head, headSize, 1, f);

    int buffer;
    buffer = 1;

    int i;
    for(i = 1; i < dicSize; i++) {
        buffer += dicSize - i;
        fwrite(&buffer, BLOCK_SIZE, 1, f);
    }

    fclose(f);

    return 0;
}
