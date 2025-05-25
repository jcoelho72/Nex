#pragma once
#include "TProcuraAdversa.h"

typedef enum { casaVazia = 0, pecaLetra, pecaNumero, pecaBranca } TCasa;
enum { heurAmeaca = parametrosAdversas, distDiagonal };

#define _XORBIT(mapa,indice)  mapa[indice >> 6] ^= ((uint64_t)1 << (indice & 63))
#define _BIT(mapa,indice) (mapa[indice >> 6] & ((uint64_t)1 << (indice & 63)))

///////////////////////////////////////////////////////////////////////////////
//	CNex class
///////////////////////////////////////////////////////////////////////////////
//	Author: José Coelho
//	Last revision: 2025-03-19
//	Description:
//    Jogo Nex
///////////////////////////////////////////////////////////////////////////////
class CNex :
	public TProcuraAdversa
{
public:
	CNex(void);
	~CNex(void);

	// estrutura de dados
	uint64_t numero[2]; // 121 bits é o máximo necessário
	uint64_t letra[2]; // basta numero e letra, já que o branco é ambos os bits

	// variáveis da instância
	static int N; // dimensão do tabuleiro, 4 a 11
	// pré-calculado
	static int vizinhos[6];
	// variáveis internas
	static TVector<int> ligado, vizinhosVazios, vizinhosBrancos;


	// métodos virtuais redefinidos

	TProcuraConstrutiva* Duplicar(void);
	void Copiar(TProcuraConstrutiva* objecto);
	void SolucaoVazia(void);
	void Sucessores(TVector<TNo>&sucessores);
	bool SolucaoCompleta(void);
	void Debug(void);
	const char* Acao(TProcuraConstrutiva* sucessor);
	bool Acao(const char* acao);
	void TesteManual(const char* nome);
	void ResetParametros();
	void Codifica(uint64_t estado[OBJETO_HASHTABLE]);
	int Heuristica(void);

private:
	// acesso com linha/coluna
	TCasa Casa(int linha, int coluna) { return Casa(linha * N + coluna); }
	void Casa(int linha, int coluna, TCasa valor) { Casa(linha * N + coluna, valor); }
	// acesso com índice
	TCasa Casa(int indice);
	void Casa(int indice, TCasa valor);

	void Precacular();

	bool LigacoesCasa(int casa, int origem, TCasa lado, bool brancos = false);
	int DistanciaLigacao(TCasa lado, bool binario = false);
	int DistanciaDiagonal(); // calcula a distância de todas as peças à diagonal principal

	void TrocaCor();
};

/* Torneios

Torneio (#instâncias melhores):
 |-01-|-02-|
 1    | -2 | -2
 |----|----|
 2  0 |    |  2
 |----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 64
P13(PodarHard): 1000 | P14(Maior Ameaça): 2 | P15(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 64
P13(PodarHard): 500 | P14(Maior Ameaça): 2 | P15(Distância): 0


Torneio (#instâncias melhores):
 |-01-|-02-|-03-|-04-|
 1    |  0 | -2 |  0 | -2
 |----|----|----|----|
 2 -2 |    |  0 | -2 |-10
 |----|----|----|----|
 3 -2 |  2 |    |  2 | -2
 |----|----|----|----|
 4  4 |  4 |  6 |    | 14
 |----|----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 16
P13(PodarHard): 1000 | P14(Maior Ameaça): 2 | P15(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 8
P13(PodarHard): 1000 | P14(Maior Ameaça): 2 | P15(Distância): 0
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 32
P13(PodarHard): 1000 | P14(Maior Ameaça): 2 | P15(Distância): 0
Configuração 4
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 64
P13(PodarHard): 1000 | P14(Maior Ameaça): 2 | P15(Distância): 0

Torneio (#instâncias melhores):                          P13(Distância): 0
 |-01-|-02-|-03-|
 1    |  4 |  4 |  2
 |----|----|----|
 2  2 |    |  2 | -6
 |----|----|----|
 3  4 |  6 |    |  4
 |----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 100
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 25
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0

Torneio (#instâncias melhores):                  P7(Repetidos): ignorar, P11(Ordenar): 1
 |-01-|-02-|-03-|-04-|-05-|
 1    |  6 |  6 |  4 |  6 | 20
 |----|----|----|----|----|
 2  4 |    |  4 |  4 |  6 |  2
 |----|----|----|----|----|
 3  4 |  6 |    |  2 |  2 |  0
 |----|----|----|----|----|
 4 -2 |  2 |  2 |    |  2 | -2
 |----|----|----|----|----|
 5 -4 |  2 |  2 | -4 |    |-20
 |----|----|----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 0 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): gerados | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 0 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 4
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): gerados | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 5
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): gerados | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 2 | P12(Maior Ameaça): 1
P13(Distância): 0


Torneio (#instâncias melhores):                     P10(baralhar): 1
 |-01-|-02-|
 1    | -4 | -4
 |----|----|
 2  0 |    |  4
 |----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 0 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0


Torneio (#instâncias melhores):                         P9(ruido): -10
 |-01-|-02-|-03-|
 1    |  2 |  2 | -4
 |----|----|----|
 2  8 |    |  2 |  6
 |----|----|----|
 3  0 |  2 |    | -2
 |----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): 0 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -25 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0

Torneio (#instâncias melhores):                      P12(Maior Ameaça): 2
 |-01-|-02-|-03-|
 1    |  4 |  0 |  0
 |----|----|----|
 2  0 |    |  0 | -8
 |----|----|----|
 3  4 |  4 |    |  8
 |----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 1
P13(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 0
P13(Distância): 0
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Maior Ameaça): 2
P13(Distância): 0


Torneio (#instâncias melhores):                         P12(Podar): 16
 |-01-|-02-|-03-|-04-|
 1    |  2 | -2 |  2 | -8
 |----|----|----|----|
 2  0 |    | -4 |  0 |-16
 |----|----|----|----|
 3  6 |  6 |    |  4 | 24
 |----|----|----|----|
 4  4 |  4 | -2 |    |  0
 |----|----|----|----|
Configuração 1
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 0
P13(Maior Ameaça): 2 | P14(Distância): 0
Configuração 2
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 4
P13(Maior Ameaça): 2 | P14(Distância): 0
Configuração 3
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 16
P13(Maior Ameaça): 2 | P14(Distância): 0
Configuração 4
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Limite): 0 | P7(Repetidos): ignorar | P8(pesoAStar): 100
P9(ruido): -10 | P10(baralhar): 1 | P11(Ordenar): 1 | P12(Podar): 256
P13(Maior Ameaça): 2 | P14(Distância): 0

*/

