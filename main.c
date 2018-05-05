/**
*   @author Patrick Araújo: https://github.com/patrickaraujo
*   Árvore Patricia
*   Referência 1: http://www2.dcc.ufmg.br/livros/algoritmos/cap5/slides/c/completo1/cap5.pdf
*   Referência 2: http://www2.dcc.ufmg.br/livros/algoritmos-edicao2/cap5/codigo/c/5.16a5.21-patricia.c
*/

#include<stdlib.h>
#include <sys/time.h>
#include "TAP.h"
#include "TAP.c"

int main(int argc, char *argv[]){
	Arvore a = NULL;
	int  c, i, j, k, n, D = 8;  //	depende de ChaveTipo
	int  min = 32, max = 126;
	int  vetor[95];
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
		a = Insere(c, &a, D);
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
		Pesquisa(c, a, D);
	}
	return 0;
}
