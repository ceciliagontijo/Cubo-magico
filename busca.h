#ifndef BUSCA_H
#define BUSCA_H

#include "cubo.h"
#include <string>
#include <vector>

std::vector<std::pair<char,CubeState>> gerarSucessores(const CubeState& atual);

int heuristica(const CubeState& st);

struct No {
    CubeState estado;
    std::string caminho; 
    int g = 0;      
    int h = 0;           
};

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

ResultadoBusca buscaGenerica(Frontier& estrutura, const CubeState& inicial,
                              int limiteProfundidade = 1000000000,
                              long limiteSeguranca = 4000000);


ResultadoBusca resolverBFS(const CubeState& inicial);
ResultadoBusca resolverAEstrela(const CubeState& inicial);
ResultadoBusca resolverIDDFS(const CubeState& inicial, int limiteMaximo = 25);

void menuResolverComIA(CubeState& estadoAtual);

#endif
