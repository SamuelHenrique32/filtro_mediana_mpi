//OBS: BMP is stored upside down

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// --------------------------------------------------------------------------------------------------------

#define kQTD_PARAMS 3
#define kARQ_SAIDA "saida.bmp"
#define kQTD_BITS_IMG 24
#define kQTD_MAX_ELEMENTOS_MASCARA 49
#define kMAIN_PROC 0

#define kCOPY_TO_FINAL_IMG_OFFSET 3

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

typedef enum pos_to_copy_final_img {
    POS1,
    POS2,
    POS3,
} POS_TO_COPY_FINAL_IMG;

// --------------------------------------------------------------------------------------------------------

void bubble_sort(int *v, int size);
int need_copy_to_final_img_line(int pos, int height);
int need_copy_to_final_img_column(int pos, int width);
int median(int *v, int tamanhoMascara);
void apply_median_pixels(int id, int tamanhoMascara, int deslPosMascara, int nroProc, int largura, int altura, RGB **img, RGB **imgCopy);
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

int need_copy_to_final_img_line(int pos, int height) {

    switch(pos) {
        case POS1:
        case POS2:
        case POS3:
            return 1;
        break;

        default:
            
            if(pos >= (height-kCOPY_TO_FINAL_IMG_OFFSET)) {
                return 1;
            }

        break;
    }

    return 0;
}

