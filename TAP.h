#ifndef TAP_H_INCLUDED
#define TAP_H_INCLUDED

typedef enum {
	Interno, Externo
}NoTipo;

typedef struct PatNo* Arvore;

typedef struct PatNo {
	NoTipo nt;
	union {
		struct {
			int Index;
			Arvore Esq, Dir;
		}NInterno;
		int Chave;
	}NO;
}PatNo;

int Bit(int, int, int);
int EExterno(Arvore);
Arvore CriaNoInt(Arvore*,  Arvore*, int);
Arvore CriaNoExt(int);
void Pesquisa(Arvore, int, int);
Arvore InsereEntre(Arvore*, int, int, int);
Arvore Insere(Arvore*, int, int);

#endif // TAP_H_INCLUDED
