// =====================================================================
//  cubo.cpp  -  implementacao do ESTADO do cubo (matriz 2x2x2) e da
//               parte "jogar manualmente" (ver cubo.h para a
//               explicacao da modelagem)
// =====================================================================

#include "cubo.h"
#include <iostream>
#include <random>
#include <cctype>

using namespace std;

const array<char,6>   LETRA_COR = {'W','Y','G','C','O','R'}; // Branco,Amarelo,Verde,Ciano,Laranja,Vermelho
const array<string,6> NOME_COR  = {"Branco","Amarelo","Verde","Azul","Laranja","Vermelho"};
const array<char,12>  MOVIMENTOS = {'U','D','F','B','L','R','u','d','f','b','l','r'};

// ---------------------------------------------------------------------
// 1) ESTADO (matriz 2x2x2: cubo[x][y][z], x,y,z em {0,1})
// ---------------------------------------------------------------------
CubeState::CubeState() {
    for (int x = 0; x < 2; x++) for (int y = 0; y < 2; y++) for (int z = 0; z < 2; z++) {
        Corner c;
        c.cLR = (x == 1) ? COR_R : COR_L; // x=1 (direita) -> R, x=0 (esquerda) -> L
        c.cUD = (y == 1) ? COR_U : COR_D; // y=1 (cima)    -> U, y=0 (baixo)    -> D
        c.cFB = (z == 1) ? COR_F : COR_B; // z=1 (frente)  -> F, z=0 (fundo)    -> B
        cubo[x][y][z] = c;
    }
}

string CubeState::key() const {
    string k; k.reserve(24);
    for (int x = 0; x < 2; x++) for (int y = 0; y < 2; y++) for (int z = 0; z < 2; z++) {
        k += char('0' + cubo[x][y][z].cLR);
        k += char('0' + cubo[x][y][z].cUD);
        k += char('0' + cubo[x][y][z].cFB);
    }
    return k;
}

// FUNCAO AVALIADORA (parte 1: teste de objetivo) -----------------------
// O cubo esta resolvido quando cada face tem as 4 pecas da mesma cor.
bool CubeState::isSolved() const {
    for (int y : {0,1}) { // faces U (y=1) e D (y=0)
        int ref = -1;
        for (int x : {0,1}) for (int z : {0,1}) {
            int v = cubo[x][y][z].cUD;
            if (ref == -1) ref = v; else if (v != ref) return false;
        }
    }
    for (int z : {0,1}) { // faces F (z=1) e B (z=0)
        int ref = -1;
        for (int x : {0,1}) for (int y : {0,1}) {
            int v = cubo[x][y][z].cFB;
            if (ref == -1) ref = v; else if (v != ref) return false;
        }
    }
    for (int x : {0,1}) { // faces L (x=0) e R (x=1)
        int ref = -1;
        for (int y : {0,1}) for (int z : {0,1}) {
            int v = cubo[x][y][z].cLR;
            if (ref == -1) ref = v; else if (v != ref) return false;
        }
    }
    return true;
}

array<int,24> CubeState::paraAdesivos() const {
    array<int,24> a{};
    int idx;
    idx = 0; for (int x : {0,1}) for (int z : {0,1}) a[U0+idx++] = cubo[x][1][z].cUD;
    idx = 0; for (int x : {0,1}) for (int z : {0,1}) a[D0+idx++] = cubo[x][0][z].cUD;
    idx = 0; for (int x : {0,1}) for (int y : {0,1}) a[F0+idx++] = cubo[x][y][1].cFB;
    idx = 0; for (int x : {0,1}) for (int y : {0,1}) a[B0+idx++] = cubo[x][y][0].cFB;
    idx = 0; for (int y : {0,1}) for (int z : {0,1}) a[L0+idx++] = cubo[0][y][z].cLR;
    idx = 0; for (int y : {0,1}) for (int z : {0,1}) a[R0+idx++] = cubo[1][y][z].cLR;
    return a;
}

void CubeState::print() const {
    array<int,24> a = paraAdesivos();
    auto c = [&](int idx){ return LETRA_COR[a[idx]]; };
    cout << "\n";
    cout << "        " << c(U0+0) << " " << c(U0+1) << "\n";
    cout << "        " << c(U0+2) << " " << c(U0+3) << "\n";
    cout << c(L0+0) << " " << c(L0+1) << " "
         << c(F0+0) << " " << c(F0+1) << " "
         << c(R0+0) << " " << c(R0+1) << " "
         << c(B0+0) << " " << c(B0+1) << "\n";
    cout << c(L0+2) << " " << c(L0+3) << " "
         << c(F0+2) << " " << c(F0+3) << " "
         << c(R0+2) << " " << c(R0+3) << " "
         << c(B0+2) << " " << c(B0+3) << "\n";
    cout << "        " << c(D0+0) << " " << c(D0+1) << "\n";
    cout << "        " << c(D0+2) << " " << c(D0+3) << "\n";
    cout << "(U=topo D=baixo F=frente B=fundo L=esquerda R=direita)\n";
}

// ---------------------------------------------------------------------
// Movimentos de face
// ---------------------------------------------------------------------
// Quando um canto gira em torno do eixo Y (movimentos U/D), os
// adesivos que estavam voltados para X e para Z trocam de eixo entre
// si (o adesivo de cima/baixo do canto nao muda). Em torno do eixo Z
// (F/B) trocam X<->Y. Em torno do eixo X (L/R) trocam Y<->Z.
enum Eixo { EIXO_Y, EIXO_Z, EIXO_X };

