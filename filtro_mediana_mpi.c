
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------------------------------------

#define kQTD_PARAMS 4
#define kARQ_SAIDA "saida.bmp"
#define kQTD_BITS_IMG 24
#define kQTD_MAX_ELEMENTOS_MASCARA 49

#pragma pack (1)

// --------------------------------------------------------------------------------------------------------

typedef struct header {
    // Cabecalho arquivo
    unsigned short int tipo; // 2 bytes
    unsigned int tamanhoArquivo; // 4 bytes
    unsigned short int reservado1;
    unsigned short int reservado2;
    unsigned int offset;

    // Cabecalho imagem
    unsigned int tamanhoCabecalho;
    unsigned int largura;
    unsigned int altura;
    unsigned short int planos; // Deve ser 1
    unsigned short int nbits;
    unsigned int compressao;
    unsigned int tamanhoImagem;
    unsigned int xres; // Resolucao x
    unsigned int yres; // Resolucao y
    unsigned int ucores;
    unsigned int sigcores;
    
} HEADER;

// 8 bits por cor = 24 bits (0 a 255)
// Armazenado ao contrario
typedef struct rgb {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} RGB;

// --------------------------------------------------------------------------------------------------------

void bubble_sort(int *v, int size);
int median(int *v, int tamanhoMascara);
void apply_median_pixels(int nroThreads, int tamanhoMascara, int deslPosMascara, HEADER c, RGB **img, RGB **imgCopy, int **matMascaras);
int main(int argc, char **argv);

// --------------------------------------------------------------------------------------------------------

void bubble_sort(int *v, int size) {
    int k, j, aux;

    for(k=(size - 1) ; k>0 ; k--) {
        for(j=0 ; j<k ; j++) {

            if(v[j] > v[j + 1]) {
                aux = v[j];
                v[j] = v[j+1];
                v[j+1] = aux;
            }
        }
    }
}

int median(int *v, int tamanhoMascara) {

    int posElemento = 0, val1 = 0, val2 = 0;

    if((tamanhoMascara*tamanhoMascara)%2 == 0) {

        val1 = v[(tamanhoMascara*tamanhoMascara/2) - 1];

        val2 = v[tamanhoMascara*tamanhoMascara/2];

        val1 += val2;

        return (val1/2);
    }
    else {

        val1 = v[(tamanhoMascara*tamanhoMascara-1)/2];

        return val1;
    }
}

void apply_median_pixels(int nroThreads, int tamanhoMascara, int deslPosMascara, HEADER c, RGB **img, RGB **imgCopy, int **matMascaras) {
}

int main(int argc, char **argv) {

}

// --------------------------------------------------------------------------------------------------------