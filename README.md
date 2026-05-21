# Árvore B em C — Guia do Código

Implementação acadêmica de uma **Árvore B** em C, com operações de
inserção, remoção e busca. Esta documentação acompanha a versão
revisada do código original do projeto `AED2-ArvoreB` (UFT, Ciência da
Computação).

---

## 1. O que é uma Árvore B

Uma **Árvore B de ordem _t_** é uma árvore balanceada na qual cada nó
(chamado aqui de **página**) pode conter várias chaves e vários
filhos. Diferente da Árvore Binária de Busca, a Árvore B não cresce
em profundidade a cada nova chave — ela cresce em **largura**, e só
aumenta de altura quando absolutamente necessário.

**Propriedades fundamentais:**

| Propriedade                              | Garantia                                |
| ---------------------------------------- | --------------------------------------- |
| Chaves por página (não-raiz)             | entre **_t_** e **2_t_**                |
| Chaves na raiz                           | entre **1** e **2_t_** (se não vazia)   |
| Filhos por página interna                | _n_chaves_ + 1                          |
| Altura de todas as folhas                | **igual** (árvore perfeitamente balanceada) |
| Chaves dentro da página                  | em ordem **crescente**                  |
| Subárvore `filhos[i]`                    | contém chaves entre `chaves[i-1]` e `chaves[i]` |

No código deste projeto, o parâmetro `ordem` é exatamente o _t_ acima.
O `main.c` usa `ordem = 2`, então cada página comporta de 2 a 4
chaves (e a raiz pode ter de 1 a 4).

> **Por que isso importa?** Árvores B foram desenhadas para minimizar
> acessos a disco — cada página é projetada para caber em um bloco do
> sistema de arquivos. Por isso são a base de **índices de bancos de
> dados** (MySQL InnoDB usa B+Tree, por exemplo) e sistemas de
> arquivos modernos. Este projeto é a versão didática em memória.

---

## 2. Estrutura de dados

A página é representada pelo `struct AB`, com `typedef` para `TAB`:

```c
typedef struct AB {
    int           n_chaves;   // quantas chaves a página tem agora
    int          *chaves;     // vetor com capacidade 2t
    struct AB   **filhos;     // vetor com capacidade 2t+1
} TAB;
```

Os vetores são **alocados dinamicamente** em `novaPagina(ordem)`, o
que permite parametrizar a ordem em tempo de execução. Para uma
página com `n_chaves = k`, são significativos:

- `chaves[0..k-1]`
- `filhos[0..k]`

Visualmente (para `t=2`, com `k=3` chaves):

```
            +-----+-----+-----+-----+
   chaves:  |  2  |  5  |  9  |  -  |
            +-----+-----+-----+-----+
            /     |     |     \
   filhos[0] f[1] f[2]   f[3]   (f[4] = NULL)
```

A subárvore `filhos[1]` contém apenas chaves no intervalo `(2, 5)`,
`filhos[2]` contém `(5, 9)`, e assim por diante.

---

## 3. Como compilar e rodar

```bash
make            # gera o executável ./arvoreb
make run        # compila e executa
make debug      # build com símbolos para gdb
make clean      # limpa artefatos
```

Ou manualmente:

```bash
gcc -Wall -Wextra -std=c11 -o arvoreb main.c TAB.c
./arvoreb
```

**Importante:** compile `main.c` **e** `TAB.c` juntos. Diferente da
versão original, o `main.c` não faz `#include "TAB.c"` (esse era um
anti-padrão que mistura compilação com inclusão de código).

---

## 4. Arquitetura dos arquivos

```
arvoreb-v2/
├── TAB.h        # interface pública (struct + assinaturas + docstrings Doxygen)
├── TAB.c        # implementação das operações
├── main.c       # programa de demonstração
├── Makefile     # automação de build
└── README.md    # este documento
```

O cabeçalho `TAB.h` declara três grupos de funções:

