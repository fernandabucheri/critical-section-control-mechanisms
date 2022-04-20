#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM 2048
#define NUM_GERACOES 2000
#define N_THREADS 2 // define o número de threads (1, 2, 4 e 8)

int **primeiraMatriz, **segundaMatriz;

int total = 0;

void inicializar() {
    int i, j;

    primeiraMatriz = malloc(TAM * sizeof(int *));
    segundaMatriz = malloc(TAM * sizeof(int *));

    for (i = 0; i < TAM; i++) {
        primeiraMatriz[i] = malloc(TAM * sizeof(int));
        segundaMatriz[i] = malloc(TAM * sizeof(int));

        for (j = 0; j < TAM; j++) {
            primeiraMatriz[i][j] = 0;
            segundaMatriz[i][j] = 0;
        }
    }
}

void preencher(int **matriz) {
    //GLIDER
    int lin = 1, col = 1;
    matriz[lin][col + 1] = 1;
    matriz[lin + 1][col + 2] = 1;
    matriz[lin + 2][col] = 1;
    matriz[lin + 2][col + 1] = 1;
    matriz[lin + 2][col + 2] = 1;

    //R-pentomino
    lin = 10;
    col = 30;
    matriz[lin][col + 1] = 1;
    matriz[lin][col + 2] = 1;
    matriz[lin + 1][col] = 1;
    matriz[lin + 1][col + 1] = 1;
    matriz[lin + 2][col + 1] = 1;
}

/*
a) Utilizar a diretiva #pragma omp critical para realizar a operação entre as threads, totalizando os resultados em uma variável global ao final;
*/

void totalVivos(int **matriz) {
    int i, j, vivos = 0;

#pragma omp critical
    for (i = 0; i < TAM; i++)
        for (j = 0; j < TAM; j++)
            total = total + matriz[i][j];
}

int getNeighbors(int i, int j, int **matriz) {
    int linha_cima, linha_meio, linha_baixo;
    int coluna_esquerda, coluna_meio, coluna_direita;
    int vivos = 0;

    linha_meio = i;

    if (i == 0)
        linha_cima = TAM - 1;
    else
        linha_cima = i - 1;

    linha_baixo = (i + 1) % TAM;

    coluna_meio = j;

    if (j == 0)
        coluna_esquerda = TAM - 1;
    else
        coluna_esquerda = j - 1;

    coluna_direita = (j + 1) % TAM;

    vivos = vivos + matriz[linha_cima][coluna_esquerda];
    vivos = vivos + matriz[linha_meio][coluna_esquerda];
    vivos = vivos + matriz[linha_baixo][coluna_esquerda];
    vivos = vivos + matriz[linha_cima][coluna_meio];
    vivos = vivos + matriz[linha_baixo][coluna_meio];
    vivos = vivos + matriz[linha_cima][coluna_direita];
    vivos = vivos + matriz[linha_meio][coluna_direita];
    vivos = vivos + matriz[linha_baixo][coluna_direita];

    return vivos;
}

void geracoes(int **geracaoAtual, int **novaGeracao) {
    int i, j, vizinhosVivos;

#pragma omp parallel for shared(geracaoAtual, novaGeracao) private(i, j, vizinhosVivos) num_threads(N_THREADS)
    for (i = 0; i < TAM; i++) {
        for (j = 0; j < TAM; j++) {
            vizinhosVivos = getNeighbors(i, j, geracaoAtual);

            if (geracaoAtual[i][j] == 1 && (vizinhosVivos == 2 || vizinhosVivos == 3))
                novaGeracao[i][j] = 1;

            else if (geracaoAtual[i][j] == 0 && vizinhosVivos == 3)
                novaGeracao[i][j] = 1;

            else
                novaGeracao[i][j] = 0;
        }
    }
}

void jogoDaVida() {
    int i;
    for (i = 1; i <= NUM_GERACOES; i++) {
        if (i % 2 != 0)
            geracoes(primeiraMatriz, segundaMatriz);
        else
            geracoes(segundaMatriz, primeiraMatriz);
    }
}

int main() {
    int vivos;

    printf("** Game of Life\n");

    inicializar();
    preencher(primeiraMatriz);

    printf("Condição inicial: %d\n", total);

    jogoDaVida();

    double start, end;
    if (NUM_GERACOES % 2 == 0) {

        start = omp_get_wtime(); // iniciando a medida do tempo
        totalVivos(primeiraMatriz);
        end = omp_get_wtime(); //finalizando a medida do tempo

        printf("\nTempo de execução do trecho que calcula o total de vivos da última geração: %f segundos.\n",
               end - start);

        vivos = total;
    } else {
        totalVivos(segundaMatriz);
        vivos = total;
    }

    printf("Última geração (%d iterações): %d células vivas\n", NUM_GERACOES, vivos);

    return 0;
}
