#ifndef CUBO_H
#define CUBO_H

#include <array>
#include <string>

constexpr int COR_U = 0, COR_D = 1, COR_F = 2, COR_B = 3, COR_L = 4, COR_R = 5;
extern const std::array<char,6>        LETRA_COR; 
extern const std::array<std::string,6> NOME_COR;  

constexpr int U0 = 0, D0 = 4, F0 = 8, B0 = 12, L0 = 16, R0 = 20;

extern const std::array<char,12> MOVIMENTOS;

struct Corner {
    int cLR, cUD, cFB; 
};

struct CubeState {
    Corner cubo[2][2][2];

    CubeState(); 

    std::string key() const;
    bool isSolved() const;  

    std::array<int,24> paraAdesivos() const;

    void print() const;
};

CubeState aplicarMovimento(const CubeState& in, char mv);

CubeState embaralhar(unsigned seed, int quantidadeMovimentos, std::string& movimentosUsados);

void mostrarLegenda();

void jogarManualmente(CubeState& cubo);

void aplicarSequenciaComAnimacao(CubeState& estado, const std::string& seq);

#endif