# Árvore Patrícia em C — Guia do Código

Implementação acadêmica da **Árvore Patrícia** em C, variante clássica
de Sedgewick (cap. 17 de *Algorithms in C*), com nó-cabeça sentinela
e back-edges. Esta documentação acompanha a versão revisada do código.

---

## 1. O que é uma Árvore Patrícia

**PATRICIA** = *Practical Algorithm To Retrieve Information Coded In
Alphanumeric*. Foi proposta por Morrison em 1968 e popularizada por
Sedgewick. É uma variante comprimida da **trie binária** que indexa
chaves comparando-as **bit a bit**.

A ideia central: em vez de comparar a chave inteira em cada nó (como
faz uma ABB), comparamos **um único bit** por nó. Cada nó interno
guarda o **índice** do bit a ser testado — e direciona para o filho
esquerdo (se o bit for 0) ou direito (se for 1).

Vantagens da Patrícia frente à ABB:

- **Não degenera** com chaves em ordem — a altura é limitada pelo
  número de bits da chave, não pelo número de chaves.
- **Comparações são apenas de bit único** — caminhamento muito rápido.
- **Suporta operações eficientes** de busca por prefixo (sem precisar
  de strings, naturalmente, com bits).

Aplicações reais: tabelas de roteamento IP em kernels Linux e BSD,
índices em bancos de dados orientados a bytes, motores de busca por
prefixo de DNA.

---

## 2. Variante implementada: Patrícia clássica

Esta é a variante **clássica de Sedgewick**, com duas características
distintivas:

### 2.1 Nó-cabeça sentinela

A árvore não começa em um nó comum. Existe um **nó-cabeça permanente**
com `bit=-1` e `item=ITEMNULO`, cujo `esq` aponta para a raiz "real"
(ou para si mesmo, se a árvore estiver vazia).

```
                ┌──────────────────────┐
                │   nó-cabeça (bit=-1) │
                │   item = ITEMNULO    │
                └──────────────────────┘
                       │
                       ▼ esq aponta para...
                ┌──────────────────────┐
                │   raiz "real"        │
                │   (ou ele mesmo se   │
                │    a árvore vazia)   │
                └──────────────────────┘
```

**Por que sentinela?** Simplifica MUITO os casos de borda:

- A árvore vazia é apenas o nó-cabeça apontando para si mesmo
  (`raiz->esq == raiz`).
- A inserção do primeiro elemento não precisa de tratamento especial.
- Em todas as funções recursivas, basta verificar "voltei ao
  sentinela?" para detectar fim de caminho.

### 2.2 Back-edges (links de retorno)

Diferente da variante "híbrida" comum (onde folhas são nós separados),
a Patrícia clássica usa **um único tipo de nó** para tudo. Quando um
ponteiro "desceria" para uma folha, ele aponta de volta para um nó já
existente da árvore — esse ponteiro é uma **back-edge**.

**Como identificar uma back-edge?** Pela regra:

```
bit do nó pai (durante descida)  <  bit do nó filho   →  aresta normal
bit do nó pai                    ≥  bit do nó filho   →  BACK-EDGE
```

Como o bit cresce monotonicamente durante uma descida normal, qualquer
filho com bit menor ou igual é necessariamente uma back-edge. Quem
chega a esse nó pelo "caminho de back-edge" lê a `chave` armazenada
nele como se fosse uma folha lógica.

```
        ┌─[S | 10011 | bit=0]─┐         ← nó interno (raiz real)
        0│                    │1
         ▼                    ▼
   [H | 01000 | bit=1]  [R | 10010 | bit=4]
       0│       1│         0│        1│
        ▼        ▼          ▼         ▼
       ...     ...    ← R (back-edge   ← S (back-edge
                         para o R)        para a raiz!)
```

Repare na linha de baixo: o lado direito de `R` aponta de volta
para o `S` (raiz), que é a back-edge contendo a chave `S` como folha
lógica.

---

## 3. Estrutura de dados

