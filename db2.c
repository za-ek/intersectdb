#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <common.h>

char storage_path[255];

struct db2 {
    int dic_size;
    int block_size;
    char*path;
    FILE*fp;
};

struct db2 getDb(char *db_name, char *mode);
void closeDb(struct db2 db);

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
    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));
    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");

    unsigned int elementCount = (dicSize - 1) * dicSize / 2;

    unsigned int head;
    head = 1 << 24;
    head += dicSize;

    unsigned int headSize = sizeof(head);
    unsigned int BLOCK_SIZE = 2;

    int64_t total_byte_count = (int64_t) (headSize + ((dicSize - 2) + elementCount)*BLOCK_SIZE);

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
    if(f != NULL) {
        fseek(f, 0, SEEK_SET);
        fwrite(&head, headSize, 1, f);

        int buffer;
        buffer = 1;

        unsigned int i;
        for(i = 1; i < dicSize; i++) {
            buffer += dicSize - i;
            fwrite(&buffer, BLOCK_SIZE, 1, f);
        }

        fclose(f);
    } else {
        printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        z_err("Couldn't open database file\n");
        return -1;
    }

    return 0;
}

/**
 * Increment intersection value
 */
int intersect2_inc(char*db_name, int from, int to)
{
    int position;
    struct db2 db;

    if(from > 0 && to > 0) {
        if(from > to) {
            int16_t tmp = to;
            to = from;
            from = tmp;
        }

        db = getDb(db_name, "rb+");

        if(db.fp != NULL) {
            if(from > 1) {
                fseek(db.fp, 4 + (from-2)*db.block_size, SEEK_SET);
                fread(&position, db.block_size, 1, db.fp);
                position += to - from - 2;
            } else {
                position = to - from;
            }

            fseek(db.fp, 4 + (db.dic_size-2)*db.block_size + position * db.block_size, SEEK_SET);

            int val = 0;
            fread(&val, db.block_size, 1, db.fp);
            fseek(db.fp, -db.block_size, SEEK_CUR);

            val++;
            fwrite(&val, db.block_size, 1, db.fp);

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
       if(el1 > el2) {
           int16_t tmp = el2;
           el2 = el1;
           el1 = tmp;
       }

       db = getDb(db_name, "rb");

       if(db.fp != NULL) {
           int position;

           if(el1 > 1) {
               fseek(db.fp, 4 + (el1-2)*db.block_size, SEEK_SET);
               fread(&position, db.block_size, 1, db.fp);
               position += el2 - el1 - 2;
           } else {
               position = el2 - el1;
           }

           fseek(db.fp, 4 + (db.dic_size-2)*db.block_size + position * db.block_size, SEEK_SET);

           int val = 0;
           fread(&val, db.block_size, 1, db.fp);

           closeDb(db);

           return (int)val;
       } else {
           return -1;
       }
   }

   return -2;
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

    char* path = (char*)malloc((strlen(storage_path)+1+5+strlen(db_name)) * sizeof(char));
    sprintf(path, "%s%s%s", storage_path, db_name, ".idb2");

    db.path = path;

    FILE * f = fopen(path, mode);
    if(f != NULL) {
        fseek(f, 0, SEEK_SET);
        fread(&head, 4, 1, f);

        db.dic_size = head << 8 >> 8;

        db.block_size = head >> 24;
        db.block_size += 1;

        db.fp = f;
    } else {
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
