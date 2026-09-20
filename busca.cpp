// =====================================================================
//  busca.cpp  -  implementacao da FUNCAO SUCESSORA, da FUNCAO
//                AVALIADORA (heuristica) e dos algoritmos de busca
//                (BFS, Busca em Profundidade Limitada Iterativa e A*)
// =====================================================================

#include "busca.h"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <chrono>

using namespace std;

// ---------------------------------------------------------------------
// 2) FUNCAO SUCESSORA
// ---------------------------------------------------------------------
vector<pair<char,CubeState>> gerarSucessores(const CubeState& atual) {
    vector<pair<char,CubeState>> filhos;
    filhos.reserve(12);
    for (char mv : MOVIMENTOS) filhos.push_back({mv, aplicarMovimento(atual, mv)});
    return filhos;
}

// ---------------------------------------------------------------------
// 3) FUNCAO AVALIADORA (heuristica usada pela busca A*)
// ---------------------------------------------------------------------
int heuristica(const CubeState& st) {
    static const CubeState resolvido; // cubo de referencia (resolvido)
    array<int,24> atual = st.paraAdesivos();
    array<int,24> ref = resolvido.paraAdesivos();
    int foraDoLugar = 0;
    for (int i = 0; i < 24; i++) if (atual[i] != ref[i]) foraDoLugar++;
    return foraDoLugar / 8;
}

// ---------------------------------------------------------------------
// 4) ESTRUTURAS DE BUSCA
// ---------------------------------------------------------------------

// Fila (FIFO) -> Busca em Largura (BFS)
class FrontierFIFO : public Frontier {
    queue<No> fila;
public:
    void push(const No& n) override { fila.push(n); }
    No pop() override { No n = fila.front(); fila.pop(); return n; }
    bool empty() const override { return fila.empty(); }
};

// Pilha (LIFO) -> Busca em Profundidade (usada dentro do IDDFS)
class FrontierLIFO : public Frontier {
    vector<No> pilha;
public:
    void push(const No& n) override { pilha.push_back(n); }
    No pop() override { No n = pilha.back(); pilha.pop_back(); return n; }
    bool empty() const override { return pilha.empty(); }
};

// Fila de prioridade -> busca A* (ordena por f = g + h)
class FrontierPrioridade : public Frontier {
    struct Comparador {
        bool operator()(const No& a, const No& b) const {
            return (a.g + a.h) > (b.g + b.h); // menor f = maior prioridade
        }
    };
    priority_queue<No, vector<No>, Comparador> fila;
public:
    void push(const No& n) override { fila.push(n); }
    No pop() override { No n = fila.top(); fila.pop(); return n; }
    bool empty() const override { return fila.empty(); }
};

// ---------------------------------------------------------------------
// 5) LACO GENERICO DE BUSCA
// ---------------------------------------------------------------------
// Este e o laco pedido no enunciado, usado SEM NENHUMA ALTERACAO pelas
// tres estrategias (BFS, IDDFS, A*). O que muda de fora e apenas
// (a) qual implementacao de Frontier e passada, e (b) o parametro
// opcional "limiteProfundidade" (usado somente pelo IDDFS).
ResultadoBusca buscaGenerica(Frontier& estrutura, const CubeState& inicial,
                              int limiteProfundidade, long limiteSeguranca) {
    unordered_set<string> visitados;

    // 1. Adicionar estado na estrutura
    estrutura.push({inicial, "", 0, heuristica(inicial)});
    visitados.insert(inicial.key());

    ResultadoBusca resultado;

    // 2. Enquanto a estrutura nao estiver vazia
    while (!estrutura.empty()) {
        // 2.1 Remover proximo estado da estrutura
        No atual = estrutura.pop();
        resultado.estadosVisitados++;

        // 2.2 Avaliar estado -> estado final?
        if (atual.estado.isSolved()) {
            resultado.encontrado = true;
            resultado.movimentos = atual.caminho;
            return resultado;
        }

        if (resultado.estadosVisitados >= limiteSeguranca) break; // protecao

        if ((int)atual.caminho.size() >= limiteProfundidade) continue; // respeita o limite (IDDFS)

        // 2.3 Adicionar estados seguintes na estrutura
        for (auto& [mv, proximo] : gerarSucessores(atual.estado)) {
            string k = proximo.key();
            if (!visitados.count(k)) {
                visitados.insert(k);
                No filho{proximo, atual.caminho + mv, atual.g + 1, heuristica(proximo)};
                estrutura.push(filho);
            }
        }
    }

    // 3. Sem solucao
    resultado.encontrado = false;
    return resultado;
}

