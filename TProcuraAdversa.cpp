#include "TProcuraAdversa.h"
#include <stdio.h>
#include <time.h>

// valor de infinito (vitoria/derrota), omissao 1000
int TProcuraAdversa::infinito = 1000;
// controlo para indicar se a procura foi realizada de forma completa (c.c. foi cortada)
bool TProcuraAdversa::completo;
// hashtable / valor e nível obtido
TValorEstado TProcuraAdversa::valorHT[TAMANHO_HASHTABLE];
// profundidade máxima no método iterativo
int TProcuraAdversa::nivelOK = 0;


TProcuraAdversa::TProcuraAdversa(void) : minimizar(true), indiceHT(-1)
{
	
}

TProcuraAdversa::~TProcuraAdversa(void)
{
}

void TProcuraAdversa::ResetParametros()
{
	static const char* nomesAlgoritmos[] = { "MiniMax",	"MiniMax alfa/beta" };
	TProcuraConstrutiva::ResetParametros();

	// adicionar parâmetros da procura adversa
	// alterar algoritmos
	parametro[algoritmo] = { 1,1,2,"Algoritmo","Seleção do algoritmo de procura adversa base.", nomesAlgoritmos };

	parametro[limite].valor = 0; // procura iterativa preferencial

	// O "infinito" é dependente do problema, não faz sentido alterar senão no código
	parametro.Add({ 0,0,2, "Ordenar","0 não ordena sucessores, 1 ordena por heurística, 2 usa o melhor valor de procuras anteriores.",NULL });
	parametro.Add({ 0,0,1000, "PodaHeuristica","0 não existe poda, caso contrário é o número máximo de sucessores a considerar (tem de se ordenar sucessores).",NULL });
	parametro.Add({ 0,0,10000, "PodaCega","Igual a PodaHeuristica, mas é efetuado de forma aleátoria, sem calcular a heurística. Utilizar um valor sempre maior que Poda. ",NULL });

}


///////////////////////////////////////////////////////////////////////////////
// Algoritmo MiniMax
///////////////////////////////////////////////////////////////////////////////
// retorna o valor do estado actual, apos procura de profundidade nivel
int TProcuraAdversa::MiniMax(int nivel)
{
	DebugChamada();
	// no final da árvore, retornar (valor da heuristica)
	if (nivel == 1 || Parar()) {
		completo = false; // árvore cortada, a procura deixa de ser completa
		return NoFolha(true, nivel);
	}

	if (nivel == 0)
		return MetodoIterativo(false);

	// expandir o estado
	TVector<TNo> sucessores;
	Sucessores(sucessores);
	// caso não existam sucessores, é como se fosse um nó folha
	if (sucessores.Count() == 0)
		return NoFolha(false,nivel);

	TVector<int> id; // índice para ordenar os sucessores por heurística
	OrdenarSucessores(sucessores, id, nivel);

	int resultado = 0, melhor = -1;
	// processar os sucessores
	for (int i = 0; i < sucessores.Count(); i++) {
		DebugExpansao(i, sucessores.Count(), minimizar);
		int valor;
		TValorEstado valorConhecido;

		if (parametro[ordenarSucessores].valor == 2 &&
			((TProcuraAdversa*)sucessores[id[i]])->ValorEstado(valorConhecido, ler) &&
			valorConhecido.nivel >= nivel - 1) 
		{
			valor = valorConhecido.valor;
		} 
		else {
			// chamada recursiva, com um nível a menos, idêntico à procura em profundidade
			valor = ((TProcuraAdversa*)sucessores[id[i]])->MiniMax(nivel - 1);
			if (parametro[ordenarSucessores].valor == 2) {
				valorConhecido = { valor, nivel - 1 };
				((TProcuraAdversa*)sucessores[id[i]])->ValorEstado(valorConhecido, gravar);
			}
		}

		// pretende-se obter o melhor resultado, que oscila entre minimizar ou maximizar
		if (i == 0 || (minimizar ? resultado > valor : resultado < valor)) {
			resultado = valor;
			melhor = id[i];
			if (nivel > 1 && parametro[nivelDebug].valor >= 2) { // colocar valor actual alterado
				NovaLinha();
				printf("(%d)", resultado);
			}
		}
	}
	// todos os sucessores analizados, se houver uma solução melhor, retornar
	if (melhor >= 0) {
		if (solucao != NULL)
			delete solucao;
		solucao = sucessores[melhor];
	}
	DebugCorte(sucessores.Count(),minimizar);
	LibertarVector(sucessores, melhor);
	return resultado;
}

