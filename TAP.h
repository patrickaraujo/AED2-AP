#ifndef TAP_H_INCLUDED
#define TAP_H_INCLUDED

typedef struct AP{
	int bit;
	char info;
	struct AP* esq;
	struct AP* dir;
} TAP;

int pegaBit(char, int);
TAP* insere(TAP*, char, int);
TAP* inserePatricia(TAP*, char, int);
TAP* removePatricia(TAP*, char);
void caminhaEmOrdem(TAP*);
void imprimeLetraBin(char, int);

#endif // TAP_H_INCLUDED
