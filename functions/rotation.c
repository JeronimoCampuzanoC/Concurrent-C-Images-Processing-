#include "rotation.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> 

unsigned char ***asignarMatriz(int alto, int ancho, int canales)
{
    unsigned char ***matriz = (unsigned char ***)malloc(alto * sizeof(unsigned char **));
    if (!matriz)
    {
        fprintf(stderr, "Error de memoria al asignar filas\n");
        return NULL;
    }

    for (int y = 0; y < alto; y++)
    {
        matriz[y] = (unsigned char **)malloc(ancho * sizeof(unsigned char *));
        if (!matriz[y])
        {
            fprintf(stderr, "Error de memoria al asignar columnas en fila %d\n", y);
            // Liberar memoria ya asignada
            for (int i = 0; i < y; i++)
            {
                free(matriz[i]);
            }
            free(matriz);
            return NULL;
        }

        for (int x = 0; x < ancho; x++)
        {
            matriz[y][x] = (unsigned char *)malloc(canales * sizeof(unsigned char));
            if (!matriz[y][x])
            {
                fprintf(stderr, "Error de memoria al asignar canales en (%d,%d)\n", y, x);
                // Liberar memoria ya asignada
                for (int i = 0; i <= y; i++)
                {
                    int maxX = (i == y) ? x : ancho;
                    for (int j = 0; j < maxX; j++)
                    {
                        free(matriz[i][j]);
                    }
                    free(matriz[i]);
                }
                free(matriz);
                return NULL;
            }

            // Inicializar píxeles a negro (0)
            for (int c = 0; c < canales; c++)
            {
                matriz[y][x][c] = 0;
            }
        }
    }
    return matriz;
}

void liberarMatriz(unsigned char ***matriz, int alto, int ancho)
{
    if (!matriz)
        return;

    for (int y = 0; y < alto; y++)
    {
        if (matriz[y])
        {
            for (int x = 0; x < ancho; x++)
            {
                free(matriz[y][x]);
            }
            free(matriz[y]);
        }
    }
    free(matriz);
}

void *rotarImagenHilo(void *args)
{
    RotacionArgs *rArgs = (RotacionArgs *)args;
    int angulo = (int)rArgs->angulo; // Convertir a entero (90, 180, 270)

    for (int y = rArgs->inicioY; y < rArgs->finY; y++)
    {
        for (int x = 0; x < rArgs->anchoDestino; x++)
        {
            int xOrigen, yOrigen;

            // Mapeo directo de coordenadas según el ángulo
            switch (angulo)
            {
            case 90:
                // 90° horario: (x,y) -> (y, alto-1-x)
                xOrigen = y;
                yOrigen = rArgs->anchoDestino - 1 - x;
                break;
            case 180:
                // 180°: (x,y) -> (ancho-1-x, alto-1-y)
                xOrigen = rArgs->anchoDestino - 1 - x;
                yOrigen = rArgs->altoDestino - 1 - y;
                break;
            case 270:
                // 270° horario: (x,y) -> (alto-1-y, x)
                xOrigen = rArgs->altoDestino - 1 - y;
                yOrigen = x;
                break;
            default:
                continue; // Saltar píxel si ángulo no válido
            }

            // Verificar límites
            if (xOrigen >= 0 && xOrigen < rArgs->anchoOrigen &&
                yOrigen >= 0 && yOrigen < rArgs->altoOrigen)
            {
                // Copia directa del píxel (sin interpolación)
                for (int c = 0; c < rArgs->canales; c++)
                {
                    rArgs->destino[y][x][c] = rArgs->origen[yOrigen][xOrigen][c];
                }
            }
            // Si está fuera de límites, el píxel queda negro (inicializado en asignarMatriz)
        }
    }
    return NULL;
}

void rotarImagenConcurrente(ImagenInfo *info, float angulo)
{
    if (!info->pixeles)
    {
        printf("No hay imagen cargada.\n");
        return;
    }

    // Normalizar ángulo y redondear al múltiplo de 90 más cercano
    int anguloInt = ((int)(angulo + 45) / 90) * 90;
    anguloInt = anguloInt % 360;
    if (anguloInt < 0)
        anguloInt += 360;

    // Validar que sea múltiplo de 90
    if (anguloInt != 90 && anguloInt != 180 && anguloInt != 270)
    {
        printf("Error: Solo se soportan rotaciones de 90°, 180° y 270°.\n");
        printf("Ángulo %.1f° redondeado a %d° no es válido.\n", angulo, anguloInt);
        return;
    }

    printf("Rotando imagen %d grados...\n", anguloInt);

    // Calcular nuevas dimensiones según el ángulo
    int nuevoAncho, nuevoAlto;
    switch (anguloInt)
    {
    case 90:
    case 270:
        // Para 90° y 270°, intercambiar ancho y alto
        nuevoAncho = info->alto;
        nuevoAlto = info->ancho;
        break;
    case 180:
        // Para 180°, las dimensiones se mantienen
        nuevoAncho = info->ancho;
        nuevoAlto = info->alto;
        break;
    default:
        return; 
    }

    printf("Dimensiones: %dx%d → %dx%d\n", info->ancho, info->alto, nuevoAncho, nuevoAlto);

    unsigned char ***nuevaMatriz = asignarMatriz(nuevoAlto, nuevoAncho, info->canales);
    if (!nuevaMatriz)
    {
        fprintf(stderr, "Error al asignar memoria para imagen rotada\n");
        return;
    }

    const int numHilos = 4; // Número configurable de hilos
    pthread_t hilos[numHilos];
    RotacionArgs args[numHilos];
    int filasPorHilo = (int)ceil((double)nuevoAlto / numHilos);

    for (int i = 0; i < numHilos; i++)
    {
        args[i].origen = info->pixeles;
        args[i].destino = nuevaMatriz;
        args[i].anchoOrigen = info->ancho;
        args[i].altoOrigen = info->alto;
        args[i].anchoDestino = nuevoAncho;
        args[i].altoDestino = nuevoAlto;
        args[i].canales = info->canales;
        args[i].angulo = (float)anguloInt;
        args[i].inicioY = i * filasPorHilo;
        args[i].finY = (i + 1) * filasPorHilo < nuevoAlto ? (i + 1) * filasPorHilo : nuevoAlto;

        if (pthread_create(&hilos[i], NULL, rotarImagenHilo, &args[i]) != 0)
        {
            fprintf(stderr, "Error al crear hilo %d\n", i);
            liberarMatriz(nuevaMatriz, nuevoAlto, nuevoAncho);
            return;
        }
    }

    for (int i = 0; i < numHilos; i++)
    {
        pthread_join(hilos[i], NULL);
    }

    liberarMatriz(info->pixeles, info->alto, info->ancho);
    info->pixeles = nuevaMatriz;
    info->ancho = nuevoAncho;
    info->alto = nuevoAlto;

    printf("Imagen rotada exitosamente usando %d hilos (%s).\n", numHilos,
           info->canales == 1 ? "grises" : "RGB");
}
