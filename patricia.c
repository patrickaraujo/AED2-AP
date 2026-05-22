/**
 * @file    patricia.c
 * @brief   Implementação da Árvore Patrícia (variante clássica de Sedgewick).
 *
 * Esta é uma versão **comentada e documentada** da implementação original.
 * O algoritmo segue o esquema clássico de Sedgewick (cap. 17 de "Algorithms
 * in C"), com nó-cabeça sentinela e back-edges.
 *
 * ## Reconhecendo back-edges
 *
 * A regra-chave para entender o código é:
 *
 *     "Ao descer de um nó com @c p->bit=k, todo filho normal terá
 *      bit > k. Se em algum momento eu chegar a um filho cujo bit é
 *      <= k, esse filho é uma BACK-EDGE — ele guarda a chave que
 *      'moraria' nessa posição da árvore lógica."
 *
 * Esse é o motivo de quase todas as funções recursivas levarem um
 * parâmetro @c bitAnterior: ele é o @c bit do PAI durante a descida,
 * e a comparação @c p->bit <= bitAnterior é como detectamos folhas.
 *
 * ## O nó-cabeça
 *
 * O nó-cabeça tem @c bit=-1 e @c esq aponta para a verdadeira raiz da
 * árvore (ou para si mesmo, se vazia). Como -1 é menor que qualquer
 * bit válido, qualquer ponteiro de volta para a cabeça é
 * automaticamente reconhecido como "sem mais nada aqui".
 *
 * @author  Patrick Araújo (versão original)
 * @note    Versão revisada com documentação Doxygen e comentários didáticos.
 */

#include "patricia.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Funções utilitárias internas
 * ====================================================================== */

/**
 * @brief Aloca e inicializa um nó da árvore.
 *
 * Função privada (static): só pode ser usada dentro deste arquivo.
 * Mantemos @c esq e @c dir como @c NULL — o chamador é responsável
 * por religá-los (em geral, formando back-edges).
 */
static ApNodo criaNodo(TipoItem item, int bit) {
    ApNodo n = (ApNodo) malloc(sizeof(Nodo));
    if (n == NULL) {
        fprintf(stderr, "Erro: sem memoria para criar nodo.\n");
        exit(EXIT_FAILURE);
    }

    n->item = item;
    n->bit  = bit;
    n->esq  = NULL;
    n->dir  = NULL;
    return n;
}

/**
 * @brief Verifica se uma chave é aceitável para esta árvore.
 *
 * Regras:
 *   1. Não pode ser @c ITEMNULO (reservado para o nó-cabeça).
 *   2. Deve caber em @c PATRICIA_BITS bits.
 *
 * A condição (2) é checada com um shift: se @c PATRICIA_BITS já cobre
 * todos os bits de @c TipoItem (caso extremo), qualquer valor cabe.
 */
static int itemValido(TipoItem v) {
    if (v == ITEMNULO)
        return 0;

    /* Caso de borda: PATRICIA_BITS cobre toda a largura do TipoItem.
       Não dá para fazer (1UL << PATRICIA_BITS) sem UB; aceitamos qualquer
       valor positivo. */
    if (PATRICIA_BITS >= (int)(sizeof(TipoItem) * CHAR_BIT))
        return 1;

    return v < (1UL << PATRICIA_BITS);
}

/**
 * @brief Extrai o bit na posição @p bit da chave @p v.
 *
 * Convenção: posição 0 = MSB dentro dos @c PATRICIA_BITS bits.
 * Para PATRICIA_BITS=5: posição 0 vale 16, posição 4 vale 1.
 */
static int digito(TipoItem v, int bit) {
    int deslocamento = PATRICIA_BITS - 1 - bit;
    return (int)((v >> deslocamento) & 1UL);
}

/**
 * @brief Acha a primeira posição em que duas chaves diferem.
 * @return Índice (0..PATRICIA_BITS-1), ou @c -1 se as chaves são iguais.
 *
 * Essa posição é o "bit decisivo" — onde um novo nó interno será
 * inserido durante a inserção de uma nova chave.
 */
