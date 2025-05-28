#include "Nex.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int CNex::N=4; // dimensão do tabuleiro, 4 a 11
int CNex::vizinhos[6];
int CNex::vizinhosExtra[6][4];
// variáveis internas
TVector<int> CNex::ligado, CNex::vizinhosVazios, CNex::vizinhosBrancos;
CNex CNex::linhaTopo; // tem as casas marcadas do topo, para verificação de conectividade 


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

/*
*   Matriz
*         -N    -N+1
*    -1    X    +1
*    N-1   N
*   Hexágono
*        -N      -N+1
*    -1       X       +1
*        N-1      N
*   ID dos vizinhos
*        0     1
*     2     X     3
*        4     5
*  Vizinhos Extra
*             -2N+1 
* -N-1     -N       -N+1    -N+2
*      -1       X        +1
*  N-2     N-1       N       N+1
*              2N-1
*   ID dos vizinhos extra
*           0
*  5     0     1     1
*     2     X     3
*  4     4     5     2
*           3
*/
void CNex::Precacular() {
	int vizCurta[] = { -N, -N + 1, -1, +1, +N - 1, N };
	int vizExtra[][4] = {
		{-2 * N + 1,0,1,1},
		{-N + 2,1,3,2},
		{N + 1,3,5,1},
		{2 * N - 1,4,5,-1},
		{N - 2,2,4,-2},
		{-N - 1,0,2,-1}
	};
	// vizinhos
	for (int i = 0; i < 6; i++) {
		vizinhos[i] = vizCurta[i];
		for (int j = 0; j < 4; j++)
			vizinhosExtra[i][j] = vizExtra[i][j];
	}
	// marcar as casas da linha de topo (letras/números)
	linhaTopo.Reset();
	for (int i = 0; i < N; i++) 
		if (i == 0)
			linhaTopo.Casa(0, pecaBranca);
		else {
			// trocar, já que ao processar as letras, 
			// colocam-se números nas casas já processadas e vice-versa
			linhaTopo.Casa(i, pecaNumero);
			linhaTopo.Casa(i * N, pecaLetra);
		}
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
				if (parametro[podaBranca].valor != 0) {
					TVector<int> casaBranca;
					for (int j = N * N - 1; j >= 0; j--)
						if ((i != j) && Casa(j) == casaVazia)
							casaBranca.Add(j);
					casaBranca.RandomOrder();
					// processar as primeiras
					for (int j = 0; j < casaBranca.Count() && j < parametro[podaBranca].valor; j++) {
						sucessores.Add(suc = (CNex*)Duplicar());
						suc->minimizar = !minimizar;
						suc->Casa(i, minimizar ? pecaNumero : pecaLetra);
						suc->Casa(casaBranca[j], pecaBranca);
					}
				} else
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
		default: break;
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
	return (DistanciaLigacao(pecaLetra, true) == 0 || 
		DistanciaLigacao(pecaNumero, true) == 0);
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
	instancia = { NULL, 4,4,11, NULL, NULL };
	TProcuraAdversa::TesteManual(nome);
}

