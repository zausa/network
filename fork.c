#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>

#define PORT    "8080" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

//html
char webpage[]=
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>froggy</title></head>\r\n"
"<style>body {background-color: #F892A0 }</style></head>\r\n"
"<body><center><h1>froggy is the best!!!</h1><br>\r\n"
"<img align=\"middle\" src=\"froggy.jpg\"></center></body><html>\r\n";





/* Signal handler to reap zombie processes */
static void wait_for_child(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle(int newsock)
{
    send(newsock,webpage,sizeof(webpage),0 );
}

int main(void)
{
    int sock;
    struct sigaction sa;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */
    char buf[2048];

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }

    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    freeaddrinfo(res);

    /* Set up the signal handler */
    sa.sa_handler = wait_for_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }



    /* Main loop */
    while (1) {
        struct sockaddr_in their_addr;
        socklen_t size = sizeof(struct sockaddr_in);
        int newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
        int pid;

        if (newsock == -1) {
            perror("accept");
            return 0;
        }

        printf("Got a connection from %s on port %d\n", inet_ntoa(their_addr.sin_addr),
                        htons(their_addr.sin_port));


        if (pid =fork() == 0) {

            /* In child process */
            close(sock);
            handle(newsock);

            //memset(buf,0,2048);
          //read(newsock,buf,2047);
          //  printf("%s\n",buf);


            //write(newsock,webpage,sizeof(webpage)-1);


            return 0;
        }
        else {
            /* Parent process */
            if (pid == -1) {
                perror("fork");
                return 1;
            }
            else {
                close(newsock);
            }
        }
    }

    close(sock);

    return 0;
}
