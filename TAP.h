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

int Bit(int i, int k, int D);
int EExterno(Arvore p);
Arvore CriaNoInt(int i, Arvore *Esq,  Arvore *Dir);
Arvore CriaNoExt(int k);
void Pesquisa(int k, Arvore t, int D);
Arvore InsereEntre(int k, Arvore *t, int i, int D);
Arvore Insere(int k, Arvore *t, int D);

#endif // TAP_H_INCLUDED
