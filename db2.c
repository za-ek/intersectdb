#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <common.h>

char storage_path[255];

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

int intersect2_createDb(char*db_name, int dicSize)
{
    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));
    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");

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

/**
 * Increment intersection value
 */
int intersect2_inc(char*db_name, int from, int to)
{
    int DIC_SIZE = 0;
    int BLOCK_SIZE;
    int position;
    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));

    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");

    printf("Path is <%s>\n", path);

    if(from > 0 && to > 0) {
        if(from > to) {
            int16_t tmp = to;
            to = from;
            from = tmp;
        }

        unsigned int head;
        FILE * f = fopen(path, "rb+");
        if(f != NULL) {

            fseek(f, 0, SEEK_SET);
            fread(&head, 4, 1, f);

            DIC_SIZE = head << 8 >> 8;

            BLOCK_SIZE = head >> 24;
            BLOCK_SIZE += 1;

            if(from > 1) {
                fseek(f, 4 + (from-2)*BLOCK_SIZE, SEEK_SET);
                fread(&position, BLOCK_SIZE, 1, f);
                position += to - from - 2;
            } else {
                position = to - from;
            }
            printf("Position is %i\n",  4 + (DIC_SIZE-2)*BLOCK_SIZE + position * BLOCK_SIZE);
            fseek(f, 4 + (DIC_SIZE-2)*BLOCK_SIZE + position * BLOCK_SIZE, SEEK_SET);

            int val = 0;
            fread(&val, BLOCK_SIZE, 1, f);
            fseek(f, -BLOCK_SIZE, SEEK_CUR);

            val++;
            fwrite(&val, BLOCK_SIZE, 1, f);
            fclose(f);

            return val;
        } else {
            printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
            z_err("Couldn't open database file\n");
            return -1;
        }
    } else {
        return -2;
    }
}

int intersect2_get(char*db_name, int el1, int el2)
{
    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));
    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");


    int BLOCK_SIZE, DIC_SIZE;
    unsigned int head;

    if(el1 > 0 && el2 > 0) {
       if(el1 > el2) {
           int16_t tmp = el2;
           el2 = el1;
           el1 = tmp;
       }

       FILE * f = fopen(path, "rb");
       if(f != NULL) {
           fseek(f, 0, SEEK_SET);
           fread(&head, 4, 1, f);

           DIC_SIZE = head << 8 >> 8;

           BLOCK_SIZE = head >> 24;
           BLOCK_SIZE += 1;

           int position;

           if(el1 > 1) {
               fseek(f, 4 + (el1-2)*BLOCK_SIZE, SEEK_SET);
               fread(&position, BLOCK_SIZE, 1, f);
               position += el2 - el1 - 2;
           } else {
               position = el2 - el1;
           }

           fseek(f, 4 + (DIC_SIZE-2)*BLOCK_SIZE + position * BLOCK_SIZE, SEEK_SET);

           int val = 0;
           fread(&val, BLOCK_SIZE, 1, f);
           fclose(f);

           return (int)val;
       } else {
           return -1;
       }
   }

   return -2;
}