static int primeiroBitDiferente(TipoItem a, TipoItem b) {
    for (int i = 0; i < PATRICIA_BITS; i++)
        if (digito(a, i) != digito(b, i))
            return i;
    return -1;
}

/* ========================================================================
 * Utilidades de exibição
 * ====================================================================== */

void itemParaBits(TipoItem v, char *saida, size_t tamanho) {
    /* Validação defensiva. */
    if (saida == NULL || tamanho < (size_t) PATRICIA_BITS + 1) {
        if (saida != NULL && tamanho > 0)
            saida[0] = '\0';
        return;
    }

    for (int i = 0; i < PATRICIA_BITS; i++)
        saida[i] = digito(v, i) ? '1' : '0';
    saida[PATRICIA_BITS] = '\0';
}

void escreveItem(TipoItem v) {
    /* Mapeamento didático: 1..26 → A..Z, qualquer outro valor → decimal. */
    if (v >= 1 && v <= 26)
        printf("%c", (char) ('A' + v - 1));
    else
        printf("%lu", v);
}

/* ========================================================================
 * Construção: nó-cabeça
 * ====================================================================== */

ApNodo criaArv(void) {
    /*
     * O nó-cabeça é um "sentinela": tem bit=-1 (menor que qualquer bit
     * válido) e aponta os dois filhos para si mesmo. A condição "árvore
     * vazia" se traduz em "raiz->esq == raiz".
     *
     * Por que apontar para si mesmo? Porque assim a regra de back-edge
     *     "p->bit <= bitAnterior é uma folha"
     * já cobre o caso de árvore vazia: ao chamar buscaR(raiz->esq, -1),
     * recebemos imediatamente raiz->esq = raiz, com bit=-1 <= -1,
     * sinalizando "essa é a 'folha'" (que na verdade é o próprio
     * sentinela).
     */
    ApNodo cabeca = criaNodo(ITEMNULO, -1);
    cabeca->esq = cabeca;
    cabeca->dir = cabeca;
    return cabeca;
}

/* ========================================================================
 * Busca
 * ====================================================================== */

/**
 * @brief Núcleo recursivo da busca.
 *
 * Desce pela árvore guiada pelos bits de @c k. Quando encontra uma
 * back-edge (@c p->bit <= bitAnterior), devolve o nó apontado por
 * ela — esse nó contém a única chave que poderia bater com @c k.
 */
static ApNodo buscaR(TipoItem k, ApNodo p, int bitAnterior) {
    /* Chegamos a uma folha lógica (back-edge ou sentinela). */
    if (p->bit <= bitAnterior)
        return p;

    /* Caso recursivo: desce pelo lado indicado pelo bit. */
    if (digito(k, p->bit) == 0)
        return buscaR(k, p->esq, p->bit);
    return buscaR(k, p->dir, p->bit);
}

ApNodo busca(TipoItem v, ApNodo raiz) {
    if (raiz == NULL || !itemValido(v))
        return NULL;

    /*
     * Começa a busca em raiz->esq (a raiz "real"), passando
     * bitAnterior = -1 (que é menor que qualquer bit válido, mas IGUAL
     * a bit do nó-cabeça — importante para reconhecer árvore vazia).
     */
    ApNodo p = buscaR(v, raiz->esq, -1);

    /* p é a única chave da árvore que pode ser igual a v. Verifica. */
    if (p != NULL && p->item == v)
        return p;
    return NULL;
}

/* ========================================================================
 * Inserção
 * ====================================================================== */

/**
 * @brief Núcleo recursivo da inserção.
 *
 * @param v             Chave a inserir.
 * @param p             Subárvore visitada (raiz da subárvore).
 * @param bitDiferente  Bit decisivo onde o novo nó deve ser inserido.
 * @param paiP          PAI lógico de @c p — necessário para detectar quando
 *                      a descida cruzou uma back-edge.
 *
 * Estratégia:
 *   1. Se @c p->bit >= bitDiferente OU se @c p é uma back-edge
 *      (@c p->bit <= paiP->bit): este é o ponto de inserção.
 *      Cria o novo nó com bit=@c bitDiferente, que aponta para @c p
 *      por um lado e para si mesmo pelo outro (formando uma back-edge
 *      para a nova chave).
 *   2. Senão, desce recursivamente pelo lado apropriado.
 */