void TProcuraAdversa::OrdenarSucessores(
	TVector<TNo>& sucessores, TVector<int>& id, int nivel) 
{
	if (parametro[podaCega].valor > 0 && 
		sucessores.Count() > parametro[podaCega].valor) 
	{
		// podar de forma aleatória, nem heurística deve ser calculada
		while (sucessores.Count() > parametro[podaCega].valor) {
			int id = TRand::rand() % sucessores.Count();
			delete sucessores[id];
			sucessores[id] = sucessores.Last();
			sucessores.Pop();
		}
	}

	id.Count(0);
	if (nivel>2 && parametro[ordenarSucessores].valor >= 1) {
		if (parametro[limite].valor != 0) // não é iterativo
			CalcularHeuristicas(sucessores, &id); // id fica ordenado por heurística
		else { // iterativo
			// obter heurísticas da HT
			TValorEstado valorConhecido;
			TVector<int> heuristicas;
			// obter os valores conhecidos, e utilizar na heurística
			for (int i = 0; i < sucessores.Count(); i++) {
				if (((TProcuraAdversa*)sucessores[i])->ValorEstado(valorConhecido, ler)) 
					sucessores[i]->heuristica = valorConhecido.valor;
				else
					sucessores[i]->heuristica = sucessores[i]->Heuristica();
				heuristicas.Add(sucessores[i]->heuristica);
			}
			heuristicas.Sort(&id); // ordenar id
		}

		if (!minimizar)
			id.Invert();

		// podar sucessores
		if (parametro[podaHeuristica].valor > 0 &&
			parametro[podaHeuristica].valor < id.Count()) {
			TVector<TNo> manterSuc;
			for (int i = 0; i < parametro[podaHeuristica].valor; i++)
				manterSuc.Add(sucessores[id[i]]);
			for (int i = parametro[podaHeuristica].valor; i < id.Count(); i++) 
				delete sucessores[id[i]];
			sucessores = manterSuc;
			id.Count(0);
			for (int i = 0; i < sucessores.Count(); i++)
				id.Add(i);
		}

	}
	else
		for (int i = 0; i < sucessores.Count(); i++)
			id.Add(i);
}

// iteração, aumentando o nível progressivamente
int TProcuraAdversa::MetodoIterativo(int alfaBeta) {
	int resultado = 0, resOK = 0, nivel = 0;
	nivelOK = 0;
	TProcuraConstrutiva* solOK = NULL;
	do {
		DebugIteracao(nivel + 1);
		completo = true;
		// chamar a profundidade nível 1, e se não resolver, o nível 2, e assim sucessivamente
		resultado = (alfaBeta ? MiniMaxAlfaBeta(++nivel) : MiniMax(++nivel));
		if (!Parar() || nivel == 0) {
			// guardar a última solução realizada sem interrupções, bem como o resultado
			if (solOK != NULL)
				delete solOK;
			solOK = solucao;
			solucao = NULL;
			resOK = resultado;
			nivelOK = nivel;
			if (parametro[nivelDebug].valor > 0 && solOK != NULL)
				printf("\n%d: %s (%d)", nivel, Acao(solOK), resultado);
		}
		else
			completo = false;
	} while (!completo && !Parar());
	if (solucao != NULL)
		delete solucao;
	solucao = solOK;
	return resOK;
}


// fim da procura, por corte de nível (ou não haver sucessores), retornar heurística
int TProcuraAdversa::NoFolha(bool nivel, int valor) {
	int resultado;
	TValorEstado valorConhecido;
	DebugCorte(nivel ? -1 : 0, minimizar);

	if (parametro[ordenarSucessores].valor == 2) {
		if (!ValorEstado(valorConhecido, ler)) {
			resultado = Heuristica();
			valorConhecido = { resultado,0 };
			ValorEstado(valorConhecido, gravar);
		}
		else
			resultado = heuristica = valorConhecido.valor;
	} else
		resultado = Heuristica();

	if (resultado >= infinito) {
		resultado = infinito + valor; // subtrair
		// jogo ganho pelo branco
		// se o agente minimiza, tentar o jogo mais longo possível
		// caso contrário, quanto maior o jogo pior, quer o jogo mais curto
		// a minimizar, entre um nível 10 e 20, irá preferir o 20, já que -20 é menor que -10
		// a maximizar, entre um nível 10 e 20, irá preferir o 10, já que 10 é menor que 20
	}
	else if (resultado <= -infinito) {
		resultado = -valor - infinito; // somar
		// jogo ganho pelo preto
		// se o agente maximiza, tentar o jogo mais longo possível
		// caso contrário, quanto maior o jogo pior, quer o jogo mais curto
		// a maximizar, entre 10 e 20, irá preferir 20, sempre é maior
		// a minimizar, entre 10 e 20, irá preferir 10 que é menor
	}
	if (parametro[nivelDebug].valor > 1)
		printf(" %d", resultado);
	return resultado;
}

