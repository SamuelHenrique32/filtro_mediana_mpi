
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// --------------------------------------------------------------------------------------------------------

#define kQTD_PARAMS 3
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
void apply_median_pixels(int tamanhoMascara, int deslPosMascara, HEADER c, RGB **img, RGB **imgCopy);
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

void apply_median_pixels(int tamanhoMascara, int deslPosMascara, HEADER c, RGB **img, RGB **imgCopy) {

    int posVetMascaraRed = 0, posVetMascaraGreen = 0, posVetMascaraBlue = 0, posX, posY, startX, startY, i, j;
    int medianRed, medianGreen, medianBlue;
    int *vetmascaraRedInt = NULL, *vetmascaraGreenInt = NULL, *vetmascaraBlueInt = NULL;
    int mascaraVet[kQTD_MAX_ELEMENTOS_MASCARA];

    // Posicao atual na matriz
    posX = deslPosMascara;
    posY = deslPosMascara;

    // Posicao inicial de deslocamento em X para mascara
    startX = 0;

    // Posicao corrente de deslocamento em X para mascara
    j = startX;

    // Posicao inicial de deslocamento em Y para mascara
    startY = 0;

    // Posicao corrente de deslocamento em Y para mascara
    i = startY;

    // Para cada pixel da imagem
    while((posX<c.largura) && (posY<c.altura)) {

        if((startX>=0) && (startY>=0)) {

            vetmascaraRedInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
            vetmascaraGreenInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
            vetmascaraBlueInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);

            // Processamento para pixel atual
            while(1) {

                vetmascaraRedInt[posVetMascaraRed] = img[i][j].red;
                vetmascaraGreenInt[posVetMascaraGreen] = img[i][j].green;
                vetmascaraBlueInt[posVetMascaraBlue] = img[i][j].blue;

                medianRed = 0;
                medianGreen = 0;
                medianBlue = 0;

                posVetMascaraRed++;
                posVetMascaraGreen++;
                posVetMascaraBlue++;

                // Incrementa coluna
                if((j+1-startX) <= (deslPosMascara*2)) {
                    j++;
                }
                else {

                    // Troca linha
                    i++;
                    j = startX;
                }

                // Proximo pixel
                if(posVetMascaraRed >= (tamanhoMascara*tamanhoMascara)) {

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraRedInt[x];
                    }

                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);

                    medianRed = median(mascaraVet, tamanhoMascara);

                    imgCopy[posY][posX].red = medianRed;                   

                    free(vetmascaraRedInt);

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraGreenInt[x];
                    }

                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);

                    medianGreen = median(mascaraVet, tamanhoMascara);

                    imgCopy[posY][posX].green = medianGreen;

                    free(vetmascaraGreenInt);

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraBlueInt[x];
                    }

                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);

                    medianBlue = median(mascaraVet, tamanhoMascara);

                    imgCopy[posY][posX].blue = medianBlue;

                    free(vetmascaraBlueInt);

                    posVetMascaraRed = 0;
                    posVetMascaraGreen = 0;
                    posVetMascaraBlue = 0;

                    break;
                }
            }
        }

        // Pixel a direita
        if((posX+1) <= ((c.largura-1)-deslPosMascara)) {

            posX++;

            // Posicao inicial de deslocamento em X para mascara
            startX = posX-deslPosMascara;

            // Posicao corrente de deslocamento em X para mascara
            j = startX;

            // Posicao inicial de deslocamento em Y para mascara
            startY = posY-deslPosMascara;

            // Posicao corrente de deslocamento em Y para mascara
            i = startY;
        }
        // Pixel abaixo
        else if((posY+1) <= ((c.altura-1)-deslPosMascara)) {

            posX = deslPosMascara;

            posY++;

            // Posicao inicial de deslocamento em X para mascara
            startX = posX-deslPosMascara;

            // Posicao corrente de deslocamento em X para mascara
            j = startX;

            // Posicao inicial de deslocamento em Y para mascara
            startY = posY-deslPosMascara;

            // Posicao corrente de deslocamento em Y para mascara
            i = startY;
        }
        else {
            break;
        }
    }
}

int main(int argc, char **argv) {

    int tamanhoMascara, deslPosMascara, i, j, id, np;

    unsigned char media;

    HEADER c;

    RGB **img = NULL;     // Original
    RGB **imgCopy = NULL; // Final
    RGB *vetMascaraRGB = NULL, pixel;    
    RGB *matImg = NULL, *matImgCopy = NULL;

    // Descritor
    FILE *in, *out;

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &np);

    if(argc!=kQTD_PARAMS) {

        printf("%s <tamanho_mascara> <numero_threads> <arquivo_entrada>\n", argv[0]);

        exit(0);
    }

    tamanhoMascara = atoi(argv[1]);

    vetMascaraRGB = malloc(tamanhoMascara*tamanhoMascara*sizeof(RGB));

    in = fopen(argv[2], "rb");

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

    printf("Tamanho mascara: %d\nQuantidade de threads: %d\n", tamanhoMascara, np);

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

    matImg = malloc(c.altura*c.largura*sizeof(RGB));
    matImgCopy = malloc(c.altura*c.largura*sizeof(RGB));

    for(i=0 ; i<c.altura ; i++) {
        img[i] = &matImg[i*c.largura];
        imgCopy[i] = &matImgCopy[i*c.largura];
    }

    // Le 1 pixel por vez
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {
            fread(&img[i][j], sizeof(RGB), 1, in);
            imgCopy[i][j] = img[i][j];
        }
    }

    apply_median_pixels(tamanhoMascara, deslPosMascara, c, img, imgCopy);

    // Percorre matriz ja carregada
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {

            // Grava pixel
            fwrite(&imgCopy[i][j], sizeof(RGB), 1, out);
        }
    }

    printf("Imagem escrita para arquivo: %s\n", kARQ_SAIDA);

    free(matImg);
    free(matImgCopy);
    free(img);
    free(imgCopy);

    fclose(in);
    fclose(out);
}

// --------------------------------------------------------------------------------------------------------