/*
** client.c => arquivo que requisita a conexão deste cliente com o servidor,
*        trata das entradas, transformando-as em mensagens a serem compreendidas pelo servidos
*        e trata das mensagens recebidas do servidor para dar um feedback par ao cliente
*        segundo o protocolo TCP
*   Autor: Rebeca Portes Stroh, 186407
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
#include <ctype.h>
#include <arpa/inet.h>

#define PORT "4000"     // A porta a ser conectada
#define MAXDATASIZE 150 // Número máximo de bytes transferidos na mensagem
#define MAXTITLE 40     // Número máximo de bytes em um título
#define MAXDIRECTOR 40  // Número máximo de bytes para um diretor
#define MAXGENDER 20    // Número máximo de bytes em um genero
#define MAXYEAR 5       // Número máximo de bytes de um ano
#define MAXID 5         // Número máximo de bytes de um id
#define MAXNGENDER 3    // Número máximo de generos por filme

char buf[MAXDATASIZE];  // buffer auxiliar no envio/recebimento de mensagens
int numbytes;           // Número de bytes lidos

// Pega o endereço do socket (IPv4 ou IPv6)
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) // se for IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}

// Opção 1: Solicita a adição de um filme
int newMovie(int sockfd) {
    char title[MAXTITLE];
    char gender[MAXGENDER*MAXNGENDER];
    char director[MAXDIRECTOR];
    char year[MAXYEAR];
    char c;

    // Adiciona a função desejada à mensagem
    char msg[MAXDATASIZE] = "1|";
    // Adiciona o título à mensagem, lendo-o
    printf("Escreva um título: ");
    fflush (stdin);
    fgets(title, MAXTITLE, stdin);
    title[strcspn(title, "\n")] = 0;
    strcat(msg, title);
    strcat(msg,"|");
    // Adiciona o diretor à mensagem, lendo-o
    printf("Escreva o nome do diretor: ");
    fflush (stdin);
    fgets(director, MAXDIRECTOR, stdin);
    director[strcspn(director, "\n")] = 0;
    strcat(msg, director);
    strcat(msg,"|");
    // Adiciona o ano à mensagem, lendo-o
    printf("Escreva um ano: ");
    fflush (stdin);
    fgets(year, MAXYEAR, stdin);
    year[strcspn(year, "\n")] = 0;
    strcat(msg, year);
    strcat(msg,"|");
    // Adiciona o(s) genero(s) à mensagem, lendo-o(s)
    do {c = getchar();} while (c != EOF && c != '\n');
    printf("Escreva os generos: ");
    fflush (stdin);
    fgets(gender, MAXGENDER*MAXNGENDER, stdin);
    gender[strcspn(gender, "\n")] = 0;
    for (int i = 0; i < strlen(gender); i++) {
        gender[i] = tolower((unsigned char) gender[i]);
    }
    strcat(msg, gender);

    // Envia a mensagem requisitada
    if (send(sockfd, msg, MAXDATASIZE-1, 0) == -1)
        perror("send");

    // Espera a resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    if (buf[0] == '1') { // Caso o filme tenha sido adicionado com sucesso
        printf("\nFilme adicionado com sucesso!\n");
    } else { // Caso houveram problemas ao adicionar o filme
        printf("\nHouve algum problema ao adicionar o seu filme...\n");
    }

    return 0;
}

// Opção 2: Solicita a adição de um novo genero a um filme dado
int newGenderInMovie(int sockfd) {
    char gender[MAXGENDER];
    char id[MAXID];
    char c;

    // Adiciona a função desejada à mensagem
    char msg[MAXDATASIZE] = "2|";
    // Adiciona o id à mensagem, lendo-o
    printf("Escreva o id do filme: ");
    fflush (stdin);
    fgets(id, MAXID, stdin);
    id[strcspn(id, "\n")] = 0;
    strcat(msg, id);
    strcat(msg,"|");
    // Adiciona o genero à mensagem, lendo-o
    printf("Escreva o genero desejado: ");
    fflush (stdin);
    fgets(gender, MAXGENDER, stdin);
    gender[strcspn(gender, "\n")] = 0;
    for (int i = 0; i < strlen(gender); i++) {
        gender[i] = tolower((unsigned char) gender[i]);
    }
    strcat(msg, gender);

    // Envia a mensagem requisitada
    if (send(sockfd, msg, MAXDATASIZE-1, 0) == -1)
        perror("send");

    // Espera a resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    if (buf[0] == '1') { // Se a adição foi feita com sucesso
        printf("\nGenero adicionado com sucesso!\n");
    } else { // Caso encontrou-se problemas na adição
        printf("\nFilme com id inexistente ou genero já existe neste filme\n");
    }

    return 0;
}

// Opção 3: Solicita todos os filmes e seus respectivos identificadores
int getMoviesTitleId(int sockfd) {
    // Envia a mensagem requisitada
    if (send(sockfd, "3", 2, 0) == -1) {
        perror("send");
        return 1;
    }

    // Espera a resposta do servidor com o numero de filmes a serem enviados
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    // Indica o fim da string
    buf[numbytes] = '\0';

    // converte pata int o número de filmes a ser enviado
    int n = atoi(buf);

    if (n == 0) { // Caso ainda não existam filmes cadastrados
        printf("\nNão existem filmes adicionados ainda! \n");
    } else { // Le e mostra os filmes cadatrados
        printf("\nExistem %i filmes cadastrados: \n", n);

        for (int i=0; i<n; i++) {
            // Espera a resposta do servidor com um filme
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                return 1;
            }
            if (numbytes == 0) {
                printf("\nA conexão foi perdida com o servidor\n");
                return 1;
            }

            // Indica o fim da string
            buf[numbytes] = '\0';

            // Printa o filme
            printf("  => ");
            char * item = strtok(buf, "|"); // id
            printf("id: %s, título: ", item);
            item = strtok(NULL, "\n"); // title
            printf("%s\n", item);
        }
    }
    return 0;
}

// Opção 4: Solicita dados (título, diretor e ano) de filmes de um dado genero
int getMoviesFromGender(int sockfd) {
    char gender[MAXGENDER];

    // Adiciona a função desejada à mensagem
    char msg[MAXDATASIZE] = "4|";
    // Adiciona o genero à mensagem, lendo-o
    printf("Escreva o genero desejado: ");
    fflush (stdin);
    fgets(gender, MAXGENDER, stdin);
    gender[strcspn(gender, "\n")] = 0;
    strcat(msg, gender);

    // Envia a mensagem requisitada
    if (send(sockfd, msg, MAXDATASIZE-1, 0) == -1) {
        perror("send");
        return 1;
    }

    // Espera uma mensagem, que pode ser um outro filme ou indicio que não tem filme com tal genero (/end)
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    // Indica o fim da string
    buf[numbytes] = '\0';

    if (strcmp(buf, "/end") == 0) {
        printf("\nNão existem filmes adicionados nesse genero! \n");
    } else {
        printf("\nOs filmes com o genero %s são: \n", gender);

        while (strcmp(buf, "/end") != 0) { // Enquanto não for o fim da listagem
            // Printa dados do filme
            printf("  => ");
            char * item = strtok(buf, "|"); // título
            printf("título: %s, ", item);
            item = strtok(NULL, "|");       // diretor
            printf("diretor: %s, ", item);
            item = strtok(NULL, "\n");      // ano
            printf("ano: %s\n", item);

            // Espera próxima mensagem, que pode ser um outro filme ou o fim da listagem (/end)
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                return 1;
            }
            if (numbytes == 0) {
                printf("\nA conexão foi perdida com o servidor\n");
                return 1;
            }

            // Indica o fim da string
            buf[numbytes] = '\0';
        }
    }
    return 0;
}

// Opção 5: Solicita os dados de todos os filmes
int getAllMovies(int sockfd) {
    // Envia a mensagem requisitada
    if (send(sockfd, "5", 2, 0) == -1) {
        perror("send");
        return 1;
    }

    // Espera a resposta do servidor com o numero de filmes a serem enviados
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    // Indica o fim da string
    buf[numbytes] = '\0';
    
    // converte pata int o número de filmes a ser enviado
    int n = atoi(buf);

    if (n == 0) { // Caso ainda não existam filmes cadastrados
        printf("\nNão existem filmes adicionados ainda! \n");
    } else { // Le e mostra os filmes cadatrados
        printf("\nExistem %i filmes cadastrados: \n", n);

        for (int i=0; i<n; i++) {
            // Espera a resposta do servidor com um filme
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                return 1;
            }
            if (numbytes == 0) {
                printf("\nA conexão foi perdida com o servidor\n");
                return 1;
            }

            // Indica o fim da string
            buf[numbytes] = '\0';

            // Printa o filme
            printf("  => ");
            char * item = strtok(buf, "|"); // id
            printf("id: %s, ", item);
            item = strtok(NULL, "|");       // título
            printf("título: %s, ", item);
            item = strtok(NULL, "|");       // diretor
            printf("diretor: %s, ", item);
            item = strtok(NULL, "|");       // ano
            printf("ano: %s, ", item);
            item = strtok(NULL, "\n");      // genero
            printf("generos: %s\n", item);
        }
    }
    return 0;
}

// Opção 6: Solicita as informações de um filme
int getMovie(int sockfd) {
    char id[MAXID];

    // Adiciona a função desejada à mensagem
    char msg[MAXDATASIZE] = "6|";
    // Adiciona o id à mensagem, lendo-o
    printf("Escreva o id do filme: ");
    fflush (stdin);
    fgets(id, MAXID, stdin);
    id[strcspn(id, "\n")] = 0;
    strcat(msg, id);

    // Envia a mensagem requisitada
    if (send(sockfd, msg, MAXDATASIZE-1, 0) == -1)
        perror("send");

    // Espera a resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    // Indica o fim da string
    buf[numbytes] = '\0';

    if (strcmp(buf, "-1") == 0) { // Se não achou
        printf("\nNão existe nenhum filme com esse id!\n");
    } else { // Se achou, printa as informações
        printf("\nSegue as informações do filme desejado:\n");
        printf("  => ");
        char * item = strtok(buf, "|"); // id
        printf("id: %s, ", item);
        item = strtok(NULL, "|");       // título
        printf("título: %s, ", item);
        item = strtok(NULL, "|");       // diretor
        printf("diretor: %s, ", item);
        item = strtok(NULL, "|");       // ano
        printf("ano: %s, ", item);
        item = strtok(NULL, "\n");      // generos
        printf("generos: %s\n", item);
    }

    return 0;
}

// Opção 7: Solicita a remoção de um filme a partir de um identificador
int removeMovie(int sockfd) {
    char id[MAXID];

    // Adiciona a função desejada à mensagem
    char msg[MAXDATASIZE] = "7|";
    // Adiciona o id à mensagem, lendo-o
    printf("Escreva o id do filme: ");
    fflush (stdin);
    fgets(id, MAXID, stdin);
    id[strcspn(id, "\n")] = 0;
    strcat(msg, id);

    // Envia a mensagem requisitada
    if (send(sockfd, msg, MAXDATASIZE-1, 0) == -1) {
        perror("send");
        return 1;
    }

    // Espera a resposta do servidor
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    if (numbytes == 0) {
        printf("\nA conexão foi perdida com o servidor\n");
        return 1;
    }

    // Indica o fim da string
    buf[numbytes] = '\0';

    if (buf[0] == '0') { // Se não deu certo
        printf("\nNão existe um filme com tal id\n");
    } else { // Se deu certo
        printf("\nFilme de id %s deletado com sucesso\n", id);
    }
    return 0;
}

// Função que cuida de tratar a escolha do usuário e printar as opções
int menu(int sockfd) {
    while (1) {
        // Imprime opções possíveis
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

        // Le opção escolhida
        fflush (stdin);
        char option, c;
        printf("Opção: ");
        int success = scanf("%c", &option);
        do {c = getchar();} while (c != EOF && c != '\n');

        // Se a opção escolhida não for válida, encerra a sessão, avisando o servidor
        if ((option != '1' && option != '2' && option != '3' && option != '4' && option != '5' && option != '6' && option != '7') || !success){
            if (send(sockfd, "q", 2, 0) == -1)
                perror("send");
            return 0;
        }

        // Verifica que opção foi escolhida
        switch (option)
        {
        case '1':
            newMovie(sockfd);
            break;
        case '2':
            newGenderInMovie(sockfd);
            break;
        case '3':
            getMoviesTitleId(sockfd);
            break;
        case '4':
            getMoviesFromGender(sockfd);
            break;
        case '5':
            getAllMovies(sockfd);
            break;
        case '6':
            getMovie(sockfd);
            break;
        case '7':
            removeMovie(sockfd);
            break;
        
        default:
            return 0;
        }

        printf("\n");

        system("read -p 'Pressione alguma tecla para continuar...' var");
    }
}

int main(int argc, char *argv[])
{
    int sockfd, rv;  
    struct addrinfo hints, *servinfo, *p;
    char s[INET6_ADDRSTRLEN]; // Espaço para armazenar a string IPv6

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints); // Garante que a estrutura está vazia
    hints.ai_family = AF_UNSPEC; // Não importa se IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // pesquisas de DNS e service name, e preenche as estruturas que você precisa, após o sucesso, como o servinfo
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) { // argv[1] tem o host name ou IP do servidor a se conectar
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Ajuda a printar o erro
        return 1;
    }

    // Navegue nos resultados do getaddrinfo que foram alocados no servinfo e vincular o primeiro que conseguir
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Cria um socket seguindo os parâmetros (IPv4 ou IPv6, stream ou datagram, TCP ou UDP).
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue; // se tiver erro, tenta o próximo
        }

        // Se conecta com o servidor achado. Se der certo retorna o descritor do socket, se não -1
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue; // se tiver erro, tenta o próximo
        }

        break;
    }

    // Caso não tenha achado, retorna erro
    if (p == NULL) {
        fprintf(stderr, "Erro ao se conectar com o servidor\n");
        return 2;
    }

    // Com o endereço em mãos, traduz a struct in_addr em notação de números e pontos
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Conectando à %s\n", s);

    freeaddrinfo(servinfo); // libera o servinfo
    
    menu(sockfd); // começa a tratar das entradas e transforma em mensagens par ao servidor, esperando respostas

    printf("Desconectando");
    close(sockfd); // fecha a conexão

    return 0;
}