void CNex::ResetParametros() {
	static const char* heuristicaNex[] = {
		"Sem heuristica",
		"Heuristica Hex",
		"Heurística Nex" };

	TProcuraAdversa::ResetParametros();
	/* Nível 1: limite=2 limiteTempo=1s heurNex=1 (profundidade 1)
	*  Nivel 2: limite=3 limiteTempo=1s heurNex=1 (profundidade 2)	
	*  Nivel 3: limite=0 limiteTempo=2s heurNex=1  (iterativo) --- base
	*  Nivel 4: limite=0 limiteTempo=5s heurNex=1 
	*  Nível 5: limite=0 limiteTempo=10s heurNex=1
	*/

	parametro[algoritmo].valor = 2;
	parametro[estadosRepetidos].valor = ignorados;
	parametro[limiteTempo].valor = 2; 
	parametro[limite].valor = 0; // profundidade 1 - teste heurísticas
	parametro[ordenarSucessores].valor = 2;
	parametro[baralharSuc].valor = 1;
	parametro[ruidoHeur].valor = 0;
	parametro[podaHeuristica].valor = 0;
	parametro[podaCega].valor = 0;

	// podaBranca
	parametro.Add({ "Poda Branca",0,0,121, "0 não executa a poda das peças brancas, c.c. é o número de posições brancas geradas (aleatório).",NULL });

	// heurNex
	parametro.Add({ "Heuristica",1,0,2, "Possibilidades para a heurística", heuristicaNex });

	/*
	Poda Branca: 
	Há um trade-off entre tempo e qualidade, embora não muito pronunciado, e oscilante. 
	Com profundidade fixa 1 (P9=2) é possível observar, quanto maior a instância mais observável, mas é incerto.
	Com tempo de 1s iterativo, com várias podas, jogo em 11x11:
Torneio (#instâncias melhores):
 |-01-|-02-|-03-|-04-|
 1    |  1 | -1 |  1 |  2
 |----|----|----|----|
 2 -1 |    |  1 |  1 |  0
 |----|----|----|----|
 3  1 | -1 |    | -1 |  0
 |----|----|----|----|
 4 -1 |  1 | -1 |    | -2
 |----|----|----|----|
Tempos: 179,366s 175,216s 178,777s 162,575s
Parametros comuns a 4 configurações:
P1(Algoritmo): MiniMax alfa/beta | P2(Debug): nada | P3(Ver): 4 | P4(Seed): 1
P5(Tempo): 1 | P6(Gerações): 0 | P7(Expansões): 0 | P8(Avaliações): 0
P9(Limite): 0 | P10(Repetidos): ignorar | P11(pesoAStar): 100 | P12(ruido): 0
P13(baralhar): 1 | P14(Ordenar): 1 | P15(PodaHeuristica): 0 | P16(PodaCega): 0
--- Configuração 1
P17(Poda Branca): 0
--- Configuração 2
P17(Poda Branca): 32
--- Configuração 3
P17(Poda Branca): 4
--- Configuração 4
P17(Poda Branca): 1	
	*/


}