static Corner trocarEixos(const Corner& c, Eixo eixo) {
    switch (eixo) {
        case EIXO_Y: return { c.cFB, c.cUD, c.cLR }; // cLR <-> cFB
        case EIXO_Z: return { c.cUD, c.cLR, c.cFB }; // cLR <-> cUD
        case EIXO_X: return { c.cLR, c.cFB, c.cUD }; // cUD <-> cFB
    }
    return c;
}

// As 6 rotacoes de face no sentido horario (vistas de fora da face).
// Cada uma pega os 4 cantos daquela camada (a fatia da matriz com um
// eixo fixo) e calcula a NOVA posicao deles na matriz (rotacao de 90
// graus) + troca os eixos dos adesivos (funcao trocarEixos acima).
//
// Como x,y,z so valem 0 ou 1, "virar para o outro lado" nesse eixo e
// simplesmente "1 - valor" (0 vira 1, 1 vira 0).
static CubeState moveU(const CubeState& in) {
    CubeState out = in; int y = 1; // camada de cima
    for (int x : {0,1}) for (int z : {0,1}) {
        int nx = 1 - z, ny = y, nz = x;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_Y);
    }
    return out;
}
static CubeState moveD(const CubeState& in) {
    CubeState out = in; int y = 0; // camada de baixo
    for (int x : {0,1}) for (int z : {0,1}) {
        int nx = z, ny = y, nz = 1 - x;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_Y);
    }
    return out;
}
static CubeState moveF(const CubeState& in) {
    CubeState out = in; int z = 1; // camada da frente
    for (int x : {0,1}) for (int y : {0,1}) {
        int nx = y, ny = 1 - x, nz = z;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_Z);
    }
    return out;
}
static CubeState moveB(const CubeState& in) {
    CubeState out = in; int z = 0; // camada do fundo
    for (int x : {0,1}) for (int y : {0,1}) {
        int nx = 1 - y, ny = x, nz = z;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_Z);
    }
    return out;
}
static CubeState moveL(const CubeState& in) {
    CubeState out = in; int x = 0; // camada da esquerda
    for (int y : {0,1}) for (int z : {0,1}) {
        int nx = x, ny = 1 - z, nz = y;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_X);
    }
    return out;
}
static CubeState moveR(const CubeState& in) {
    CubeState out = in; int x = 1; // camada da direita
    for (int y : {0,1}) for (int z : {0,1}) {
        int nx = x, ny = z, nz = 1 - y;
        out.cubo[nx][ny][nz] = trocarEixos(in.cubo[x][y][z], EIXO_X);
    }
    return out;
}

static CubeState aplicarMovimentoCW(const CubeState& in, char mv) {
    switch (mv) {
        case 'U': return moveU(in);
        case 'D': return moveD(in);
        case 'F': return moveF(in);
        case 'B': return moveB(in);
        case 'L': return moveL(in);
        case 'R': return moveR(in);
    }
    return in;
}

// Funcao publica: aceita tanto o sentido horario ('U') quanto o
// anti-horario ('u'). O anti-horario e o mesmo giro horario repetido
// 3 vezes (matematicamente equivalente a girar uma vez ao contrario).
CubeState aplicarMovimento(const CubeState& in, char mv) {
    char base = (char)toupper(mv);
    CubeState out = aplicarMovimentoCW(in, base);
    if (islower((unsigned char)mv)) {
        out = aplicarMovimentoCW(out, base);
        out = aplicarMovimentoCW(out, base);
    }
    return out;
}

// ---------------------------------------------------------------------
// Parte "jogar manualmente" / interface com o usuario
// ---------------------------------------------------------------------
CubeState embaralhar(unsigned seed, int quantidadeMovimentos, string& movimentosUsados) {
    CubeState estado; // comeca resolvido
    mt19937 gerador(seed);
    uniform_int_distribution<int> dist(0, 11);
    movimentosUsados.clear();
    for (int i = 0; i < quantidadeMovimentos; i++) {
        char mv = MOVIMENTOS[dist(gerador)];
        estado = aplicarMovimento(estado, mv);
        movimentosUsados += mv;
    }
    return estado;
}

void mostrarLegenda() {
    cout << "Legenda de cores -> ";
    for (int i = 0; i < 6; i++) cout << LETRA_COR[i] << "=" << NOME_COR[i] << "  ";
    cout << "\n";
}

void jogarManualmente(CubeState& cubo) {
    cout << "Movimentos possiveis: U D F B L R (sentido horario)\n";
    cout << "                      u d f b l r (sentido anti-horario)\n";
    cout << "Digite o movimento: ";
    string mv; cin >> mv;
    char m = mv[0];
    bool valido = false;
    for (char x : MOVIMENTOS) if (x == m) valido = true;
    if (!valido) {
        cout << "Movimento invalido!\n";
        return;
    }
    cubo = aplicarMovimento(cubo, m);
    cubo.print();
    if (cubo.isSolved()) cout << ">> Parabens, voce resolveu o cubo!\n";
}

void aplicarSequenciaComAnimacao(CubeState& estado, const string& seq) {
    for (char mv : seq) {
        estado = aplicarMovimento(estado, mv);
        cout << "\nMovimento aplicado: " << mv;
        estado.print();
        cout << "----------------------------------------\n";
    }
}