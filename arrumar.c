/**
*   @author Patrick Araújo: https://github.com/patrickaraujo
*   Árvore Patricia
*   Referência 1: http://www2.dcc.ufmg.br/livros/algoritmos/cap5/slides/c/completo1/cap5.pdf
*   Referência 2: http://www2.dcc.ufmg.br/livros/algoritmos-edicao2/cap5/codigo/c/5.16a5.21-patricia.c
*/

#include<stdlib.h>
#include <sys/time.h>
#define D	8	//	depende de ChaveTipo

typedef unsigned char ChaveTipo;

//	a definir, dependendo da aplicacao
typedef unsigned char IndexAmp;

typedef unsigned char Dib;

typedef enum {
	Interno, Externo
}NoTipo;

typedef struct PatNo* Arvore;

typedef struct PatNo {
	NoTipo nt;
	union {
		struct {
			IndexAmp Index;
			Arvore Esq, Dir;
		}NInterno;
		ChaveTipo Chave;
	}NO;
}PatNo;

Dib Bit(IndexAmp i, ChaveTipo k){	//	Retorna o i-esimo bit da chave k a partir da esquerda
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

short EExterno(Arvore p){	//	Verifica se p^ e nodo externo
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

Arvore CriaNoExt(ChaveTipo k){
	Arvore p;
	p = (Arvore)malloc(sizeof(PatNo));
	p->nt = Externo;
	p->NO.Chave = k;
	return p;
}

void Pesquisa(ChaveTipo k, Arvore t){
	if (EExterno(t)){
		if (k == t->NO.Chave)
			printf("Elemento encontrado\n");
		else
			printf("Elemento nao encontrado\n");
		return;
	}
	if (Bit(t->NO.NInterno.Index, k) == 0)
		Pesquisa(k, t->NO.NInterno.Esq);
	else
		Pesquisa(k, t->NO.NInterno.Dir);
}

Arvore InsereEntre(ChaveTipo k, Arvore *t, int i){
	Arvore p;
	if (EExterno(*t) || i < (*t)->NO.NInterno.Index) {	//	cria um novo no externo
		p = CriaNoExt(k);
		if (Bit(i, k) == 1)
			return (CriaNoInt(i, t, &p));
		else
			return (CriaNoInt(i, &p, t));
	}
	else{
		if (Bit((*t)->NO.NInterno.Index, k) == 1)
			(*t)->NO.NInterno.Dir = InsereEntre(k, &(*t)->NO.NInterno.Dir, i);
		else
			(*t)->NO.NInterno.Esq = InsereEntre(k, &(*t)->NO.NInterno.Esq, i);
		return (*t);
	}
}

Arvore Insere(ChaveTipo k, Arvore *t){
	Arvore p;
	int i;
	if (!(*t))
		return (CriaNoExt(k));
	else {
		p = *t;
		while (!EExterno(p)) {
			if (Bit(p->NO.NInterno.Index, k) == 1)
				p = p->NO.NInterno.Dir;
			else
				p = p->NO.NInterno.Esq;
		}
		//	acha o primeiro bit diferente */
		i = 1;
		while ((i <= D) & (Bit((int)i, k) == Bit((int)i, p->NO.Chave)))
			i++;
		if (i > D) {
			printf("Erro: chave ja esta na arvore\n");
			return (*t);
		}
		else
			return (InsereEntre(k, t, i));
    }
}

int main(int argc, char *argv[]){
	Arvore a = NULL;
	ChaveTipo c;
	int  i, j, k, n;
	int  min = 32, max = 126;
	ChaveTipo  vetor[95];
	//	Gera uma permutacao aleatoria de chaves dos caracteres ASCII 32 a  126
	struct timeval semente;
	gettimeofday(&semente,NULL);
	srand((int)(semente.tv_sec + 1000000*semente.tv_usec));  
	for (i = min; i <= max; i++)
		vetor[i-32] = i;
	for (i = min; i <= max; i++) {
		k = min + (int) ((float)(max-min)*rand()/(RAND_MAX+1.0)); 
		j = min + (int) ((float)(max-min)*rand()/(RAND_MAX+1.0));
		n = vetor[k-32];
		vetor[k-32] = vetor[j-32];
		vetor[j-32] = n; 
	}
	//	Insere cada chave na arvore
	for (i = min; i <= max; i++) {
		c = vetor[i-32];
		printf("Inserindo chave: %c\n", c);
		a = Insere(c, &a);
    }
	//	Gera outra permutacao aleatoria de chaves
	for (i = min; i <= max; i++) {
		k = min + (int) ((float)(max-min)*rand()/(RAND_MAX+1.0));
		j = min + (int) ((float)(max-min)*rand()/(RAND_MAX+1.0));
		n = vetor[k-32];
		vetor[k-32] = vetor[j-32];
		vetor[j-32] = n;
	}
	//	Pesquisa cada chave na arvore
	for (i = min; i <= max; i++) {
		c = vetor[i-32];
		printf("Pesquisando chave: %c\n", c);
		Pesquisa(c, a);
	}
	return 0;
}