///////////////////////////////////////////////////////////////////////////////
// Algoritmo MiniMaxAlfaBeta
///////////////////////////////////////////////////////////////////////////////
// retorna o valor do estado actual, apos procura de profundidade nivel
// idêntico a MiniMax
int TProcuraAdversa::MiniMaxAlfaBeta(int nivel, int alfa, int beta)
{
	DebugChamada();
	if (nivel == 1 || Parar()) {
		completo = false; 
		return NoFolha(true, nivel);
	}

	if (nivel == 0)
		return MetodoIterativo(true);

	TVector<TNo> sucessores;
	Sucessores(sucessores);
	if (sucessores.Count() == 0)
		return NoFolha(false,nivel);

	TVector<int> id; // índice para ordenar os sucessores por heurística
	OrdenarSucessores(sucessores, id, nivel);

	int resultado = 0, melhor = -1;
	for (int i = 0; i < sucessores.Count(); i++) {
		DebugExpansao(i, sucessores.Count(), minimizar);
		int valor;
		TValorEstado valorConhecido;

		if (parametro[ordenarSucessores].valor == 2 &&
			((TProcuraAdversa*)sucessores[id[i]])->ValorEstado(valorConhecido, ler) &&
			valorConhecido.nivel >= nivel - 1)
		{
			valor = valorConhecido.valor;
		}
		else {
			// chamada recursiva, com um nível a menos, idêntico à procura em profundidade
			valor = ((TProcuraAdversa*)sucessores[id[i]])->MiniMaxAlfaBeta(nivel - 1, alfa, beta);
			if (parametro[ordenarSucessores].valor == 2) {
				valorConhecido = { valor, nivel - 1 };
				((TProcuraAdversa*)sucessores[id[i]])->ValorEstado(valorConhecido, gravar);
			}
		}

		if (i == 0 || (minimizar ? resultado > valor : resultado < valor)) {
			resultado = valor;
			melhor = id[i];
			if (nivel > 1 && parametro[nivelDebug].valor >= 2) { // colocar valor actual alterado
				NovaLinha();
				printf("(%d)", resultado);
			}
		}
		// corte alfa/beta bem como atualização
		if (i < sucessores.Count() - 1) { // nao interessa cortar quando não há mais nada para cortar
			if (CorteAlfaBeta(resultado, alfa, beta))
				break;
		}
	}
	if (melhor >= 0) {
		if(solucao!=NULL)
			delete solucao;
		solucao = sucessores[melhor];
	}
	DebugCorte(sucessores.Count(),minimizar);
	LibertarVector(sucessores,melhor);
	return resultado;
}

// verifica se há um corte alfa/beta, atualizando alfa e beta
bool TProcuraAdversa::CorteAlfaBeta(int valor, int& alfa, int& beta) {
	if (minimizar) { // pretas
		if (alfa >= valor) {
			// corte alfa
			if (parametro[nivelDebug].valor > 1) {
				// substituir o ultimo caracter por um corte
				ramo.Last() = '>';
				NovaLinha();
				printf(" alfa");
			}
			return true; // as brancas tem uma alternativa, e escusado continuar a procurar aqui
		}
		// atualização beta
		if (beta > valor)
			beta = valor;
	}
	else { // brancas
		if (beta <= valor) {
			// corte beta
			if (parametro[nivelDebug].valor > 1) {
				ramo.Last() = '>';
				NovaLinha();
				printf(" beta");
			}
			return true; // as pretas tem uma alternativa, e escusado continuar a procurar aqui
		}
		// atualização alfa
		if (alfa < valor)
			alfa = valor;
	}
	// não há corte
	return false;
}

