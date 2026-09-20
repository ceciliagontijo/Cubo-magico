// =====================================================================
//  cubo.h
// =====================================================================
//  Contem:
//    1) ESTADO do cubo (struct CubeState) e sua representacao interna
//    2) Os movimentos de face e a funcao que aplica um movimento
//    3) Tudo que envolve o usuario MONTAR/JOGAR o cubo manualmente
//       (mostrar o cubo na tela, embaralhar, aplicar um movimento
//       digitado pelo usuario, animar uma sequencia de movimentos)
//
//  MODELAGEM DO CUBO
//  ------------------
//  Um cubo 2x2x2 so tem "cantos" (nao tem arestas nem centros). Guardamos
//  os 8 cantos numa MATRIZ 2x2x2 de verdade: cubo[x][y][z], onde cada
//  indice vale 0 ou 1:
//     x: 0 = esquerda,  1 = direita
//     y: 0 = baixo,     1 = cima
//     z: 0 = fundo,     1 = frente
//  Por exemplo, cubo[0][1][1] e o canto Cima-Esquerda-Frente.
//
//  Cada posicao da matriz guarda 3 cores: a cor voltada para o eixo X
//  (cLR), a voltada para o eixo Y (cUD) e a voltada para o eixo Z
//  (cFB). Girar uma face RODA fisicamente 4 cantos em torno de um
//  eixo: eles trocam de posicao NA MATRIZ e os adesivos de cada canto
//  trocam de eixo entre si (isso e o que da o "efeito de torcer" dos
//  cantos de um cubo magico de verdade). Modelar assim garante que so
//  estados FISICAMENTE POSSIVEIS sao gerados (o espaco de busca fica
//  do tamanho real do jogo, em vez de explodir com estados impossiveis
//  que aconteceriam se cada adesivo fosse tratado solto, sem pertencer
//  a peca nenhuma).
// =====================================================================

#ifndef CUBO_H
#define CUBO_H

#include <array>
#include <string>

// Cores das 6 faces (usadas tambem como "eixo" de referencia)
constexpr int COR_U = 0, COR_D = 1, COR_F = 2, COR_B = 3, COR_L = 4, COR_R = 5;
extern const std::array<char,6>        LETRA_COR; // letra usada na exibicao
extern const std::array<std::string,6> NOME_COR;  // nome da cor por extenso

// Indices para montar a matriz de 24 adesivos (somente para EXIBICAO)
constexpr int U0 = 0, D0 = 4, F0 = 8, B0 = 12, L0 = 16, R0 = 20;

// Movimentos possiveis: MAIUSCULA = sentido horario, minuscula = anti-horario.
extern const std::array<char,12> MOVIMENTOS;

// ---------------------------------------------------------------------
// 1) ESTADO
// ---------------------------------------------------------------------
struct Corner {
    int cLR, cUD, cFB; // cor atual no adesivo voltado para o eixo X, Y, Z
};

struct CubeState {
    // A MATRIZ 2x2x2 do cubo: cubo[x][y][z], com x,y,z em {0,1}.
    Corner cubo[2][2][2];

    CubeState(); // comeca resolvido

    std::string key() const; // chave textual usada para marcar estados visitados
    bool isSolved() const;   // FUNCAO AVALIADORA (parte 1: teste de objetivo)

    // Converte a matriz de cantos para um vetor de 24 adesivos,
    // usado somente para desenhar o cubo na tela.
    std::array<int,24> paraAdesivos() const;

    void print() const;
};

// Aplica um movimento (mv em MOVIMENTOS) e devolve o NOVO estado.
// Aceita tanto o sentido horario ('U') quanto o anti-horario ('u').
CubeState aplicarMovimento(const CubeState& in, char mv);

// ---------------------------------------------------------------------
// Parte "jogar manualmente" / interface com o usuario
// ---------------------------------------------------------------------

// Embaralha a partir do resolvido usando uma seed fixa (repetivel) e
// devolve a sequencia de movimentos usada (para o usuario conferir).
CubeState embaralhar(unsigned seed, int quantidadeMovimentos, std::string& movimentosUsados);

void mostrarLegenda();

// Le um movimento do teclado, valida e aplica no cubo (opcao "Jogar"
// do menu principal). Mostra o cubo apos o movimento.
void jogarManualmente(CubeState& cubo);

// Aplica cada movimento da sequencia, um de cada vez, mostrando o
// cubo apos cada passo (usado para "ver a solucao passo a passo").
void aplicarSequenciaComAnimacao(CubeState& estado, const std::string& seq);

#endif