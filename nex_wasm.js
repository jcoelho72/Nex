
/* Funções de acesso ao código WASM */
var Inicializar, ExecutarLance, Parametros, GetParametroX, SetParametro, ExecutarX;
const board = document.getElementById("board"); // objeto principal, global a todas as funções

Module.onRuntimeInitialized = function () {
    // Inicializar funções globalmente
    Inicializar = Module.cwrap('Inicializar', null, ['number']);
    ExecutarLance = Module.cwrap('ExecutarLance', 'boolean', ['string']);
    Parametros = Module.cwrap('Parametros', 'number', []);
    GetParametroX = Module.cwrap('GetParametro', null, ['number', 'number', 'number', 'number', 'string']);
    SetParametro = Module.cwrap('SetParametro', null, ['number', 'number']);
    ExecutarX = Module.cwrap('Executar', null, ['number', 'number']);

    // chamar a inicialização do tabuleiro  apenas após estar tudo carregado
    // antes deste ponto nenhuma função pode ser chamada
    IniciarTabuleiro(4);
};

function Executar() {
    return new Promise(resolve => {
        let acaoPtr = Module.ccall('malloc', 'number', ['number'], [20]); // Aloca memória
        let estPtr = Module.ccall('malloc', 'number', ['number'], [1024]); // Aloca memória

        board.classList.add("pensando"); // Ativa a animação
        document.getElementById("statsContent").textContent = "A pensar..."

        setTimeout(() => {
            ExecutarX(acaoPtr, estPtr); // Executa a função WebAssembly

            let acao = Module.UTF8ToString(acaoPtr); // Converte de memória WebAssembly para string
            let est = Module.UTF8ToString(estPtr); // Converte de memória WebAssembly para string
            Module.ccall('free', null, ['number'], [acaoPtr]); // Libera memória
            Module.ccall('free', null, ['number'], [estPtr]); // Libera memória

            board.classList.remove("pensando"); // Remove a animação

            resolve([acao, est]); // Agora retorna corretamente
        }, 100); // Pequeno atraso para a animação ser visível
    });
}

function GetParametro(i) {
    let valorPtr = Module.ccall('malloc', 'number', ['number'], [4]);
    let minPtr = Module.ccall('malloc', 'number', ['number'], [4]);
    let maxPtr = Module.ccall('malloc', 'number', ['number'], [4]);
    let nomePtr = Module.ccall('malloc', 'number', ['number'], [256]); // Buffer maior para o nome

    GetParametroX(i, valorPtr, minPtr, maxPtr, nomePtr);

    let valor = Module.getValue(valorPtr, 'i32');
    let min = Module.getValue(minPtr, 'i32');
    let max = Module.getValue(maxPtr, 'i32');
    let nome = Module.UTF8ToString(nomePtr);

    Module.ccall('free', null, ['number'], [valorPtr]);
    Module.ccall('free', null, ['number'], [minPtr]);
    Module.ccall('free', null, ['number'], [maxPtr]);
    Module.ccall('free', null, ['number'], [nomePtr]);

    return [valor, min, max, nome];
}

// Função para definir um cookie
function SetCookie(nome, valor, dias) {
    const dataExp = new Date();
    dataExp.setTime(dataExp.getTime() + (dias * 24 * 60 * 60 * 1000));
    const expires = "expires=" + dataExp.toUTCString();
    document.cookie = nome + "=" + valor + ";" + expires + ";path=/";
}

