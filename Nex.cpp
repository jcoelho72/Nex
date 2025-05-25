#include "Nex.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int CNex::N=4; // dimensão do tabuleiro, 4 a 11
int CNex::vizinhos[6];
// variáveis internas
TVector<int> CNex::ligado, CNex::vizinhosVazios, CNex::vizinhosBrancos;


CNex::CNex(void)
{
}

CNex::~CNex(void)
{
}

TProcuraConstrutiva* CNex::Duplicar(void)
{
	CNex* clone = new CNex;
	if (clone != NULL)
		clone->Copiar(this);
	else
		memoriaEsgotada = true;
	return clone;
}

void CNex::Copiar(TProcuraConstrutiva*objecto)
{
	for (int i = 0; i < 2; i++) {
		numero[i] = ((CNex*)objecto)->numero[i];
		letra[i] = ((CNex*)objecto)->letra[i];
	}
	minimizar = ((CNex*)objecto)->minimizar;
}

void CNex::SolucaoVazia(void)
{
	TProcuraConstrutiva::SolucaoVazia();
	minimizar = false;
	N = instancia.valor;
	for (int i = 0; i < 2; i++)
		numero[i] = letra[i] = 0;
	Precacular();
	tamanhoCodificado = 4; // o estado máximo leva 4 inteiros grandes
}

TCasa CNex::Casa(int indice) {
	bool valorAzul = _BIT(numero, indice);
	bool valorRosa = _BIT(letra, indice);
	return (valorAzul ?
		(valorRosa ? pecaBranca : pecaNumero) :
		(valorRosa ? pecaLetra : casaVazia));
}

void CNex::Casa(int indice, TCasa valor) {
	// assumir passagem de vazia>branca,numero,letra, e de branca>numero,letra, ou numero,letra>branca
	TCasa atual = Casa(indice);
	if (atual == casaVazia) {
		if (valor == pecaNumero || valor == pecaBranca)
			// marcar bit número, que estava a 0
			_XORBIT(numero, indice);
		if (valor == pecaLetra || valor == pecaBranca)
			// marcar bit letra, que estava a 0
			_XORBIT(letra, indice);
	}
	else if (atual == pecaBranca) {
		if (valor == pecaNumero || valor == casaVazia)
			// remover bit letra, que estava a 1
			_XORBIT(letra, indice);
		if (valor == pecaLetra || valor == casaVazia)
			// remover bit número, que estava a 1
			_XORBIT(numero, indice);
	}
	else if (atual == pecaNumero) {
		if (valor == pecaBranca || valor == pecaLetra)
			// marcar bit letra, que estava a 0
			_XORBIT(letra, indice);
		if(valor == pecaLetra || valor == casaVazia)
			// remover bit número, que estava a 1
			_XORBIT(numero, indice);
	}
	else if (atual == pecaLetra) {
		if (valor == pecaBranca || valor == pecaNumero)
			// marcar bit número, que estava a 0
			_XORBIT(numero, indice);
		if (valor == pecaNumero || valor == casaVazia)
			// remover bit número, que estava a 1
			_XORBIT(letra, indice);
	}
}

void CNex::Precacular() {
	// vizinhos
	vizinhos[0] = -N;
	vizinhos[1] = -N + 1;
	vizinhos[2] = -1;
	vizinhos[3] = +1;
	vizinhos[4] = +N - 1;
	vizinhos[5] = N;
}


void CNex::Sucessores(TVector<TNo>&sucessores)
{
	CNex* suc;
	int casasOcupadas = 0;
	// nos sucessores, na primeira jogada pode trocar de cor
	if (!SolucaoCompleta()) {
		// movimentos A, casa vazia par a sua cor
		for (int i = N * N - 1; i >= 0; i--)
			if (Casa(i) == casaVazia) {
				for (int j = N * N - 1; j >= 0; j--)
					if ((i != j) && Casa(j) == casaVazia) {
						// temos as duas casas vazias
						sucessores.Add(suc = (CNex*)Duplicar());
						suc->minimizar = !minimizar;
						suc->Casa(i, minimizar ? pecaNumero : pecaLetra);
						suc->Casa(j, pecaBranca);
					}
			}
			else
				casasOcupadas++;
		// movimentos B, procurando duas peças brancas e uma da sua cor
		for (int i = N * N - 1; i >= 0; i--)
			if (Casa(i) == pecaBranca) // primeira peça branca
				for (int j = i - 1; j >= 0; j--)
					if (Casa(j) == pecaBranca) // segunda peça branca
						for (int k = N * N - 1; k >= 0; k--)
							if (Casa(k) == (minimizar ? pecaNumero : pecaLetra)) {
								// temos duas peças brancas e uma peça da própria cor
								sucessores.Add(suc = (CNex*)Duplicar());
								suc->minimizar = !minimizar;
								suc->Casa(i, minimizar ? pecaNumero : pecaLetra);
								suc->Casa(j, minimizar ? pecaNumero : pecaLetra);
								suc->Casa(k, pecaBranca);
							}
		if (casasOcupadas == 2) {
			// primeira jogada, possibilidade de troca de cor 
			sucessores.Add(suc = (CNex*)Duplicar());
			// fica com a posição da peça jogada, trocando linha/coluna, mas mantém a cor
			suc->minimizar = !minimizar;
			suc->TrocaCor();
		}
		TProcuraAdversa::Sucessores(sucessores);
	}
}

