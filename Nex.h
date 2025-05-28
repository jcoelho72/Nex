#pragma once
#include "TProcuraAdversa/TProcuraAdversa.h"

typedef enum ECasaNex { casaVazia = 0, pecaLetra, pecaNumero, pecaBranca } TCasa;
enum EParametrosNex { podaBranca = parametrosAdversas, heurNex };

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
	static int vizinhosExtra[6][4]; // delta a somar a X, ID vizinho passagem 1, ID vizinho passagem 2, delta colunas
	// variáveis internas
	static TVector<int> ligado, vizinhosVazios, vizinhosBrancos;
	static CNex linhaTopo; // tem as casas marcadas do topo, para verificação de conectividade 


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

	void LigacoesCasa(int casa, TCasa lado, bool binario = false, CNex* fim = NULL);
	int DistanciaLigacao(TCasa lado, bool binario = false, int brancas = 0);
	int DistanciaDiagonal(); // calcula a distância de todas as peças à diagonal principal

	void TrocaCor();

	// verificar se estas duas casas de suporte, dão ligação tática
	bool LigacaoTatica(int suporte1, int suporte2);
	void LigacoesBorda(TCasa lado, bool topo, CNex* processado = NULL, bool binario = false);
	void ExpandirLigacoes(TCasa lado, CNex* processado = NULL, bool binario = false, int brancas = 0);
	bool VizinhoOK(int casa, int origem, bool extra = false);

	bool Intersecta(bool letras, CNex &estado); // verifica se estado interseta as letras (binário), ou números

	void Reset();
};

