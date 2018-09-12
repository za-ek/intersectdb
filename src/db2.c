#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
#include "db2.h"

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

int intersect2_createDb(char*db_name, unsigned int dicSize)
{
    if(dicSize > 65535) {
        z_err("Couldn't add more than 65535 elements");
        return -1;
    }

    char* path = getDbPath(db_name);
    unsigned int elementCount = (dicSize - 1) * dicSize / 2;

    unsigned int head;
    head = 1 << 24;
    head += dicSize;

    unsigned int headSize = sizeof(head);
    unsigned int blockSize = 2;

    long unsigned int total_byte_count = (int64_t) (headSize + ((dicSize - 2) + elementCount)*blockSize);

    // Truncate() doesn't fill with zeros an existing file of a smaller size
    if(access( path, F_OK ) != -1) {
        remove(path);
    }

    FILE *fp = fopen(path, "w");
    if(fp == NULL) {
        return -1;
    }
    fclose(fp);
    if(errno) {
        errno = 0;
    }

    truncate(path, total_byte_count);
    if(errno) {
        printf("%s\n", strerror(errno));
        remove(path);
        return -1;
    }

    FILE*f = fopen(path, "rb+");
    if(f != NULL) {
        fseek(f, 0, SEEK_SET);
        fwrite(&head, headSize, 1, f);

        unsigned int buffer;
        // First offset starts on that byte
        buffer = 4 + (dicSize - 2) * blockSize;

        unsigned int i;
        for(i = 1; i < dicSize - 1; i++) {
            printf("Value is %i\n", buffer);
            buffer += (dicSize - i) * blockSize;
            fwrite(&buffer, blockSize, 1, f);
        }

        fclose(f);
    } else {
        printf("Open error\n");
        printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        z_err("Couldn't open database file\n");
        return -1;
    }

    return 0;
}

/**
 * Increment intersection value
 */
int intersect2_inc(char*db_name, int el1, int el2, int n)
{
    unsigned int position;
    unsigned int val = 0;

    struct db2 db;

    if(el1 > 0 && el2 > 0) {

        db = getDb(db_name, "rb+");

        if(db.opened) {
            position = intersect2_offset(db, el1, el2);
            fseek(db.fp, position, SEEK_SET);

            fread(&val, db.block_size, 1, db.fp);

            val = val + n;

            fseek(db.fp, position, SEEK_SET);
            fwrite(&val, 2, 1, db.fp);

            closeDb(db);

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

/**
 * @brief intersect2_get
 * @param db_name
 * @param el1
 * @param el2
 * @return
 */
int intersect2_get(char*db_name, int el1, int el2)
{
    struct db2 db;

    if(el1 > 0 && el2 > 0) {
       db = getDb(db_name, "rb");

       if(db.fp != NULL) {
           int val = 0;
           int position = intersect2_offset(db, el1, el2);

           fseek(db.fp, position, SEEK_SET);
           fread(&val, db.block_size, 1, db.fp);

           closeDb(db);

           return (int)val;
       } else {
           return -1;
       }
   }

   return -2;
}

void dumpDb(struct db2 db)
{
    printf("****\n");
    printf("* Database path <%s>\n", db.path);
    printf("* Database has <%u> elements\n", db.dic_size);
    printf("* Database block defined as <%u> bytes\n", db.block_size);
    printf("****\n");
}

char * getDbPath(char*db_name)
{
    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));
    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");
    return path;
}

/**
 * @brief getDb
 * @param db_name
 * @param mode
 * @return
 */
struct db2 getDb(char*db_name, char*mode)
{
    unsigned int head;
    struct db2 db;

    db.name = db_name;

    char* path = getDbPath(db_name);

    db.path = path;

    FILE * f = fopen(path, mode);
    if(f != NULL) {
        db.opened = 1;
        fseek(f, 0, SEEK_SET);
        fread(&head, 4, 1, f);

        db.dic_size = head << 8 >> 8;

        db.block_size = head >> 24;
        db.block_size += 1;

        db.fp = f;
    } else {
        db.opened = 0;
        z_err("Couldn't init db structure\n");
    }

    return db;
}

void closeDb(struct db2 db)
{
    if(db.fp != NULL) {
        fclose(db.fp);
    }
}
/**
 * @brief Return an absolute offset of intersection or starting row
 *        in case that one of elements is equal to zero
 * @param db
 * @param el1
 * @param el2
 * @return
 */
unsigned int intersect2_offset(struct db2 db, int el1, int el2)
{
    if(!db.opened) {
        return -1;
    }

    if(el1 == el2) {
        el1 = 0;
    } else if(el1 > el2) {
        int tmp = el2;
        el2 = el1;
        el1 = tmp;
    }

    if(el2 < 1) {
        return -1;
    }

    int position = 0;

    if(el1 > 1) {
        fseek(db.fp, 4 + (el1-2)*db.block_size, SEEK_SET);
        fread(&position, db.block_size, 1, db.fp);
    } else if(el1 == 0 && el2 > 1) {
        fseek(db.fp, 4 + (el2-2)*db.block_size, SEEK_SET);
        fread(&position, db.block_size, 1, db.fp);
    } else {
        position = 4 + (db.dic_size-2)*db.block_size;
    }

    if(el1 != 0) {
        position += (el2 - el1 - 1) * db.block_size;
    }

    return (unsigned int) position;
}

/**
 * @brief Fetch all intersections with <el>
 * @param db_name
 * @param el
 * @return
 */
unsigned int * intersect2_fetch(char*db_name, unsigned int el)
{
    unsigned int *result, position;
    int16_t buff;

    int i;
    struct db2 db;
    db = getDb(db_name, DB_OPEN_READ);
    if(db.opened && el > 0) {
        result =(unsigned int*)malloc((db.dic_size+1) * db.block_size);
        result[0] = db.dic_size;
        result[el] = el;

        if(el != db.dic_size) {
            position = intersect2_offset(db, el, 0);
            printf("Got %i as start offset\n", position);
            fseek(db.fp, position, SEEK_SET);

            for(i = el; i < db.dic_size; i++) {
                printf("Move to %i for %i\n", ftell(db.fp), i);
                fread(&buff, db.block_size, 1, db.fp);
                result[i+1] = buff;
                printf("Got %i\n", result[i+1]);
            }
        } else {
            position = intersect2_offset(db, el - 1, 0) + db.block_size;
        }

        if(el > 1) {
            // Move one row up and one cell left
            position = position - (db.dic_size - el + 1) * db.block_size;
            printf("Position is %i\n", position);
            fseek(db.fp, position, SEEK_SET);
            int delta = (db.dic_size - el) * db.block_size;
            for(i = el-2; i >= 0; i--) {
                printf("Move back to %i\n", ftell(db.fp));
                fread(&result[i+1], db.block_size, 1, db.fp);
                delta += db.block_size;
                position -= delta;
                printf("Position %i\n", position);
                fseek(db.fp, position, SEEK_SET);

                printf("Got %i for %i\n", result[i+1], i);
            }
        }
    }

    return result;
}