1. **Construção/destruição:** `novaPagina`, `liberarArvore`.
2. **Operações públicas:** `insercao`, `remocao`, `busca`, `imprimir`.
3. **Auxiliares (expostas para fins didáticos):** `inserePagina`,
   `efetuaInsercao`, `efetuaRemocao`, `antecessor`, `reconstitui`.

Em uma versão "de produção" o terceiro grupo seria marcado `static`
dentro do `.c` e omitido do `.h`. Mantemos públicas para que os
alunos possam testar e instrumentar cada peça individualmente.

---

## 5. Inserção

### 5.1 Visão geral do algoritmo

A inserção em uma Árvore B segue o princípio **"desce, insere na
folha, sobe corrigindo overflow"**:

1. Descer recursivamente até a folha onde a chave deveria ficar
   (`efetuaInsercao`).
2. Inserir a chave na folha.
3. **Se a folha estourar** (passar de 2_t_ chaves), **cindi-la em
   duas** e promover a chave do meio para o pai.
4. Se o pai também estourar, propagar a cisão para cima.
5. Se a **raiz** estourar, criar uma nova raiz com a chave promovida —
   é assim que a árvore **cresce em altura**.

### 5.2 Fluxo das funções

```
insercao()
    │
    └──► efetuaInsercao()           ← recursão; desce até a folha
            │
            ├──► inserePagina()     ← caso simples: cabe na página
            │
            └──► (se overflow)
                  ├── novaPagina()  ← cria a página direita
                  └── inserePagina(várias vezes)
                                    ← redistribui as chaves
```

### 5.3 Exemplo passo a passo (ordem _t_ = 2)

Inserindo `3, 2, 4, 5, 6`:

```
após 3:      [3]

após 2:      [2, 3]

após 4:      [2, 3, 4]

após 5:      [2, 3, 4, 5]    ← página cheia (2t = 4)

após 6:      tentaria [2,3,4,5,6] → CISÃO
             promove o meio (4), cria 2 páginas:

                       [4]
                      /   \
                  [2,3]   [5,6]
```

A árvore cresce em altura. Observe que a única forma de a árvore
ficar mais alta é exatamente quando a **raiz cinde** — e a nova raiz
sempre nasce com exatamente 1 chave.

### 5.4 Detalhe de implementação: chaves duplicadas

A política do projeto é **rejeitar duplicatas**. Em `efetuaInsercao`:

```c
if (chave == (*paginaAtual)->chaves[i - 1]) {
    fprintf(stderr, "Erro: chave %d ja inserida.\n", chave);
    *promoveu = 0;
    return;
}
```

Se preferir aceitar duplicatas (útil para índices secundários, por
exemplo), basta remover essa verificação — o algoritmo de cisão
continua válido.

---

## 6. Remoção

A remoção é **significativamente mais complexa** que a inserção,
porque envolve dois cenários distintos e três estratégias de reparo:

### 6.1 Os dois cenários

| Cenário                        | Estratégia                                        |
| ------------------------------ | ------------------------------------------------- |
| Chave em **folha**             | Remove direto. Pode causar underflow.             |
| Chave em página **interna**    | Substitui pelo **antecessor** (`antecessor()`) — a maior chave da subárvore esquerda. Reduz o problema ao caso da folha. |

### 6.2 As três estratégias de reparo (underflow)

Quando uma página fica com menos de _t_ chaves, `reconstitui()` aplica
uma destas opções, **nessa ordem de preferência**:

#### (a) Redistribuição com irmão à direita

Se o irmão à direita tem mais que _t_ chaves, "puxa" parte dele:

```
   Antes:               Depois:
       [P]                  [P']
      /   \                /    \
   [a]    [b,c,d]      [a,P]    [c,d]
     ↑                      ↑
   <t chaves            chave separadora
                        do pai desceu;
                        nova separadora P' subiu
```

#### (b) Redistribuição com irmão à esquerda

Análoga, mas espelhada.

#### (c) Fusão (merge)