// --- Wrappers de cada algoritmo, todos usando o MESMO buscaGenerica ---

ResultadoBusca resolverBFS(const CubeState& inicial) {
    FrontierFIFO f;
    return buscaGenerica(f, inicial);
}

ResultadoBusca resolverAEstrela(const CubeState& inicial) {
    FrontierPrioridade f;
    return buscaGenerica(f, inicial);
}

// Busca em Profundidade Limitada Iterativa (IDDFS): repete a busca em
// profundidade aumentando o limite a cada rodada, ate achar a solucao.
ResultadoBusca resolverIDDFS(const CubeState& inicial, int limiteMaximo) {
    long totalVisitados = 0;
    for (int limite = 1; limite <= limiteMaximo; limite++) {
        FrontierLIFO f;
        ResultadoBusca r = buscaGenerica(f, inicial, limite);
        totalVisitados += r.estadosVisitados;
        if (r.encontrado) {
            r.estadosVisitados = totalVisitados;
            return r;
        }
    }
    ResultadoBusca semSolucao;
    semSolucao.encontrado = false;
    semSolucao.estadosVisitados = totalVisitados;
    return semSolucao;
}

// ---------------------------------------------------------------------
// Menu de resolucao com IA
// ---------------------------------------------------------------------
void menuResolverComIA(CubeState& estadoAtual) {
    cout << "\nQual IA deseja usar?\n";
    cout << "1 - Busca em Largura (BFS)\n";
    cout << "2 - Busca em Profundidade Limitada Iterativa (IDDFS)\n";
    cout << "3 - Busca A*\n";
    cout << "Opcao: ";
    int op; cin >> op;

    auto inicio = chrono::high_resolution_clock::now();
    ResultadoBusca r;
    string nomeAlgoritmo;

    if (op == 1) { r = resolverBFS(estadoAtual); nomeAlgoritmo = "BFS"; }
    else if (op == 2) { r = resolverIDDFS(estadoAtual); nomeAlgoritmo = "IDDFS"; }
    else if (op == 3) { r = resolverAEstrela(estadoAtual); nomeAlgoritmo = "A*"; }
    else { cout << "Opcao invalida.\n"; return; }

    auto fim = chrono::high_resolution_clock::now();
    double segundos = chrono::duration<double>(fim - inicio).count();

    cout << "\n===== Resultado (" << nomeAlgoritmo << ") =====\n";
    cout << "Estados visitados: " << r.estadosVisitados << "\n";
    cout << "Tempo: " << segundos << " s\n";

    if (!r.encontrado) {
        cout << "Sem solucao (ou limite de seguranca atingido).\n";
        return;
    }

    cout << "Quantidade de movimentos da solucao: " << r.movimentos.size() << "\n";
    cout << "Sequencia de movimentos: " << (r.movimentos.empty() ? "(cubo ja estava resolvido)" : r.movimentos) << "\n";

    cout << "\nDeseja ver a solucao sendo aplicada passo a passo? (s/n): ";
    char resp; cin >> resp;
    if (resp == 's' || resp == 'S') {
        aplicarSequenciaComAnimacao(estadoAtual, r.movimentos);
    } else {
        for (char mv : r.movimentos) estadoAtual = aplicarMovimento(estadoAtual, mv);
    }
    cout << "\nCubo resolvido!\n";
}
