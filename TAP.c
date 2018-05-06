#include <stdio.h>
#include <stdlib.h>
#include "TAP.h"

int Bit(int i, int k, int D){	//	Retorna o i-esimo bit da chave k a partir da esquerda
	int  c, j;
	if (!i)
		return NULL;
	else {
		c = k;
		for (j = 1; j <= D - i; j++)
		c /= 2;
		return (c & 1);
	}
}

int EExterno(TAP p){	//	Verifica se p^ e nodo externo
	return (p->nt == Externo);
}

TAP CriaNoInt(TAP *Esq,  TAP *Dir, int i){
	TAP p = (TAP)malloc(sizeof(AP));
	p->nt = Interno;
	p->NO.NInterno.Esq = *Esq;
	p->NO.NInterno.Dir = *Dir;
	p->NO.NInterno.Index = i;
	return p;
}

TAP CriaNoExt(int k){
	TAP p = (TAP)malloc(sizeof(AP));
	p->nt = Externo;
	p->NO.Chave = k;
	return p;
}

void Pesquisa(TAP main, int k, int D){
	if (EExterno(main)){
		if (k == main->NO.Chave)
			printf("Elemento encontrado\n");
		else
			printf("Elemento nao encontrado\n");
		return;
	}
	if (Bit(main->NO.NInterno.Index, k, D) == 0)
		Pesquisa(main->NO.NInterno.Esq, k, D);
	else
		Pesquisa(main->NO.NInterno.Dir, k, D);
}

TAP InsereEntre(TAP *main, int k, int i, int D){
	TAP p;
	if (EExterno(*main) || i < (*main)->NO.NInterno.Index) {	//	cria um novo no externo
		p = CriaNoExt(k);
		if (Bit(i, k, D) == 1)
			return (CriaNoInt(main, &p, i));
		else
			return (CriaNoInt(&p, main, i));
	}
	else{
		if (Bit((*main)->NO.NInterno.Index, k, D) == 1)
			(*main)->NO.NInterno.Dir = InsereEntre(&(*main)->NO.NInterno.Dir, k, i, D);
		else
			(*main)->NO.NInterno.Esq = InsereEntre(&(*main)->NO.NInterno.Esq, k, i, D);
		return (*main);
	}
}

TAP Insere(TAP *main, int k, int D){
	TAP p;
	int i;
	if (!(*main))
		return (CriaNoExt(k));
	else {
		p = *main;
		while (!EExterno(p)) {
			if (Bit(p->NO.NInterno.Index, k, D) == 1)
				p = p->NO.NInterno.Dir;
			else
				p = p->NO.NInterno.Esq;
		}
		//	acha o primeiro bit diferente
		i = 1;
		while ((i <= D) & (Bit((int)i, k, D) == Bit((int)i, p->NO.Chave, D)))
			i++;
		if (i > D) {
			printf("Erro: chave ja esta na arvore\n");
			return (*main);
		}
		else
			return (InsereEntre(main, k, i, D));
    }
}
