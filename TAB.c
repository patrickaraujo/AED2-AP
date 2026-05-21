/**
 * @file    TAB.c
 * @brief   Implementação da Árvore B (T.A.B.).
 *
 * Esta é uma reescrita comentada e corrigida da implementação original.
 * Os algoritmos de inserção e remoção seguem o método clássico descrito
 * em Ziviani (cap. 6 do "Projeto de Algoritmos"), no qual:
 *
 *   - A INSERÇÃO desce recursivamente até a folha apropriada e, ao
 *     retornar, propaga eventuais cisões (page splits) de baixo para cima.
 *     Apenas a inserção pode fazer a árvore crescer em altura, e isso só
 *     ocorre quando a raiz precisa ser cindida.
 *
 *   - A REMOÇÃO localiza a chave; se ela estiver em uma folha, apenas
 *     desloca os elementos; se estiver em uma página interna, substitui
 *     a chave por seu antecessor (maior chave da subárvore esquerda).
 *     Em seguida, propaga ajustes de underflow para cima, redistribuindo
 *     com irmãos ou fundindo páginas. Apenas a remoção pode fazer a
 *     árvore diminuir em altura, e isso só ocorre quando a raiz fica
 *     com 0 chaves após uma fusão.
 *
 * Convenções:
 *   - "ordem t" significa que cada página (não-raiz) tem entre t e 2t
 *     chaves, e entre t+1 e 2t+1 filhos.
 *   - Os vetores @c chaves e @c filhos usam indexação base 0; o código
 *     usa @c i como contador base 1 em vários trechos, por isso aparece
 *     com frequência @c chaves[i-1]. Esse padrão foi preservado por ser
 *     idêntico ao do livro de referência.
 *
 * @author  Patrick Araújo (versão original)
 * @note    Versão revisada com correções e documentação adicional.
 */

#include <stdio.h>
#include <stdlib.h>
#include "TAB.h"

/* ========================================================================
 * Construção e destruição
 * ====================================================================== */

TAB *novaPagina(int ordem) {
    TAB *p = (TAB *) malloc(sizeof(TAB));
    if (!p) {
        fprintf(stderr, "Erro: falha ao alocar TAB.\n");
        exit(EXIT_FAILURE);
    }

    p->n_chaves = 0;

    /* Vetor de chaves: capacidade 2t (ordem * 2). */
    p->chaves = (int *) malloc(sizeof(int) * (2 * ordem));

    /*
     * Vetor de filhos: capacidade 2t+1.
     * CORREÇÃO: a versão original usava sizeof(TAB) em vez de sizeof(TAB*),
     * o que alocava ~3x mais memória do que o necessário (sem causar
     * erro funcional, mas desperdiçando memória).
     */
    p->filhos = (TAB **) malloc(sizeof(TAB *) * ((2 * ordem) + 1));

    if (!p->chaves || !p->filhos) {
        fprintf(stderr, "Erro: falha ao alocar vetores internos.\n");
        exit(EXIT_FAILURE);
    }

    /* Inicializa todos os ponteiros de filhos como NULL — marca de folha. */
    for (int i = 0; i <= (2 * ordem); i++)
        p->filhos[i] = NULL;

    return p;
}

void liberarArvore(TAB *raiz) {
    if (!raiz) return;

    /* Percorre recursivamente todos os filhos (pós-ordem). */
    for (int i = 0; i <= raiz->n_chaves; i++)
        liberarArvore(raiz->filhos[i]);

    free(raiz->chaves);
    free(raiz->filhos);
    free(raiz);
}

/* ========================================================================
 * Inserção
 * ====================================================================== */

