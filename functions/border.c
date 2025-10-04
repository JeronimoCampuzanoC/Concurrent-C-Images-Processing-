#include "border.h"
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int crearimagenagenVaciaLocal(ImagenInfo *out, int ancho, int alto, int canales)
{
    out->ancho = ancho;
    out->alto = alto;
    out->canales = canales;
    out->pixeles = (unsigned char ***)malloc(alto * sizeof(unsigned char **));
    if (!out->pixeles)
        return 0;

    for (int y = 0; y < alto; y++)
    {
        out->pixeles[y] = (unsigned char **)malloc(ancho * sizeof(unsigned char *));
        if (!out->pixeles[y])
        {
            // liberar parcial
            for (int yy = 0; yy < y; yy++)
            {
                for (int xx = 0; xx < ancho; xx++)
                    free(out->pixeles[yy][xx]);
                free(out->pixeles[yy]);
            }
            free(out->pixeles);
            out->pixeles = NULL;
            out->ancho = out->alto = out->canales = 0;
            return 0;
        }
        for (int x = 0; x < ancho; x++)
        {
            out->pixeles[y][x] = (unsigned char *)malloc(canales * sizeof(unsigned char));
            if (!out->pixeles[y][x])
            {
                // liberar parcial de esta fila
                for (int xx = 0; xx < x; xx++)
                    free(out->pixeles[y][xx]);
                free(out->pixeles[y]);
                // liberar filas anteriores
                for (int yy = 0; yy < y; yy++)
                {
                    for (int xx2 = 0; xx2 < ancho; xx2++)
                        free(out->pixeles[yy][xx2]);
                    free(out->pixeles[yy]);
                }
                free(out->pixeles);
                out->pixeles = NULL;
                out->ancho = out->alto = out->canales = 0;
                return 0;
            }
            for (int c = 0; c < canales; c++)
                out->pixeles[y][x][c] = 0;
        }
    }
    return 1;
}

// Convierte a escala de grises porque se necesita para el metodo del Sobel ese
static int escalaDeGrises(const ImagenInfo *src, ImagenInfo *dst)
{
    if (!crearimagenagenVaciaLocal(dst, src->ancho, src->alto, 1))
        return 0;

    if (src->canales == 1)
    {
        for (int y = 0; y < src->alto; y++)
            for (int x = 0; x < src->ancho; x++)
                dst->pixeles[y][x][0] = src->pixeles[y][x][0];
        return 1;
    }
    else
    {
        for (int y = 0; y < src->alto; y++)
        {
            for (int x = 0; x < src->ancho; x++)
            {
                int r = src->pixeles[y][x][0];
                int g = src->pixeles[y][x][1];
                int b = src->pixeles[y][x][2];
                int gray = (r + g + b) / 3; // simple y didáctico
                if (gray < 0)
                    gray = 0;
                else if (gray > 255)
                    gray = 255;
                dst->pixeles[y][x][0] = (unsigned char)gray;
            }
        }
        return 1;
    }
}

// ---------- El Sobel ese ----------

typedef struct
{
    unsigned char ***src; // [alto][ancho][1]
    unsigned char ***dst; // [alto][ancho][1]
    int ancho, alto;
    int fila_ini, fila_fin; // [ini, fin)
} AgrumentosSobel;

// Con este clampi y sample es que manejamos los bordes (Los que no alcanzan a tener la matriz 3x3 completa)
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static inline unsigned char sample(unsigned char ***A, int y, int x, int alto, int ancho)
{
    y = clampi(y, 0, alto - 1);
    x = clampi(x, 0, ancho - 1);
    return A[y][x][0];
}

