// =====================================================================
//  busca.h
// =====================================================================
//  Contem:
//    2) FUNCAO SUCESSORA   -> gerarSucessores()
//    3) FUNCAO AVALIADORA  -> heuristica() (isSolved() fica em cubo.h,
//                              pois e uma propriedade do proprio ESTADO)
//    4) ESTRUTURAS DE BUSCA -> Frontier (fila / pilha / fila de prioridade)
//    5) LACO GENERICO DE BUSCA -> buscaGenerica() (NAO MUDA entre
//       BFS / IDDFS / A*; so muda a Frontier usada por fora)
//    + wrappers resolverBFS / resolverIDDFS / resolverAEstrela
//    + menuResolverComIA(): menu que deixa o usuario escolher a IA
// =====================================================================

#ifndef BUSCA_H
#define BUSCA_H

#include "cubo.h"
#include <string>
#include <vector>

// ---------------------------------------------------------------------
// 2) FUNCAO SUCESSORA
// ---------------------------------------------------------------------
// A partir de um estado, gera todos os estados alcancaveis aplicando
// cada um dos 12 movimentos possiveis (6 sentidos horarios + 6 anti-horarios).
std::vector<std::pair<char,CubeState>> gerarSucessores(const CubeState& atual);

// ---------------------------------------------------------------------
// 3) FUNCAO AVALIADORA (heuristica usada pela busca A*)
// ---------------------------------------------------------------------
// Heuristica escolhida: conta quantos adesivos NAO estao na cor da
// posicao correspondente do cubo resolvido, e divide o total por 8.
// Cada movimento mexe em 12 adesivos; dividir por um numero alto
// mantem a estimativa "otimista" (nao superestima o custo restante),
// o que ajuda o A* a manter solucoes proximas do caminho minimo.
int heuristica(const CubeState& st);

// ---------------------------------------------------------------------
// 4) ESTRUTURAS DE BUSCA (a "estrutura" generica pedida no enunciado)
// ---------------------------------------------------------------------
struct No {
    CubeState estado;
    std::string caminho; // sequencia de movimentos desde o estado inicial
    int g = 0;            // custo ate aqui (quantidade de movimentos)
    int h = 0;            // heuristica (0 quando o algoritmo nao usa heuristica)
};

// Interface comum: nao importa o algoritmo, ele so sabe empilhar/
// desempilhar/perguntar se esta vazia. Isso permite reaproveitar o
// MESMO laco de busca para BFS, IDDFS e A*.
class Frontier {
public:
    virtual ~Frontier() = default;
    virtual void push(const No& n) = 0;
    virtual No pop() = 0;
    virtual bool empty() const = 0;
};

struct ResultadoBusca {
    bool encontrado = false;
    std::string movimentos;
    long estadosVisitados = 0;
};

// ---------------------------------------------------------------------
// 5) LACO GENERICO DE BUSCA
// ---------------------------------------------------------------------
//   1. Adicionar estado inicial na estrutura
//   2. Enquanto a estrutura nao estiver vazia:
//        2.1 Remover proximo estado da estrutura
//        2.2 Avaliar estado -> se for final, retorna a solucao
//        2.3 Adicionar estados seguintes (sucessores) na estrutura
//   3. Retornar "Sem solucao"
//
// limiteProfundidade so e usado pelo IDDFS (busca em profundidade
// limitada); BFS e A* chamam com o valor padrao (sem limite).
ResultadoBusca buscaGenerica(Frontier& estrutura, const CubeState& inicial,
                              int limiteProfundidade = 1000000000,
                              long limiteSeguranca = 4000000);

// --- Wrappers de cada algoritmo, todos usando o MESMO buscaGenerica ---
ResultadoBusca resolverBFS(const CubeState& inicial);
ResultadoBusca resolverAEstrela(const CubeState& inicial);
ResultadoBusca resolverIDDFS(const CubeState& inicial, int limiteMaximo = 25);

// Menu que pergunta qual IA usar, roda a busca escolhida e mostra o
// resultado (estados visitados, tempo, sequencia de movimentos), com
// a opcao de aplicar a solucao passo a passo no cubo.
void menuResolverComIA(CubeState& estadoAtual);

#endif
