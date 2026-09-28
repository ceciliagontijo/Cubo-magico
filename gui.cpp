#include "raylib.h"
#include "cubo.h"
#include "busca.h"
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

static Color corDoAdesivo(int idx) {
    switch (idx) {
        case COR_U: return WHITE;
        case COR_D: return YELLOW;
        case COR_F: return GREEN;
        case COR_B: return BLUE;
        case COR_L: return ORANGE;
        case COR_R: return RED;
    }
    return GRAY;
}

struct Botao {
    Rectangle rect;
    string texto;
};

static bool desenharBotao(const Botao& b, Color cor = LIGHTGRAY, int fonte = 18) {
    Vector2 mouse = GetMousePosition();
    bool sobre = CheckCollisionPointRec(mouse, b.rect);
    DrawRectangleRec(b.rect, sobre ? Fade(cor, 0.75f) : cor);
    DrawRectangleLinesEx(b.rect, 1, DARKGRAY);
    int largura = MeasureText(b.texto.c_str(), fonte);
    DrawText(b.texto.c_str(),
             (int)(b.rect.x + (b.rect.width - largura) / 2.0f),
             (int)(b.rect.y + (b.rect.height - fonte) / 2.0f),
             fonte, BLACK);
    return sobre && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void desenharCubo(const CubeState& cubo, int baseX, int baseY, int tam) {
    array<int,24> a = cubo.paraAdesivos();
    int gap = 4;
    auto celula = [&](int col, int row, int idxAdesivo) {
        int x = baseX + col * (tam + gap);
        int y = baseY + row * (tam + gap);
        DrawRectangle(x, y, tam, tam, corDoAdesivo(a[idxAdesivo]));
        DrawRectangleLines(x, y, tam, tam, BLACK);
    };
    celula(2,0,U0+0); celula(3,0,U0+1);
    celula(2,1,U0+2); celula(3,1,U0+3);
    celula(0,2,L0+0); celula(1,2,L0+1);
    celula(0,3,L0+2); celula(1,3,L0+3);
    celula(2,2,F0+0); celula(3,2,F0+1);
    celula(2,3,F0+2); celula(3,3,F0+3);
    celula(4,2,R0+0); celula(5,2,R0+1);
    celula(4,3,R0+2); celula(5,3,R0+3);
    celula(6,2,B0+0); celula(7,2,B0+1);
    celula(6,3,B0+2); celula(7,3,B0+3);
    celula(2,4,D0+0); celula(3,4,D0+1);
    celula(2,5,D0+2); celula(3,5,D0+3);
}

int main() {
    const int LARGURA = 1000, ALTURA = 840;
    InitWindow(LARGURA, ALTURA, "Cubo Magico 2x2x2 - IA (BFS / IDDFS / A*)");
    SetTargetFPS(60);

    CubeState cubo; 
    string seedTexto = "7";
    bool seedFocado = false;
    int profundidade = 7;
    string movimentosEmbaralhamento;

    bool temResultado = false;
    string nomeAlgoritmo;
    ResultadoBusca resultado;
    double tempoSegundos = 0.0;

    string animSeq;      
    size_t animIndex = 0;
    int animContador = 0;
    const int FRAMES_POR_MOVIMENTO = 25;

    const int Y_TITULO      = 12;
    const int Y_CUBO        = 55;               
    const int Y_LEGENDA     = 368;
    const int Y_LABEL_JOGAR = 400;
    const int Y_BOTOES_HOR  = 424;
    const int Y_BOTOES_ANTI = Y_BOTOES_HOR + 38 + 8;
    const int Y_LABEL_SEED  = Y_BOTOES_ANTI + 38 + 22;
    const int Y_CAIXA_SEED  = Y_LABEL_SEED + 20;
    const int Y_EMBARALHADO = Y_CAIXA_SEED + 34 + 16;
    const int Y_LABEL_IA    = Y_EMBARALHADO + 24;
    const int Y_BOTOES_IA   = Y_LABEL_IA + 22;
    const int Y_RESULTADO   = Y_BOTOES_IA + 42 + 22;

    const char letras[6] = {'U','D','F','B','L','R'};
    const int  teclas[6] = {KEY_U, KEY_D, KEY_F, KEY_B, KEY_L, KEY_R};
    Botao botaoHor[6], botaoAnti[6];
    {
        int bx = 40, bw = 55, bh = 38, gap = 8;
        for (int i = 0; i < 6; i++) {
            botaoHor[i]  = { {(float)(bx+i*(bw+gap)), (float)Y_BOTOES_HOR, (float)bw, (float)bh}, string(1,letras[i]) };
            botaoAnti[i] = { {(float)(bx+i*(bw+gap)), (float)Y_BOTOES_ANTI, (float)bw, (float)bh}, string(1,(char)tolower(letras[i])) };
        }
    }

    Rectangle caixaSeed  = { 40, (float)Y_CAIXA_SEED, 150, 34 };
    Botao botaoMenosProf = { {330, (float)Y_CAIXA_SEED-3, 34, 34}, "-" };
    Botao botaoMaisProf  = { {430, (float)Y_CAIXA_SEED-3, 34, 34}, "+" };
    Botao botaoEmbaralhar= { {480, (float)Y_CAIXA_SEED-3, 150, 40}, "Embaralhar" };
    Botao botaoReset     = { {650, (float)Y_CAIXA_SEED-3, 150, 40}, "Resolvido (reset)" };

    Botao botaoBFS   = { {40,  (float)Y_BOTOES_IA, 140, 42}, "BFS" };
    Botao botaoIDDFS = { {200, (float)Y_BOTOES_IA, 190, 42}, "IDDFS" };
    Botao botaoAstar = { {410, (float)Y_BOTOES_IA, 140, 42}, "A*" };

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            seedFocado = CheckCollisionPointRec(GetMousePosition(), caixaSeed);

        if (seedFocado) {
            int tecla = GetCharPressed();
            while (tecla > 0) {
                if (tecla >= '0' && tecla <= '9' && seedTexto.size() < 9) seedTexto += (char)tecla;
                tecla = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !seedTexto.empty()) seedTexto.pop_back();
        }

        bool animando = animIndex < animSeq.size();
        if (!seedFocado && !animando) {
            bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            for (int i = 0; i < 6; i++) {
                if (IsKeyPressed(teclas[i])) {
                    char mv = shift ? (char)tolower(letras[i]) : letras[i];
                    cubo = aplicarMovimento(cubo, mv);
                }
            }
        }

        bool cliqueBFS = false, cliqueIDDFS = false, cliqueAstar = false;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Cubo Magico 2x2x2", 40, Y_TITULO, 26, BLACK);
        desenharCubo(cubo, 40, Y_CUBO, 46);

        if (cubo.isSolved())
            DrawText("RESOLVIDO!", 500, Y_CUBO+5, 24, (Color){0,150,0,255});

        {
            const char* nomes[6] = {"Branco(U)","Amarelo(D)","Verde(F)","Azul(B)","Laranja(L)","Vermelho(R)"};
            int cores[6] = {COR_U,COR_D,COR_F,COR_B,COR_L,COR_R};
            for (int i = 0; i < 6; i++) {
                DrawRectangle(40 + i*115, Y_LEGENDA, 16, 16, corDoAdesivo(cores[i]));
                DrawRectangleLines(40 + i*115, Y_LEGENDA, 16, 16, BLACK);
                DrawText(nomes[i], 40 + i*115 + 20, Y_LEGENDA, 12, BLACK);
            }
        }

        DrawText("Jogar manualmente (clique ou tecle U D F B L R; SHIFT = anti-horario):", 40, Y_LABEL_JOGAR, 16, DARKGRAY);
        for (int i = 0; i < 6 && !animando; i++) {
            if (desenharBotao(botaoHor[i]))  cubo = aplicarMovimento(cubo, letras[i]);
            if (desenharBotao(botaoAnti[i])) cubo = aplicarMovimento(cubo, (char)tolower(letras[i]));
        }
        if (animando) {
            for (int i = 0; i < 6; i++) { desenharBotao(botaoHor[i], GRAY); desenharBotao(botaoAnti[i], GRAY); }
        }

        DrawText("Seed:", (int)caixaSeed.x, Y_LABEL_SEED, 16, DARKGRAY);
        DrawRectangleRec(caixaSeed, seedFocado ? Fade(SKYBLUE, 0.3f) : LIGHTGRAY);
        DrawRectangleLinesEx(caixaSeed, 1, DARKGRAY);
        DrawText(seedTexto.c_str(), (int)caixaSeed.x + 8, (int)caixaSeed.y + 8, 18, BLACK);

        DrawText("Profundidade:", 200, Y_LABEL_SEED, 16, DARKGRAY);
        DrawText(TextFormat("%d", profundidade), 375, (int)caixaSeed.y + 8, 18, BLACK);
        if (desenharBotao(botaoMenosProf)) profundidade = max(1, profundidade - 1);
        if (desenharBotao(botaoMaisProf))  profundidade = min(15, profundidade + 1);

        if (desenharBotao(botaoEmbaralhar)) {
            unsigned seed = seedTexto.empty() ? 0u : (unsigned)strtoul(seedTexto.c_str(), nullptr, 10);
            cubo = embaralhar(seed, profundidade, movimentosEmbaralhamento);
            temResultado = false;
            animSeq.clear(); animIndex = 0;
        }
        if (desenharBotao(botaoReset)) {
            cubo = CubeState();
            movimentosEmbaralhamento.clear();
            temResultado = false;
            animSeq.clear(); animIndex = 0;
        }

        if (!movimentosEmbaralhamento.empty())
            DrawText(("Ultimo embaralhamento: " + movimentosEmbaralhamento).c_str(), 40, Y_EMBARALHADO, 15, DARKGRAY);

        DrawText("Resolver com IA:", 40, Y_LABEL_IA, 16, DARKGRAY);
        if (desenharBotao(botaoBFS))   cliqueBFS = true;
        if (desenharBotao(botaoIDDFS)) cliqueIDDFS = true;
        if (desenharBotao(botaoAstar)) cliqueAstar = true;

        int ry = Y_RESULTADO;
        if (temResultado) {
            DrawText(("Algoritmo: " + nomeAlgoritmo).c_str(), 40, ry, 18, BLACK);
            DrawText(TextFormat("Estados visitados: %ld     Tempo: %.4f s", resultado.estadosVisitados, tempoSegundos), 40, ry+26, 18, BLACK);
            if (resultado.encontrado) {
                string linha = "Solucao (" + to_string(resultado.movimentos.size()) + " mov): " + resultado.movimentos;
                DrawText(linha.c_str(), 40, ry+52, 18, BLACK);
                if (animando)
                    DrawText(TextFormat("Aplicando passo %d de %d...", (int)animIndex+1, (int)animSeq.size()), 40, ry+78, 18, (Color){0,100,200,255});
            } else {
                DrawText("Sem solucao encontrada (ou limite de seguranca atingido).", 40, ry+52, 18, MAROON);
            }
        }

        EndDrawing();

        if (cliqueBFS || cliqueIDDFS || cliqueAstar) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Resolvendo, aguarde...", 40, 300, 28, DARKGRAY);
            EndDrawing();

            double t0 = GetTime();
            if (cliqueBFS)        { resultado = resolverBFS(cubo);        nomeAlgoritmo = "BFS"; }
            else if (cliqueIDDFS) { resultado = resolverIDDFS(cubo);      nomeAlgoritmo = "IDDFS (Profundidade Limitada Iterativa)"; }
            else                  { resultado = resolverAEstrela(cubo);   nomeAlgoritmo = "A*"; }
            tempoSegundos = GetTime() - t0;
            temResultado = true;

            animSeq = resultado.encontrado ? resultado.movimentos : "";
            animIndex = 0;
            animContador = 0;
        }

        if (animIndex < animSeq.size()) {
            animContador++;
            if (animContador >= FRAMES_POR_MOVIMENTO) {
                cubo = aplicarMovimento(cubo, animSeq[animIndex]);
                animIndex++;
                animContador = 0;
            }
        }
    }

    CloseWindow();
    return 0;
}