void CNex::TrocaCor() {
	int pecaL = -1, pecaB = -1;
	for (int i = N * N - 1; i >= 0; i--)
		switch (Casa(i)) {
		case pecaBranca: pecaB = i; break;
		case pecaLetra: pecaL = i; break;
		}
	if (pecaL >= 0 && pecaB >= 0) {
		Casa(pecaL, casaVazia);
		Casa(pecaB, casaVazia);
		// troca de linha com coluna
		pecaL = (pecaL % N) * N + (pecaL / N);
		pecaB = (pecaB % N) * N + (pecaB / N);
		Casa(pecaL, pecaNumero);
		Casa(pecaB, pecaBranca);
	}
}


bool CNex::SolucaoCompleta(void)
{
	// solução é completa se algum dos lados estiver ligado
	return DistanciaLigacao(pecaLetra, true) == 0 || 
		DistanciaLigacao(pecaNumero, true) == 0; 
}

const char* CNex::Acao(TProcuraConstrutiva* sucessor) {
	static char str[40], aux[20];
	CNex* suc = (CNex*)sucessor;
	TVector<int> dif;
	// verificar o número de diferenças
	for (int i = 0; i < N * N; i++)
		if (Casa(i) != suc->Casa(i))
			dif.Add(i);
	if (dif.Count() >= 2 && dif.Count() <= 3) {
		strcpy(str, "");
		// ordenar no caso de jogada normal, primeiro cor, depois branco
		if (dif.Count() == 2 && suc->Casa(dif[0]) == pecaBranca) 
			// inverter
			dif.Invert();
		if (dif.Count() == 3 && Casa(dif[2]) == pecaBranca) {
			// casas da cor fica no fim
			if (Casa(dif[0]) != pecaBranca) {
				dif.Add(dif[0]);
				dif.Delete(0);
			}
			else {
				dif.Add(dif[1]);
				dif.Delete(1);
			}
		}
		for (int i = 0; i < dif.Count(); i++) {
			sprintf(aux, "%c%d", 'a' + dif[i] % N, 1 + dif[i] / N);
			strcat(str, aux);
		}
	}
	else
		return "Inv";
	return str;
}

bool CNex::Acao(const char* acao) {
	if (strlen(acao) >= 4) {
		char coluna[2];
		int linha[2];
		if (sscanf(acao, "%c%d%c%d", coluna, linha, coluna + 1, linha + 1) == 4) {
			int i = coluna[0] - 'a' + (linha[0] - 1) * N;
			int j = coluna[1] - 'a' + (linha[1] - 1) * N;
			if (Casa(i) == casaVazia && Casa(j) == casaVazia) {
				Casa(i, minimizar ? pecaNumero : pecaLetra);
				Casa(j, pecaBranca);
				minimizar = !minimizar;
				return true;
			}
		}
	}
	if (strlen(acao) >= 6) {
		char coluna[3];
		int linha[3];
		if (sscanf(acao, "%c%d%c%d%c%d", 
			coluna, linha, 
			coluna + 1, linha + 1, 
			coluna + 2, linha + 2) == 6) 
		{
			int i = coluna[0] - 'a' + (linha[0] - 1) * N;
			int j = coluna[1] - 'a' + (linha[1] - 1) * N;
			int k = coluna[2] - 'a' + (linha[2] - 1) * N;
			if (Casa(i) == pecaBranca && 
				Casa(j) == pecaBranca && 
				Casa(k) == (minimizar ? pecaNumero : pecaLetra)) 
			{
				Casa(i, minimizar ? pecaNumero : pecaLetra);
				Casa(j, minimizar ? pecaNumero : pecaLetra);
				Casa(k, pecaBranca);
				minimizar = !minimizar;
				return true;
			}
		}
	}
	if (strcmp(acao, "inv") == 0) {
		int casasOcupadas = 0;
		for (int i = N * N - 1; i >= 0; i--)
			if (Casa(i) != casaVazia)
				casasOcupadas++;
		if (casasOcupadas == 2) {
			minimizar = !minimizar;
			TrocaCor();
		}
		return true;
	}
	return false;
}

