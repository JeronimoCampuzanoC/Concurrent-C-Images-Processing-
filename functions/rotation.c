#include "rotation.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// QUÉ: Asignar memoria para una nueva matriz de píxeles 3D.
// CÓMO: Crea estructura 3D dinámicamente: [alto][ancho][canales].
// POR QUÉ: Necesaria para crear la imagen rotada con nuevas dimensiones.
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

// QUÉ: Liberar memoria de una matriz de píxeles 3D.
// CÓMO: Libera cada nivel de la estructura 3D recursivamente.
// POR QUÉ: Evita fugas de memoria al cambiar dimensiones de imagen.
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

// QUÉ: Función ejecutada por cada hilo para procesar parte de la rotación.
// CÓMO: Para cada píxel en su rango, calcula la posición original usando
//       transformación inversa y copia el píxel con interpolación bilineal.
// POR QUÉ: Permite paralelizar el cálculo de píxeles rotados.
void *rotarImagenHilo(void *args)
{
    RotacionArgs *rArgs = (RotacionArgs *)args;

    // QUÉ: Procesar cada píxel en el rango asignado a este hilo.
    // CÓMO: Para cada píxel (x,y) en la imagen destino, calcular su posición
    //       en la imagen original usando transformación inversa.
    // POR QUÉ: La transformación inversa evita "huecos" en la imagen rotada.
    for (int y = rArgs->inicioY; y < rArgs->finY; y++)
    {
        for (int x = 0; x < rArgs->anchoDestino; x++)
        {
            // QUÉ: Trasladar coordenadas al centro de la imagen destino.
            // CÓMO: Resta el centro para rotar alrededor del punto central.
            // POR QUÉ: La rotación debe ser sobre el centro, no la esquina.
            float xRelativo = x - rArgs->centroXDestino;
            float yRelativo = y - rArgs->centroYDestino;

            // QUÉ: Aplicar transformación inversa para encontrar píxel original.
            // CÓMO: Usa la matriz de rotación inversa (transpuesta).
            //       x' = x*cos(-θ) - y*sin(-θ) = x*cos(θ) + y*sin(θ)
            //       y' = x*sin(-θ) + y*cos(-θ) = -x*sin(θ) + y*cos(θ)
            // POR QUÉ: Transformación inversa mapea destino → origen.
            float xOriginal = xRelativo * rArgs->cosAngulo + yRelativo * rArgs->sinAngulo;
            float yOriginal = -xRelativo * rArgs->sinAngulo + yRelativo * rArgs->cosAngulo;

            // QUÉ: Trasladar de vuelta al sistema de coordenadas original.
            // CÓMO: Suma el centro de la imagen original.
            // POR QUÉ: Convertir coordenadas relativas a absolutas.
            xOriginal += rArgs->centroXOrigen;
            yOriginal += rArgs->centroYOrigen;

            // QUÉ: Verificar si las coordenadas están dentro de la imagen original.
            // CÓMO: Comprueba límites con margen para interpolación bilineal.
            // POR QUÉ: Evitar acceso fuera de memoria y píxeles indefinidos.
            if (xOriginal >= 0 && xOriginal < rArgs->anchoOrigen - 1 &&
                yOriginal >= 0 && yOriginal < rArgs->altoOrigen - 1)
            {
                // QUÉ: Interpolación bilineal para píxeles no enteros.
                // CÓMO: Usa los 4 píxeles vecinos más cercanos y pondera por distancia.
                // POR QUÉ: Produce rotaciones más suaves que el redondeo simple.
                int x1 = (int)xOriginal;
                int y1 = (int)yOriginal;
                int x2 = x1 + 1;
                int y2 = y1 + 1;

                float dx = xOriginal - x1;
                float dy = yOriginal - y1;

                // QUÉ: Copiar cada canal con interpolación bilineal.
                // CÓMO: Pondera los 4 píxeles según distancia al punto exacto.
                // POR QUÉ: Mantiene calidad visual en rotaciones no múltiplos de 90°.
                for (int c = 0; c < rArgs->canales; c++)
                {
                    float valor = (1 - dx) * (1 - dy) * rArgs->origen[y1][x1][c] +
                                  dx * (1 - dy) * rArgs->origen[y1][x2][c] +
                                  (1 - dx) * dy * rArgs->origen[y2][x1][c] +
                                  dx * dy * rArgs->origen[y2][x2][c];

                    rArgs->destino[y][x][c] = (unsigned char)(valor + 0.5); // Redondeo
                }
            }
            // Si está fuera de límites, el píxel queda negro (inicializado en asignarMatriz)
        }
    }
    return NULL;
}

