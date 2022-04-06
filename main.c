#include "main.h"

// OPERAÇÕES:
// • cadastrar um novo filme, determinando um identificador numérico no cadastro;
// • acrescentar um novo gênero em um filme;
// • listar todos os títulos, junto a seus respectivos identificadores;
// • listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero;
// • listar todas as informações de todos os filmes;
// • listar todas as informações de um filme a partir de seu identificador;
// • baixar uma imagem capa de um filme a partir de seu identificador (somente no projeto 2, com UDP);
// • remover um filme a partir de seu identificador.

// Exemplos de filmes:
    // Título: O Resgate do Soldado Brian
    // Gênero: Drama, Guerra
    // Diretor(a): Steven Spielburg
    // Ano de Lançamento: 1999

    // Título: Djanko Livre
    // Gênero: Ação, Faroeste
    // Diretor(a): Quentin Tarantoni
    // Ano de Lançamento: 2013

int menu() {
    printf("-------------------------------------------------------------------------------------------------------- \n");
    printf("Escolha uma das ações para realizar: \n");
    printf("    1 - Cadastrar um novo filme, determinando um identificador numérico no cadastro \n");
    printf("    2 - Acrescentar um novo gênero em um filme \n");
    printf("    3 - Listar todos os títulos, junto a seus respectivos identificadores \n");
    printf("    4 - Listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero \n");
    printf("    5 - Listar todas as informações de todos os filmes \n");
    printf("    6 - Listar todas as informações de um filme a partir de seu identificador \n");
    printf("    7 - Remover um filme a partir de seu identificador \n");
    printf("    Qualquer Outra Tecla - Sair do programa \n");
    printf("-------------------------------------------------------------------------------------------------------- \n \n");

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

    system("PAUSE");
    menu();
}

int main(void) {
    setlocale(LC_ALL, "Portuguese");

    printf("        Seja bem-vindo ao projeto de socket TCP - Servidor de streaming de vídeo \n");
    menu();
    printf("\n        Tchau! Até logo!        \n");

    return 0;
}