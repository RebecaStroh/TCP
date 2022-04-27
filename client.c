/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 120 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int newMovie(int sockfd) { // OPTION 1
    char title[30];
    char gender[50];
    char director[30];
    char year[5];

    // add option to msg
    char msg[116] = "1|";
    // add title to msg
    printf("Escreva um título: ");
    fflush (stdin);
    fgets(title, 30, stdin);
    title[strcspn(title, "\n")] = 0;
    strcat(msg, title);
    strcat(msg,"|");
    // add director to msg
    printf("Escreva o nome do diretor: ");
    fflush (stdin);
    fgets(director, 30, stdin);
    director[strcspn(director, "\n")] = 0;
    strcat(msg, director);
    strcat(msg,"|");
    // add year to msg
    printf("Escreva um ano: ");
    scanf("%s", year);
    strcat(msg, year);
    strcat(msg,"|");
    // add gender to msg
    printf("Escreva os generos: ");
    fflush (stdin);
    fgets(gender, 30, stdin);
    gender[strcspn(gender, "\n")] = 0;
    strcat(msg, gender);

    printf("\ncliente vai enviar a seguinte mensagem: %s\n", msg);

    if (send(sockfd, msg, 115, 0) == -1)
        perror("send");
    return 1;
}

int newGenderInMovie(int sockfd) { // OPTION 2

    if (send(sockfd, "newGenderInMovie: {gender: , movie: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMoviesTitleId(int sockfd) { // OPTION 3
    if (send(sockfd, "getMoviesTitleId", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMoviesFromGender(int sockfd) {  // OPTION 4
    int gender = 1;

    if (send(sockfd, "getMoviesFromGender : { gender: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int getAllMovies(int sockfd) { // OPTION 5
    if (send(sockfd, "getAllMovies", 30, 0) == -1)
        perror("send");
    return 1;
}

int getMovie(int sockfd) { // OPTION 6
    int id = 1;

    if (send(sockfd, "getMovie: { id: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int removeMovie(int sockfd) { // OPTION 7
    int id = 1;

    if (send(sockfd, "removeMovie: { id: }", 30, 0) == -1)
        perror("send");
    return 1;
}

int menu(int sockfd) {
    char buf[MAXDATASIZE];
    int numbytes;
    printf("------------------------------------------------------------ \n");
    printf("Escolha uma das ações para realizar: \n");
    printf("    1 - Cadastrar um novo filme, determinando um identificador numérico no cadastro \n");
    printf("    2 - Acrescentar um novo gênero em um filme \n");
    printf("    3 - Listar todos os títulos, junto a seus respectivos identificadores \n");
    printf("    4 - Listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero \n");
    printf("    5 - Listar todas as informações de todos os filmes \n");
    printf("    6 - Listar todas as informações de um filme a partir de seu identificador \n");
    printf("    7 - Remover um filme a partir de seu identificador \n");
    printf("    Qualquer Outra Tecla - Sair do programa \n");
    printf("------------------------------------------------------------ \n \n");

    fflush (stdin);
    int option;
    int success = scanf("%i", &option);

    if (!(0<option && option<8 && success))
        return 0;

    switch (option)
    {
    case 1:
        newMovie(sockfd);
        break;
    case 2:
        newGenderInMovie(sockfd);
        break;
    case 3:
        getMoviesTitleId(sockfd);
        break;
    case 4:
        getMoviesFromGender(sockfd);
        break;
    case 5:
        getAllMovies(sockfd);
        break;
    case 6:
        getMovie(sockfd);
        break;
    case 7:
        removeMovie(sockfd);
        break;
    
    default:
        break;
    }

    printf("\n\n");

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    if (numbytes == 0) {
        printf("client: a conexao foi fechada antes de vc receber algo");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    // system("PAUSE");
    system("read -p 'Press Enter to continue...' var");
    menu(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    
    menu(sockfd);

    close(sockfd);

    return 0;
}