int need_copy_to_final_img_column(int pos, int width) {

    switch(pos) {
        case POS1:
        case POS2:
        case POS3:
            return 1;
        break;

        default:
            
            if(pos >= (width-kCOPY_TO_FINAL_IMG_OFFSET)) {
                return 1;
            }

        break;
    }

    return 0;
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

void apply_median_pixels(int id, int tamanhoMascara, int deslPosMascara, int nroProc, int largura, int altura, RGB **img, RGB **imgCopy) {

    int posVetMascaraRed = 0, posVetMascaraGreen = 0, posVetMascaraBlue = 0, posX, posY, startX, startY, i, j;
    int medianRed, medianGreen, medianBlue;
    int *vetmascaraRedInt = NULL, *vetmascaraGreenInt = NULL, *vetmascaraBlueInt = NULL;
    int mascaraVet[kQTD_MAX_ELEMENTOS_MASCARA];

    // Posicao atual na matriz
    posX = deslPosMascara;
    posY = (id==0) ? id*(altura/nroProc)+deslPosMascara : id*(altura/nroProc);

    // Posicao inicial de deslocamento em X para mascara
    startX = 0;

    // Posicao corrente de deslocamento em X para mascara
    j = startX;

    // Posicao inicial de deslocamento em Y para mascara
    startY = posY-deslPosMascara;

    // Posicao corrente de deslocamento em Y para mascara
    i = startY;

    // Para cada pixel da imagem
    while((posX<largura) && (posY<altura)) {

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

                    //printf("Process %d posY= %d posX= %d\n", id, posY, posX);

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
        if((posX+1) <= ((largura-1)-deslPosMascara)) {

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
        else if(posY < (id+1)*((altura/nroProc))) {

            posX = deslPosMascara;

            posY += 1;

            // Nao processa bordas
            if((id == nroProc-1) && (posY>=(altura-deslPosMascara))) {                
                break;
            }

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

    int i = 0, j = 0, id = 0, np = 0, recvOffset = 0, sendOffset = 0;

    // Dados para serem enviados
    int larguraSend = 0, alturaSend = 0, tamanhoMascaraSend = 0, deslPosMascaraSend = 0;

    HEADER c;

    RGB **img = NULL;     // Original
    RGB **imgCopy = NULL; // Final
    RGB *vetMascaraRGB = NULL, pixel;    
    RGB *matImg = NULL, *matImgCopy = NULL;

    // Descritor
    FILE *in, *out;

    MPI_Status status;

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &np);

    if(id == kMAIN_PROC) {
        if(argc!=kQTD_PARAMS) {

            printf("%s <tamanho_mascara> <arquivo_entrada>\n", argv[0]);

            exit(0);
        }

        tamanhoMascaraSend = atoi(argv[1]);

        vetMascaraRGB = malloc(tamanhoMascaraSend*tamanhoMascaraSend*sizeof(RGB));

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

         printf("Tamanho mascara: %d\nQuantidade de processos: %d\n", tamanhoMascaraSend, np);

        // Le cabecalho de entrada
        fread(&c, sizeof(HEADER), 1, in);

        if(c.nbits != kQTD_BITS_IMG) {
            printf("\nA imagem lida nao possui %d bits", kQTD_BITS_IMG);

            exit(0);
        }

        deslPosMascaraSend = tamanhoMascaraSend/2;

        larguraSend = c.largura;
        alturaSend = c.altura;
    }

    MPI_Bcast(&larguraSend, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alturaSend, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tamanhoMascaraSend, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&deslPosMascaraSend, 1, MPI_INT, 0, MPI_COMM_WORLD);    

    // Altura * tam para ponteiro de RGB
    img = (RGB**) malloc(alturaSend*sizeof(RGB *));
    
    matImg = malloc(alturaSend*larguraSend*sizeof(RGB));

    for(i=0 ; i<alturaSend ; i++) {
        img[i] = &matImg[i*larguraSend];
    }

    imgCopy = (RGB**) malloc(alturaSend*sizeof(RGB *));
    matImgCopy = malloc(alturaSend*larguraSend*sizeof(RGB));
    for(i=0 ; i<alturaSend ; i++) {
        imgCopy[i] = &matImgCopy[i*larguraSend];
    }

    if(id == kMAIN_PROC) {
        // Le 1 pixel por vez
        for(i=0 ; i<c.altura ; i++) {
            for(j=0 ; j<c.largura ; j++) {
                fread(&img[i][j], sizeof(RGB), 1, in);
            }
        }
    }

    MPI_Bcast(&(img[0][0]), larguraSend*alturaSend, MPI_INT, 0, MPI_COMM_WORLD);

    for(i=0 ; i<alturaSend ; i++) {
        for(j=0 ; j<larguraSend ; j++) {
            // Copy edges where median won't be applied
            if((need_copy_to_final_img_line(i, alturaSend)) || (need_copy_to_final_img_column(j, larguraSend))) {
                imgCopy[i][j] = img[i][j];
            }            
        }
    }

    apply_median_pixels(id, tamanhoMascaraSend, deslPosMascaraSend, np, larguraSend, alturaSend, img, imgCopy);

    if(id != kMAIN_PROC) {

        sendOffset = id*(alturaSend/np);

        MPI_Send(&imgCopy[sendOffset][0], larguraSend*(alturaSend/np), MPI_INT, kMAIN_PROC, 1, MPI_COMM_WORLD);
    }
    else {

        if(np>1) {

            recvOffset = (alturaSend/np);

            for(i=(kMAIN_PROC+1) ; i<np ; i++) {

                MPI_Recv(&imgCopy[recvOffset][0], larguraSend*(alturaSend/np), MPI_INT, i, 1, MPI_COMM_WORLD, &status);

                recvOffset += (alturaSend/np);
            }            
        }

        // Arquivo binario para escrita
        out = fopen(kARQ_SAIDA, "wb");

        // Cabecalho de saida
        fwrite(&c, sizeof(HEADER), 1, out);

        // Percorre matriz final
        for(i=0 ; i<alturaSend ; i++) {
            for(j=0 ; j<larguraSend ; j++) {
                // Grava pixel
                fwrite(&imgCopy[i][j], sizeof(RGB), 1, out);
            }
        }

        printf("Imagem escrita para arquivo: %s\n", kARQ_SAIDA);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    free(matImg);
    free(matImgCopy);
    free(img);
    free(imgCopy);

    if(id == kMAIN_PROC) {
        fclose(in);
        fclose(out);
    }

    MPI_Finalize();
}

// --------------------------------------------------------------------------------------------------------