int CNex::Heuristica(void)
{
	TVector<int> qMin, qMax;
	// colocar resultado na variável heurística
	heuristica = 0;
	// existem duas ameaças, ligação de números, ligação de letras
	// - número de movimentos necessários para terminar cada ameaça (peças brancas podem ser utiliadas duas pelo custo de 1)
	// - utilizar a ameaça mais forte com o dobro do peso (ou um fator em parâmetros)
	// - quem é a jogar, tem mais um movimento

	if (ExisteHeuritica())
		return heuristica;

	// ver primeiro informação binária
	// verificar apenas conectividade
	// Distância para ligar Letras
	if (DistanciaLigacao(pecaLetra, true, 0) == 0)
		heuristica = infinito;
	// Distância para ligar Números
	else if (DistanciaLigacao(pecaNumero, true, 0) == 0)
		heuristica = -infinito;
	else if (parametro[heurNex].valor == 1) { // Heuristica Hex
		int distLetras = 0, distNumeros = 0;
		qMin.Count(N);
		qMax.Count(N);
		qMin.Reset(0);
		qMax.Reset(0);
		// Distância para ligar Letras
		distLetras = DistanciaLigacao(pecaLetra, false, 0);
		// Distância para ligar Números
		distNumeros = DistanciaLigacao(pecaNumero, false, 0);

		Dominio(distLetras, 1, N);
		Dominio(distNumeros, 1, N);

		qMax[distLetras - 1]++;
		qMin[distNumeros - 1]++;
		heuristica = MaiorAmeaca(qMin, qMax, N);
	}
	else if (parametro[heurNex].valor == 2) { // heuristica Nex
		qMin.Count(N);
		qMax.Count(N);
		qMin.Reset(0);
		qMax.Reset(0);
		
		for (int i = 0; i < 3; i++) {
			int distLetras = 0, distNumeros = 0;
			// Distância para ligar Letras
			distLetras = DistanciaLigacao(pecaLetra, false, i);
			// Distância para ligar Números
			distNumeros = DistanciaLigacao(pecaNumero, false, i);

			if (i == 0) { // calculo das distâncias contando com brancas como sendo vazias
			}
			else if (i == 2) { // calculo das distâncias contando com brancas como sendo peças suas: somar N/2
				distLetras += N/2;
				distNumeros += N/2;
			}
			else if (i == 1) { // calculo das distâncias contando com brancas como bloqueando
				distLetras += 1;
				distNumeros += 1;
			}

			Dominio(distLetras, 1, N);
			Dominio(distNumeros, 1, N);

			qMax[distLetras - 1]++;
			qMin[distNumeros - 1]++;
		}

		heuristica = MaiorAmeaca(qMin, qMax, N);
	}

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

void CNex::LigacoesBorda(TCasa lado, bool topo, CNex* processado, bool binario) {
	if (lado == pecaLetra) {
		if (topo) {
			// vizinhos da borda de cima 
			for (int i = 0; i < N; i++) { 
				LigacoesCasa(i, lado, binario, processado);
				if (!binario && i > 0 && LigacaoTatica(i, i - 1))
					LigacoesCasa(i + N - 1, lado, binario, processado);
			}
		}
		else {
			// vizinhos da borda de baixo
			for (int i = N*N-1; i >= N*(N-1); i--) { 
				LigacoesCasa(i, lado, binario, processado);
				if (!binario && i < N * N - 1 && LigacaoTatica(i, i + 1))
					LigacoesCasa(i - N + 1, lado, binario, processado);
			}
		}
	}
	else {
		if (topo) {
			// vizinhos da borda da esquerda
			for (int i = 0; i < N * N; i += N) {
				LigacoesCasa(i, lado, binario, processado);
				if (!binario && i > 0 && LigacaoTatica(i, i - N))
					LigacoesCasa(i - N + 1, lado, binario, processado);
			}
		}
		else {
			// vizinhos da borda da direita
			for (int i = N * N - 1; i >= 0; i -= N) {
				LigacoesCasa(i, lado, binario, processado);
				if (!binario && i < N * N - 1 && LigacaoTatica(i, i + N))
					LigacoesCasa(i + N - 1, lado, binario, processado);
			}
		}
	}
}


bool CNex::VizinhoOK(int casa, int origem, bool extra) {
	if (!extra) {
		// casa dentro do mapa
		return (casa >= 0 && casa < N* N && // casa no tabuleiro
			(origem < 0 || abs((casa % N) - (origem % N)) <= 1)); // número de colunas para a origem é apenas 1 
	}
	if (casa + vizinhosExtra[origem][0] < 0 || casa + vizinhosExtra[origem][0] >= N * N)
		return false;
	// diferença entre colunas têm de ser exatamente o que é suposto
	return (((casa + vizinhosExtra[origem][0]) % N) - (casa % N) == vizinhosExtra[origem][3]);
}

void CNex::Reset()
{
	for (int i = 0; i < 2; i++)
		numero[i] = letra[i] = 0;
}


void CNex::ExpandirLigacoes(TCasa lado, CNex* processado, bool binario, int brancas) {
	// iniciar o processamento das casas ligadas, com a vizinhança base
	for (int i = 0; i < ligado.Count(); i++) {
		// percorrer vizinhos
		for (int j = 0; j < 6; j++)
			if(VizinhoOK(ligado[i] + vizinhos[j], ligado[i]))
				LigacoesCasa(ligado[i] + vizinhos[j], lado, binario, processado);
		// ver vizinhos extra
		for (int j = 0; j < 6 && !binario; j++)
			if (VizinhoOK(ligado[i], j, true) &&
				LigacaoTatica(ligado[i] + vizinhos[vizinhosExtra[j][1]],
					ligado[i] + vizinhos[vizinhosExtra[j][2]]))
				LigacoesCasa(ligado[i] + vizinhosExtra[j][0], lado, binario, processado);
		if (brancas == 2 && vizinhosBrancos.Count() > 0) { // casas brancas é como se fossem peças da cor
			ligado += vizinhosBrancos;
			vizinhosBrancos.Count(0);
		}
	}
}

// verifica se estado interseta as letras (binário), ou números
bool CNex::Intersecta(bool letras, CNex& estado) {
	if (letras)
		return (letra[0] & estado.letra[0]) || (letra[1] & estado.letra[1]);
	return (numero[0] & estado.numero[0]) || (numero[1] & estado.numero[1]);
}


// lado: lado a jogar
// binário: se true, interessa apenas se é final ou não (distância curta)
// brancas: 0 - contam como vazias; 1 - contam com bloqueantes; 2 - contam como casas da sua cor
int CNex::DistanciaLigacao(TCasa lado, bool binario, int brancas) {
	CNex dup, fim, inicio;
	bool ligacao = false;
	int distancia = 1;
	dup.Copiar(this);
	//printf("\nEstado atual: ");
	//dup.Debug();
	ligado.Count(0);
	vizinhosVazios.Count(0);
	vizinhosBrancos.Count(0);
	fim.Reset(); // guarda as casas de ligação à borda final
	dup.LigacoesBorda(lado, false, &fim, binario);
	dup.ExpandirLigacoes(lado, &fim, binario, brancas);
	//printf("\nFim:\n");
	//fim.Debug();
	if (binario) {
		if (linhaTopo.Intersecta(lado == pecaNumero, fim)) {
			//printf("\nFim do jogo:");
			//Debug();
			//fim.Debug();
			return 0;
		}
		return infinito;
	}
	inicio.Reset();
	ligado.Count(0);
	vizinhosVazios.Count(0);
	vizinhosBrancos.Count(0);
	dup.LigacoesBorda(lado, true, &inicio);
	dup.ExpandirLigacoes(lado, &inicio, false, brancas);
	//printf("\nDistancia %d:\n", distancia);
	//inicio.Debug();
	// teste de ligação é existir interseção entre números/letras (o lado oposto) de dup e fim
	while (!inicio.Intersecta(lado == pecaNumero, fim)) {
		// aumento da distância
		distancia++;

		ligado.Count(0); // os estados já processados não são necessários mais
		if (brancas == 0) {
			// adicionar os vizinhos brancos, como sse fossem vazias
			ligado += vizinhosBrancos;
			vizinhosBrancos.Count(0);
		}
		else if (brancas == 1) { // não processar brancas
			vizinhosBrancos.Count(0);
		}
		// adicionar os vizinhos de casas vazias e continuar
		ligado += vizinhosVazios;
		vizinhosVazios.Count(0);
		if (ligado.Count() == 0) // lado disconexo
			return infinito;

		dup.ExpandirLigacoes(lado, &inicio, false, brancas);
		//printf("\nDistancia %d:\n", distancia);
		//inicio.Debug();
	}
	return distancia;
}

// verificar se estas duas casas de suporte, dão ligação tática
bool CNex::LigacaoTatica(int suporte1, int suporte2) {
	int casasVazias = 0;
	if (Casa(suporte1) == casaVazia)
		casasVazias++;
	if (Casa(suporte2) == casaVazia)
		casasVazias++;
	if (casasVazias == 0)
		return false;
	if (casasVazias == 2)
		return true;
	// ou são ambas vazias, ou há uma delas com uma peça branca, mas não ambas
	return Casa(suporte1) == pecaBranca || Casa(suporte2) == pecaBranca;
}

void CNex::LigacoesCasa(int casa, TCasa lado, bool binario, CNex* fim) {
	if (lado == pecaLetra) {
		if (fim == NULL || fim->Casa(casa) != pecaNumero) {
			switch (Casa(casa)) {
			case pecaLetra: ligado.Add(casa); break; // casas ligadas
			case casaVazia:	vizinhosVazios.Add(casa); 
				if (binario)
					return; // não adiciona casas que não sejam da própria cor
				break; // vizinhos de casas vazias
			case pecaBranca: vizinhosBrancos.Add(casa); 
				if (binario)
					return; // não adiciona casas que não sejam da própria cor
				break; // vizinhos de casas brancas
			default: return; // peça adeversária, não registar no mapa
			}
			if (fim != NULL)
				fim->Casa(casa, pecaNumero);
			else
				Casa(casa, pecaNumero); // casa já processada, não reutilizar
		}
	}
	else if (lado == pecaNumero) {
		if (fim == NULL || fim->Casa(casa) != pecaLetra) {
			switch (Casa(casa)) {
			case pecaNumero: ligado.Add(casa); break; // casas ligadas
			case casaVazia:	vizinhosVazios.Add(casa); 
				if (binario)
					return; // não adiciona casas que não sejam da própria cor
				break; // vizinhos de casas vazias
			case pecaBranca: vizinhosBrancos.Add(casa); 
				if (binario)
					return; // não adiciona casas que não sejam da própria cor
				break; // vizinhos de casas brancas
			default: return; // peça adeversária, não registar no mapa
			}
			if (fim != NULL)
				fim->Casa(casa, pecaLetra);
			else
				Casa(casa, pecaLetra); // casa já processada, não reutilizar
		}
	}
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



