/**
 * @file    main.c
 * @brief   Árvore B: Demonstração de uso.
 *
 * Insere uma sequência de chaves, exibe a árvore após algumas operações
 * e libera a memória ao final. Bom ponto de partida para testar
 * extensões (ler do stdin, ler de arquivo, etc).
 *
 * Compilação (sugerida): use o Makefile fornecido, ou:
 *     gcc -Wall -Wextra -std=c11 -o arvoreb main.c TAB.c
 *
 * @author Patrick Araújo: https://github.com/patrickaraujo
 * Referência 1: http://www2.dcc.ufmg.br/livros/algoritmos/cap6/slides/c/completo1/cap6.pdf
 * Referência 2: http://www2.dcc.ufmg.br/livros/algoritmos-edicao2/cap6/codigo/c/6.3a6.9-arvore-b.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "TAB.h"   /* observe: incluímos apenas o header, NÃO o .c */

int main(void) {
    TAB *raiz  = NULL;   /* árvore começa vazia */
    int  ordem = 2;      /* ordem t=2: cada página tem entre 2 e 4 chaves */

    /* ---- Inserções ---- */
    int valores[] = {3, 2, 4, 5, 6, 6, 7, 8, 9};  /* o 6 duplicado será rejeitado */
    int n = sizeof(valores) / sizeof(valores[0]);

    for (int i = 0; i < n; i++)
        insercao(&raiz, valores[i], ordem);

    printf("\n--- Apos as insercoes ---\n");
    imprimir(raiz, 0);

    /* ---- Remoção de uma chave em folha ---- */
    remocao(&raiz, 8, ordem);
    printf("\n--- Apos remover 8 ---\n");
    imprimir(raiz, 0);

    /* ---- Remoção de uma chave em página interna ---- */
    remocao(&raiz, 3, ordem);
    printf("\n--- Apos remover 3 ---\n");
    imprimir(raiz, 0);

    /* ---- Busca ---- */
    int alvo = 12;
    if (busca(raiz, alvo))
        printf("\nNumero %d esta na arvore.\n", alvo);
    else
        printf("\nNumero %d nao esta na arvore.\n", alvo);

    alvo = 5;
    if (busca(raiz, alvo))
        printf("Numero %d esta na arvore.\n", alvo);
    else
        printf("Numero %d nao esta na arvore.\n", alvo);

    /* ---- Liberação de memória ---- */
    liberarArvore(raiz);
    raiz = NULL;

    return EXIT_SUCCESS;
}