```c
typedef struct Nodo {
    TipoItem item;   // chave (ITEMNULO no sentinela)
    ApNodo   esq;    // filho esquerdo — bit = 0
    ApNodo   dir;    // filho direito — bit = 1
    int      bit;    // bit testado (-1 só no sentinela)
} Nodo;
```

O **mesmo struct** serve para nó-cabeça, nós internos e (logicamente)
folhas. O papel é determinado pelo contexto: o nó alvo de uma
back-edge funciona como folha; o nó atravessado normalmente funciona
como interno.

### Convenção de bits

- `PATRICIA_BITS` define a largura da chave em bits (configurável).
- **Posição 0 = MSB** dentro dos `PATRICIA_BITS` bits.
- Para `PATRICIA_BITS=5` e chave `9` (binário `01001`):
  - posição 0 vale 0
  - posição 4 vale 1

---

## 4. Como compilar e rodar

```bash
make            # default: 5 bits (suficiente para A..Z = 1..26)
make run        # compila e executa
make bits8      # PATRICIA_BITS=8 (chaves de um byte)
make bits32     # PATRICIA_BITS=32 (chaves de int)
make debug      # build com -g -O0 para gdb
make sanitize   # build com AddressSanitizer + UBSanitizer
make clean
```

Ou manualmente:

```bash
gcc -DPATRICIA_BITS=8 -Wall -Wextra -std=c11 -o patricia main.c patricia.c
```

---

## 5. Impressão hierárquica

A impressão usa estilo `tree(1)` com prefixos acumulados. Para a
sequência clássica de aula `A, S, E, R, C, H, I, N` (com 5 bits):

```
`-- raiz.esq [S | 10011 | bit=0]
    |-- 0 [H | 01000 | bit=1]
    |   |-- 0 [E | 00101 | bit=2]
    |   |   |-- 0 [C | 00011 | bit=3]
    |   |   |   |-- 0 [A | 00001 | bit=4]
    |   |   |   |   |-- 0 -> SENTINELA/NULL
    |   |   |   |   `-- 1 -> folha [A | 00001 | bit=4]
    |   |   |   `-- 1 -> folha [C | 00011 | bit=3]
    |   |   `-- 1 -> folha [E | 00101 | bit=2]
    |   `-- 1 [N | 01110 | bit=2]
    |       |-- 0 [I | 01001 | bit=4]
    |       |   |-- 0 -> folha [H | 01000 | bit=1]
    |       |   `-- 1 -> folha [I | 01001 | bit=4]
    |       `-- 1 -> folha [N | 01110 | bit=2]
    `-- 1 [R | 10010 | bit=4]
        |-- 0 -> folha [R | 10010 | bit=4]
        `-- 1 -> folha [S | 10011 | bit=0]
```

**Como ler:**

- `[X | bits | bit=k]` é um **nó interno** que testa o bit `k`.
- `-> folha [...]` indica uma **back-edge** (folha lógica).
- `-> SENTINELA/NULL` indica retorno ao nó-cabeça (típico em árvores
  com poucas chaves do lado em questão).
- As arestas são rotuladas com `0` ou `1` para mostrar qual valor do
  bit conduz por cada caminho.

**Verifique visualmente:** para qualquer folha, siga o caminho desde a
raiz comparando o bit indicado em cada nó interno com o binário da
chave. Por exemplo, chegar à folha `[A | 00001 | bit=4]`:
- raiz testa bit 0 → A tem bit 0 = 0 → desce esq
- nó H testa bit 1 → A tem bit 1 = 0 → desce esq
- nó E testa bit 2 → A tem bit 2 = 0 → desce esq
- nó C testa bit 3 → A tem bit 3 = 0 → desce esq
- nó A testa bit 4 → A tem bit 4 = 1 → desce dir (back-edge para A)

A invariante crucial: **o bit cresce ao descer pelo caminho normal**.
Quebra disso = back-edge.

---

## 6. Operações

### 6.1 Busca — `busca(v, raiz)`

```
1. Comece em raiz->esq, com bitAnterior = -1.
2. Em cada nó p, se p->bit <= bitAnterior, isso é uma folha lógica:
   compare p->item com v e devolva o resultado.
