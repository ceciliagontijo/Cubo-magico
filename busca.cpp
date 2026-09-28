#include "busca.h"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <chrono>

using namespace std;

vector<pair<char,CubeState>> gerarSucessores(const CubeState& atual) {
    vector<pair<char,CubeState>> filhos;
    filhos.reserve(12);
    for (char mv : MOVIMENTOS) filhos.push_back({mv, aplicarMovimento(atual, mv)});
    return filhos;
}

int heuristica(const CubeState& st) {
    static const CubeState resolvido; 
    array<int,24> atual = st.paraAdesivos();
    array<int,24> ref = resolvido.paraAdesivos();
    int foraDoLugar = 0;
    for (int i = 0; i < 24; i++) if (atual[i] != ref[i]) foraDoLugar++;
    return foraDoLugar / 8;
}

class FrontierFIFO : public Frontier {
    queue<No> fila;
public:
    void push(const No& n) override { fila.push(n); }
    No pop() override { No n = fila.front(); fila.pop(); return n; }
    bool empty() const override { return fila.empty(); }
};

class FrontierLIFO : public Frontier {
    vector<No> pilha;
public:
    void push(const No& n) override { pilha.push_back(n); }
    No pop() override { No n = pilha.back(); pilha.pop_back(); return n; }
    bool empty() const override { return pilha.empty(); }
};

class FrontierPrioridade : public Frontier {
    struct Comparador {
        bool operator()(const No& a, const No& b) const {
            return (a.g + a.h) > (b.g + b.h); 
        }
    };
    priority_queue<No, vector<No>, Comparador> fila;
public:
    void push(const No& n) override { fila.push(n); }
    No pop() override { No n = fila.top(); fila.pop(); return n; }
    bool empty() const override { return fila.empty(); }
};

ResultadoBusca buscaGenerica(Frontier& estrutura, const CubeState& inicial,
                              int limiteProfundidade, long limiteSeguranca) {
    unordered_set<string> visitados;

    estrutura.push({inicial, "", 0, heuristica(inicial)});
    visitados.insert(inicial.key());

    ResultadoBusca resultado;

    while (!estrutura.empty()) {
        No atual = estrutura.pop();
        resultado.estadosVisitados++;

        if (atual.estado.isSolved()) {
            resultado.encontrado = true;
            resultado.movimentos = atual.caminho;
            return resultado;
        }

        if (resultado.estadosVisitados >= limiteSeguranca) break; 

        if ((int)atual.caminho.size() >= limiteProfundidade) continue; 

        for (auto& [mv, proximo] : gerarSucessores(atual.estado)) {
            string k = proximo.key();
            if (!visitados.count(k)) {
                visitados.insert(k);
                No filho{proximo, atual.caminho + mv, atual.g + 1, heuristica(proximo)};
                estrutura.push(filho);
            }
        }
    }

    resultado.encontrado = false;
    return resultado;
}


ResultadoBusca resolverBFS(const CubeState& inicial) {
    FrontierFIFO f;
    return buscaGenerica(f, inicial);
}

ResultadoBusca resolverAEstrela(const CubeState& inicial) {
    FrontierPrioridade f;
    return buscaGenerica(f, inicial);
}

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
