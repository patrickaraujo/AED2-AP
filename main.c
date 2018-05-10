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
	TAP *arvore = NULL;
    int numMaxBits = 8;
	imprimeLetraBin('B', numMaxBits);
	imprimeLetraBin('C', numMaxBits);
	imprimeLetraBin('J', numMaxBits);
	imprimeLetraBin('H', numMaxBits);
	arvore = inserePatricia(arvore, 'B', numMaxBits);
	arvore = inserePatricia(arvore, 'C', numMaxBits);
	arvore = inserePatricia(arvore, 'J', numMaxBits);
	arvore = inserePatricia(arvore, 'H', numMaxBits);
	caminhaEmOrdem(arvore);
	printf("\n");
	printf("\n");
	printf("\n");
	arvore = removePatricia(arvore, 'J');
	caminhaEmOrdem(arvore);
	printf("\n");
	printf("\n");
	printf("\n");
	arvore = removePatricia(arvore, 'C');
	arvore = removePatricia(arvore, 'B');
	caminhaEmOrdem(arvore);
	printf("\n");
	printf("\n");
	printf("\n");
	arvore = removePatricia(arvore, 'H');
	arvore = removePatricia(arvore, 'B');
	caminhaEmOrdem(arvore);

	return 0;
}
