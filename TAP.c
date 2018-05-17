#include <stdio.h>
#include <stdlib.h>
#include "TAP.h"

int pegaBit(char info, int pos){
	return (info >> 7-pos) & 1;
}

TAP* insere(TAP *main, char info, int numMaxBits) {
	TAP *novo, *novoFilho, *retorno;
	int posBit;

	if (!main) {
		novo = malloc(sizeof(TAP));
		novo->info = info;
		novo->bit = -1;
		novo->esq = NULL; novo->dir = NULL;
		retorno = novo;
	}
	else{
		if (main->bit == -1){
			//	printf("%d\n", main->bit);
			posBit = 0;
			while ((posBit < numMaxBits) && (pegaBit(main->info, posBit) == pegaBit(info, posBit)))
				posBit++;
			if (posBit < numMaxBits) {
				//	printf("Posicao do bit = %d\n", posBit);
				novoFilho = malloc(sizeof(TAP));
				novoFilho->info = info;
				novoFilho->bit = -1;
				novoFilho->esq = NULL; novoFilho->dir = NULL;
				novo = malloc(sizeof(TAP));
				novo->info = info;
				novo->bit = posBit;
				if (!pegaBit(info,posBit)) {
					novo->esq = novoFilho;
					novo->dir = NULL;
				}
				else{
					novo->dir = novoFilho;
					novo->esq = NULL;
				}
				retorno = novo;
			}
			else
				retorno = NULL;
		}
		else{
			if (!pegaBit(info, main->bit)){
				// printf("vai inserir a esquerda do %d\n", main->bit);
				novo = insere(main->esq, info, numMaxBits);
				// printf("%d\n", novo->bit);
				if (novo) {
					if (novo->bit < main->bit)
						retorno = novo;
					else{
						if (!novo->esq)
							novo->esq = main->esq;
						else
							novo->dir = main->esq;
						main->esq = novo;
						retorno = NULL;
					}
				}
				else{
					retorno = main;
				}
			}else{
				// printf("vai inserir a direita do %d\n", main->bit);
				novo = insere(main->dir, info, numMaxBits);
				if (novo){
					if (novo->bit < main->bit)
						retorno = novo;
					else{
						if (!novo->esq)
							novo->esq = main->dir;
						else
							novo->dir = main->dir;
						main->dir = novo;
						retorno = NULL;
					}
				}
				else
					retorno = main;
			}
		}
	}
	return retorno;
}

TAP* inserePatricia(TAP *main, char info, int numMaxBits) {
	TAP *retorno, *novo = insere(main, info, numMaxBits);
	if (!novo)
		retorno = main;
	else {
		// printf("Novo retornou para a raiz com bit = %d\n", novo->bit);
		if (novo->bit != -1)
			if (!novo->esq) {
				//	printf("direita -> %d    %c\n", novo->bit, novo->dir->info);
				novo->esq = main;
			}
			else
				novo->dir = main;	//	printf("esquerda -> %d    %c\n", novo->bit, novo->esq->info);
			retorno = novo;
	}
	return retorno;
}

TAP* removePatricia(TAP *main, char info) {
	TAP *retorno;

	if (!main){
		retorno = NULL;
	}
	else{
		if (main->bit == -1){
			if (main->info == info){
				free(main);
				retorno = NULL;
			}
			else
				retorno = main;
		}
		else{
			if (!pegaBit(info, main->bit)){
				main->esq = removePatricia(main->esq, info);
				if (!main->esq){
					retorno = main->dir;
					free(main);
				}
				else
					retorno = main;
			}
			else{
				main->dir = removePatricia(main->dir, info);
				if (!main->dir){
					retorno = main->esq;
					free(main);
				}
				else
					retorno = main;
			}
		}
	}
	return retorno;
}

void caminhaEmOrdem(TAP *main) {
	if (main) {
		caminhaEmOrdem(main->esq);
		if ((!main->esq) && (!main->dir))
			printf("%c\n", main->info);
		caminhaEmOrdem(main->dir);
	}
}

void imprimeLetraBin(char letra, int numMaxBits) {
	int i;
	printf("%c = ", letra);
	for(i = 0; i < numMaxBits; i++)
		printf("%d", pegaBit(letra,i));
	printf("\n");
}
//bingo
