/**
 * @file    main.c
 * @brief   Programa interativo de demonstração da Árvore Patrícia.
 *
 * Pré-carrega o exemplo clássico de aula (chaves A, S, E, R, C, H, I, N)
 * e oferece um menu de operações: inserir, buscar, remover, imprimir
 * e listar chaves ordenadas.
 *
 * Compilação (sugerida): use o Makefile, ou:
 *     gcc -Wall -Wextra -std=c11 -o patricia main.c patricia.c
 *
 * Para usar outro tamanho de chave (em bits):
 *     gcc -DPATRICIA_BITS=8 -o patricia main.c patricia.c
 *
 * @author Patrick Araújo
 */

#include "patricia.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Leitura de entrada
 * ====================================================================== */

/**
 * @brief Lê uma chave do stdin, aceitando letra (A..Z) ou número.
 * @param saida  Endereço onde gravar a chave lida.
 * @return 1 em sucesso, 0 em entrada inválida.
 *
 * Letras são automaticamente convertidas para 1..26 (A=1, B=2, ..., Z=26),
 * coerente com a função @c escreveItem() do módulo @c patricia.
 */
static int lerItem(TipoItem *saida) {
    char texto[64];

    if (scanf("%63s", texto) != 1)
        return 0;

    /* Caso 1: caractere único alfabético → mapeia para 1..26. */
    if (strlen(texto) == 1 && isalpha((unsigned char) texto[0])) {
        *saida = (TipoItem) (toupper((unsigned char) texto[0]) - 'A' + 1);
        return 1;
    }

    /* Caso 2: número decimal. strtoul reporta o ponto de parada em fim. */
    char *fim;
    unsigned long valor = strtoul(texto, &fim, 10);
    if (*fim != '\0')
        return 0;   /* texto tinha conteúdo além do número */

    *saida = (TipoItem) valor;
    return 1;
}

/** @brief Imprime a chave seguida de sua representação binária. */
static void imprimeItemComBits(TipoItem v) {
    char bits[PATRICIA_BITS + 1];
    itemParaBits(v, bits, sizeof bits);
    escreveItem(v);
    printf(" (%s)", bits);
}

/* ========================================================================
 * Menu
 * ====================================================================== */

static void menu(void) {
    printf("\nMenu\n");
    printf("1 - Inserir chave\n");
    printf("2 - Buscar chave\n");
    printf("3 - Remover chave\n");
    printf("4 - Imprimir arvore\n");
    printf("5 - Listar chaves ordenadas\n");
    printf("0 - Sair\n");
    printf("Opcao: ");
}

/* ========================================================================
 * Programa principal
 * ====================================================================== */

int main(void) {
    /* Cria a árvore (apenas o nó-cabeça sentinela). */
    ApNodo raiz = criaArv();

    /*
     * Exemplo de aula: A(1) S(19) E(5) R(18) C(3) H(8) I(9) N(14).
     * Sequência clássica do livro do Sedgewick para mostrar a evolução
     * da estrutura à medida que chaves são inseridas em ordem variada.
     */
    TipoItem exemplo[] = {1, 19, 5, 18, 3, 8, 9, 14};
    size_t qtdExemplo  = sizeof exemplo / sizeof exemplo[0];

    printf("Exemplo inicial da aula: A, S, E, R, C, H, I, N\n");
    for (size_t i = 0; i < qtdExemplo; i++)
        insere(exemplo[i], raiz);

    imprimeArv(raiz);

    /* Loop de menu. */
    int opcao;
    do {
        menu();
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida. Encerrando.\n");
            break;
        }

        TipoItem v;
        ApNodo encontrado;

        switch (opcao) {
            case 1:  /* Inserir */
                /* Calcula o maior valor representável para a mensagem. */
                printf("Digite uma letra A-Z ou um numero de 1 a %lu: ",
                       (PATRICIA_BITS >= (int)(sizeof(TipoItem) * 8))
                           ? ~0UL
                           : ((1UL << PATRICIA_BITS) - 1));
                if (!lerItem(&v)) {
                    printf("Entrada invalida.\n");
                    break;
                }
                if (insere(v, raiz)) {
                    printf("Inserido: ");
                    imprimeItemComBits(v);
                    printf("\n");
                } else {
                    printf("Nao inserido. A chave pode ja existir ou ser invalida.\n");
                }
                break;

            case 2:  /* Buscar */
                printf("Digite a chave para buscar: ");
                if (!lerItem(&v)) {
                    printf("Entrada invalida.\n");
                    break;
                }
                encontrado = busca(v, raiz);
                if (encontrado != NULL) {
                    printf("Encontrado: ");
                    imprimeItemComBits(encontrado->item);
                    printf(", bit do nodo = %d\n", encontrado->bit);
                } else {
                    printf("Chave nao encontrada.\n");
                }
                break;

            case 3:  /* Remover */
                printf("Digite a chave para remover: ");
                if (!lerItem(&v)) {
                    printf("Entrada invalida.\n");
                    break;
                }
                if (removeItem(v, raiz)) {
                    printf("Removido: ");
                    imprimeItemComBits(v);
                    printf("\n");
                } else {
                    printf("Chave nao encontrada.\n");
                }
                break;

            case 4:  /* Imprimir árvore inteira */
                imprimeArv(raiz);
                break;

            case 5:  /* Listar chaves ordenadas */
                printf("Chaves ordenadas: ");
                sort(raiz);
                break;

            case 0:
                printf("Encerrando.\n");
                break;

            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (opcao != 0);

    /* Libera toda a memória. */
    freeArv(raiz);
    raiz = NULL;

    return EXIT_SUCCESS;
}