void inserePagina(TAB *pagina, TAB *novoFilho, int chave) {
    /*
     * Insere @p chave (e o ponteiro @p novoFilho como filho à direita
     * dessa chave) em @p pagina, mantendo a ordem crescente.
     *
     * Pré-condição: pagina->n_chaves < 2t (existe espaço livre).
     *
     * Estratégia: percorre do final para o começo, deslocando para a
     * direita cada chave maior que @p chave, até encontrar a posição
     * correta.
     */
    int i = pagina->n_chaves;
    int posicaoNaoEncontrada = (i > 0);

    while (posicaoNaoEncontrada) {
        if (chave >= pagina->chaves[i - 1]) {
            posicaoNaoEncontrada = 0;
            break;
        }
        /* Desloca chave e filho-à-direita uma casa para a direita. */
        pagina->chaves[i]     = pagina->chaves[i - 1];
        pagina->filhos[i + 1] = pagina->filhos[i];
        i--;
        if (i < 1)
            posicaoNaoEncontrada = 0;
    }

    pagina->chaves[i]     = chave;
    pagina->filhos[i + 1] = novoFilho;
    pagina->n_chaves++;
}

void efetuaInsercao(TAB **paginaAtual, TAB **paginaDir, int *promoveu,
                    int *chavePromov, int chave, int ordem) {
    /*
     * Caso-base: descemos até abaixo de uma folha. Significa que a
     * chave precisa ser inserida na folha pai (que é o chamador).
     * Marcamos @p promoveu = 1 para que o nível superior insira a
     * chave em si mesmo.
     */
    if (!(*paginaAtual)) {
        *promoveu    = 1;
        *chavePromov = chave;
        *paginaDir   = NULL;
        return;
    }

    /*
     * Procura na página atual a posição da chave: percorre as chaves
     * enquanto a buscada for maior. Ao final, @c i indica em qual
     * subárvore descer (ou que a chave já existe).
     */
    int i = 1;
    while ((i < (*paginaAtual)->n_chaves) && (chave > (*paginaAtual)->chaves[i - 1]))
        i++;

    /* Chave duplicada: política do projeto é rejeitar. */
    if (chave == (*paginaAtual)->chaves[i - 1]) {
        fprintf(stderr, "Erro: chave %d ja inserida.\n", chave);
        *promoveu = 0;
        return;
    }

    /* Se a chave é menor que a da posição i, descemos pelo filho à esquerda. */
    if (chave < (*paginaAtual)->chaves[i - 1])
        i--;

    /* Recursão: tenta inserir na subárvore apropriada. */
    efetuaInsercao(&((*paginaAtual)->filhos[i]), paginaDir, promoveu,
                   chavePromov, chave, ordem);

    /* Se a recursão não promoveu nada, terminamos. */
    if (!(*promoveu))
        return;

    /* ---- Há chave a ser promovida para a página atual. ---- */

    if ((*paginaAtual)->n_chaves < (2 * ordem)) {
        /* Caso simples: ainda há espaço — basta inserir e parar a propagação. */
        inserePagina(*paginaAtual, *paginaDir, *chavePromov);
        *promoveu = 0;
    } else {
        /*
         * Overflow: a página tem 2t chaves e receberia uma 2t+1-ésima.
         * Precisamos cindi-la em duas páginas de t chaves cada,
         * promovendo a chave do meio para o nível superior.
         *
         * Estratégia:
         *   1. Cria uma página nova @c novaPag (à direita).
         *   2. Distribui as chaves: as menores ficam em paginaAtual,
         *      as maiores vão para novaPag.
         *   3. A chave central é promovida via @c chavePromov.
         */
        TAB *novaPag = novaPagina(ordem);

        if (i < ordem + 1) {
            /*
             * A nova chave pertence à metade ESQUERDA.
             * Move a última chave da página cheia para a página nova,
             * abre espaço e insere a nova chave na esquerda.
             */
            inserePagina(novaPag,
                         (*paginaAtual)->filhos[2 * ordem],
                         (*paginaAtual)->chaves[(2 * ordem) - 1]);
            (*paginaAtual)->n_chaves--;
            inserePagina(*paginaAtual, *paginaDir, *chavePromov);
        } else {
            /* A nova chave pertence à metade DIREITA — vai direto para novaPag. */
            inserePagina(novaPag, *paginaDir, *chavePromov);
        }

        /* Transfere a metade superior das chaves para novaPag. */
        for (int j = ordem + 2; j <= (2 * ordem); j++)
            inserePagina(novaPag, (*paginaAtual)->filhos[j], (*paginaAtual)->chaves[j - 1]);

        /* paginaAtual fica com t chaves; novaPag também. */
        (*paginaAtual)->n_chaves = ordem;

        /*
         * O filho à esquerda de novaPag é o filho da chave promovida.
         * (Truque didático: inserePagina coloca em filhos[1]; movemos
         *  para filhos[0].)
         */
        novaPag->filhos[0] = novaPag->filhos[ordem + 1];

        /* A chave do meio (índice 'ordem' em paginaAtual) sobe. */
        *chavePromov = (*paginaAtual)->chaves[ordem];
        *paginaDir   = novaPag;
        /* @c promoveu permanece 1 — a cisão se propaga para o nível acima. */
    }
}

