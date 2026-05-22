/**
 * @file    patricia.h
 * @brief   Interface pública da Árvore Patrícia (variante clássica).
 *
 * Implementação acadêmica da **Árvore Patrícia** (PATRICIA = "Practical
 * Algorithm To Retrieve Information Coded In Alphanumeric"), proposta
 * por Morrison em 1968 e popularizada por Sedgewick. É uma variante
 * comprimida da trie binária que indexa chaves comparando-as bit a bit.
 *
 * ## Variante implementada
 *
 * Esta é a variante **clássica de Sedgewick**, com duas características
 * distintivas:
 *
 *   1. **Nó-cabeça sentinela.** A árvore não começa em um nó comum: ela
 *      tem um nó-cabeça permanente com @c bit=-1 e @c item=ITEMNULO,
 *      cujo @c esq aponta para a raiz "real" (ou para si mesmo, se a
 *      árvore está vazia). Isso simplifica MUITO os casos de borda:
 *      a árvore "vazia" é apenas o nó-cabeça apontando para si mesmo,
 *      e a inserção do primeiro elemento não precisa de tratamento
 *      especial.
 *
 *   2. **Back-edges (links de retorno).** Diferente de uma trie comum,
 *      a Patrícia clássica usa **um único tipo de nó** para tudo —
 *      não há "folhas separadas". Em vez disso, quando um ponteiro
 *      "desceria" para uma folha, ele aponta de volta para um nó já
 *      existente da árvore (a back-edge). Identificamos uma back-edge
 *      verificando se @c filho->bit <= pai->bit: como o bit cresce
 *      monotonicamente durante uma descida normal, qualquer ponteiro
 *      que aponte para um bit MENOR (ou IGUAL) é necessariamente uma
 *      back-edge — e o nó apontado contém a chave que "moraria" lá.
 *
 * ## Configuração
 *
 * O número de bits da chave é parametrizável em tempo de compilação:
 *
 * @code
 *   gcc -DPATRICIA_BITS=8 main.c patricia.c -o patricia
 * @endcode
 *
 * O default é 5 bits (suporta A..Z mapeados para 1..26), por ser o
 * exemplo mais comum em sala de aula. Use 8 para um @c char inteiro,
 * 32 para um @c int, etc. Vale qualquer valor de 1 até o número de
 * bits de @c TipoItem.
 *
 * ## Convenção de numeração de bits
 *
 * **Posição 0 = bit mais significativo** dentro dos @c PATRICIA_BITS
 * bits da chave. Para @c PATRICIA_BITS=5 e chave @c 9 (binário 01001):
 * posição 0 vale 0, posição 1 vale 1, posição 2 vale 0, posição 3
 * vale 0, posição 4 vale 1.
 *
 * @author  Patrick Araújo
 */

#ifndef PATRICIA_H
#define PATRICIA_H

#include <stddef.h>

/* ========================================================================
 * Configuração
 * ====================================================================== */

/**
 * @brief Número de bits usados em cada chave.
 *
 * Pode ser redefinido pela linha de compilação (@c -DPATRICIA_BITS=N).
 * Default = 5 (suficiente para 1..26, ou seja, A..Z).
 */
#ifndef PATRICIA_BITS
#define PATRICIA_BITS 5
#endif

/**
 * @brief Valor reservado que NUNCA pode ser inserido na árvore.
 *
 * Usado pelo nó-cabeça sentinela e como retorno de erro. Insira chaves
 * a partir de 1, não de 0.
 */
#define ITEMNULO 0UL

/* ========================================================================
 * Tipos
 * ====================================================================== */

/**
 * @brief Tipo das chaves armazenadas na árvore.
 *
 * Pode ser trocado por outro tipo inteiro sem sinal (e ajustando
 * @c PATRICIA_BITS proporcionalmente).
 */
typedef unsigned long TipoItem;

/** @brief Forward declaration do nó (ponteiro). */
typedef struct Nodo *ApNodo;

/**
 * @brief Nó da Árvore Patrícia (clássica de Sedgewick).
 *
 * O MESMO struct serve para nó-cabeça, nós internos e (logicamente)
 * folhas. O papel de cada nó é determinado pelo contexto:
 *
 *   - **Nó-cabeça (sentinela):** @c bit=-1, @c item=ITEMNULO. Único na
 *     árvore. @c esq aponta para a raiz "real" (ou para si mesmo).
 *   - **Nó interno:** @c bit indica qual bit testar para escolher entre
 *     @c esq (bit=0) e @c dir (bit=1). @c item guarda a chave que
 *     "subiu" durante a inserção — não é usada na busca, mas serve
 *     como chave de comparação quando esse nó é alvo de uma back-edge.
 *   - **Folha (logicamente):** representada por uma back-edge. Não
 *     existe um @c struct específico — uma "folha" é o nó interno
 *     para o qual aponta a back-edge.
 */
typedef struct Nodo {
    TipoItem item;   /**< Chave armazenada (significativa quando o nó é alvo de back-edge). */
    ApNodo   esq;    /**< Filho esquerdo — caminho para bit = 0. */
    ApNodo   dir;    /**< Filho direito — caminho para bit = 1. */
    int      bit;    /**< Índice do bit testado (0 a PATRICIA_BITS-1). @c -1 só na cabeça. */
} Nodo;

/* ========================================================================
 * Construção / destruição
 * ====================================================================== */

