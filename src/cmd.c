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

        if(strcmp("CREATE2", cmd) == 0) {
            char*db_name;
            unsigned int db_size;
            int result;

            db_name = strsep(&string, " ");
            db_size = atoi(strsep(&string, " "));

            printf("Create database <%s> with <%i> elements\n", db_name, db_size);

            result = intersect2_createDb(db_name, db_size);
            if(result >= 0) {
                sendResult("OK");
            } else {
                sendResult("ERROR");
            }
        } else if (strcmp("INC", cmd) == 0) {
            char*db_name,msg[10]={0x0};
            unsigned int el1, el2, n = 1;
            int result;

            db_name = strsep(&string, " ");

            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));

            if(strlen(&string) > 1) {
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

        } else if (strcmp("GET", cmd) == 0) {
            char*db_name, tmp[12]={0x0};
            int el1, el2;
            int result;

            db_name = strsep(&string, " ");
            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));

            result = intersect2_get(db_name, el1, el2);

            sprintf(tmp,"%i", result);
            sendResult(tmp);
        }
    }

    return CMD_RESULT_SKIP;
}

void sendResult(char*result)
{
    write(sock_fd,result,strlen(result));
    write(sock_fd,"\n",1);
}