void CNex::Debug(void)
{
	static const char* marca = ":@#?";
	// letras no topo
	NovaLinha();
	printf("  ");
	for (int i = 0; i < N; i++)
		printf(" %c", 'A' + i);
	for (int i = 0; i < N; i++) {
		NovaLinha();
		for (int j = 0; j < i; j++)
			printf(" ");
		printf("%2d ", i + 1);
		for (int j = 0; j < N; j++)
			printf(" %c", marca[Casa(i, j)]);
		printf("  %d ", i + 1);
	}
	// letras na base
	NovaLinha();
	for (int j = 0; j < N; j++)
		printf(" ");
	printf("   ");
	for (int i = 0; i < N; i++)
		printf(" %c", 'A' + i);
}

void CNex::TesteManual(const char* nome)
{
	instancia = { 4,4,11, NULL, NULL, NULL };
	TProcuraAdversa::TesteManual(nome);
}

void CNex::ResetParametros() {
	TProcuraAdversa::ResetParametros();
	/* Nível 1: limite=2 ruido=-100 (profundidade 1)
	*  Nivel 2: limite=3 podaCega=1000 ruido=-50 (profundidade 2)	
	*  Nivel 3: limite=0 limiteTempo=2s podaHeur=32  (iterativo) --- base
	*  Nivel 4: limite=0 limiteTempo=5s podaHeur=32 
	*  Nível 5: limite=0 limiteTempo=10s podaHeur=32 
	*/

	/*
	parametro[algoritmo].valor = 2;
	parametro[estadosRepetidos].valor = ignorados;
	parametro[limiteTempo].valor = 1; // 1 segundo por jogada 
	parametro[ordenarSucessores].valor = 1;
	parametro[baralharSuc].valor = 1;
	parametro[ruidoHeur].valor = -10;
	parametro[podaHeuristica].valor = 64;
	parametro[podaCega].valor = 500;*/

	parametro[algoritmo].valor = 2;
	parametro[estadosRepetidos].valor = ignorados;
	parametro[limiteTempo].valor = 2; // sem limite por agora (10s)
	parametro[limite].valor = 0; // profundidade 1 - teste heurísticas
	parametro[ordenarSucessores].valor = 1;
	parametro[baralharSuc].valor = 1;
	parametro[ruidoHeur].valor = 0;
	// mesmo -100 aparenta não fazer diferença
	parametro[podaHeuristica].valor = 32;
	parametro[podaCega].valor = 0;
	// com 100 reduz bastante, visivel com profundidade 2
	// com 1000 está OK em instãncias pequenas, perde nas maiores

	// heuristica
	parametro.Add({ 0,0,10, "Maior Ameaça","0 ignora a maior ameaça, fica por igual, maior que 1 apenas liga à maior ameaça.",NULL });
	// 0, 1, 2 ou 4 não aparenta fazer diferença
	parametro.Add({ 0,0,100, "Distância","0 ignora a distância à diagonal, c.c. é a percentagem do total da distância a somar à heurística.",NULL });
	// com 0 ou 100 não aparenta fazer diferença
}

int CNex::Heuristica(void)
{
	static CNex dup;
	int distLetras = 0, distNumeros = 0;
	// colocar resultado na variável heurística
	heuristica = 0;
	// existem duas ameaças, ligação de números, ligação de letras
	// - número de movimentos necessários para terminar cada ameaça (peças brancas podem ser utiliadas duas pelo custo de 1)
	// - utilizar a ameaça mais forte com o dobro do peso (ou um fator em parâmetros)
	// - quem é a jogar, tem mais um movimento

	// Distância para ligar Letras
	distLetras = DistanciaLigacao(pecaLetra);
	// Distância para ligar Números
	distNumeros = DistanciaLigacao(pecaNumero);

	// reduzir uma unidade a quem for a jogar
	if (false && distLetras > 0 && distNumeros > 0) {
		if (minimizar)
			distNumeros--;
		else
			distLetras--;
	}

	if (distNumeros > distLetras) {
		// ameaça de letras é mais forte
		// distância 0 é infinito, distância 1 é infinito/2, distância 2 é infinito/3, e assim sucessivamente
		heuristica = infinito / (distLetras + 1);
		// distNumeros conta alguma coisa
		if (distLetras > 0)
			heuristica -= infinito / (distNumeros + 1 + parametro[heurAmeaca].valor);
	}
	else {
		// quanto menos melhor
		heuristica = -infinito / (distNumeros + 1);
		// distLetras conta alguma coisa
		if (distNumeros > 0)
			heuristica += infinito / (distLetras + 1 + parametro[heurAmeaca].valor);
	}

	// caso a diferença entre números seja baixa, 
	// e a distância para a ligação seja alta
	// dar importância à distância para a diagonal central
	if (parametro[distDiagonal].valor != 0) 
		heuristica += DistanciaDiagonal() * distNumeros * distLetras *
			parametro[distDiagonal].valor / (100 * N * N * (1+abs(distNumeros - distLetras)));

	return TProcuraAdversa::Heuristica();
}

