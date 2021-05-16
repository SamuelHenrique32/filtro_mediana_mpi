
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
    int tamanhoMascara, deslPosMascara, nroThreads, i, j;
    int **matMascaras;

    unsigned char media;

    HEADER c;

    // Vetor de ponteiros
    RGB **img = NULL, **imgCopy = NULL, *vetMascaraRGB = NULL, pixel;    

    // Descritor
    FILE *in, *out;

    if(argc!=kQTD_PARAMS) {

        printf("%s <tamanho_mascara> <numero_threads> <arquivo_entrada>\n", argv[0]);

        exit(0);
    }

    tamanhoMascara = atoi(argv[1]);

    nroThreads = atoi(argv[2]);

    matMascaras = (int **)malloc((nroThreads*3) * sizeof(int*));

    for(int i=0 ; i<(nroThreads*3) ; i++) {
        matMascaras[i] = (int *)malloc((tamanhoMascara*tamanhoMascara)*sizeof(int));
    }
    
    vetMascaraRGB = malloc(tamanhoMascara*tamanhoMascara*sizeof(RGB));

    in = fopen(argv[3], "rb");

    if(in == NULL) {
        printf("Erro ao abrir arquivo \"%s\" de entrada\n", argv[3]);

        exit(0);
    }

    // Abre arquivo binario para escrita
    out = fopen(kARQ_SAIDA, "wb");

    if(out == NULL) {
        printf("Erro ao abrir arquivo de saida\n");

        exit(0);
    }

    printf("Tamanho mascara: %d\nQuantidade de threads: %d\n", tamanhoMascara, nroThreads);

    // Le cabecalho de entrada
    fread(&c, sizeof(HEADER), 1, in);

    if(c.nbits != kQTD_BITS_IMG) {
        printf("\nA imagem lida nao possui %d bits", kQTD_BITS_IMG);

        exit(0);
    }

    deslPosMascara = tamanhoMascara/2;

    // Escreve cabecalho de saida
    fwrite(&c, sizeof(HEADER), 1, out);

    // Altura * tam para ponteiro de RGB
    img = (RGB**) malloc(c.altura*sizeof(RGB *));
    imgCopy = (RGB**) malloc(c.altura*sizeof(RGB *));

    // Cada pos aponta para vetor de RGB
    for(i=0 ; i<c.altura ; i++) {
        img[i] = (RGB*) malloc(c.largura*sizeof(RGB));
    }

    for(i=0 ; i<c.altura ; i++) {
        imgCopy[i] = (RGB*) malloc(c.largura*sizeof(RGB));
    }

    // Le 1 pixel por vez
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {
            fread(&img[i][j], sizeof(RGB), 1, in);
            imgCopy[i][j] = img[i][j];
        }
    }

    apply_median_pixels(nroThreads, tamanhoMascara, deslPosMascara, c, img, imgCopy, matMascaras);

    // Percorre matriz ja carregada
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {

            // Grava pixel
            fwrite(&imgCopy[i][j], sizeof(RGB), 1, out);
        }
    }

    for(i=0 ; i<c.altura ; i++) {
        free(img[i]);
        free(imgCopy[i]);
    }

    free(img);
    free(imgCopy);

    fclose(in);
    fclose(out);
}

// --------------------------------------------------------------------------------------------------------