void insercao(TAB **raiz, int chave, int ordem) {
    /*
     * CORREÇÃO IMPORTANTE: a versão original declarava
     *     int* valida, nI;
     * o que cria um PONTEIRO não inicializado e era passado para
     * efetuaInsercao. Isso é comportamento indefinido (UB) — funcionava
     * por sorte. Aqui usamos variáveis locais inteiras inicializadas.
     */
    int  promoveu    = 0;
    int  chavePromov = 0;
    TAB *paginaDir   = NULL;

    efetuaInsercao(raiz, &paginaDir, &promoveu, &chavePromov, chave, ordem);

    /*
     * Se a raiz cindiu, a árvore cresce em altura: criamos uma nova
     * raiz com uma única chave (a promovida) e dois filhos.
     */
    if (promoveu) {
        TAB *novaRaiz = novaPagina(ordem);
        novaRaiz->n_chaves  = 1;
        novaRaiz->chaves[0] = chavePromov;
        novaRaiz->filhos[0] = *raiz;     /* árvore antiga vira filho esquerdo */
        novaRaiz->filhos[1] = paginaDir; /* página criada pela cisão da raiz  */
        *raiz = novaRaiz;
    }
}

/* ========================================================================
 * Remoção
 * ======================================================================
 *
 * Após uma remoção em uma folha (ou substituição por antecessor) a
 * página pode ficar com menos de t chaves — chamamos isso de
 * UNDERFLOW. Quando isso acontece, @c reconstitui() tenta resolver de
 * duas formas:
 *
 *   (a) REDISTRIBUIÇÃO ("existe folga"): pega chaves emprestadas do
 *       irmão adjacente que tem mais que t chaves. O underflow é
 *       resolvido localmente e a propagação cessa.
 *
 *   (b) FUSÃO ("não há folga"): junta a página com seu irmão e desce
 *       uma chave do pai entre as duas. Como o pai perde uma chave,
 *       o underflow pode subir de nível.
 */