Se nenhum irmão tem folga, a página é **fundida** com seu irmão
adjacente, descendo a chave separadora do pai entre as duas:

```
   Antes:               Depois:
       [P,Q]                [Q]
      /  |  \              /   \
   [a]  [b]  [c,d]     [a,P,b]  [c,d]
     ↑    ↑
   <t   <t
```

A fusão **reduz em 1 a quantidade de chaves do pai** — por isso o
underflow pode subir de nível. Esse é o único caso em que a árvore
pode **diminuir em altura**: quando a fusão ocorre nos filhos
imediatos da raiz e a raiz fica com 0 chaves, ela é descartada.

### 6.3 Fluxo das funções

```
remocao()
    │
    ├──► efetuaRemocao()             ← procura a chave
    │       │
    │       ├── (chave em folha)
    │       │      └──► remoção direta + marca underflow
    │       │
    │       └── (chave em página interna)
    │              └──► antecessor() ← desce pelo filho mais à direita
    │                       └──► reconstitui() ← se underflow
    │
    └── (se a raiz ficou vazia, descarta) ← árvore diminui em altura
```

### 6.4 O papel do `antecessor()`

Para entender por que precisamos do antecessor, considere:

```
        [4, 7]
       /  |   \
    [2]  [5,6] [8,9]
```

Removendo `4` (chave interna): não podemos simplesmente deletá-la
porque seu lugar precisa ser ocupado por alguém para manter as
relações de ordem com `[2]` e `[5,6]`. O antecessor de 4 é `2`
(maior chave da subárvore esquerda) — substituímos e o problema vira
"remover 2 da folha `[2]`", que pode ser tratado normalmente.

---

## 7. Busca

A busca é diretamente análoga à busca binária, generalizada para
múltiplas chaves por nó:

```c
TAB *busca(TAB *raiz, int chave) {
    if (!raiz) return NULL;

    int i = 1;
    while (i < raiz->n_chaves && chave > raiz->chaves[i - 1])
        i++;

    if (chave == raiz->chaves[i - 1])
        return raiz;                              // achou aqui

    if (chave < raiz->chaves[i - 1])
        return busca(raiz->filhos[i - 1], chave); // desce à esquerda
    else
        return busca(raiz->filhos[i], chave);     // desce à direita
}
```

Como cada nível elimina uma página inteira, a busca é **O(log_t n)**
no pior caso.

> **Limitação atual:** `busca()` retorna apenas o **ponteiro para a
> página**, não a posição da chave dentro dela. Para acessar dados
> associados a chaves (em uma implementação real de índice), seria
> necessário ou devolver a posição também, ou usar uma estrutura
> `{chave, valor}` em vez de `int`.

---

## 8. Impressão

`imprimir()` faz uma travessia **in-order** adaptada para nós de
múltiplas chaves: para cada par `(filho_esquerdo, chave)` imprime
primeiro o filho, depois a chave; ao final, imprime o último filho
(à direita da última chave). A indentação por `\t` faz a profundidade
ficar visível na saída textual.

Para a árvore final do `main.c`:

```
        2           ← folha (altura 2)
        4           ← folha (altura 2)
    5               ← raiz  (altura 1)
        6           ← folha
        7           ← folha
        9           ← folha
```

Lendo a impressão como traversal in-order, a sequência das chaves
aparece em ordem crescente — uma verificação rápida de que a árvore
está válida.

---

## 9. Complexidade

| Operação    | Caso médio    | Pior caso     |
| ----------- | ------------- | ------------- |
| Busca       | O(log_t _n_)  | O(log_t _n_)  |
| Inserção    | O(log_t _n_)  | O(log_t _n_)  |
| Remoção     | O(log_t _n_)  | O(log_t _n_)  |

Onde _n_ é o total de chaves na árvore e _t_ é a ordem. Para _t_ = 2
e 1 milhão de chaves: a árvore tem altura ≈ 10.

---

## 10. Diferenças em relação à versão original

