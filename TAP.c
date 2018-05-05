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

int EExterno(Arvore p){	//	Verifica se p^ e nodo externo
	return (p->nt == Externo);
}

Arvore CriaNoInt(int i, Arvore *Esq,  Arvore *Dir){
	Arvore p;
	p = (Arvore)malloc(sizeof(PatNo));
	p->nt = Interno;
	p->NO.NInterno.Esq = *Esq;
	p->NO.NInterno.Dir = *Dir;
	p->NO.NInterno.Index = i;
	return p;
}

Arvore CriaNoExt(int k){
	Arvore p;
	p = (Arvore)malloc(sizeof(PatNo));
	p->nt = Externo;
	p->NO.Chave = k;
	return p;
}

void Pesquisa(int k, Arvore t, int D){
	if (EExterno(t)){
		if (k == t->NO.Chave)
			printf("Elemento encontrado\n");
		else
			printf("Elemento nao encontrado\n");
		return;
	}
	if (Bit(t->NO.NInterno.Index, k, D) == 0)
		Pesquisa(k, t->NO.NInterno.Esq, D);
	else
		Pesquisa(k, t->NO.NInterno.Dir, D);
}

Arvore InsereEntre(int k, Arvore *t, int i, int D){
	Arvore p;
	if (EExterno(*t) || i < (*t)->NO.NInterno.Index) {	//	cria um novo no externo
		p = CriaNoExt(k);
		if (Bit(i, k, D) == 1)
			return (CriaNoInt(i, t, &p));
		else
			return (CriaNoInt(i, &p, t));
	}
	else{
		if (Bit((*t)->NO.NInterno.Index, k, D) == 1)
			(*t)->NO.NInterno.Dir = InsereEntre(k, &(*t)->NO.NInterno.Dir, i, D);
		else
			(*t)->NO.NInterno.Esq = InsereEntre(k, &(*t)->NO.NInterno.Esq, i, D);
		return (*t);
	}
}

Arvore Insere(int k, Arvore *t, int D){
	Arvore p;
	int i;
	if (!(*t))
		return (CriaNoExt(k));
	else {
		p = *t;
		while (!EExterno(p)) {
			if (Bit(p->NO.NInterno.Index, k, D) == 1)
				p = p->NO.NInterno.Dir;
			else
				p = p->NO.NInterno.Esq;
		}
		//	acha o primeiro bit diferente */
		i = 1;
		while ((i <= D) & (Bit((int)i, k, D) == Bit((int)i, p->NO.Chave, D)))
			i++;
		if (i > D) {
			printf("Erro: chave ja esta na arvore\n");
			return (*t);
		}
		else
			return (InsereEntre(k, t, i, D));
    }
}
