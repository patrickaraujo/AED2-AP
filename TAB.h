/**
 * @file    TAB.h
 * @brief   Interface pública da Árvore B (T.A.B.).
 *
 * Implementação acadêmica de uma Árvore B parametrizada por sua
 * ordem @c t. Cada página (nó) armazena:
 *   - de @c t a @c 2t chaves (a raiz pode ter de 1 a @c 2t);
 *   - de @c t+1 a @c 2t+1 ponteiros para filhos (ou 0 em folhas).
 *
 * Propriedades garantidas pela implementação:
 *   1. Todas as folhas estão no mesmo nível.
 *   2. As chaves dentro de cada página estão em ordem crescente.
 *   3. A subárvore @c filhos[i] contém chaves estritamente entre
 *      @c chaves[i-1] e @c chaves[i].
 *
 * Referências utilizadas no projeto original:
 *   - Ziviani, "Projeto de Algoritmos", cap. 6 (UFMG).
 *
 * @author  Patrick Araújo
 */

#ifndef TAB_H_INCLUDED
#define TAB_H_INCLUDED

/* ========================================================================
 * Estrutura de dados
 * ====================================================================== */

/**
 * @brief Página (nó) de uma Árvore B.
 *
 * Os vetores @c chaves e @c filhos são alocados dinamicamente em
 * função da ordem @c t informada em @c novaPagina(), com capacidades
 * @c 2t e @c 2t+1 respectivamente.
 */
typedef struct AB {
    int           n_chaves;   /**< Quantidade atual de chaves na página. */
    int          *chaves;     /**< Vetor de chaves ordenadas (tamanho 2t).        */
    struct AB   **filhos;     /**< Vetor de ponteiros para filhos (tamanho 2t+1). */
} TAB;

/* ========================================================================
 * Construção / destruição
 * ====================================================================== */

/**
 * @brief Aloca e inicializa uma nova página vazia.
 * @param ordem  Ordem @c t da árvore.
 * @return Ponteiro para a página recém-criada, ou aborta em caso de
 *         falha de alocação.
 *
 * Todos os filhos são inicializados como @c NULL, sinalizando que a
 * página recém-criada é uma folha.
 */
TAB *novaPagina(int ordem);

/**
 * @brief Libera recursivamente toda a memória ocupada pela árvore.
 * @param raiz  Raiz da árvore (pode ser @c NULL).
 *
 * Após a chamada, o ponteiro original do chamador continua válido
 * sintaticamente, mas referencia memória liberada — o chamador deve
 * atribuir @c NULL a ele.
 */
void liberarArvore(TAB *raiz);

/* ========================================================================
 * Operações da Árvore B
 * ====================================================================== */

/**
 * @brief Insere uma chave na árvore, mantendo todas as propriedades da Árvore B.
 * @param raiz   Endereço do ponteiro para a raiz (pode ser modificado se a
 *               árvore crescer em altura).
 * @param chave  Valor inteiro a inserir. Chaves duplicadas são rejeitadas
 *               com mensagem de erro em @c stderr.
 * @param ordem  Ordem @c t da árvore.
 */
void insercao(TAB **raiz, int chave, int ordem);

/**
 * @brief Remove uma chave da árvore, mantendo todas as propriedades da Árvore B.
 * @param raiz   Endereço do ponteiro para a raiz (pode ser modificado se a
 *               árvore diminuir em altura).
 * @param chave  Valor inteiro a remover. Se a chave não existir, é emitida
 *               mensagem em @c stderr.
 * @param ordem  Ordem @c t da árvore.
 */
void remocao(TAB **raiz, int chave, int ordem);

/**
 * @brief Busca uma chave na árvore.
 * @param raiz   Raiz (ou subárvore) onde buscar.
 * @param chave  Valor a procurar.
 * @return Ponteiro para a página que contém a chave, ou @c NULL se
 *         a chave não estiver na árvore.
 */
TAB *busca(TAB *raiz, int chave);

/**
 * @brief Imprime a árvore em ordem (in-order traversal), indentada por nível.
 * @param raiz    Raiz (ou subárvore) a imprimir.
 * @param altura  Profundidade atual (use @c 0 ao chamar pela raiz).
 *
 * A saída é puramente textual em @c stdout; útil para depuração.
 */
void imprimir(TAB *raiz, int altura);

/* ========================================================================
 * Funções auxiliares (uso interno, expostas para fins didáticos)
 * ====================================================================== */

/**
 * @brief Insere uma chave (e ponteiro associado) já em uma página sem cisão.
 * @param pagina   Página de destino — deve ter espaço (@c n_chaves < 2t).
 * @param novoFilho  Subárvore que ficará à direita da nova chave.
 * @param chave    Chave a inserir.
 *
 * Função utilitária que apenas desloca elementos para manter a ordem
 * crescente das chaves. Não trata overflow.
 */
void inserePagina(TAB *pagina, TAB *novoFilho, int chave);

/**
 * @brief Núcleo recursivo de @c insercao(): desce até a folha apropriada
 *        e, ao retornar, propaga eventuais cisões para cima.
 *
 * @param paginaAtual  Endereço da página visitada na recursão.
 * @param paginaDir    [saída] Página direita resultante de uma eventual cisão.
 * @param promoveu     [saída] 1 se há chave a ser promovida ao nível superior; 0 caso contrário.
 * @param chavePromov  [saída] Chave promovida quando há cisão.
 * @param chave        Chave original sendo inserida.
 * @param ordem        Ordem @c t da árvore.
 */
void efetuaInsercao(TAB **paginaAtual, TAB **paginaDir, int *promoveu,
                    int *chavePromov, int chave, int ordem);

/**
 * @brief Núcleo recursivo de @c remocao(): desce até localizar a chave,
 *        executa a remoção (substituindo por antecessor quando necessário)
 *        e propaga ajustes de underflow para cima.
 *
 * @param paginaAtual  Endereço da página visitada na recursão.
 * @param underflow    [saída] 1 se a página atual ficou com menos de @c t chaves.
 * @param chave        Chave a remover.
 * @param ordem        Ordem @c t da árvore.
 */
void efetuaRemocao(TAB **paginaAtual, int *underflow, int chave, int ordem);

/**
 * @brief Substitui uma chave interna pelo seu antecessor (maior chave da
 *        subárvore esquerda), descendo sempre pelo filho mais à direita.
 *
 * @param paginaAlvo   Página que contém a chave a ser substituída.
 * @param subarvore    Subárvore esquerda da chave alvo.
 * @param underflow    [saída] 1 se a folha de origem ficou com menos de @c t chaves.
 * @param indice       Posição (1-based) da chave alvo em @c paginaAlvo.
 * @param ordem        Ordem @c t da árvore.
 */
void antecessor(TAB *paginaAlvo, TAB *subarvore, int *underflow,
                int indice, int ordem);

/**
 * @brief Restaura a propriedade de ocupação mínima após uma remoção,
 *        por redistribuição (com irmão à esquerda ou à direita) ou
 *        por fusão de páginas.
 *
 * @param paginaFilha  Endereço do filho que sofreu underflow.
 * @param pai          Página pai imediata de @c paginaFilha.
 * @param underflow    [saída] 1 se o pai ficou abaixo do mínimo após o ajuste.
 * @param indiceP      Posição do filho em @c pai->filhos[].
 * @param ordem        Ordem @c t da árvore.
 */
void reconstitui(TAB **paginaFilha, TAB *pai, int *underflow,
                 int indiceP, int ordem);

#endif /* TAB_H_INCLUDED */