/**
 * @brief Cria uma árvore Patrícia vazia (apenas o nó-cabeça).
 * @return Ponteiro para o nó-cabeça sentinela. Use este ponteiro como
 *         a "raiz" em todas as outras funções; ele é estável (não muda
 *         após inserções/remoções).
 *
 * O nó-cabeça é inicializado com @c esq=dir=ele_mesmo, sinalizando
 * "árvore vazia". As funções de inserção, busca e remoção sabem
 * detectar esse estado.
 */
ApNodo criaArv(void);

/**
 * @brief Libera toda a memória da árvore, incluindo o nó-cabeça.
 * @param raiz  Ponteiro para o nó-cabeça (pode ser @c NULL).
 *
 * Após a chamada, o ponteiro do chamador continua válido sintaticamente
 * mas referencia memória liberada — atribua @c NULL a ele.
 */
void freeArv(ApNodo raiz);

/* ========================================================================
 * Operações principais
 * ====================================================================== */

/**
 * @brief Busca uma chave na árvore.
 * @param v     Chave a procurar.
 * @param raiz  Nó-cabeça.
 * @return Ponteiro para o nó que contém a chave, ou @c NULL se a
 *         chave não estiver na árvore (ou se a árvore é inválida).
 *
 * Complexidade: O(b), onde b = @c PATRICIA_BITS.
 */
ApNodo busca(TipoItem v, ApNodo raiz);

/**
 * @brief Insere uma chave na árvore.
 * @param v     Chave a inserir. Deve ser != @c ITEMNULO e caber em
 *              @c PATRICIA_BITS bits.
 * @param raiz  Nó-cabeça.
 * @return 1 se inseriu com sucesso; 0 se a chave já existe ou é inválida.
 *
 * Chaves duplicadas são rejeitadas silenciosamente (retorna 0).
 * Complexidade: O(b).
 */
int insere(TipoItem v, ApNodo raiz);

/**
 * @brief Remove uma chave da árvore.
 * @param v     Chave a remover.
 * @param raiz  Nó-cabeça.
 * @return 1 se removeu; 0 se a chave não estava na árvore.
 *
 * **Implementação didática:** a remoção é feita por **reconstrução** —
 * a árvore inteira é desmontada e reconstruída sem o elemento alvo.
 * É O(n·b), bem mais cara que o teórico O(b), mas simples de entender
 * e provadamente correta. Para uma implementação "de produção" você
 * usaria a remoção in-place clássica, mais complexa.
 */
int removeItem(TipoItem v, ApNodo raiz);

/* ========================================================================
 * Travessias e impressão
 * ====================================================================== */

/**
 * @brief Imprime a árvore em formato hierárquico com destaque para a
 *        estrutura Patrícia (bits e back-edges).
 * @param raiz  Nó-cabeça.
 *
 * Cada nó interno é mostrado como @c [chave | bits | bit=k], e cada
 * back-edge (folha lógica) é marcada explicitamente como
 * @c "-> folha [...]" para facilitar a leitura.
 *
 * Exemplo de saída (chaves A,S,E,R,C,H,I,N com PATRICIA_BITS=5):
 *
 * @verbatim
 *   `-- raiz.esq [S | 10011 | bit=0]
 *       |-- 0 [H | 01000 | bit=1]
 *       |   |-- 0 [E | 00101 | bit=2]
 *       |   |   |-- 0 [C | 00011 | bit=3]
 *       |   |   |   |-- 0 [A | 00001 | bit=4]
 *       |   |   |   |   |-- 0 -> SENTINELA/NULL
 *       |   |   |   |   `-- 1 -> folha [A | 00001 | bit=4]
 *       |   |   |   `-- 1 -> folha [C | 00011 | bit=3]
 *       |   |   `-- 1 -> folha [E | 00101 | bit=2]
 *       ...
 * @endverbatim
 */
void imprimeArv(ApNodo raiz);

/**
 * @brief Imprime as chaves da árvore em ORDEM CRESCENTE (apenas folhas lógicas).
 * @param raiz  Nó-cabeça.
 *
 * Como as chaves "menores em bits" ficam à esquerda e as "maiores" à
 * direita em cada nível, uma travessia pré-ordem que só imprime as
 * folhas (back-edges) produz a sequência ordenada — sem usar nenhum
 * algoritmo de ordenação tradicional. Isso é a "Patrícia-sort".
 */
void sort(ApNodo raiz);

/* ========================================================================
 * Utilidades de exibição
 * ====================================================================== */

/**
 * @brief Imprime a representação da chave em @c stdout.
 *
 * Por padrão: se @c 1 <= v <= 26, imprime a letra correspondente
 * (A..Z); caso contrário, imprime o número decimal. Útil para examples
 * pedagógicos com letras do alfabeto.
 */
void escreveItem(TipoItem v);

/**
 * @brief Preenche @p saida com a representação binária da chave (MSB→LSB).
 * @param v       Chave.
 * @param saida   Buffer de destino. Será uma string terminada em '\0'.
 * @param tamanho Tamanho disponível em @p saida (deve ser >= PATRICIA_BITS+1).
 *
 * Se @p tamanho for insuficiente, @p saida recebe apenas '\0' (string vazia).
 */
void itemParaBits(TipoItem v, char *saida, size_t tamanho);

#endif /* PATRICIA_H */