static ApNodo insereR(TipoItem v, ApNodo p, int bitDiferente, ApNodo paiP) {
    /*
     * Caso-base: inserimos AQUI.
     * Duas condições disparam isso:
     *   (a) p->bit >= bitDiferente: passamos do nível certo, ou seja,
     *       o novo nó (com bit=bitDiferente, menor) vem antes de p.
     *   (b) p->bit <= paiP->bit: p é uma back-edge — chegamos numa
     *       folha lógica e não há mais nós para descer.
     */
    if (p->bit >= bitDiferente || p->bit <= paiP->bit) {
        ApNodo n = criaNodo(v, bitDiferente);

        /*
         * O novo nó precisa apontar para SI MESMO em um lado (back-edge
         * para a nova chave) e para a subárvore atual @c p no outro.
         * Qual lado vai onde? Depende do bit decisivo de @c v:
         *   - se digito(v, bitDiferente) == 0, @c v vai à esquerda
         *     (back-edge p/ si mesmo no @c esq), e @c p vai à direita.
         *   - se for 1, é o contrário.
         */
        if (digito(v, bitDiferente) == 0) {
            n->esq = n;   /* back-edge: "esta posição guarda a chave v" */
            n->dir = p;
        } else {
            n->esq = p;
            n->dir = n;   /* back-edge */
        }
        return n;
    }

    /* Caso recursivo: desce pelo lado indicado pelo bit do PRÓPRIO nó p. */
    if (digito(v, p->bit) == 0)
        p->esq = insereR(v, p->esq, bitDiferente, p);
    else
        p->dir = insereR(v, p->dir, bitDiferente, p);

    return p;
}

int insere(TipoItem v, ApNodo raiz) {
    if (raiz == NULL || !itemValido(v))
        return 0;

    /*
     * Primeiro, encontre a chave que CAIRIA na mesma posição que @c v
     * (descida normal pela árvore). Se for igual a @c v, é duplicata.
     */
    ApNodo p = buscaR(v, raiz->esq, -1);
    if (p->item == v)
        return 0;  /* duplicata silenciosa */

    /*
     * Caso especial: árvore vazia. Aqui p == raiz (o sentinela),
     * p->item == ITEMNULO. primeiroBitDiferente daria -1 pois as
     * chaves não diferem em nenhum bit válido. Tratamos como bit 0.
     */
    int bitDiferente = primeiroBitDiferente(v, p->item);
    if (bitDiferente < 0) {
        /* p == sentinela (ITEMNULO). Inserir como primeira chave. */
        if (p == raiz) {
            ApNodo n = criaNodo(v, 0);
            /* O novo nó é a raiz da árvore real e aponta para si
               mesmo nos dois lados? Não — apenas em UM lado
               (correspondente ao bit 0 da chave v). O outro lado
               aponta para o sentinela. */
            if (digito(v, 0) == 0) {
                n->esq = n;
                n->dir = raiz;
            } else {
                n->esq = raiz;
                n->dir = n;
            }
            raiz->esq = n;
            return 1;
        }
        return 0; /* nunca deve acontecer com itemValido garantido */
    }

    /* Caso geral: descer recursivamente. */
    raiz->esq = insereR(v, raiz->esq, bitDiferente, raiz);
    return 1;
}

/* ========================================================================
 * Impressão hierárquica
 * ======================================================================
 *
 * Strategy: pré-ordem com prefixos acumulados (estilo `tree(1)`).
 * Cada nó interno é mostrado com sua chave, bits e o bit testado;
 * cada back-edge é destacada com "-> folha" para deixar claro que
 * ali termina o caminho.
 */

