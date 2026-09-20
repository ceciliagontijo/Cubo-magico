// =====================================================================
//  main.cpp
// =====================================================================
//  Trabalho T1 - Simulador de cubo magico 2x2x2 com solucao por IA.
//
//  Este arquivo so cuida do fluxo principal (menu). A logica do cubo
//  e do jogo manual esta em cubo.h/cubo.cpp; os algoritmos de busca
//  (BFS, IDDFS, A*) estao em busca.h/busca.cpp.
//
//  Compilar:  g++ -std=c++17 -O2 -o cubo2x2 main.cpp cubo.cpp busca.cpp
//  Executar:  ./cubo2x2
// =====================================================================

#include "cubo.h"
#include "busca.h"
#include <iostream>

using namespace std;

int main() {
    cout << "=====================================================\n";
    cout << " SIMULADOR DE CUBO MAGICO 2x2x2 (com solucao por IA)\n";
    cout << "=====================================================\n";
    mostrarLegenda();

    unsigned seed;
    int qtdMovimentos;
    cout << "\nDigite uma seed (numero inteiro) para o embaralhamento: ";
    cin >> seed;
    cout << "Quantidade de movimentos do embaralhamento (ex: 6 a 10): ";
    cin >> qtdMovimentos;

    string movimentosDoEmbaralhamento;
    CubeState cubo = embaralhar(seed, qtdMovimentos, movimentosDoEmbaralhamento);

    cout << "\nCubo embaralhado com a seed " << seed << " usando os movimentos: "
         << movimentosDoEmbaralhamento << "\n";
    cubo.print();

    bool sair = false;
    while (!sair) {
        cout << "\n================= MENU =================\n";
        cout << "1 - Ver estado atual do cubo\n";
        cout << "2 - Jogar (aplicar um movimento manualmente)\n";
        cout << "3 - Resolver com IA (BFS / IDDFS / A*)\n";
        cout << "4 - Gerar novo embaralhamento (nova seed)\n";
        cout << "5 - Sair\n";
        cout << "Opcao: ";
        int opcao; cin >> opcao;

        if (opcao == 1) {
            cubo.print();
            if (cubo.isSolved()) cout << ">> O cubo esta resolvido!\n";
        } else if (opcao == 2) {
            jogarManualmente(cubo);
        } else if (opcao == 3) {
            menuResolverComIA(cubo);
        } else if (opcao == 4) {
            cout << "Nova seed: "; cin >> seed;
            cout << "Quantidade de movimentos: "; cin >> qtdMovimentos;
            cubo = embaralhar(seed, qtdMovimentos, movimentosDoEmbaralhamento);
            cout << "Cubo embaralhado com movimentos: " << movimentosDoEmbaralhamento << "\n";
            cubo.print();
        } else if (opcao == 5) {
            sair = true;
        } else {
            cout << "Opcao invalida!\n";
        }
    }

    cout << "Ate mais!\n";
    return 0;
}