// QUÉ: Rotar imagen usando múltiples hilos para paralelizar el procesamiento.
// CÓMO: Calcula nuevas dimensiones, crea matriz destino, divide trabajo entre hilos.
// POR QUÉ: Demuestra concurrencia aplicada a transformaciones geométricas complejas.
void rotarImagenConcurrente(ImagenInfo *info, float angulo)
{
    if (!info->pixeles)
    {
        printf("No hay imagen cargada.\n");
        return;
    }

    // QUÉ: Normalizar ángulo y convertir a radianes.
    // CÓMO: Usa fmod para mantener en [0,360) y convierte a radianes.
    // POR QUÉ: Simplifica cálculos trigonométricos y evita valores extremos.
    angulo = fmod(angulo, 360.0f);
    if (angulo < 0)
        angulo += 360.0f;
    float anguloRad = angulo * M_PI / 180.0f;

    printf("Rotando imagen %.1f grados...\n", angulo);

    // QUÉ: Precalcular valores trigonométricos.
    // CÓMO: Calcula seno y coseno una vez para uso en todos los hilos.
    // POR QUÉ: Evita recálculos costosos en cada píxel.
    float cosAngulo = cos(anguloRad);
    float sinAngulo = sin(anguloRad);

    // QUÉ: Calcular nuevas dimensiones de la imagen rotada.
    // CÓMO: Proyecta las 4 esquinas y encuentra bounding box.
    // POR QUÉ: La imagen rotada puede tener diferentes dimensiones.
    float esquinas[4][2] = {
        {0, 0},
        {info->ancho - 1, 0},
        {info->ancho - 1, info->alto - 1},
        {0, info->alto - 1}};

    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    float centroXOrig = (info->ancho - 1) / 2.0f;
    float centroYOrig = (info->alto - 1) / 2.0f;

    for (int i = 0; i < 4; i++)
    {
        float x = esquinas[i][0] - centroXOrig;
        float y = esquinas[i][1] - centroYOrig;
        float xRot = x * cosAngulo - y * sinAngulo;
        float yRot = x * sinAngulo + y * cosAngulo;

        if (i == 0)
        {
            minX = maxX = xRot;
            minY = maxY = yRot;
        }
        else
        {
            if (xRot < minX)
                minX = xRot;
            if (xRot > maxX)
                maxX = xRot;
            if (yRot < minY)
                minY = yRot;
            if (yRot > maxY)
                maxY = yRot;
        }
    }

    int nuevoAncho = (int)(maxX - minX + 1);
    int nuevoAlto = (int)(maxY - minY + 1);

    printf("Dimensiones: %dx%d → %dx%d\n", info->ancho, info->alto, nuevoAncho, nuevoAlto);

    // QUÉ: Crear matriz para la imagen rotada.
    // CÓMO: Asigna memoria con las nuevas dimensiones calculadas.
    // POR QUÉ: La imagen rotada necesita espacio diferente al original.
    unsigned char ***nuevaMatriz = asignarMatriz(nuevoAlto, nuevoAncho, info->canales);
    if (!nuevaMatriz)
    {
        fprintf(stderr, "Error al asignar memoria para imagen rotada\n");
        return;
    }

    // QUÉ: Configurar parámetros de concurrencia.
    // CÓMO: Divide las filas entre múltiples hilos para paralelizar.
    // POR QUÉ: Acelera el procesamiento usando múltiples núcleos.
    const int numHilos = 4; // Número configurable de hilos
    pthread_t hilos[numHilos];
    RotacionArgs args[numHilos];
    int filasPorHilo = (int)ceil((double)nuevoAlto / numHilos);

    // QUÉ: Calcular centros de las imágenes.
    // CÓMO: Usa las dimensiones para encontrar puntos centrales.
    // POR QUÉ: La rotación debe ser alrededor del centro de cada imagen.
    int centroXDestino = nuevoAncho / 2;
    int centroYDestino = nuevoAlto / 2;

    // QUÉ: Crear y lanzar hilos de trabajo.
    // CÓMO: Configura argumentos para cada hilo y los ejecuta.
    // POR QUÉ: Paraleliza el cálculo de la transformación.
    for (int i = 0; i < numHilos; i++)
    {
        args[i].origen = info->pixeles;
        args[i].destino = nuevaMatriz;
        args[i].anchoOrigen = info->ancho;
        args[i].altoOrigen = info->alto;
        args[i].anchoDestino = nuevoAncho;
        args[i].altoDestino = nuevoAlto;
        args[i].canales = info->canales;
        args[i].angulo = anguloRad;
        args[i].inicioY = i * filasPorHilo;
        args[i].finY = (i + 1) * filasPorHilo < nuevoAlto ? (i + 1) * filasPorHilo : nuevoAlto;
        args[i].cosAngulo = cosAngulo;
        args[i].sinAngulo = sinAngulo;
        args[i].centroXOrigen = (int)centroXOrig;
        args[i].centroYOrigen = (int)centroYOrig;
        args[i].centroXDestino = centroXDestino;
        args[i].centroYDestino = centroYDestino;

        if (pthread_create(&hilos[i], NULL, rotarImagenHilo, &args[i]) != 0)
        {
            fprintf(stderr, "Error al crear hilo %d\n", i);
            liberarMatriz(nuevaMatriz, nuevoAlto, nuevoAncho);
            return;
        }
    }

    // QUÉ: Esperar a que todos los hilos terminen.
    // CÓMO: Usa pthread_join para sincronizar la finalización.
    // POR QUÉ: Garantiza que la rotación esté completa antes de continuar.
    for (int i = 0; i < numHilos; i++)
    {
        pthread_join(hilos[i], NULL);
    }

    // QUÉ: Reemplazar la imagen original con la rotada.
    // CÓMO: Libera memoria anterior y actualiza la estructura ImagenInfo.
    // POR QUÉ: La imagen rotada se convierte en la nueva imagen activa.
    liberarMatriz(info->pixeles, info->alto, info->ancho);
    info->pixeles = nuevaMatriz;
    info->ancho = nuevoAncho;
    info->alto = nuevoAlto;

    printf("Imagen rotada exitosamente usando %d hilos (%s).\n", numHilos,
           info->canales == 1 ? "grises" : "RGB");
}
