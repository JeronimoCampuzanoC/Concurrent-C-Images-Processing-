#ifndef CONVOLUTION_H
#define CONVOLUTION_H

// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include "imagen_info.h"

// Function declarations for convolution operations

// Estructura para pasar datos a los hilos de convolución
typedef struct
{
    unsigned char ***pixeles;
    unsigned char ***pixelesResultado;
    int inicio;
    int fin;
    int ancho;
    int alto;
    int canales;
    float **kernel;
    int tamKernel;
} ConvolucionArgs;

// Función para generar kernel Gaussiano
float **generarKernelGaussiano(int tamKernel, float sigma);

// Función para liberar memoria del kernel
void liberarKernel(float **kernel, int tamKernel);

// Función que ejecuta cada hilo para convolución
void *convolucionHilo(void *args);

// Función principal de convolución concurrente
int aplicarConvolucionConcurrente(ImagenInfo *info, int tamKernel, float sigma, int numHilos);

#endif // CONVOLUTION_H