/** Imprime "[chave | bits | bit=k]" do nó. */
static void imprimeNoCurto(ApNodo p) {
    char bits[PATRICIA_BITS + 1];

    if (p == NULL) {
        printf("NULL");
        return;
    }

    if (p->item == ITEMNULO && p->bit == -1) {
        printf("SENTINELA");
        return;
    }

    itemParaBits(p->item, bits, sizeof bits);
    printf("[");
    escreveItem(p->item);
    printf(" | %s | bit=%d]", bits, p->bit);
}

/**
 * @brief Auxiliar recursivo da impressão.
 *
 * @param p             Nó atual.
 * @param bitAnterior   bit do pai (para detectar back-edges).
 * @param raiz          Nó-cabeça (para detectar retorno ao sentinela).
 * @param prefixo       String acumulada de "|   " e "    " dos níveis acima.
 * @param aresta        Rótulo da aresta de chegada ("0", "1" ou "raiz.esq").
 * @param ultimo        1 se este é o último filho do pai (decide o conector).
 */
static void imprimeR(ApNodo p, int bitAnterior, ApNodo raiz,
                     const char *prefixo, const char *aresta, int ultimo) {
    char novoPrefixo[1024];

    /* Imprime o conector + o rótulo da aresta. */
    printf("%s%s%s", prefixo, ultimo ? "`-- " : "|-- ", aresta);

    /* Caso 1: a aresta volta ao nó-cabeça (sentinela). */
    if (p == raiz) {
        printf(" -> SENTINELA/NULL\n");
        return;
    }

    /* Caso 2: back-edge para uma folha lógica. */
    if (p->bit <= bitAnterior) {
        printf(" -> folha ");
        imprimeNoCurto(p);
        printf("\n");
        return;
    }

    /* Caso 3: nó interno — imprime e desce nos dois filhos. */
    printf(" ");
    imprimeNoCurto(p);
    printf("\n");

    /* Constrói o prefixo dos filhos: "|   " se houver mais irmãos,
       "    " se for o último. Cria a aparência de árvore conectada. */
    snprintf(novoPrefixo, sizeof novoPrefixo, "%s%s",
             prefixo, ultimo ? "    " : "|   ");

    imprimeR(p->esq, p->bit, raiz, novoPrefixo, "0", /*ultimo=*/0);
    imprimeR(p->dir, p->bit, raiz, novoPrefixo, "1", /*ultimo=*/1);
}

void imprimeArv(ApNodo raiz) {
    if (raiz == NULL) {
        printf("Arvore inexistente.\n");
        return;
    }

    printf("\nArvore Patricia\n");
    printf("Legenda: [chave | bits | bit testado]\n");
    printf("         aresta 0 = esquerda, aresta 1 = direita\n");
    printf("         folha = link externo/retorno da Patricia (back-edge)\n\n");

    /* Árvore vazia: o sentinela aponta para si mesmo. */
    if (raiz->esq == raiz) {
        printf("(arvore vazia)\n");
        return;
    }

    imprimeR(raiz->esq, -1, raiz, "", "raiz.esq", /*ultimo=*/1);
    printf("\n");
}

/* ========================================================================
 * Patrícia-sort (chaves em ordem crescente)
 * ====================================================================== */

/**
 * @brief Núcleo recursivo do sort.
 *
 * Pré-ordem que imprime apenas folhas lógicas (back-edges).
 * Como bit-0 está na esquerda e bit-1 na direita em todo nível,
 * a sequência de folhas em pré-ordem é a sequência ordenada das chaves.
 */
static void sortR(ApNodo p, int bitAnterior, ApNodo raiz) {
    if (p == NULL || p == raiz)
        return;

    if (p->bit <= bitAnterior) {
        /* Folha lógica: imprime a chave + binário. */
        char bits[PATRICIA_BITS + 1];
        itemParaBits(p->item, bits, sizeof bits);
        escreveItem(p->item);
        printf("(%s) ", bits);
        return;
    }

    sortR(p->esq, p->bit, raiz);
    sortR(p->dir, p->bit, raiz);
}

void sort(ApNodo raiz) {
    if (raiz == NULL || raiz->esq == raiz) {
        printf("(vazia)\n");
        return;
    }

    sortR(raiz->esq, -1, raiz);
    printf("\n");
}

/* ========================================================================
 * Liberação de memória
 * ====================================================================== */