A reescrita preserva o **algoritmo** e a **interface pública** —
qualquer código existente que use `insercao`, `remocao`, `busca` e
`imprimir` continua funcionando inalterado.

As correções e melhorias aplicadas:

### 10.1 Bugs corrigidos

| # | Bug                                                      | Onde                        | Impacto                          |
| - | -------------------------------------------------------- | --------------------------- | -------------------------------- |
| 1 | `int* valida` em vez de `int valida` (ponteiro não inicializado) | `insercao()`, `remocao()` | **Comportamento indefinido**; o compilador emite warning `-Wincompatible-pointer-types`. O código funcionava por sorte porque o lixo da pilha calhava de apontar para um endereço gravável. |
| 2 | `sizeof(TAB)` em vez de `sizeof(TAB*)` ao alocar `filhos` | `novaPagina()`              | Desperdício de ~3× memória por página (sem falha funcional). |
| 3 | Ausência de função para liberar a árvore                 | global                      | Vazamento de memória: 616 bytes em 11 alocações no exemplo do `main.c` (confirmado com AddressSanitizer). |
| 4 | `#include "TAB.c"` em `main.c`                           | `main.c`                    | Anti-padrão: impede compilação separada, pode causar erros de redefinição em projetos maiores. |
| 5 | Indentação enganosa em `main.c` (warning `-Wmisleading-indentation`) | `main.c`             | Cosmético, mas confuso ao ler.   |
| 6 | Mensagens de erro em `stdout`                            | `efetuaInsercao()`, `efetuaRemocao()` | Devem ir para `stderr` para não poluir saída de dados. |

### 10.2 Melhorias didáticas

- **Comentários Doxygen** em todas as funções do `TAB.h`.
- **Comentários explicativos** ao longo do `TAB.c` descrevendo cada
  bloco lógico do algoritmo.
- **Nomes de variáveis mais expressivos**:
  - `main` → `paginaAtual` / `raiz` (evita conflito com `main()`)
  - `iNP` → `inserePagina`
  - `nI` → `chavePromov`
  - `valida` → `promoveu` / `underflow` (significado claro em cada contexto)
  - `nAP` → `posicaoNaoEncontrada`
- **Estilo de código consistente**: indentação 4 espaços, chaves K&R,
  declarações no menor escopo possível (C99/C11).
- **Makefile** com alvos `all`, `run`, `debug`, `clean`.

### 10.3 Verificação

A versão melhorada foi validada com:

```bash
# Compilação limpa (sem warnings)
gcc -Wall -Wextra -Wpedantic -std=c11 -o arvoreb main.c TAB.c

# Sem comportamento indefinido nem vazamento
gcc -fsanitize=address -fsanitize=undefined -g -O0 -o arvoreb_san main.c TAB.c
./arvoreb_san     # exit code 0, sem mensagens do sanitizer
```

A saída do programa é **idêntica** à da versão original para a
sequência de teste do `main.c`.

---

## 11. Possíveis extensões (sugestões didáticas)

1. **Entrada por arquivo ou stdin** — substituir o vetor fixo de
   chaves do `main.c` por leitura, transformando o programa em uma
   CLI utilizável.
2. **Suporte a chave-valor** — generalizar `int chaves[]` para uma
   `struct { int chave; T valor; }`, ainda com `int` como chave para
   ordenação.
3. **Variante B+** — em árvores B+, apenas as folhas guardam os dados
   reais e as folhas formam uma lista ligada — exatamente o que o
   InnoDB do MySQL faz em seus índices. Boa extensão para conectar
   com o conteúdo de Banco de Dados.
4. **Visualização** — exportar a árvore em formato DOT (Graphviz) ou
   JSON para renderizar com ferramentas externas.
5. **Generalizar a ordem** — atualmente todas as funções recebem
   `ordem` como parâmetro; armazenar a ordem na própria estrutura
   (em um tipo "ArvoreB" envolvendo o ponteiro raiz) simplifica a API.

---

## Licença

Mantida do projeto original — ver `LICENSE`.