void reconstitui(TAB **paginaFilha, TAB *pai, int *underflow,
                 int indiceP, int ordem) {
    TAB *irmao;
    int  qtdTransferir, j;

    if (indiceP < pai->n_chaves) {
        /* ------ Caso 1: existe irmão à DIREITA ------ */
        irmao = pai->filhos[indiceP + 1];
        qtdTransferir = (irmao->n_chaves - ordem + 1) / 2;

        /* Desce a chave separadora do pai para o final de paginaFilha. */
        (*paginaFilha)->chaves[(*paginaFilha)->n_chaves]     = pai->chaves[indiceP];
        (*paginaFilha)->filhos[(*paginaFilha)->n_chaves + 1] = irmao->filhos[0];
        (*paginaFilha)->n_chaves++;

        if (qtdTransferir > 0) {
            /* (a) Redistribuição: irmão direito tem folga. */
            for (j = 1; j < qtdTransferir; j++)
                inserePagina(*paginaFilha, irmao->filhos[j], irmao->chaves[j - 1]);

            /* Nova chave separadora sobe do irmão para o pai. */
            pai->chaves[indiceP] = irmao->chaves[qtdTransferir - 1];
            irmao->n_chaves -= qtdTransferir;

            /* Compacta o irmão removendo as chaves transferidas. */
            for (j = 0; j < irmao->n_chaves; j++)
                irmao->chaves[j] = irmao->chaves[j + qtdTransferir];
            for (j = 0; j <= irmao->n_chaves; j++)
                irmao->filhos[j] = irmao->filhos[j + qtdTransferir];

            *underflow = 0;
        } else {
            /* (b) Fusão: paginaFilha absorve o irmão direito. */
            for (j = 1; j <= ordem; j++)
                inserePagina(*paginaFilha, irmao->filhos[j], irmao->chaves[j - 1]);

            free(irmao->chaves);
            free(irmao->filhos);
            free(irmao);

            /* Compacta o pai removendo a chave separadora e o ponteiro do irmão. */
            for (j = indiceP + 1; j < pai->n_chaves; j++) {
                pai->chaves[j - 1] = pai->chaves[j];
                pai->filhos[j]     = pai->filhos[j + 1];
            }
            pai->n_chaves--;

            /* Se o pai ainda tem >= t chaves, a propagação cessa aqui. */
            if (pai->n_chaves >= ordem)
                *underflow = 0;
        }
    } else {
        /* ------ Caso 2: só há irmão à ESQUERDA ------ */
        irmao = pai->filhos[indiceP - 1];
        qtdTransferir = (irmao->n_chaves - ordem + 1) / 2;

        /* Abre espaço no início de paginaFilha. */
        for (j = (*paginaFilha)->n_chaves; j >= 1; j--)
            (*paginaFilha)->chaves[j] = (*paginaFilha)->chaves[j - 1];
        (*paginaFilha)->chaves[0] = pai->chaves[indiceP - 1];

        for (j = (*paginaFilha)->n_chaves; j >= 0; j--)
            (*paginaFilha)->filhos[j + 1] = (*paginaFilha)->filhos[j];
        (*paginaFilha)->n_chaves++;

        if (qtdTransferir > 0) {
            /* (a) Redistribuição: irmão esquerdo tem folga. */
            for (j = 1; j < qtdTransferir; j++)
                inserePagina(*paginaFilha,
                             irmao->filhos[irmao->n_chaves - j + 1],
                             irmao->chaves[irmao->n_chaves - j]);

            (*paginaFilha)->filhos[0] = irmao->filhos[irmao->n_chaves - qtdTransferir + 1];
            pai->chaves[indiceP - 1]  = irmao->chaves[irmao->n_chaves - qtdTransferir];
            irmao->n_chaves -= qtdTransferir;

            *underflow = 0;
        } else {
            /* (b) Fusão: irmão esquerdo absorve paginaFilha. */
            for (j = 1; j <= ordem; j++)
                inserePagina(irmao, (*paginaFilha)->filhos[j], (*paginaFilha)->chaves[j - 1]);

            free((*paginaFilha)->chaves);
            free((*paginaFilha)->filhos);
            free(*paginaFilha);

            pai->n_chaves--;
            if (pai->n_chaves >= ordem)
                *underflow = 0;
        }
    }
}

void antecessor(TAB *paginaAlvo, TAB *subarvore, int *underflow,
                int indice, int ordem) {
    /*
     * Para remover uma chave de uma página NÃO-folha, a estratégia é
     * substituí-la pelo seu antecessor em ordem (a maior chave da
     * subárvore à sua esquerda). Para encontrar o antecessor, descemos
     * sempre pelo filho mais à direita até chegar a uma folha; a
     * última chave dessa folha é o antecessor.
     */
    if (subarvore->filhos[subarvore->n_chaves]) {
        /* Ainda não chegamos na folha: desce pelo filho mais à direita. */
        antecessor(paginaAlvo, subarvore->filhos[subarvore->n_chaves],
                   underflow, indice, ordem);
        if (*underflow)
            reconstitui(&(subarvore->filhos[subarvore->n_chaves]),
                        subarvore, underflow, subarvore->n_chaves, ordem);
    } else {
        /* Chegamos na folha: troca a chave alvo pelo antecessor. */
        paginaAlvo->chaves[indice - 1] = subarvore->chaves[subarvore->n_chaves - 1];
        subarvore->n_chaves--;
        *underflow = (subarvore->n_chaves < ordem);
    }
}

