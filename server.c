/*
** server.c => arquivo que trata das conexões com clientes e lida com o "banco de dados" e processamentos
*           lidando com o envio e tratamento de mensagens segundo o protocolo TCP
*   Autor: Rebeca Portes Stroh, 186407
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

#define PORT "4000"     // A porta a ser conectada
#define BACKLOG 10      // Quantas conexões pendentes podem existir
#define MAXDATASIZE 150 // Número máximo de bytes transferidos na mensagem
#define FILENAME "data.txt" // Nome do arquivo que salva todos os dados
#define TEMPFILENAME "delete.tmp" // Nome do arquivo que salva todos os dados

FILE* file; // O Arquivo que usaremos para salvar os dados
int numbytes;

// Recolhe todos os processos "mortos"
void sigchld_handler(int s) {
    int saved_errno = errno; // garante que o waitpid() não vai sobreescrever errno
    while(waitpid(-1, NULL, WNOHANG) > 0); // allows the calling thread to obtain status information for one of its child processes
    errno = saved_errno;
}

// Pega o endereço do socket (IPv4 ou IPv6)
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) // se for IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}

// Garante que o arquivo tem mais o primeiro item que conta a quantidade de filmes e o header
void garanteeNumberAndHeaderLinesOnFile() {
    char buf[MAXDATASIZE];
    int count = 1;

    if( access( FILENAME, F_OK ) == 0 ) { // Se o arquivo existir, pesquisa se tem as primeiras linhas
        file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
        while ((fgets(buf, MAXDATASIZE, file)) != NULL) { // Le cada linha do arquivo
            count++;
        }
        fclose(file); // fecha arquivo
    }

    if ((count == 1) || (count == 2)) {
        file = fopen(FILENAME, "w"); // Abre o arquivo como escrita
        fputs("0", file);
        fputs("\nid|title|director|year|genders", file);
        fclose(file); // fecha arquivo
    }
}

// Deleta uma dada linha do arquivo
void deleteLine(const int line) {
    char buf[MAXDATASIZE];
    char movie[MAXDATASIZE];
    int count = 1;
    int n;

    file = fopen(FILENAME, "r"); // Abre o arquivo para leitura
    FILE *temp = fopen(TEMPFILENAME, "w"); // Cria um arquivo auxiliar e o abre como escrita
    while ((fgets(buf, MAXDATASIZE, file)) != NULL) { // Le cada linha do arquivo
        buf[strcspn(buf, "\n")] = 0; // Indica o fim da string
        if (count == 1) { // Incrementa quantidade de filmes no arquivo
            n = atoi(buf) - 1;
            sprintf(buf, "%i", n);
            fputs(buf, temp);
        } else if (line != count) { // Caso não seja a linha a ser deletada
            strcpy(movie,"\n");
            strcat(movie, buf);
            fputs(movie, temp); // Insere no arquivo
        }
        count++;
    }

    fclose(file); // fecha arquivo
    fclose(temp); // fecha arquivo
    remove(FILENAME); // apaga o arquivo original
    rename(TEMPFILENAME, FILENAME); // renomeia o arquivo temporário pro nome original
}

// Opção 1: Adiciona um novo filme à lista do arquivo
int newMovie(int new_fd, char* movie) {
    char buf[MAXDATASIZE];
    int count = 1;
    int n;

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    FILE *temp = fopen(TEMPFILENAME, "w"); // Cria um arquivo auxiliar e o abre como escrita
    while ((fgets(buf, MAXDATASIZE, file)) != NULL) { // Le cada linha do arquivo
        if (count == 1) { // Aumenta a quantidade de filmes no arquivo
            n = atoi(buf) + 1;
            sprintf(buf, "%i\n", n);
        }
        fputs(buf, temp); // Insere no arquivo temporário
        count++;
    }
    // Adiciona novo filme
    char * lastId = strtok(buf, "|"); // le qual é o ultimo id
    n = atoi(lastId) + 1; // acrescenta ao número
    sprintf(buf, "\n%i", n);
    strcat(buf, "|"); // concatena com separados
    strcat(buf, movie); // concatena com dados do filme
    fputs(buf, temp); // Insere no arquivo temporário

    fclose(file); // fecha arquivo
    fclose(temp); // fecha arquivo
    remove(FILENAME); // apaga o arquivo original
    rename(TEMPFILENAME, FILENAME); // renomeia o arquivo temporário pro nome original

    if (send(new_fd, "1", 2, 0) == -1) { // Envia que deu certo
        perror("send");
        return 1;
    }

    return 0;
}

// Opção 2: Adiciona um novo genero ao filme indicado
int newGenderInMovie(int new_fd, char* args) {
    char buf[MAXDATASIZE];
    char movie[MAXDATASIZE];
    char* id = strtok(args, "|"); // primeiro argumento é o id
    char* gender = strtok(NULL, "\n"); // segundo argumento é o genero
    int movieExists = 0;
    int genderExists = 0;

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    FILE *temp = fopen(TEMPFILENAME, "w"); // Cria um arquivo auxiliar e o abre como escrita

    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo
    fputs(buf, temp); // insere a linha no arquivo temporário
    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo
    fputs(buf, temp); // insere a linha no arquivo temporário

    while ((fgets(buf, MAXDATASIZE, file)) != NULL) { // Le cada linha do arquivo
        strcpy(movie, buf); // copia o buffer
        char * item = strtok(buf, "|"); // id

        if (strcmp(id, item) == 0) { // Se é esse o filme que queremos editar
            movieExists = 1;
            item = strtok(NULL, "|"); // título
            item = strtok(NULL, "|"); // diretor
            item = strtok(NULL, "|"); // ano
            item = strtok(NULL, "|"); // genero
            char * currGender = strtok(item, ",");
            while (currGender) { // Para cada genero da lista
                currGender[strcspn(currGender, "\n")] = 0; // Indica o fim da string
                if (strcmp(currGender, gender) == 0) { // Se o genero já existe
                    genderExists = 1;
                    break;
                }
                currGender = strtok(NULL, ",");
            }
            if (!genderExists) { // Se o genero não existe, adiciona o genero no filme
                movie[strcspn(movie, "\n")] = 0; // Indica o fim da string
                strcat(movie, ",");
                strcat(movie, gender);
            }
        }
        fputs(movie, temp); // insere a linha no arquivo temporário
    }

    fclose(file); // fecha arquivo
    fclose(temp); // fecha arquivo
    remove(FILENAME); // apaga o arquivo original
    rename(TEMPFILENAME, FILENAME); // renomeia o arquivo temporário pro nome original

    if (genderExists || !movieExists) {
        if (send(new_fd, "0", 2, 0) == -1) { // Se o genero já existe ou o filme não foi encontrado, envia que houve um erro
            perror("send");
            fclose(file); // fecha o arquivo
            return 1;
        }
    } else {
        if (send(new_fd, "1", 2, 0) == -1) { // Envia que deu certo
            perror("send");
            fclose(file); // fecha o arquivo
            return 1;
        }
    }
    return 0;
}

// Opção 3: Retorna todos os filmes e seus respectivos identificadores
int getMoviesTitleId(int new_fd) { 
    char buf[MAXDATASIZE];
    char movie[MAXDATASIZE];

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo

    if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1) { // Envia a quantidade de filmes
        perror("send");
        fclose(file);
        return 1;
    }

    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo

    while (fgets(buf, MAXDATASIZE, file) != NULL) { // Le cada filme do arquivo
        char * item = strtok(buf, "|"); // id
        strcpy(movie, item);            // adiciona o id à mensagem de retorno
        item = strtok(NULL, "|");       // título
        strcat(movie, "|");             // adiciona o separador à mensagem de retorno
        strcat(movie, item);            // adiciona o título à mensagem de retorno

        if (send(new_fd, movie, MAXDATASIZE-1, 0) == -1) { // Envia o filme
            perror("send");
            fclose(file); // fecha o arquivo
            return 1;
        }
    }

    fclose(file); // fecha o arquivo
    return 0;
}

// Opção 4: Retorna todos os filmes (título, diretor e ano) de um determinado genero
int getMoviesFromGender(int new_fd, char* gender) {
    char buf[MAXDATASIZE];
    char movie[MAXDATASIZE];

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo
    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo

    while (fgets(buf, MAXDATASIZE, file) != NULL) { // Le cada filme do arquivo
        char * item = strtok(buf, "|"); // id
        item = strtok(NULL, "|");       // título
        strcpy(movie, item);
        item = strtok(NULL, "|");       // diretor
        strcat(movie, "|");
        strcat(movie, item);
        item = strtok(NULL, "|");       // ano
        strcat(movie, "|");
        strcat(movie, item);
        item = strtok(NULL, "|");       // genero
        char * currGender = strtok(item, ",");
        while (currGender) { // Enquanto tiver mais generos para serem lidos
            currGender[strcspn(currGender, "\n")] = 0; // Finaliza a string caso tenha um \n
            if (strcmp(currGender, gender) == 0) { // Se tiver o genero no filme
                if (send(new_fd, movie, MAXDATASIZE-1, 0) == -1) { // Envia o filme
                    perror("send");
                    fclose(file); // fecha o arquivo
                    return 1;
                }
                break;
            }
            currGender = strtok(NULL, ",");
        }
    }

    if (send(new_fd, "/end", 5, 0) == -1) { // Envia que não tem mais filmes a serem enviados
        perror("send");
        fclose(file); // fecha o arquivo
        return 1;
    }

    fclose(file); // fecha o arquivo
    return 0;
}

// Opção 5: Retorna todos os filmes
int getAllMovies(int new_fd) {   
    char buf[MAXDATASIZE];
    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo

    if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1) { // Envia a quantidade de filmes
        perror("send");
        fclose(file); // fecha o arquivo
        return 1;
    }

    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo

    while (fgets(buf, MAXDATASIZE, file) != NULL) { // Le cada filme do arquivo
        if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1) { // Envia o filme lido
            perror("send");
            fclose(file); // fecha o arquivo
            return 1;
        }
    }

    fclose(file); // fecha o arquivo
    return 0;
}

// Opção 6: Retorna o filme de um determinado identificador
int getMovie(int new_fd, char* id) {
    char buf[MAXDATASIZE];
    char movie[MAXDATASIZE];

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo
    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo

    while (fgets(buf, MAXDATASIZE, file) != NULL) { // Le cada filme do arquivo
        strcpy(movie, buf); // Coloca o ponteiro do movie no atual do buffer
        char * currId = strtok(buf, "|"); // Pega o primeiro item, que é o id
        if (strcmp(currId, id) == 0) { // Caso seja o id desejado
            if (send(new_fd, movie, MAXDATASIZE-1, 0) == -1) { // Envia o filme
                perror("send");
                fclose(file); // fecha o arquivo
                return 1;
            }
            fclose(file); // fecha o arquivo
            return 0;
        }
    }

    if (send(new_fd, "-1", 3, 0) == -1) { // Envia que houve um erro
        perror("send");
        return 1;
    }
    
    fclose(file); // fecha o arquivo
    return 0;
}

// Opção 7: Remove o filme de um determinado identificador
int removeMovie(int new_fd, char* id) {
    char buf[MAXDATASIZE];
    int line = 3;

    file = fopen(FILENAME, "r"); // Abre o arquivo como leitura
    fgets(buf, MAXDATASIZE, file); // Le quantos filmes existem no arquivo
    fgets(buf, MAXDATASIZE, file); // Le o header do arquivo

    while (fgets(buf, MAXDATASIZE, file) != NULL) { // Le cada filme do arquivo
        char * currId = strtok(buf, "|"); // Pega o primeiro item, que é o id
        if (strcmp(currId, id) == 0) { // Se for o id desejado
            deleteLine(line); // Apaga a linha daquele filme
            if (send(new_fd, "1", 2, 0) == -1) { // Envia que deu certo
                perror("send");
                fclose(file); // fecha o arquivo
                return 1;
            }
            // fclose(file); // fecha o arquivo
            return 0;
        }
        line++;
    }

    if (send(new_fd, "0", 2, 0) == -1) { // Envia que não deu certo
        perror("send");
        fclose(file); // fecha o arquivo
        return 1;
    }
    fclose(file); // fecha o arquivo
    
    return 1;
}

// Lida com cada cliente conectado, entendendo cada mensagem recebida e chamando a ação a ser feita
void handleOptions(int new_fd) {
    char buf[MAXDATASIZE];
    int result;

    // Enquanto o cliente não requisitar fechar a conexão
    while (1) {
        // Espera mensagem do cliente com a requisição
        if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        if (numbytes == 0) {
            printf("A conexão foi fechada antes de receber algo\n");
            exit(1);
        }

        // Indica o fim da string
        buf[numbytes] = '\0';

        // Pega o primeiro item na mensagem que é a função desejada
        char * item = strtok(buf, "|");

        // Estuda qual é a opção desejada
        switch (item[0]) {
            case '1':
                item = strtok(NULL, "\n"); // le demais argumentos
                result = newMovie(new_fd, item);
                break;
            case '2':
                item = strtok(NULL, "\n"); // le demais argumentos
                result = newGenderInMovie(new_fd, item);
                break;
            case '3':
                result = getMoviesTitleId(new_fd);
                break;
            case '4':
                item = strtok(NULL, "\n"); // le demais argumentos
                result = getMoviesFromGender(new_fd, item);
                break;
            case '5':
                result = getAllMovies(new_fd);
                break;
            case '6':
                item = strtok(NULL, "\n"); // le demais argumentos
                result = getMovie(new_fd, item);
                break;
            case '7':
                item = strtok(NULL, "\n"); // le demais argumentos
                result = removeMovie(new_fd, item);
                break;
            default: // qualquer outra opção, ele sai
                return;
        }   
    }
}

int main(void)
{
    int sockfd, new_fd, rv;  // sock_fd ficará escutando e new_fd será para novas conexões
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Contém a informação do endereço a ser conectado
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN]; // Espaço para armazenar a string IPv6

    memset(&hints, 0, sizeof hints); // Garante que a estrutura está vazia
    hints.ai_family = AF_UNSPEC; // Não importa se IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // completa com o meu IP
    
    // pesquisas de DNS e service name, e preenche as estruturas que você precisa, após o sucesso, como o servinfo
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Ajuda a printar o erro
        return 1;
    }

    // Navegue nos resultados do getaddrinfo que foram alocados no servinfo e vincular o primeiro que conseguir
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Cria um socket seguindo os parâmetros (IPv4 ou IPv6, stream ou datagram, TCP ou UDP).
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        // Permitindo reutilizar a porta
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Atribui um endereço de protocolo local (“nome”) a um soquete.
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue; // Se não foi bem sucedido, continua a procurar
        }

        break;
    }

    freeaddrinfo(servinfo); // liberar a lista encadeada pois não é mais necessária

    // Se não achou nenhum válido
    if (p == NULL)  {
        fprintf(stderr, "Falha ao ligar\n");
        exit(1);
    }

    // Anuncie a disposição de aceitar conexões, fornecendo o tamanho da fila e alterando o estado do soquete para o servidor TCP.
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask); // Inicializa um conjunto de sinais como vazio, excluindo sinais reconhecidos.
    sa.sa_flags = SA_RESTART; // Configura a flag SA_RESTART, que tem o papel de religar a chamada do sistema após ter sido interrompida.
    if (sigaction(SIGCHLD, &sa, NULL) == -1) { // Configura um manipulador de sinal
        perror("sigaction");
        exit(1);
    }

    printf("Esperando conexões...\n");

    garanteeNumberAndHeaderLinesOnFile();

    // Fica a espera de novas conexões por conta do accept()
    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); // Retorna a próxima conexão completa
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Com o endereço em mãos, traduz a struct in_addr em notação de números e pontos
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("Conexão recebida de %s\n", s);

        if (!fork()) { // cria um processo filho. Se estamos com o processo filho (0), entra no if
            close(sockfd); // O processo filho não fica escutando novas conexões, então fecha o listener

            handleOptions(new_fd); // Trata dos dados recebidos pelo cliente

            printf("Cliente %s desconectado\n", s); // Quando o cliente desconecta
            close(new_fd); // Acaba com o processo filho
            exit(0);
        }
        close(new_fd);  // O processo pai continua escutando novas conexões e se desvincula do processo filho criado, fechando-o para ele
    }

    return 0;
}