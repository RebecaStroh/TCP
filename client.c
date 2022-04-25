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

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

    int option;
    scanf("%i", &option);

    switch (option)
    {
    case 1:
        printf(" TO DO: CADASTRAR FILME ");
        break;
    case 2:
        printf(" TO DO: ACRESCENTAR GENERO EM FILME ");
        break;
    case 3:
        printf(" TO DO: LISTAR TITULOS E IDS ");
        break;
    case 4:
        printf(" TO DO: LISTAR INFOS DE FILMES DE UM GENERO ");
        break;
    case 5:
        printf(" TO DO: LISTAR TUDO ");
        break;
    case 6:
        printf(" TO DO: LISTAR INFOS FILME DE UM ID ");
        break;
    case 7:
        printf(" TO DO: REMOVER FILME A PARTIR DE ID ");
        break;
    
    default:
        return 0;
    }

    printf("\n\n");
    char msg[2];
    sprintf(msg, "%d", option);
    if (send(sockfd, msg, 10, 0) == -1)
        perror("send");

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