// utilizar para executar testes empíricos, utilizando todas as instâncias,
// Utiliza as configurações existentes, ou parâmetros atuais
// Efetua um torneio entre configurações
void TProcuraAdversa::TesteEmpirico(int inicio, int fim, bool mostrarSolucoes) {
	TVector<int> atual;
	int backupID = instancia.valor;
	if (inicio == NAO_LIDO)
		inicio = instancia.min;
	if (fim == NAO_LIDO)
		fim = instancia.max;
	Dominio(inicio, instancia.min, instancia.max);
	Dominio(fim, instancia.min, instancia.max);
	ConfiguracaoAtual(atual, ler);
	if (configuracoes.Count() == 0) {
		// não foram feitas configurações, utilizar a atual 
		configuracoes.Count(1);
		configuracoes.Last() = atual;
	}
	if (configuracoes.Count() == 1) {
		// apenas há uma configuração, utilizar a atual como segunda
		configuracoes.Count(2);
		configuracoes.Last() = atual;
	}

	TVector<TVector<int>> torneio; // pares de configurações: 1 melhor, 0 igual -1 pior
	torneio.Count(configuracoes.Count());
	for (int i = 0; i < configuracoes.Count(); i++) {
		torneio[i].Count(configuracoes.Count());
		torneio[i].Reset(0);
	}

	// dois jogadores, brancas é o primeiro a jogar, pretas é o segundo
	for (int brancas = 0; brancas < configuracoes.Count(); brancas++)
		for (int pretas = 0; pretas < configuracoes.Count(); pretas++)
			if (brancas != pretas) {
				if(mostrarSolucoes)
					printf("\nMatch %d vs %d:", brancas + 1, pretas + 1);
				for (instancia.valor = inicio; instancia.valor <= fim; instancia.valor++) {
					int resultado = -1, njogada=0;
					clock_t inicioCorrida;
					// carregar instância
					SolucaoVazia();
					// jogar ora de brancas ora de pretas, até o jogo terminar
					while (!SolucaoCompleta()) {
						ConfiguracaoAtual(configuracoes[njogada % 2 == 0 ? brancas : pretas], gravar);
						TRand::srand(parametro[seed].valor);
						inicioCorrida = clock();
						LimparEstatisticas(inicioCorrida);
						resultado = ExecutaAlgoritmo();
						if (solucao != NULL) { // efetuado um lance
							const char* strAcao = Acao(solucao);
							Copiar(solucao);
							if (mostrarSolucoes) {
								if (parametro[verAcoes].valor == 1 ||
									njogada% parametro[verAcoes].valor==0) {
									printf("\n%s %d %s",
										(njogada % 2 == 0 ? "Brancas" : "Pretas"),
										(njogada / 2) + 1, strAcao);
									Debug();
									printf(" f:%d ", resultado);
								}
								else // mostrar apenas a ação
									printf(" %s", strAcao);
							}
							njogada++;
						}
						else {
							break; // não há lance efetuado
							resultado = 0; // erro, mas considerar empatado
						}
					}
					// jogo terminou, registar resultdo
					if (resultado != 0) {
						bool inverter;
						// brancas e minimizar ou pretas e maximizar, inverter
						inverter = (njogada % 2 == 0) && minimizar || 
							(njogada % 2 == 1) && !minimizar;
						// vitória/derrota branca/preta
						torneio[brancas][pretas] += (resultado < 0 ? -1 : 1) * (inverter ? -1:1);
						if (mostrarSolucoes)
							printf("\nVitória %s", (inverter ? resultado < 0 : resultado > 0) ? "Branca" : "Preta");
					}
					else if(mostrarSolucoes)
						printf("\nEmpate");
				}
			}

	MostrarTorneio(torneio, true);
	MostrarConfiguracoes(1);
	printf("\n");
	ConfiguracaoAtual(atual, gravar);
	instancia.valor = backupID;
	TRand::srand(parametro[seed].valor);
	SolucaoVazia();
}


int TProcuraAdversa::ExecutaAlgoritmo() {
	int resultado = -1;
	switch (parametro[algoritmo].valor) {
	case 1: resultado = MiniMax(Dominio(parametro[limite].valor, 0)); break;
	case 2: resultado = MiniMaxAlfaBeta(Dominio(parametro[limite].valor, 0)); break;
	}
	return resultado;
}

// necessário redefinir para atualizar valorHT em caso de colisão
void TProcuraAdversa::SubstituirHT(int indice) {
	TProcuraConstrutiva::SubstituirHT(indice);
	valorHT[indiceHT = indice].nivel = -1;
}

bool TProcuraAdversa::ExisteHT() {
	// caso não exista, retorna falso e insere
	// se existe retorna falso à mesma para não remover, dado que é uma alternativa para este lance concreto
	// se estiver lá outro elemento, substitui
	unsigned int original = Hash();
	unsigned int indice = original % TAMANHO_HASHTABLE;
	for (int i = 0; i < tamanhoCodificado; i++)
		if (elementosHT[indice][i] != estadoCodHT[i]) {
			SubstituirHT(indice);
			return false; // não existia
		}
	indiceHT = indice;
	return false; // é como se não existisse, mas está lá
}


bool TProcuraAdversa::ValorEstado(TValorEstado& valor, int operacao) {
	if (operacao == ler) {
		if (indiceHT >= 0 && valorHT[indiceHT].nivel >= 0) {
			valor = valorHT[indiceHT];
			return true;
		}
		return false;
	}
	// gravar
	if (indiceHT > 0 && valorHT[indiceHT].nivel < valor.nivel)
		valorHT[indiceHT] = valor;
	return true;
}



