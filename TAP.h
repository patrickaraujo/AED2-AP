#ifndef TAP_H_INCLUDED
#define TAP_H_INCLUDED

typedef enum {
	Interno, Externo
}NoTipo;

typedef struct AP* TAP;

typedef struct AP {
	NoTipo nt;
	union {
		struct {
			int Index;
			TAP Esq, Dir;
		}NInterno;
		int Chave;
	}NO;
}AP;

int Bit(int, int, int);
int EExterno(TAP);
TAP CriaNoInt(TAP*,  TAP*, int);
TAP CriaNoExt(int);
void Pesquisa(TAP, int, int);
TAP InsereEntre(TAP*, int, int, int);
TAP Insere(TAP*, int, int);

#endif // TAP_H_INCLUDED
