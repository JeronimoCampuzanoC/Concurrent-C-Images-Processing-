#ifndef ROTATION_H
#define ROTATION_H

#include "imagen_info.h"

typedef struct
{
    unsigned char ***origen;
    unsigned char ***destino;
    int anchoOrigen;
    int altoOrigen;
    int anchoDestino;
    int altoDestino;
    int canales;
    float angulo;
    int inicioY;
    int finY;
    float cosAngulo;
    float sinAngulo;
    int centroXOrigen;
    int centroYOrigen;
    int centroXDestino;
    int centroYDestino;
} RotacionArgs;

void rotarImagenConcurrente(ImagenInfo *info, float angulo);

void *rotarImagenHilo(void *args);

unsigned char ***asignarMatriz(int alto, int ancho, int canales);

void liberarMatriz(unsigned char ***matriz, int alto, int ancho);

#endif // ROTATION_H