void efetuaRemocao(TAB **paginaAtual, int *underflow, int chave, int ordem) {
    if (!(*paginaAtual)) {
        fprintf(stderr, "Erro: chave %d nao esta na arvore.\n", chave);
        *underflow = 0;
        return;
    }

    TAB *p = *paginaAtual;
    int  i = 1;

    /* Procura a chave (ou a subárvore onde ela poderia estar). */
    while ((i < p->n_chaves) && (chave > p->chaves[i - 1]))
        i++;

    if (chave == p->chaves[i - 1]) {
        /* Chave encontrada nesta página. */
        if (!(p->filhos[i - 1])) {
            /* Caso A: a página é folha — remoção direta. */
            p->n_chaves--;
            *underflow = (p->n_chaves < ordem);

            /* Compacta o vetor de chaves removendo a chave na posição i-1. */
            for (int j = i; j <= p->n_chaves; j++) {
                p->chaves[j - 1] = p->chaves[j];
                p->filhos[j]     = p->filhos[j + 1];
            }
        } else {
            /* Caso B: página interna — substitui pelo antecessor. */
            antecessor(p, p->filhos[i - 1], underflow, i, ordem);
            if (*underflow)
                reconstitui(&(p->filhos[i - 1]), p, underflow, i - 1, ordem);
        }
    } else {
        /* Chave não está aqui — desce pela subárvore correta. */
        if (chave > p->chaves[i - 1])
            i++;
        efetuaRemocao(&(p->filhos[i - 1]), underflow, chave, ordem);
        if (*underflow)
            reconstitui(&(p->filhos[i - 1]), p, underflow, i - 1, ordem);
    }
}

void remocao(TAB **raiz, int chave, int ordem) {
    /*
     * CORREÇÃO: o original usava @c int* valida (ponteiro não
     * inicializado), o que era UB. Aqui usamos um @c int local.
     */
    int underflow = 0;

    efetuaRemocao(raiz, &underflow, chave, ordem);

    /*
     * Se a raiz ficou com 0 chaves após uma fusão, ela vira lixo:
     * promovemos seu único filho a nova raiz e liberamos a antiga.
     * Isso é o único caso em que a árvore diminui em altura.
     */
    if (*raiz != NULL && (*raiz)->n_chaves == 0) {
        TAB *antiga = *raiz;
        *raiz = antiga->filhos[0];
        free(antiga->chaves);
        free(antiga->filhos);
        free(antiga);
    }
}

/* ========================================================================
 * Busca e impressão
 * ====================================================================== */

TAB *busca(TAB *raiz, int chave) {
    if (!raiz)
        return NULL;

    /* Avança i enquanto a chave buscada for maior que a chave atual. */
    int i = 1;
    while (i < raiz->n_chaves && chave > raiz->chaves[i - 1])
        i++;

    if (chave == raiz->chaves[i - 1])
        return raiz; /* encontrada nesta página */

    /* Não está aqui — desce pela subárvore apropriada. */
    if (chave < raiz->chaves[i - 1])
        return busca(raiz->filhos[i - 1], chave);
    else
        return busca(raiz->filhos[i], chave);
}

void imprimir(TAB *raiz, int altura) {
    if (!raiz) return;

    /*
     * Travessia in-order adaptada para árvore B (não-binária):
     * para cada chave, imprime primeiro a subárvore à sua esquerda,
     * depois a própria chave; ao final, imprime a última subárvore
     * (à direita da última chave).
     */
    int i;
    for (i = 0; i <= raiz->n_chaves - 1; i++) {
        imprimir(raiz->filhos[i], altura + 1);
        for (int j = 0; j <= altura; j++)
            printf("\t");
        printf("%d\n", raiz->chaves[i]);
    }
    imprimir(raiz->filhos[i], altura + 1);
}
