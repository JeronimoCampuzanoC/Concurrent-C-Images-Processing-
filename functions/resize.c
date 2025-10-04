// Redimensionado bilineal concurrente

#include "resize.h"

static inline unsigned char clampToByte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (unsigned char)value;
}

// Hilo: procesa filas [filaInicio, filaFin) de la imagen destino
static void *resizeBilinealHilo(void *argsPtr)
{
    ResizeArgs *args = (ResizeArgs *)argsPtr;

    const double scaleX = (args->srcAncho > 1 && args->dstAncho > 1) ? (double)(args->srcAncho - 1) / (double)(args->dstAncho - 1) : 0.0;
    const double scaleY = (args->srcAlto > 1 && args->dstAlto > 1) ? (double)(args->srcAlto - 1) / (double)(args->dstAlto - 1) : 0.0;

    for (int y = args->filaInicio; y < args->filaFin; y++)
    {
        double srcY = scaleY * y;
        int y0 = (int)floor(srcY);
        int y1 = y0 + 1;
        if (y1 >= args->srcAlto) y1 = args->srcAlto - 1;
        double wy = srcY - y0;

        for (int x = 0; x < args->dstAncho; x++)
        {
            double srcX = scaleX * x;
            int x0 = (int)floor(srcX);
            int x1 = x0 + 1;
            if (x1 >= args->srcAncho) x1 = args->srcAncho - 1;
            double wx = srcX - x0;

            for (int c = 0; c < args->canales; c++)
            {
                int p00 = args->srcPixeles[y0][x0][c];
                int p10 = args->srcPixeles[y0][x1][c];
                int p01 = args->srcPixeles[y1][x0][c];
                int p11 = args->srcPixeles[y1][x1][c];

                // Interpolación bilineal: mezcla en X y luego en Y
                double top = p00 + wx * (p10 - p00);
                double bottom = p01 + wx * (p11 - p01);
                int value = (int)round(top + wy * (bottom - top));

                args->dstPixeles[y][x][c] = clampToByte(value);
            }
        }
    }

    return NULL;
}

int resizeBilinealConcurrente(ImagenInfo *info, int nuevoAncho, int nuevoAlto, int numHilos)
{
    if (!info || !info->pixeles)
    {
        fprintf(stderr, "No hay imagen cargada para redimensionar.\n");
        return 0;
    }
    if (nuevoAncho <= 0 || nuevoAlto <= 0)
    {
        fprintf(stderr, "Tamaño de destino inválido (%d x %d).\n", nuevoAncho, nuevoAlto);
        return 0;
    }
    if (numHilos < 1 || numHilos > 4)
    {
        fprintf(stderr, "Número de hilos debe estar entre 1 y 4\n");
        return 0;
    }

    // Asignar imagen destino
    unsigned char ***dst = (unsigned char ***)malloc(nuevoAlto * sizeof(unsigned char **));
    if (!dst)
    {
        fprintf(stderr, "Error de memoria al asignar filas destino\n");
        return 0;
    }
    for (int y = 0; y < nuevoAlto; y++)
    {
        dst[y] = (unsigned char **)malloc(nuevoAncho * sizeof(unsigned char *));
        if (!dst[y])
        {
            fprintf(stderr, "Error de memoria al asignar columnas destino\n");
            for (int i = 0; i < y; i++) free(dst[i]);
            free(dst);
            return 0;
        }
        for (int x = 0; x < nuevoAncho; x++)
        {
            dst[y][x] = (unsigned char *)malloc(info->canales * sizeof(unsigned char));
            if (!dst[y][x])
            {
                fprintf(stderr, "Error de memoria al asignar píxel destino\n");
                for (int i = 0; i < y; i++)
                {
                    for (int j = 0; j < nuevoAncho; j++) free(dst[i][j]);
                    free(dst[i]);
                }
                for (int j = 0; j < x; j++) free(dst[y][j]);
                free(dst[y]);
                free(dst);
                return 0;
            }
        }
    }

    // Crear hilos y args
    pthread_t *hilos = (pthread_t *)malloc(numHilos * sizeof(pthread_t));
    ResizeArgs *args = (ResizeArgs *)malloc(numHilos * sizeof(ResizeArgs));
    if (!hilos || !args)
    {
        fprintf(stderr, "Error de memoria al asignar estructuras de hilos\n");
        if (hilos) free(hilos);
        if (args) free(args);
        for (int y = 0; y < nuevoAlto; y++)
        {
            for (int x = 0; x < nuevoAncho; x++) free(dst[y][x]);
            free(dst[y]);
        }
        free(dst);
        return 0;
    }

    int filasPorHilo = (int)ceil((double)nuevoAlto / numHilos);
    for (int i = 0; i < numHilos; i++)
    {
        args[i].srcPixeles = info->pixeles;
        args[i].dstPixeles = dst;
        args[i].srcAncho = info->ancho;
        args[i].srcAlto = info->alto;
        args[i].canales = info->canales;
        args[i].dstAncho = nuevoAncho;
        args[i].dstAlto = nuevoAlto;
        args[i].filaInicio = i * filasPorHilo;
        args[i].filaFin = (i + 1) * filasPorHilo;
        if (args[i].filaInicio > nuevoAlto) args[i].filaInicio = nuevoAlto;
        if (args[i].filaFin > nuevoAlto) args[i].filaFin = nuevoAlto;

        if (pthread_create(&hilos[i], NULL, resizeBilinealHilo, &args[i]) != 0)
        {
            fprintf(stderr, "Error al crear hilo %d\n", i);
            // Join previos creados y limpiar
            for (int j = 0; j < i; j++) pthread_join(hilos[j], NULL);
            free(hilos);
            free(args);
            for (int y = 0; y < nuevoAlto; y++)
            {
                for (int x = 0; x < nuevoAncho; x++) free(dst[y][x]);
                free(dst[y]);
            }
            free(dst);
            return 0;
        }
    }

    for (int i = 0; i < numHilos; i++)
    {
        if (pthread_join(hilos[i], NULL) != 0)
        {
            fprintf(stderr, "Error al esperar hilo %d\n", i);
        }
    }

    // Reemplazar imagen original por la redimensionada
    for (int y = 0; y < info->alto; y++)
    {
        for (int x = 0; x < info->ancho; x++) free(info->pixeles[y][x]);
        free(info->pixeles[y]);
    }
    free(info->pixeles);

    info->pixeles = dst;
    info->ancho = nuevoAncho;
    info->alto = nuevoAlto;

    free(hilos);
    free(args);

    printf("Redimensionado bilineal aplicado con %d hilos: %dx%d -> %dx%d (%s).\n",
           numHilos,
           args[0].srcAncho, args[0].srcAlto,
           nuevoAncho, nuevoAlto,
           info->canales == 1 ? "grises" : "RGB");

    return 1;
}