// calcula a distância de todas as peças à diagonal principal
int CNex::DistanciaDiagonal() {
	int total = 0;
	TCasa casa;
	for (int i = N * N - 1; i >= 0; i--) {
		casa = Casa(i);
		if (casa == pecaLetra)
			total -= abs(i / N + i % N - N + 1);
		else if (casa == pecaNumero)
			total += abs(i / N + i % N - N + 1);
	}
	return total;
}


int CNex::DistanciaLigacao(TCasa lado, bool binario) {
	CNex dup;
	bool ligacao = false;
	int distancia = 0;
	dup.Copiar(this);
	ligado.Count(0);
	vizinhosVazios.Count(0);
	vizinhosBrancos.Count(0);
	for (int i = 0; i < N; i++) // vizinhos da borda de cima 
		dup.LigacoesCasa(i * (lado == pecaLetra ? 1 : N), -1, lado);
	do {
		// iniciar o processamento das casas ligadas, com a vizinhança base
		for (int i = 0; i < ligado.Count() && !ligacao; i++) {
			// casa já na borda
			ligacao = (lado == pecaLetra ? ligado[i] >= N * (N - 1) : (ligado[i] % N) == (N - 1));
			// percorrer vizinhos
			for (int j = 0; j < 6 && !ligacao; j++)
				ligacao = dup.LigacoesCasa(ligado[i] + vizinhos[j], ligado[i], lado);
		}
		if (!ligacao) { // custo de ligação 
			if (binario)
				return infinito;
			distancia++;
			ligado.Count(0); // os estados já processados não são necessários mais
			// adicionar os vizinhos brancos
			ligado += vizinhosBrancos;
			vizinhosBrancos.Count(0);
			// adicionar apenas os vizinhos brancos novamente
			for (int i = 0; i < ligado.Count() && !ligacao; i++)
				for (int j = 0; j < 6 && !ligacao; j++)
					ligacao = dup.LigacoesCasa(ligado[i] + vizinhos[j], ligado[i], lado, true); // apenas brancos 
			// adicionar os vizinhos de casas vazias e continuar
			ligado += vizinhosVazios;
			vizinhosVazios.Count(0);
			if (ligado.Count() == 0) // lado disconexo
				return infinito;
		}
	} while (!ligacao);
	return distancia;
}

bool CNex::LigacoesCasa(int casa, int origem, TCasa lado, bool brancos) {
	if (casa >= 0 && casa < N * N && // casa no tabuleiro
		(origem < 0 || abs((casa % N) - (origem % N)) <= 1)) // número de colunas para a origem é apenas 1 
	{
		if (brancos) {
			if (Casa(casa) == pecaBranca) {
				ligado.Add(casa);
				Casa(casa, (lado == pecaLetra ? pecaNumero : pecaLetra)); // casa já processada, não reutilizar
				if (lado == pecaLetra)
					return ligado.Last() >= N * (N - 1);
				return (ligado.Last() % N) == (N - 1);
			}
		} else if (lado == pecaLetra) {
			switch (Casa(casa)) {
			case pecaLetra: ligado.Add(casa); break; // casas ligadas
			case casaVazia:	vizinhosVazios.Add(casa); break; // vizinhos de casas vazias
			case pecaBranca: vizinhosBrancos.Add(casa); break; // vizinhos de casas brancas
			}
			Casa(casa, pecaNumero); // casa já processada, não reutilizar
			return ligado.Last() >= N * (N - 1); // última linha
		}
		else if (lado == pecaNumero) {
			switch (Casa(casa)) {
			case pecaNumero: ligado.Add(casa); break; // casas ligadas
			case casaVazia:	vizinhosVazios.Add(casa); break; // vizinhos de casas vazias
			case pecaBranca: vizinhosBrancos.Add(casa); break; // vizinhos de casas brancas
			}
			Casa(casa, pecaLetra); // casa já processada, não reutilizar
			return (ligado.Last() % N) == (N - 1); // última coluna
		}
	}
	return false;
}


// estados repetidos num nível podem ser obtidos por ordens distintas de movimentos, para além das simetrias
void CNex::Codifica(uint64_t estado[OBJETO_HASHTABLE]) {
	// obter o estado normalizado, para que os estados fiquem iguais a menos de operações de simetria
	// há uma só simetria, na diagonal /, pelo que não deverá compensar, atendendo a que os sucessores são binários

	TProcuraConstrutiva::Codifica(estado);
	// cópia direta dos inteiros de 64 bits
	for (int i = 0; i < 2; i++)
		estado[i] = numero[i];
	for (int i = 0; i < 2; i++)
		estado[i + 2] = letra[i];
}