// Função para obter o valor de um cookie
function GetCookie(nome) {
    const name = nome + "=";
    const ca = document.cookie.split(';');
    for (let c of ca) {
        c = c.trim();
        if (c.indexOf(name) === 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}

// eventos mapeados

document.getElementById('btnLimpar').addEventListener('click', function () {
    SetCookie("resultados", "", -1); // apagar o cookie
    LerResultados();
})

document.getElementById('btnJogar').addEventListener('click', function () {
    if (board.dataset.computador > -1 || board.dataset.nJogadas > 2)  
        board.dataset.jogoValido = 0;
    board.dataset.computador = board.dataset.jogador;
    // acertar nível de jogo
    const nivel = Number(document.getElementById('nivelJogo').value);
    if (nivel == 1) { // Nível 1: Nível 1: limite=2 limiteTempo=1s podaBranca=0 (profundidade 1)
        SetParametro(8, 2); // limite
        SetParametro(4, 1); // limiteTempo
        SetParametro(17, 1); // heurHex
    } else if (nivel == 2) { // Nivel 2: limite=3 limiteTempo=1s podaBranca=0 (profundidade 2)	
        SetParametro(8, 3); // limite
        SetParametro(4, 1); // limiteTempo
        SetParametro(17, 1); // heurHex
    } else if (nivel == 3) { // Nivel 3: limite=0 limiteTempo=2s podaBranca=0
        SetParametro(8, 0); // limite
        SetParametro(4, 2); // limiteTempo
        SetParametro(17, 1); // heurHex
    } else if (nivel == 4) { // Nivel 4: limite=0 limiteTempo=5s podaBranca=0 
        SetParametro(8, 0); // limite
        SetParametro(4, 5); // limiteTempo
        SetParametro(17, 1); // heurHex
    } else if (nivel == 5) { // Nível 5: limite=0 limiteTempo=10s podaBranca=0
        SetParametro(8, 0); // limite
        SetParametro(4, 10); // limiteTempo
        SetParametro(17, 1); // heurHex
    }
    // semente aleatória, com base no tempo em milisegundos
    SetParametro(3, (Date.now() % 1000000) + 1);

    SetParametro(0, 1); // algoritmo

    if (board.dataset.jogoEmCurso == 1)
        JogarComputador();
    else
        AnalisarComputador();
});

document.getElementById("btnNovoJogo").addEventListener("click", function () {
    IniciarTabuleiro(document.getElementById("boardSize").value);
});


// Cria um tabuleiro com coordenadas axiais.
function IniciarTabuleiro(size) {
    board.innerHTML = ""; // Limpa o tabuleiro anterior
    const boardMatrix = [];
    const cellSize = 50; // Base para os cálculos (largura da célula)
    const cos30 = Math.cos(Math.PI / 6);  // ~0.866
    const sin30 = Math.sin(Math.PI / 6);  // 0.5

    size = Number(size);

    // dados:
    board.dataset.casa1 = ""; // casas selecionadas iniciais
    board.dataset.casa2 = "";
    board.dataset.casa3 = "";
    board.dataset.jogador = 0; // quem joga
    board.dataset.tipoJogada = 0; // 1 - jogada normal, 2 - jogada substituição (3 peças)
    board.dataset.corSel = "beige"; // cor de uma casa selecionada
    board.dataset.corNormal = "lightblue"; // cor de uma casa não selecionada
    board.dataset.nJogadas = 0; // número de jogadas
    board.dataset.computador = -1; // não tem lado
    board.dataset.jogoValido = 1; // jogo válido, até que o jogador automático jogue em mais que um lado
    board.dataset.jogoEmCurso = 1;
    board.partida = []; // lista de lances (meios lances)

    // partida tem de ficar vazia: 
    document.getElementById("moveList").innerHTML = "";

    if (Opcao("anotarCasas"))
        AdicionarAnotacoes(size);

    // Itera pelas linhas (r) e colunas (q)
    for (let r = 0; r < size; r++) {
        boardMatrix[r] = [];
        for (let q = 0; q < size; q++) {
            const hex = document.createElement("div");
            hex.classList.add("hex");
            // Calcula a posição:
            // x = (q + r) * (cellSize * cos30)
            // y = (q + r) * (cellSize * sin30)
            const x = (r + q) * (cellSize * cos30) * 1.1;
            const y = (size - 1 + r - q) * (cellSize * sin30) * 1.1;
            hex.style.left = `${x}px`;
            hex.style.top = `${y}px`;

            // Define a posição identificadora (colunas com letras e linhas com números)
            hex.dataset.position = String.fromCharCode(65 + q) + (r + 1);
            hex.dataset.conteudo = 0;

            // Evento de clique em cada hexágono
            hex.addEventListener("click", () =>
                handleHexClick(hex));

            board.appendChild(hex);
            boardMatrix[r][q] = hex;
        }
    }

    // dimensãoo do container do tabuleiro
    const totalWidth = ((size - 1) * cellSize * cos30 * 2 * 1.1) + cellSize + 7;
    const totalHeight = ((size - 1) * cellSize * sin30 * 2 * 1.1) + cellSize;
    board.style.width = totalWidth + "px";
    board.style.height = totalHeight + "px";

    // criar casas de indicação de jogador a jogar (nos cantos)
    const positions = [
        { x: 0, y: 0, name: "sup-esq" },
        { x: totalWidth - cellSize - 7, y: totalHeight - cellSize, name: "inf-dir" },
        { x: totalWidth - cellSize - 7, y: 0, name: "sup-dir" },
        { x: 0, y: totalHeight - cellSize, name: "inf-esq" }
    ];

    positions.forEach(pos => {
        const hex = document.createElement("div");
        hex.classList.add("hex");
        hex.style.left = `${pos.x}px`;
        hex.style.top = `${pos.y}px`;
        hex.dataset.position = pos.name;
        hex.style.background = "thistle";
        if (pos.name == "sup-esq" || pos.name == "inf-dir") 
            // Evento de clique em cada hexágono
            hex.addEventListener("click", () =>
                Inverter(true));

        board.appendChild(hex);
    });
    board.boardMatrix = boardMatrix;

    document.getElementById('btnJogar').innerHTML = "Jogar";

    Inicializar(size);
    FinalizarJogada(false);
    LerResultados();
}

function AdicionarAnotacoes(size) {
    size = Number(size);
    const cellSize = 50;
    const cos30 = Math.cos(Math.PI / 6);  // ~0.866
    const sin30 = Math.sin(Math.PI / 6);  // 0.5
    for (let r = -1; r <= size; r+=size+1) {
        for (let q = 0; q < size; q++) {
            const hex = document.createElement("div");
            hex.classList.add("anotacao");
            const x = (r + q +(r<0 ? 0.35 : -0.35)) * (cellSize * cos30) * 1.1 + 14;
            const y = (size - 1 + r - q + (r < 0 ? 0.35 : -0.35)) * (cellSize * sin30) * 1.1 + 12;
            hex.style.position = "absolute";
            hex.style.left = `${x}px`;
            hex.style.top = `${y}px`;
            hex.innerText = String.fromCharCode(65 + q); // A, B, C...

            board.appendChild(hex);
        }
    }

    for (let r = 0; r < size; r ++) {
        for (let q = -1; q <= size; q+=size+1) {
            const hex = document.createElement("div");
            hex.classList.add("anotacao");
            const x = (r + q + (q < 0 ? 0.35 : -0.35)) * (cellSize * cos30) * 1.1 + 14;
            const y = (size - 1 + r - q + (q < 0 ? -0.35 : 0.35)) * (cellSize * sin30) * 1.1 + 12;
            hex.style.position = "absolute";
            hex.style.left = `${x}px`;
            hex.style.top = `${y}px`;
            hex.innerText = r + 1; // 1, 2, 3...

            board.appendChild(hex);
        }
    }
}


// retorna as casas selecionadas atuais
function CasasSelecionadas() {
    return [
        document.querySelector(`[data-position="${board.dataset.casa1}"]`),
        document.querySelector(`[data-position="${board.dataset.casa2}"]`),
        document.querySelector(`[data-position="${board.dataset.casa3}"]`)
    ];
}

async function AnalisarComputador() {
    // Possibilidade de melhoria:
    // analisar todos os lances
    // permitir parar a análise (trocar o botão para "Parar" enquanto faz a análise)
    // marcar os que estão de acordo com o agente
    // os que são distintos, e têm variação de valor, assinalar como erros, interrogação (ou ponto de exclamação)
    // assinar a análise com o nível utilizado

    // atualmente, analisa a posição atual, indicando o lance que aconselha, e estatísticas
    let [lance, est] = await Executar(); // Chama a função WebAssembly
    //console.log("Análise executada: ", lance, "Estatísticas", est);
    document.getElementById("statsContent").textContent =
        "Lance: " + lance + "\n" + est;
}

async function JogarComputador() {
    let [lance, est] = await Executar(); // Chama a função WebAssembly
    //console.log("Ação executada: ", lance, "Estatísticas", est);

    // Atualiza o painel de estatísticas
    if (Opcao("mostraEstatisticas"))
        document.getElementById("statsContent").textContent = est;
    else
        document.getElementById("statsContent").textContent = "";

    ExecutarLanceHTML(lance);
}

function ExecutarLanceHTML(lance) {
    /* executar a ação no HTML */
    lance = lance.toUpperCase(); // Converter para maiúsculas
    let match = lance.match(/^([A-Z]\d{1,2})([A-Z]\d{1,2})([A-Z]\d{1,2})?$/);

    if (match) {
        board.dataset.casa1 = match[1]; // Primeira casa
        board.dataset.casa2 = match[2]; // segunda casa
        board.dataset.casa3 = match[3] || ""; // terceira casa (pode ser vazia)

        if (board.dataset.jogoEmCurso == 1)
            EfetuarJogadaTab();
    } else if (lance === "INV") {
        Inverter(false);
    } else {
        console.error("Formato inválido de jogada:", lance);
    }

}

// função para executar lance de inverter cores na primeira jogada
function Inverter(atualizaWASM) {
    if (board.dataset.nJogadas == 1) {
        const casaAzul = document.querySelector(`[data-conteudo="2"]`);
        const casaBranca = document.querySelector(`[data-conteudo="1"]`);

        // posição simétrica (troca linha com coluna)
        const posCasaVermelha = InverterPosicao(casaAzul.dataset.position);
        const posCasaBranca = InverterPosicao(casaBranca.dataset.position);

        const novaCasaVermelha = document.querySelector(`[data-position="${posCasaVermelha}"]`);
        const novaCasaBranca = document.querySelector(`[data-position="${posCasaBranca}"]`);

        casaAzul.dataset.conteudo = 0;
        casaAzul.innerHTML = "";
        casaBranca.dataset.conteudo = 0;
        casaBranca.innerHTML = "";

        novaCasaVermelha.innerHTML = "🔴"; // Peça vermelha
        novaCasaVermelha.dataset.conteudo = 3;
        novaCasaBranca.innerHTML = "⚪"; // Peça branca
        novaCasaBranca.dataset.conteudo = 1;

        // efetuar jogada no WASM
        if (atualizaWASM) {
            let sucesso = ExecutarLance("inv");
            //console.log("Jogada enviada: Inv. Sucesso:", sucesso);
        }

        FinalizarJogada(true);
    } else
        alert("Apenas pode mudar de lado, na primeira jogada.");
}

function InverterPosicao(posicao) {
    let letras = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Extrai a letra e o número da posição
    let letra = posicao.match(/[A-Z]/)[0]; // Obtém a primeira letra
    let numero = posicao.match(/\d+/)[0]; // Obtém o número

    // Converte a letra para número (A = 1, B = 2, ...)
    let numeroConvertido = letras.indexOf(letra) + 1;

    // Converte o número para letra
    let letraConvertida = letras[parseInt(numero) - 1];

    return `${letraConvertida}${numeroConvertido}`;
}

// peça: 0 - vazia, 1 - branca, 2 - azul, 3 - vermelha
function SetHex(hex, peca) {
    // Vetor de peças onde o índice representa a peça
    const pecas = ["", "⚪", "🔵", "🔴"];
    hex.dataset.conteudo = peca;
    // Atualiza o conteúdo do hexágono com base no vetor
    hex.innerHTML = pecas[peca] || "";
}


// Função que recebe o hex clicado e o tabuleiro
function handleHexClick(hex) {
    if (board.dataset.jogoEmCurso != 1) {
        alert("Jogo terminado. Inicie um novo jogo selecionando um tamanho para o tabuleiro.");
        return;
    }
    if (board.dataset.casa1 == "") {
        if (hex.dataset.conteudo == 0) {
            board.dataset.casa1 = hex.dataset.position;
            hex.style.background = board.dataset.corSel;
            board.dataset.tipoJogada = 1;
            // colocar peça da cor do jogador
            SetHex(hex, Number(board.dataset.jogador) === 0 ? 2 : 3);
        } else if (hex.dataset.conteudo == 1) {
            // tem peça branca, para substituir
            board.dataset.casa1 = hex.dataset.position;
            hex.style.background = board.dataset.corSel;
            board.dataset.tipoJogada = 2;
        } else
            alert('Para jogada de substituição, selecionar primeiro duas brancas, e depois uma peça da sua cor.');
    }
    else if (board.dataset.casa2 == "") {
        if (board.dataset.tipoJogada == 1) {
            if (hex.dataset.conteudo == 0) {
                board.dataset.casa2 = hex.dataset.position;
                hex.style.background = board.dataset.corSel;
                SetHex(hex, 1); // Peça branca
                EfetuarJogadaWASM("");
            } else
                alert("Tem de selecionar uma casa vazia para completar a jogada");
        } else if (board.dataset.tipoJogada == 2) {
            // selecionar outra branca
            if (hex.dataset.conteudo == 1) {
                // tem peça branca, para substituir
                board.dataset.casa2 = hex.dataset.position;
                hex.style.background = board.dataset.corSel;
            } else
                alert('Para jogada de substituição, selecionar primeiro duas brancas, e depois uma peça da sua cor.');
        }
    }
    else if (board.dataset.casa3 == "") {
        const sel1 = document.querySelector(`[data-position="${board.dataset.casa1}"]`);
        const sel2 = document.querySelector(`[data-position="${board.dataset.casa2}"]`);
        if (board.dataset.jogador == 0 && hex.dataset.conteudo == 2) {
            // selecionada peça azul
            board.dataset.casa3 = hex.dataset.position;
            SetHex(hex, 1); // Peça branca
            SetHex(sel1, 2); // Peça azul
            SetHex(sel2, 2); // Peça azul
            EfetuarJogadaWASM("");
        } else if (board.dataset.jogador == 1 && hex.dataset.conteudo == 3) {
            // selecionada peça vermelha
            board.dataset.casa3 = hex.dataset.position;
            SetHex(hex, 1); // Peça branca
            SetHex(sel1, 3); // Peça vermelha
            SetHex(sel2, 3); // Peça vermelha
            EfetuarJogadaWASM("");
        } else
            alert("Escolher uma peça da sua cor");
    }
}

// efetuar jogada no HTML
function EfetuarJogadaTab() {
    const [sel1, sel2, sel3] = CasasSelecionadas();

    if (sel1) { // processar casa 1        
        if (sel1.dataset.conteudo == 0) {
            sel1.style.background = board.dataset.corSel;
            board.dataset.tipoJogada = 1;
            // colocar peça da cor do jogador
            if (board.dataset.jogador == 0) 
                SetHex(sel1, 2); // Peça azul
            else 
                SetHex(sel1, 3); // Peça vermelha
        } else if (sel1.dataset.conteudo == 1) {
            // tem peça branca, para substituir
            sel1.style.background = board.dataset.corSel;
            board.dataset.tipoJogada = 2;
        }
    }
    if (sel2) { // processar casa 2
        if (board.dataset.tipoJogada == 1) {
            if (sel2.dataset.conteudo == 0) {
                sel2.style.background = board.dataset.corSel;
                SetHex(sel2, 1); // Peça branca
            } 
        } else if (board.dataset.tipoJogada == 2) {
            // selecionar outra branca
            if (sel2.dataset.conteudo == 1) {
                // tem peça branca, para substituir
                sel2.style.background = board.dataset.corSel;
            } 
        }
    }
    if (sel3) { // processar casa 3
        if (board.dataset.jogador == 0 && sel3.dataset.conteudo == 2) {
            // selecionada peça azul
            SetHex(sel3, 1); // Peça branca
            SetHex(sel1, 2); // Peça azul
            SetHex(sel2, 2); // Peça azul
        } else if (board.dataset.jogador == 1 && sel3.dataset.conteudo == 3) {
            // selecionada peça vermelha
            SetHex(sel3, 1); // Peça branca
            SetHex(sel1, 3); // Peça vermelha
            SetHex(sel2, 3); // Peça vermelha
        } 
    }

    FinalizarJogada(true);
}

// executar uma jogada
function EfetuarJogadaWASM(lance) {
    let jogada = "";
    if (lance == "") {
        const [sel1, sel2, sel3] = CasasSelecionadas();

        // repor fundo
        if (sel1)
            sel1.style.background = board.dataset.corNormal;
        if (sel2)
            sel2.style.background = board.dataset.corNormal;
        if (sel3)
            sel3.style.background = board.dataset.corNormal;

        // efetuar jogada no WASM
        jogada = board.dataset.casa1 + board.dataset.casa2 + board.dataset.casa3; // Concatenar valores
    } else
        jogada = lance;
    jogada = jogada.toLowerCase();
    let sucesso = ExecutarLance(jogada);
    //console.log("Jogada enviada:", jogada, "Sucesso:", sucesso);

    if (lance == "")
        FinalizarJogada(true);
}

function Opcao(label) {
    return document.getElementById(label).checked;
}

function AdicionarLanceANTIGA(lance) {
    const moveList = document.getElementById("moveList");

    lance = lance.toLowerCase();
    if (lance == "")
        lance = "inv";

    board.partida.push(lance);

    // Último item da lista (para adicionar o segundo jogador no mesmo número)
    let ultimoItem = moveList.lastElementChild;

    if (board.dataset.nJogadas % 2 === 0 || !ultimoItem) {
        // Criar um novo item na lista se for jogada ímpar (nova rodada)
        let novoItem = document.createElement("li");
        novoItem.innerHTML = `🔵${lance}`;
        moveList.appendChild(novoItem);
    } else {
        // Adicionar ao último item se for jogada par (vermelho joga depois do azul)
        ultimoItem.innerHTML += `, 🔴${lance}`;
    }

    // Incrementa o contador de jogadas
    board.dataset.nJogadas++;
}

function AdicionarLance(lance) {
    const moveList = document.getElementById("moveList");

    // Normaliza o lance
    lance = lance.toLowerCase();
    if (lance === "") lance = "inv";

    // Adiciona o lance ao histórico da partida
    board.partida.push(lance);

    // O número da jogada é obtido a partir do contador (sendo lances azuis e vermelhos)
    let numJogada = Math.ceil(board.dataset.nJogadas / 2);

    // Obtém o último item da lista (para adicionar o lance vermelho na mesma rodada)
    let ultimoItem = moveList.lastElementChild;

    if (board.dataset.nJogadas % 2 === 0 || !ultimoItem) {
        // Se for a jogada ímpar (ou se não houver item anterior), cria um novo item
        let novoItem = document.createElement("li");
        novoItem.style.display = "flex";
        novoItem.style.alignItems = "center";
        novoItem.style.marginBottom = "5px"; // Espaçamento entre os itens

        // Cria o rótulo com o número da jogada
        let moveNumber = document.createElement("span");
        moveNumber.textContent = `${numJogada+1}. `;
        moveNumber.style.width = "30px";         // Largura fixa para o número
        moveNumber.style.display = "inline-block";
        moveNumber.style.textAlign = "right";     // Alinha o número à direita
        novoItem.appendChild(moveNumber);

        // Cria uma mini tabela para abrigar os botões dos lances
        let tabela = document.createElement("table");
        tabela.style.borderCollapse = "collapse";
        tabela.style.display = "inline-table";     // Para que a tabela fique na mesma linha do número

        let row = tabela.insertRow();

        // Coluna para o lance azul
        let cellAzul = row.insertCell();
        cellAzul.style.width = "100px";            // Largura fixa
        cellAzul.style.textAlign = "left";
        cellAzul.textContent = `🔵${lance}`;

        // Coluna para o lance vermelho (inicialmente vazia)
        let cellVermelho = row.insertCell();
        cellVermelho.style.width = "100px";        // Largura fixa
        cellVermelho.style.textAlign = "left";

        // Junta a mini tabela ao item e adiciona na lista
        novoItem.appendChild(tabela);
        moveList.appendChild(novoItem);
    } else {
        // Se for a jogada par, adiciona o lance ao último item criado para completar a rodada
        let tabela = ultimoItem.querySelector("table");
        let row = tabela.rows[0];
        let cellVermelho = row.cells[1];
        cellVermelho.textContent = `🔴${lance}`;
    }

    // Incrementa o contador de jogadas
    board.dataset.nJogadas++;
}



function FinalizarJogada(trocaJog) {
    const [sel1, sel2, sel3] = CasasSelecionadas();

    // troca de jogador
    if (trocaJog) {
        AdicionarLance(board.dataset.casa1 + board.dataset.casa2 + board.dataset.casa3);
        board.dataset.jogador = 1 - Number(board.dataset.jogador);
    }

    // tipo de jogada indefinido
    board.dataset.tipoJogada = 0;
    // limpar seleção
    board.dataset.casa1 = "";
    board.dataset.casa2 = "";
    board.dataset.casa3 = "";

    // atualizar quem joga
    const supdir = document.querySelector(`[data-position="sup-dir"]`);
    const supesq = document.querySelector(`[data-position="sup-esq"]`);
    const infdir = document.querySelector(`[data-position="inf-dir"]`);
    const infesq = document.querySelector(`[data-position="inf-esq"]`);
    if (board.dataset.jogador == 0) {
        SetHex(supesq, 2); // azul
        SetHex(infdir, 2); // azul
        SetHex(supdir, 0);
        SetHex(infesq, 0);
    } else {
        SetHex(supesq, 0); 
        SetHex(infdir, 0); 
        SetHex(supdir, 3); // vermelho
        SetHex(infesq, 3); // vermelho
    }
    if (JogoTerminado()) {
        SetHex(supesq, 2); // azul
        SetHex(infdir, 2); // azul
        SetHex(supdir, 3); // vermelho
        SetHex(infesq, 3); // vermelho
        if (board.dataset.jogador == 0) {
            supdir.style.background = "gold";
            infesq.style.background = "gold";
            alert("Vitória Vermelha!");
        } else {
            supesq.style.background = "gold";
            infdir.style.background = "gold";
            alert("Vitória Azul!");
        }
        if (board.dataset.jogoValido == 1) 
            RegistoJogo()

        board.dataset.computador = -1;
        board.dataset.jogoEmCurso = 0;
        document.getElementById('btnJogar').innerHTML = "Analisar";
        board.currentMove = board.partida.length - 1;
        EventosNavegacao();
    } 

    // marcar fundo, para se ver (apenas em casas não brilhantes)
    if (Opcao("marcaUltimaJogada")) {
        // marcaUltimaJogada
        if (sel1)
            if (sel1.style.background == board.dataset.corNormal)
                sel1.style.background = board.dataset.corSel;
        if (sel2)
            if (sel2.style.background == board.dataset.corNormal)
                sel2.style.background = board.dataset.corSel;
        if (sel3)
            if (sel3.style.background == board.dataset.corNormal)
                sel3.style.background = board.dataset.corSel;
    }

    // jogada do computador
    if (board.dataset.jogoEmCurso == 1 && board.dataset.computador == board.dataset.jogador)
        JogarComputador();
}



const axialDirections = [
    { dq: 1, dr: 0 },
    { dq: 1, dr: -1 },
    { dq: 0, dr: -1 },
    { dq: -1, dr: 0 },
    { dq: -1, dr: 1 },
    { dq: 0, dr: 1 }
];

function getNeighbors(matrix, r, q) {
    const neighbors = [];
    axialDirections.forEach(dir => {
        const nr = r + dir.dr;
        const nq = q + dir.dq;
        if (nr >= 0 && nr < matrix.length && nq >= 0 && nq < matrix[0].length) {
            neighbors.push({ r: nr, q: nq });
        }
    });
    return neighbors;
}

function LimpaFundo() {
    for (let r = 0; r < board.boardMatrix.length; r++) {
        for (let c = 0; c < board.boardMatrix[r].length; c++) {
            let casa = board.boardMatrix[r][c]; 
            casa.style.background = "lightblue";
        }
    }
}


function JogoTerminado() {
    // jogador=0: conectar de cima para baixo; jogador=1: conectar da esquerda para a direita
    const jogador = 1 - Number(board.dataset.jogador);
    const target = jogador == 0 ? 2 : 3; // supondo que os valores do dataset.conteudo sejam '2' ou '3'
    const visited = new Set();
    const queue = [];
    let resultado = 0;
    let corSel = "gold";

    // Função que cria uma chave para identificar cada célula
    function key(r, q) {
        return `${r},${q}`;
    }

    LimpaFundo();

    // Inicializa a queue com as células da borda de partida, dependendo do jogador
    if (jogador == 0) {
        // Liga de cima para baixo: começa na primeira linha
        for (let q = 0; q < board.boardMatrix[0].length; q++) {
            const cell = board.boardMatrix[0][q];
            if (cell.dataset.conteudo == target) {
                cell.style.background = corSel;
                visited.add(key(0, q));
                queue.push({ r: 0, q: q });
            }
        }
    } else {
        // Liga da esquerda para a direita: começa na primeira coluna
        for (let r = 0; r < board.boardMatrix.length; r++) {
            const cell = board.boardMatrix[r][0];
            if (cell.dataset.conteudo == target) {
                cell.style.background = corSel;
                visited.add(key(r, 0));
                queue.push({ r: r, q: 0 });
            }
        }
    }

    // Busca em largura (BFS) para capturar todas as células conectadas
    while (queue.length > 0) {
        const { r, q } = queue.shift();

        // Verifica se a célula atual está na borda oposta
        if ((jogador == 0 && r === board.boardMatrix.length - 1) ||
            (jogador == 1 && q === board.boardMatrix[0].length - 1)) {
            return true;
        }

        const neighCoords = getNeighbors(board.boardMatrix, r, q);
        // Use um laço for tradicional para poder sair imediatamente em caso de sucesso
        for (let i = 0; i < neighCoords.length; i++) {
            const { r: nr, q: nq } = neighCoords[i];
            const cellKey = key(nr, nq);
            if (!visited.has(cellKey)) {
                const neighbor = board.boardMatrix[nr][nq];
                if (neighbor.dataset.conteudo == target) {
                    neighbor.style.background = corSel;
                    visited.add(cellKey);
                    queue.push({ r: nr, q: nq });
                    // Se o vizinho estiver na borda oposta, retorna imediatamente
                    if ((jogador == 0 && nr === board.boardMatrix.length - 1) ||
                        (jogador == 1 && nq === board.boardMatrix[0].length - 1)) {
                        resultado = 1;
                    }
                }
            }
        }
    }
    if (resultado == 0)
        LimpaFundo();

    return resultado != 0;
}

function LerResultados() {
    const valor = GetCookie("resultados");

    if (valor) {
        let resultados = JSON.parse(decodeURIComponent(valor));
        board.result = resultados;
    } else {
        let resultados = [];
        for (let t = 4; t <= 11; t++) {
            resultados[t - 4] = [];
            for (let n = 0; n <= 5; n++)
                resultados[t - 4][n] = [[0, 0], [0, 0]];
        }
        board.result = resultados;
    }
    // refresh da tabela de resultados
    RefreshResultados();
}

function RegistoJogo() {
    const tamanho = Number(board.boardMatrix.length);
    const nivel = board.dataset.computador == -1 ? 0 : Number(document.getElementById('nivelJogo').value);
    const lado = Number(board.dataset.computador == 0 ? 1 : 0);
    const resultado = board.dataset.jogador != board.dataset.computador ? 0 : 1;

    console.log("RegistoJogo" + tamanho + nivel + lado + resultado);

    // atualizar estrutura
    board.result[tamanho - 4][nivel][lado][resultado]++;

    // colocar estrutra nos cookies
    SetCookie("resultados", encodeURIComponent(JSON.stringify(board.result)), 30);

    // refresh da tabela de resultados
    RefreshResultados();
}

function RefreshResultados() {
    const tabela = document.querySelector("#resultsTable");

    console.log("RefreshResultados");

    // Limpa apenas o conteúdo das linhas sem remover o cabeçalho
    tabela.innerHTML = "";

    // verificar as colunas e linhas a visualizar
    let verColunas = [];
    let verLinhas = [];

    for (let t = 4; t <= 11; t++) {
        let vazia = true;
        for (let n = 0; n <= 5 && vazia; n++) {
            let c = board.result[t - 4][n];
            vazia = (c[0][0] + c[0][1] + c[1][0] + c[1][1] == 0);
        }
        if (!vazia)
            verLinhas.push(t);
    }
    for (let n = 0; n <= 5; n++) {
        let vazia = true;
        for (let t = 4; t <= 11 && vazia; t++) {
            let c = board.result[t - 4][n];
            vazia = (c[0][0] + c[0][1] + c[1][0] + c[1][1] == 0);
        }
        if (!vazia)
            verColunas.push(n);
    }

    if (verLinhas.length == 0) {
        let linha=tabela.insertRow();
        let celula = linha.insertCell();
        celula.innerHTML = "Ainda não há jogos terminados. <br>Clique nas casas para jogar!<br>Para jogar contra o computador, carregue em 'Jogar' no primeiro ou segundo lance.<br> Pode também desafiar um amigo! <br>No final do jogo pode rever e analisar com o computador.";
        return;
    }

    // colocar o cabeçalho primeiro
    let cabecalho = tabela.insertRow(); 
    let celula = cabecalho.insertCell();
    celula.innerHTML = "<b>Tabuleiro</b>";
    const nomesNiveis = ["Humano", "Fácil", "Desafiante", "Complicado", "Insano", "Impossível"];
    verColunas.forEach(n => {
        let celula = cabecalho.insertCell();
        celula.innerHTML = `<b>${nomesNiveis[n]}</b>`;
    });

    verLinhas.forEach(t => {
        let linha = tabela.insertRow(); // Adiciona uma nova linha
        let novaCelula = linha.insertCell();
        novaCelula.innerHTML = t + "x" + t;

        verColunas.forEach(n => {
            let celula = board.result[t - 4][n];
            //let conteudo = `🔵 ${celula[0][1]} 🏆 ${celula[0][0]} 😞 <br>🔴 ${celula[1][1]} 🏆 ${celula[1][0]} 😞`;
            let conteudo = `
    <table class="miniTabela">
        <tr>
            <td></td>
            <td>🏆</td>
            <td>❌</td>
        </tr>
        <tr>
            <td>🔵</td>
            <td>${NaoZero(celula[0][1])}</td>
            <td>${NaoZero(celula[0][0])}</td>
        </tr>
        <tr>
            <td>🔴</td>
            <td>${NaoZero(celula[1][1])}</td>
            <td>${NaoZero(celula[1][0])}</td>
        </tr>
    </table>
`;

            if (JSON.stringify(celula) === JSON.stringify([[0, 0], [0, 0]]))
                conteudo = "";

            let novaCelula = linha.insertCell();
            novaCelula.innerHTML = conteudo;
        });
    });
}

function NaoZero(valor) {
    return (valor == 0 ? "" : valor);
}


function jumpToMove(index) {
    const partida = [...board.partida];

    // Limpa tabuleiro e restaura jogadas até o índice desejado
    IniciarTabuleiro(board.boardMatrix.length);
    board.dataset.jogoValido = 0; // não registar vitórias
    board.partida = [...partida];
    for (let lance = 0; lance <= index; lance++) {
        EfetuarJogadaWASM(partida[lance]);
        ExecutarLanceHTML(partida[lance]);
    }
    board.partida = partida;
    board.currentMove = index;
    board.dataset.jogoEmCurso = 0;
    document.getElementById('btnJogar').innerHTML = "Analisar";
    EventosNavegacao();
}

function EventosNavegacao() {
    const moveList = document.getElementById("moveList");
    moveList.innerHTML = ''; // Limpa a lista antes de adicionar os novos itens

    let currentMove = board.currentMove;

    for (let index = 0; index < board.partida.length / 2; index++) {
        // Criar o LI como container flex para manter tudo na mesma linha
        let item = document.createElement("li");
        item.style.display = "flex";
        item.style.alignItems = "left";
        item.style.marginBottom = "5px"; // espaçamento entre os itens

        // Adiciona um span para o número do movimento, com largura fixa
        let moveNumber = document.createElement("span");
        moveNumber.textContent = `${index + 1}. `;
        moveNumber.style.width = "30px"; // largura fixa para o número
        moveNumber.style.display = "inline-block";
        moveNumber.style.textAlign = "right";
        item.appendChild(moveNumber);

        // Cria a tabela que conterá os botões dos lances
        let tabela = document.createElement("table");
        tabela.style.borderCollapse = "collapse";
        tabela.style.display = "inline-table"; // faz com que a tabela fique na mesma linha do número

        let row = tabela.insertRow();

        // Coluna da jogada azul
        let cellAzul = row.insertCell();
        cellAzul.style.width = "100px"; // largura fixa
        cellAzul.style.textAlign = "left";
        let botaoAzul = document.createElement("button");
        botaoAzul.textContent = `🔵 ${board.partida[index * 2]}`;
        botaoAzul.addEventListener("click", () => jumpToMove(index * 2));
        if (index * 2 === currentMove) {
            botaoAzul.classList.add("activeMove");
        }
        cellAzul.appendChild(botaoAzul);

        // Coluna da jogada vermelha (se houver)
        let cellVermelho = row.insertCell();
        cellVermelho.style.width = "100px"; // largura fixa
        cellVermelho.style.textAlign = "left";
        if (board.partida[index * 2 + 1]) {
            let botaoVermelho = document.createElement("button");
            botaoVermelho.textContent = `🔴 ${board.partida[index * 2 + 1]}`;
            botaoVermelho.addEventListener("click", () => jumpToMove(index * 2 + 1));
            if (index * 2 + 1 === currentMove) {
                botaoVermelho.classList.add("activeMove");
            }
            cellVermelho.appendChild(botaoVermelho);
        }
        // Incluí a tabela (com os botões) no item da lista
        item.appendChild(tabela);
        moveList.appendChild(item);
    }
}



