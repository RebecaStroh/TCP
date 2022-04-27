/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
// #include "data.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 120 // max number of bytes we can get at once 

// data *Data;

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int newMovie(int new_fd) { // OPTION 1
    char *title = "Novo filme";
    int gender[] = {1, 2};
    char *director = "Carlos Alberto";
    int year = 1999;

    if (send(new_fd, "newMovie: { title: , gender: , director: , year: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int newGenderInMovie(int new_fd) { // OPTION 2
    int gender[] = {1};
    int movie = 1;

    if (send(new_fd, "newGenderInMovie: {gender: , movie: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMoviesTitleId(int new_fd) { // OPTION 3
    if (send(new_fd, "getMoviesTitleId", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMoviesFromGender(int new_fd) {  // OPTION 4
    int gender = 1;

    if (send(new_fd, "getMoviesFromGender : { gender: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int getAllMovies(int new_fd) { // OPTION 5
    if (send(new_fd, "getAllMovies", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMovie(int new_fd) { // OPTION 6
    int id = 1;

    if (send(new_fd, "getMovie: { id: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int removeMovie(int new_fd) { // OPTION 7
    int id = 1;

    if (send(new_fd, "removeMovie: { id: }", 30, 0) == -1)
        perror("send");
    return 1;
}

void handleOptions(int new_fd) {
    int numbytes, result;
    char buf[MAXDATASIZE];

    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    if (numbytes == 0) {
        printf("server: a conexao foi fechada antes de vc receber algo\n");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("servidor: escolha recebida '%s'\n",buf);

    switch (buf[0]) {
        case '1':
            result = newMovie(new_fd);
            break;
        case '2':
            result = newGenderInMovie(new_fd);
            break;
        case '3':
            result = getMoviesTitleId(new_fd);
            break;
        case '4':
            result = getMoviesFromGender(new_fd);
            break;
        case '5':
            result = getAllMovies(new_fd);
            break;
        case '6':
            result = getMovie(new_fd);
            break;
        case '7':
            result = removeMovie(new_fd);
            break;
        default: // other parameter
            return;
    }   

    handleOptions(new_fd);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Data = newData();

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            handleOptions(new_fd);

            printf("servidor: tchauuuu\n");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}