static void *sobelWorker(void *arg)
{
    AgrumentosSobel *S = (AgrumentosSobel *)arg;
    static const int Gx[3][3] = {{-1, 0, 1},
                                 {-2, 0, 2},
                                 {-1, 0, 1}};
    static const int Gy[3][3] = {{-1, -2, -1},
                                 {0, 0, 0},
                                 {1, 2, 1}};

    for (int y = S->fila_ini; y < S->fila_fin; y++)
    {
        for (int x = 0; x < S->ancho; x++)
        {
            int sx = 0, sy = 0;
            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    int p = sample(S->src, y + ky, x + kx, S->alto, S->ancho);
                    sx += Gx[ky + 1][kx + 1] * p;
                    sy += Gy[ky + 1][kx + 1] * p;
                }
            }
            int mag = (int)lround(sqrt((double)(sx * sx + sy * sy)));
            if (mag > 255)
                mag = 255;
            if (mag < 0)
                mag = 0;
            S->dst[y][x][0] = (unsigned char)mag;
        }
    }
    return NULL;
}

void liberarImagenBorder(ImagenInfo *info)
{
    if (info->pixeles)
    {
        for (int y = 0; y < info->alto; y++)
        {
            for (int x = 0; x < info->ancho; x++)
            {
                free(info->pixeles[y][x]); // Liberar canales por píxel
            }
            free(info->pixeles[y]); // Liberar fila
        }
        free(info->pixeles); // Liberar arreglo de filas
        info->pixeles = NULL;
    }
    info->ancho = 0;
    info->alto = 0;
    info->canales = 0;
}

int detectarBordesSobel(ImagenInfo *info, int nHilos)
{
    if (!info || !info->pixeles || info->ancho <= 0 || info->alto <= 0)
    {
        fprintf(stderr, "Sobel: imagen inválida.\n");
        return 0;
    }

    if (nHilos <= 0)
        nHilos = 2;
    if (nHilos > info->alto)
        nHilos = info->alto;

    // 1) Convertir a grises (buffer temporal)
    ImagenInfo gris = {0};
    if (!escalaDeGrises(info, &gris))
    {
        fprintf(stderr, "Sobel: error convirtiendo a grises.\n");
        return 0;
    }

    // 2) Crear destino
    ImagenInfo dst = {0};
    if (!crearimagenagenVaciaLocal(&dst, gris.ancho, gris.alto, 1))
    {
        fprintf(stderr, "Sobel: error creando salida.\n");
        liberarImagenBorder(&gris);
        return 0;
    }

    // 3) Hilos
    pthread_t *hilos = (pthread_t *)malloc((size_t)nHilos * sizeof(pthread_t));
    AgrumentosSobel *args = (AgrumentosSobel *)malloc((size_t)nHilos * sizeof(AgrumentosSobel));
    if (!hilos || !args)
    {
        fprintf(stderr, "Sobel: error de memoria para hilos.\n");
        free(hilos);
        free(args);
        liberarImagenBorder(&gris);
        liberarImagenBorder(&dst);
        return 0;
    }

    int filasPorHilo = (gris.alto + nHilos - 1) / nHilos;
    for (int i = 0; i < nHilos; i++)
    {
        args[i].src = gris.pixeles;
        args[i].dst = dst.pixeles;
        args[i].ancho = gris.ancho;
        args[i].alto = gris.alto;
        args[i].fila_ini = i * filasPorHilo;
        args[i].fila_fin = (i + 1) * filasPorHilo;
        if (args[i].fila_fin > gris.alto)
            args[i].fila_fin = gris.alto;

        if (pthread_create(&hilos[i], NULL, sobelWorker, &args[i]) != 0)
        {
            fprintf(stderr, "Sobel: error creando hilo %d.\n", i);
            for (int j = 0; j < i; j++)
                pthread_join(hilos[j], NULL);
            free(hilos);
            free(args);
            liberarImagenBorder(&gris);
            liberarImagenBorder(&dst);
            return 0;
        }
    }
    for (int i = 0; i < nHilos; i++)
        pthread_join(hilos[i], NULL);
    free(hilos);
    free(args);

    // 4) Reemplazar imagen original con el resultado
    liberarImagenBorder(info);
    *info = dst;       // mueve punteros
    info->canales = 1; // mapa de bordes en gris

    // 5) Liberar temporales
    liberarImagenBorder(&gris);

    printf("Bordes (Sobel) aplicados con %d hilos.\n", nHilos);
    return 1;
}
