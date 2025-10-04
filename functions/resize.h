#ifndef RESIZE_H
#define RESIZE_H

// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include "imagen_info.h"
#include <pthread.h>
#include <math.h>

// Function declarations for resize operations
typedef struct
{
    unsigned char ***srcPixeles;
    unsigned char ***dstPixeles;
    int srcAncho;
    int srcAlto;
    int canales;
    int dstAncho;
    int dstAlto;
    int filaInicio;
    int filaFin; // exclusivo
} ResizeArgs;


int resizeBilinealConcurrente(ImagenInfo *info, int nuevoAncho, int nuevoAlto, int numHilos);

#endif // RESIZE_H