/**
 * @brief Libera nós recursivamente, tratando back-edges como base.
 *
 * Como back-edges criam "ciclos" (um nó é alvo de uma back-edge mas
 * também é nó interno se o atravessamos antes via filho real), é
 * essencial só liberar cada nó UMA vez. A regra:
 *   - Se p é NULL, sentinela, ou back-edge (p->bit <= bitAnterior),
 *     retorne sem liberar. Isso garante que cada nó é liberado apenas
 *     no "caminho real" pela árvore — nunca como back-edge.
 */
static void freeSubArv(ApNodo p, int bitAnterior, ApNodo raiz) {
    if (p == NULL || p == raiz || p->bit <= bitAnterior)
        return;

    int bitAtual = p->bit;
    freeSubArv(p->esq, bitAtual, raiz);
    freeSubArv(p->dir, bitAtual, raiz);
    free(p);
}

/**
 * @brief Esvazia a árvore SEM liberar o nó-cabeça.
 *
 * Útil internamente para a operação de remoção por reconstrução.
 */
static void limpaArvSemRaiz(ApNodo raiz) {
    if (raiz == NULL) return;

    freeSubArv(raiz->esq, -1, raiz);
    raiz->esq = raiz;   /* Volta ao estado "vazia". */
    raiz->dir = raiz;
}

void freeArv(ApNodo raiz) {
    if (raiz == NULL) return;
    limpaArvSemRaiz(raiz);
    free(raiz);
}

/* ========================================================================
 * Remoção (por reconstrução — didática)
 * ====================================================================== */

/**
 * @brief Conta quantas folhas lógicas existem na (sub)árvore.
 */
static size_t contaFolhasR(ApNodo p, int bitAnterior, ApNodo raiz) {
    if (p == NULL || p == raiz)
        return 0;

    if (p->bit <= bitAnterior)
        return 1;  /* back-edge — uma folha lógica. */

    return contaFolhasR(p->esq, p->bit, raiz) +
           contaFolhasR(p->dir, p->bit, raiz);
}

/**
 * @brief Coleta as chaves de todas as folhas em um vetor.
 *
 * @c pos é o índice atual de inserção no vetor (passado por referência).
 */
static void coletaFolhasR(ApNodo p, int bitAnterior, ApNodo raiz,
                          TipoItem *itens, size_t *pos) {
    if (p == NULL || p == raiz)
        return;

    if (p->bit <= bitAnterior) {
        itens[(*pos)++] = p->item;
        return;
    }

    coletaFolhasR(p->esq, p->bit, raiz, itens, pos);
    coletaFolhasR(p->dir, p->bit, raiz, itens, pos);
}

/**
 * @brief Remoção por RECONSTRUÇÃO.
 *
 * Estratégia simples (didática):
 *   1. Verifica se a chave existe (busca normal).
 *   2. Coleta todas as chaves em um vetor.
 *   3. Esvazia a árvore.
 *   4. Reinsere todas as chaves EXCETO a que está sendo removida.
 *
 * Complexidade: O(n·b), onde n = nº de chaves e b = PATRICIA_BITS.
 * Bem mais cara que a remoção in-place clássica O(b), mas trivialmente
 * correta e fácil de revisar — o que vale ouro num contexto didático.
 */
int removeItem(TipoItem v, ApNodo raiz) {
    if (raiz == NULL || busca(v, raiz) == NULL)
        return 0;

    size_t n = contaFolhasR(raiz->esq, -1, raiz);
    TipoItem *itens = (TipoItem *) malloc(n * sizeof(TipoItem));
    if (itens == NULL) {
        fprintf(stderr, "Erro: sem memoria para remover item.\n");
        exit(EXIT_FAILURE);
    }

    size_t pos = 0;
    coletaFolhasR(raiz->esq, -1, raiz, itens, &pos);
    limpaArvSemRaiz(raiz);

    /* Reinsere tudo exceto v. */
    for (size_t i = 0; i < pos; i++)
        if (itens[i] != v)
            insere(itens[i], raiz);

    free(itens);
    return 1;
}
