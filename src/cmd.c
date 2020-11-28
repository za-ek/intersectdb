#include <string.h>
#include <glob.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "db2.h"
#include "cmd.h"

extern int sock_fd;

void sendResult(char *result);

int processCommand(char*string)
{
    char *token, *cmd;

    printf("Got a command %s\n", string);

    if((token = strsep(&string, " ")) != NULL) {
        cmd = token;

        if(strcmp("EXIT", cmd) == 0) {
            return CMD_RESULT_EXIT;
        }

        if(strcmp("CREATE", cmd) == 0) {
            char*db_name,*db_type;
            unsigned int db_size;
            int result;

            db_type = strsep(&string, " ");

            if(strcmp("DB2", db_type) == 0) {
                db_name = strsep(&string, " ");
                db_size = atoi(strsep(&string, " "));

                printf("Create database <%s> with <%i> elements\n", db_name, db_size);

                result = intersect2_createDb(db_name, db_size);
                if(result >= 0) {
                    sendResult("OK");
                } else {
                    sendResult("ERROR");
                }
            } else {
                sendResult("UNSUPPORTED DB TYPE");
            }
        } else if (strcmp("SHOW", cmd) == 0) {
            char*show_type, msg[8]={0x0},*db_name;
            show_type = strsep(&string, " ");

            if(strcmp("DB", show_type) == 0) {
                db_name = strsep(&string, " ");
                struct db2 db = getDb(db_name, DB_OPEN_INFO);
                sprintf(msg, "SIZE:%d", db.dic_size);
                sendResult(db.name);
                sendResult(msg);
            }

        } else if (strcmp("INC", cmd) == 0) {
            char*db_name,msg[10]={0x0};
            unsigned int el1, el2, n = 1;
            int result;

            db_name = strsep(&string, " ");

            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));

            if(strlen(string) > 1) {
                n = atoi(strsep(&string, " "));
            }

            result = intersect2_inc(db_name, el1, el2, n);

            if(result > 0) {
                sendResult("OK");
                sprintf(msg, "%i", result);
                sendResult(msg);
            } else {
                sendResult("ERROR");
            }
        } else if (strcmp("FETCH", cmd) == 0) {
            char*db_name, tmp[12]={0x0};
            unsigned int*result, i, length, n;

            db_name = strsep(&string, " ");
            n = atoi(strsep(&string, " "));

            result = intersect2_fetch(db_name, n);
            length = result[0];

            for(i = 1; i <= length; i++) {
                sprintf(tmp, "%i", result[i]);
                sendResult(tmp);
            }
        } else if (strcmp("READ", cmd) == 0) {
            char*db_name, tmp[12]={0x0};
            int el1, el2;
            int result;

            db_name = strsep(&string, " ");
            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));

            result = intersect2_get(db_name, el1, el2);

            sprintf(tmp,"%i", result);
            sendResult(tmp);
        } else {
            sendResult("UNKNOWN");
        }
    }

    return CMD_RESULT_SKIP;
}

void sendResult(char*result)
{
    write(sock_fd,result,strlen(result));
    write(sock_fd,"\n",1);
}