3. Senão, examine o bit de v na posição p->bit:
   - se for 0, desça pela esquerda
   - se for 1, desça pela direita
4. Atualize bitAnterior = p->bit e volte ao passo 2.
```

Complexidade: O(b), onde b = `PATRICIA_BITS`.

### 6.2 Inserção — `insere(v, raiz)`

```
1. Busque v (passo 1 da busca acima) — chegue na folha lógica p.
2. Se p->item == v, é duplicata: rejeite.
3. Calcule bitDiferente = primeiro bit em que v e p->item diferem.
4. Desça novamente pela árvore. Insira o novo nó no nível onde:
   - o nó atual tem bit > bitDiferente (passou do lugar), ou
   - chegamos a uma back-edge.
5. O novo nó tem bit = bitDiferente, um filho aponta para v
   (back-edge para a nova chave) e o outro para a subárvore atual.
```

Complexidade: O(b).

### 6.3 Remoção — `removeItem(v, raiz)`

A implementação atual é **didática por reconstrução**:

```
1. Verifique se v existe (busca normal).
2. Colete todas as chaves da árvore em um vetor.
3. Esvazie a árvore.
4. Reinsira tudo, EXCETO v.
```

Complexidade: O(n·b), onde n = número de chaves. Bem mais cara que o
teórico O(b), mas **simples de entender e provadamente correta** — o
que vale ouro em contexto didático. Para "produção", o algoritmo
in-place clássico é mais elaborado (lida com 3 casos diferentes
dependendo de onde a chave aparece na árvore).

### 6.4 Patrícia-sort — `sort(raiz)`

Faz uma travessia em pré-ordem que imprime apenas folhas lógicas
(back-edges). Como bit-0 está na esquerda e bit-1 na direita em todo
nível, a sequência das folhas em pré-ordem **já é a sequência
ordenada**. Não há comparação tradicional envolvida: é a Patrícia que
implicitamente ordena pela representação binária.

```
A(00001) C(00011) E(00101) H(01000) I(01001) N(01110) R(10010) S(10011)
```

Complexidade: O(n).

---

## 7. Arquitetura do código

```
patricia_c/
├── patricia.h    # interface pública: tipos + assinaturas + docs Doxygen
├── patricia.c    # implementação comentada
├── main.c        # programa interativo com menu
├── Makefile      # build + variantes (debug, sanitize, bits8, bits32)
└── README.md     # este documento
```

### Convenções de visibilidade

- Funções marcadas `static` no `.c` são internas (não exportadas).
- O cabeçalho `patricia.h` só expõe o que o usuário externo precisa.
- O `main.c` interage com a árvore **apenas pela API pública** —
  nunca acessa `Nodo` diretamente, exceto para mostrar `bit` em
  resultados de busca.

---

## 8. Melhorias aplicadas em relação ao código de referência

A versão de referência (fornecida no ZIP) já era de boa qualidade —
compilava limpa e passava em sanitizers. As melhorias aplicadas foram
de **documentação e infraestrutura**, não de correção de bugs:

### 8.1 Documentação

- **Comentários Doxygen** em todas as funções públicas do `patricia.h`,
  com `@brief`, `@param`, `@return` e exemplos onde apropriado.
- **Comentários didáticos** ao longo do `patricia.c` explicando cada
  parte do algoritmo: o papel do sentinela, como reconhecer
  back-edges, o protocolo recursivo da inserção, o porquê da
  remoção por reconstrução.
- **Estrutura em blocos** com cabeçalhos visuais separando
  "construção/destruição", "operações principais", "travessias",
  "utilidades".

### 8.2 Makefile robusto

O Makefile original cobria apenas `all` e `clean`. Adicionei:

- `make run` — compila e executa.
- `make debug` — build com `-g -O0` para usar em `gdb`.
- `make sanitize` — build com AddressSanitizer + UBSanitizer.
- `make bits8`, `make bits32` — atalhos para diferentes larguras.
- `-Wpedantic` adicionado às flags padrão (era só `-Wall -Wextra`).

### 8.3 Pequenos refinamentos

- Inserção do primeiro elemento numa árvore vazia: a versão original
  tinha um *fall-through* potencial quando `primeiroBitDiferente`
  retornava `-1` na comparação com o sentinela. Adicionei tratamento
  explícito para esse caso, com comentário, para deixar o caminho
  óbvio mesmo que `itemValido` já garantisse que não cairia ali.
- Variáveis locais movidas para o menor escopo possível (estilo C99/C11).
- Documentação explícita da invariante "bit cresce ao descer", que
  é a chave para entender o código todo.

### 8.4 Verificação

- Compila limpo com `-Wall -Wextra -Wpedantic -std=c11`.
- Passa em AddressSanitizer + UBSanitizer com `make sanitize`,
  exercitando insere/busca/remove/imprime/sort em sequência.
- A saída para o exemplo da aula (`A,S,E,R,C,H,I,N`) é
  **byte-a-byte idêntica** à da versão de referência, exceto por:
  - a legenda da impressão menciona explicitamente "(back-edge)"
    para deixar a terminologia clara;
  - uma linha em branco extra ao final, para separação visual.

---

## 9. Comparação com a variante "híbrida"

Se você já viu uma Patrícia implementada com **dois tipos de nó**
(internos com `bit >= 0` e folhas com `bit = -1`), essa é a chamada
"variante híbrida" — comum em projetos didáticos. As principais
diferenças:

| | Variante híbrida | Variante clássica (esta) |
|---|---|---|
| Tipos de nó | 2 (interno + folha) | 1 (todos iguais) |
| Identificar folha | Campo `bit == -1` | Back-edge: `filho.bit <= pai.bit` |
| Raiz | Ponteiro que pode mudar | Sentinela fixo (`esq` é a raiz "real") |
| Memória | `n + (n-1)` nós | `n + 1` nós (sentinela) |
| Inserção 1ª chave | Caso especial óbvio | Casa naturalmente com o sentinela |
| Tradição | Projetos didáticos | Sedgewick, Knuth, kernel Linux |

A clássica é mais eficiente em memória (~metade dos nós) e mais
elegante (caso vazio é trivialmente representado), mas a híbrida é
mais fácil de visualizar inicialmente.

---

## 10. Complexidade

| Operação    | Complexidade | Observação                         |
| ----------- | ------------ | ---------------------------------- |
| Busca       | O(b)         | b = `PATRICIA_BITS`                |
| Inserção    | O(b)         |                                    |
| Remoção     | O(n·b)       | Pela reconstrução didática         |
| Travessia   | O(n)         | Visita cada folha uma vez          |
| Sort        | O(n)         | Pré-ordem das folhas               |
| Liberação   | O(n)         | Visita cada nó uma vez             |

Onde `n` é o número de chaves e `b` é o número de bits da chave.

---

## 11. Possíveis extensões (sugestões didáticas)

1. **Remoção in-place** — substitua a remoção por reconstrução pelo
   algoritmo clássico in-place O(b). Bom exercício para entender os
   três casos de remoção.
2. **Chaves variáveis (strings)** — generalize para chaves de tamanho
   variável (strings); cada bit é um bit do byte da posição corrente.
   Base para roteamento IP.
3. **Busca por prefixo** — adicione `buscaPrefixo(t, prefixo, n_bits)`
   que retorna todas as chaves que começam com os `n_bits` primeiros
   bits de `prefixo`. Aplicação direta: longest-prefix match em
   roteadores.
4. **Visualização gráfica** — gere saída em formato DOT (Graphviz)
   para renderizar a árvore como imagem; destaque back-edges com
   setas pontilhadas.
5. **Comparativo com ABB** — exercício: insira a mesma sequência de
   1000 chaves ordenadas em uma Patrícia e em uma ABB. Compare altura
   e número de comparações em buscas posteriores. A Patrícia mantém
   altura limitada por `b`; a ABB degenera para uma lista de altura
   1000.

---

## Licença

A versão original deste código segue a licença do projeto-base
(Patrick Araújo) — ver `LICENSE` se incluído.
