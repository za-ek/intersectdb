#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <glob.h>

#include "utils.h"
#include "db2.h"

char storage_path[255];
int  port_number;

#define PORT_NO 2450
#define STORAGE_PATH "/var/lib/intersectdb/storage/"

void parseConfig(int argc, char**argv);
void startSocket(int port);
void doprocessing(int sock);
int checkStoragePath(char *path);

int main(int argc, char**argv)
{
    parseConfig(argc, argv);

    if(checkStoragePath(storage_path) == 1) {
        startSocket(port_number);
    }
    return 0;
}

/**
 * @brief parseConfig
 */
void parseConfig(int argc, char**argv)
{
    if(argc > 1) {
        port_number = atoi(argv[1]);
    } else {
        port_number = PORT_NO;
    }

    strcpy(storage_path, STORAGE_PATH);
}

void startSocket(int port)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int  pid;

    /* First call to socket() function */
    int option = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /*
     * Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
     */

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    while(1) {
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        /* Create child process */
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(sockfd);
            doprocessing(newsockfd);
            exit(0);
        }
        else {
            close(newsockfd);
        }
    }
}

void doprocessing (int sock)
{
    int n;
    char buffer[256];
    char *cmd, *string, *token;

    bzero(buffer,256);
    n = read(sock,buffer,255);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Received the message: %s\n",buffer);


    string = strdup(buffer);
    string = z_trim(string);

    if((token = strsep(&string, " ")) != NULL) {
        printf("Found <%s>\n", token);

        cmd = token;

        write(sock, "Command found:", 14);
        write(sock, cmd, strlen(cmd));
        write(sock, "\n", 1);

        if(strcmp("CREATE2", cmd) == 0) {
            char*db_name;
            db_name = strsep(&string, " ");

            unsigned int db_size;
            db_size = atoi(strsep(&string, " "));

            write(sock, "Will create a database: <", 25);
            write(sock, db_name, strlen(db_name));
            write(sock, ":", 1);

            char tmp[12]={0x0};
            sprintf(tmp,"%i", db_size);
            write(sock, tmp, strlen(tmp));

            write(sock, ">\n", 2);

            printf("Create database <%s> with <%i> elements\n", db_name, db_size);

            int result;
            result = intersect2_createDb(db_name, db_size);
            if(result < 0) {
                write(sock, "There was an error\n", 19);
            }
        } else if (strcmp("INC", cmd) == 0) {
            char*db_name;
            int el1, el2, n;

            db_name = strsep(&string, " ");
            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));
            n = atoi(strsep(&string, " "));
            if(n == 0) {
                n = 1;
            }

            intersect2_inc(db_name, el1, el2, n);
        } else if (strcmp("FETCH", cmd) == 0) {
            char*db_name;
            int el1, el2;
            int result;

            db_name = strsep(&string, " ");
            el1 = atoi(strsep(&string, " "));
            el2 = atoi(strsep(&string, " "));

            result = intersect2_get(db_name, el1, el2);

            char tmp[12]={0x0};
            sprintf(tmp,"%i", result);
            write(sock, tmp, strlen(tmp));
            write(sock, "\n", 1);
        }
    }

    n = write(sock,"I got your message\n",19);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}

/**
 * @brief checkStoragePath
 * @return int for correct
 */
int checkStoragePath(char*path)
{
    int result = z_dirExist(path);
    if (result == 1) {
        return 1;
    } else if (result == 2) {
        char*err = "";
        // So poor program :(
        sprintf(err, "Please create a directory <%s>", path);
        z_err(err);
    } else {
        z_err("Couldn't check storage path");
    }

    return 2;
}
