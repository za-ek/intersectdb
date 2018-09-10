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

#include "common.h"
#include "db2.h"

#define PORT_NO 2450
#define STORAGE_PATH "/var/lib/intersectdb/storage/"

void startSocket();
void doprocessing(int sock);
int checkStoragePath(char *path);

int main()
{
    printf("Hi!\n");
    if(checkStoragePath(STORAGE_PATH) == 1) {
        startSocket(PORT_NO);
    }
    return 0;
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

            int db_size;
            db_size = atoi(strsep(&string, " "));

            write(sock, "Will create a database: <", 25);
            write(sock, db_name, strlen(db_name));
            write(sock, ":", 1);

            char tmp[12]={0x0};
            sprintf(tmp,"%i", db_size);
            write(sock, tmp, strlen(tmp));

            write(sock, ">\n", 2);

            printf("Create database <%s> with <%i> elements\n", db_name, db_size);
            intersect2_createDb(STORAGE_PATH, db_name, db_size);
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
