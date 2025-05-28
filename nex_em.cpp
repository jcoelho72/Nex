
// ficheiros do projeto, para compilar com um só ficheiro no emsripten

#include "TProcuraAdversa/TProcuraConstrutiva/TRand.cpp"
#include "TProcuraAdversa/TProcuraConstrutiva/TProcuraConstrutiva.cpp"
#include "TProcuraAdversa/TProcuraAdversa.cpp"
#include "Nex.cpp"

// não colocar o main
// #include "ProcuraEngine.cpp"

// objeto global, com o estado do jogo
extern "C" {
	CNex nex;
}

// inicialização do tabuleiro vazio de um dado tamanho
extern "C" void Inicializar(int tamanho) {
	nex.instancia = { NULL, 4,4,11, NULL, NULL };
	nex.ResetParametros();
	if (tamanho < 4)
		tamanho = 4;
	if (tamanho > 11)
		tamanho = 11;
	nex.instancia.valor = tamanho;
	TRand::srand(nex.parametro[seed].valor);
	nex.SolucaoVazia();
//	printf("\nNovo tabuleiro %d.", tamanho);
}

// executar uma ação(lance) na posição atual(que o utilizador tenha feito)
extern "C" bool ExecutarLance(const char* accao) {
	//printf("\nEm ExecutarLance(%s)", accao);
	bool resultado = nex.Acao(accao); // ação executada, ficando o nex com o mesmo estado que a interface
	if (!resultado)
		nex.Debug();
	return resultado;
}

// alterar parametros de execução
extern "C" int Parametros() { // retorna número de parâmetros
	return nex.parametro.Count();
} 
extern "C" void GetParametro(int i, int& valor, int& min, int& max, char nome[256]) { // obtem informação sobre o parâmetro i
	if (i >= 0 && i < nex.parametro.Count()) {
		valor = nex.parametro[i].valor;
		min = nex.parametro[i].min;
		max = nex.parametro[i].max;
		strcpy(nome, nex.parametro[i].nome);
	}
} 
extern "C" void SetParametro(int i, int valor) { // altera o valor do parâmetro i
	if (i >= 0 && i < nex.parametro.Count()) {
		if (valor < nex.parametro[i].min)
			valor = nex.parametro[i].min;
		if (valor > nex.parametro[i].max)
			valor = nex.parametro[i].max;
		nex.parametro[i].valor = valor;
//		printf("Parametro %d valor %d.\n", i, nex.parametro[i].valor);
	}
} 

// executar o algoritmo e retornar ação(lance)
extern "C" void Executar(char accao[20], char estatisticas[1024]) {
	TVector<TNo> backup;
	clock_t inicio;
	int resultado;
	backup = nex.caminho;
	TRand::srand(nex.parametro[seed].valor);
	nex.caminho.Count(0);
	nex.LimparEstatisticas(inicio);
	resultado = nex.ExecutaAlgoritmo();
	if (nex.solucao != NULL) {
		// extrair a ação que passa do estado atual para o resultado da execução
		sprintf(accao, "%s", nex.Acao(nex.solucao));
		// o estado atual será o resultado da execução
		nex.Copiar(nex.solucao);
		delete nex.solucao;
		nex.solucao = NULL;
	}
	else 
		sprintf(accao, "Sem lance (%d)", resultado);

	// estatísticas:
	sprintf(estatisticas, "Profundidade: %d (%d%%)\nEstados: %d (%.1fs)",
		nex.nivelOK, resultado / 10,
		nex.avaliacoes, 1.0 * (clock() - inicio) / CLOCKS_PER_SEC);

	nex.caminho = backup;
}

