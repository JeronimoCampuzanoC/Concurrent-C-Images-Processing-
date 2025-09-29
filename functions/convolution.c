#include "convolution.h"
#include <pthread.h>
#include <math.h>
#include <string.h>

// QUÉ: Genera un kernel Gaussiano para filtro de desenfoque.
// CÓMO: Usa la fórmula matemática de la distribución Gaussiana 2D.
// POR QUÉ: El kernel Gaussiano suaviza la imagen eliminando ruido y detalles finos.
float** generarKernelGaussiano(int tamKernel, float sigma)
{
    // Asignar memoria para el kernel
    float **kernel = (float**)malloc(tamKernel * sizeof(float*));
    if (!kernel) {
        fprintf(stderr, "Error de memoria al asignar kernel\n");
        return NULL;
    }
    
    for (int i = 0; i < tamKernel; i++) {
        kernel[i] = (float*)malloc(tamKernel * sizeof(float));
        if (!kernel[i]) {
            fprintf(stderr, "Error de memoria al asignar fila del kernel\n");
            // Liberar memoria ya asignada
            for (int j = 0; j < i; j++) {
                free(kernel[j]);
            }
            free(kernel);
            return NULL;
        }
    }
    
    // Calcular centro del kernel
    int centro = tamKernel / 2;
    float suma = 0.0f;
    
    // Generar valores del kernel usando fórmula Gaussiana
    for (int y = 0; y < tamKernel; y++) {
        for (int x = 0; x < tamKernel; x++) {
            int dx = x - centro;
            int dy = y - centro;
            
            // Fórmula de distribución Gaussiana 2D
            float exponente = -(dx * dx + dy * dy) / (2.0f * sigma * sigma);
            kernel[y][x] = exp(exponente) / (2.0f * M_PI * sigma * sigma);
            suma += kernel[y][x];
        }
    }
    
    // Normalizar el kernel para que la suma sea 1
    for (int y = 0; y < tamKernel; y++) {
        for (int x = 0; x < tamKernel; x++) {
            kernel[y][x] /= suma;
        }
    }
    
    return kernel;
}

// QUÉ: Libera la memoria asignada para el kernel.
// CÓMO: Libera cada fila y luego el arreglo de filas.
// POR QUÉ: Evita fugas de memoria al limpiar recursos.
void liberarKernel(float **kernel, int tamKernel)
{
    if (kernel) {
        for (int i = 0; i < tamKernel; i++) {
            free(kernel[i]);
        }
        free(kernel);
    }
}

// QUÉ: Aplica convolución en un rango de filas (para hilos).
// CÓMO: Para cada píxel en el rango, multiplica con el kernel y maneja bordes.
// POR QUÉ: Procesa píxeles en paralelo para demostrar concurrencia.
void* convolucionHilo(void* args)
{
    ConvolucionArgs *cArgs = (ConvolucionArgs*)args;
    int centro = cArgs->tamKernel / 2;
    
    for (int y = cArgs->inicio; y < cArgs->fin; y++) {
        for (int x = 0; x < cArgs->ancho; x++) {
            for (int c = 0; c < cArgs->canales; c++) {
                float suma = 0.0f;
                
                // Aplicar kernel en la vecindad del píxel
                for (int ky = 0; ky < cArgs->tamKernel; ky++) {
                    for (int kx = 0; kx < cArgs->tamKernel; kx++) {
                        // Calcular coordenadas con manejo de bordes
                        int px = x + kx - centro;
                        int py = y + ky - centro;
                        
                        // Manejo de bordes: replicar píxeles de borde
                        if (px < 0) px = 0;
                        if (px >= cArgs->ancho) px = cArgs->ancho - 1;
                        if (py < 0) py = 0;
                        if (py >= cArgs->alto) py = cArgs->alto - 1;
                        
                        // Multiplicar píxel por valor del kernel
                        suma += cArgs->pixeles[py][px][c] * cArgs->kernel[ky][kx];
                    }
                }
                
                // Clamp el resultado entre 0-255
                int resultado = (int)round(suma);
                if (resultado < 0) resultado = 0;
                if (resultado > 255) resultado = 255;
                
                cArgs->pixelesResultado[y][x][c] = (unsigned char)resultado;
            }
        }
    }
    
    return NULL;
}

// QUÉ: Aplica convolución Gaussiana usando múltiples hilos.
// CÓMO: Divide las filas entre hilos, genera kernel, aplica convolución y sincroniza.
// POR QUÉ: Usa concurrencia para acelerar el procesamiento y enseñar hilos.
int aplicarConvolucionConcurrente(ImagenInfo* info, int tamKernel, float sigma, int numHilos)
{
    if (!info->pixeles) {
        fprintf(stderr, "No hay imagen cargada para aplicar convolución.\n");
        return 0;
    }
    
    if (tamKernel % 2 == 0) {
        fprintf(stderr, "El tamaño del kernel debe ser impar (3, 5, 7, etc.)\n");
        return 0;
    }
    
    if (numHilos < 1 || numHilos > 8) {
        fprintf(stderr, "Número de hilos debe estar entre 1 y 8\n");
        return 0;
    }
    
    // Generar kernel Gaussiano
    float **kernel = generarKernelGaussiano(tamKernel, sigma);
    if (!kernel) {
        return 0;
    }
    
    // Crear matriz de resultado
    unsigned char ***pixelesResultado = (unsigned char***)malloc(info->alto * sizeof(unsigned char**));
    if (!pixelesResultado) {
        fprintf(stderr, "Error de memoria al asignar matriz de resultado\n");
        liberarKernel(kernel, tamKernel);
        return 0;
    }
    
    for (int y = 0; y < info->alto; y++) {
        pixelesResultado[y] = (unsigned char**)malloc(info->ancho * sizeof(unsigned char*));
        if (!pixelesResultado[y]) {
            fprintf(stderr, "Error de memoria al asignar fila de resultado\n");
            // Limpiar memoria ya asignada
            for (int i = 0; i < y; i++) {
                free(pixelesResultado[i]);
            }
            free(pixelesResultado);
            liberarKernel(kernel, tamKernel);
            return 0;
        }
        
        for (int x = 0; x < info->ancho; x++) {
            pixelesResultado[y][x] = (unsigned char*)malloc(info->canales * sizeof(unsigned char));
            if (!pixelesResultado[y][x]) {
                fprintf(stderr, "Error de memoria al asignar píxel de resultado\n");
                // Limpiar memoria ya asignada
                for (int i = 0; i < y; i++) {
                    for (int j = 0; j < info->ancho; j++) {
                        free(pixelesResultado[i][j]);
                    }
                    free(pixelesResultado[i]);
                }
                for (int j = 0; j < x; j++) {
                    free(pixelesResultado[y][j]);
                }
                free(pixelesResultado[y]);
                free(pixelesResultado);
                liberarKernel(kernel, tamKernel);
                return 0;
            }
        }
    }
    
    // Configurar hilos
    pthread_t *hilos = (pthread_t*)malloc(numHilos * sizeof(pthread_t));
    ConvolucionArgs *args = (ConvolucionArgs*)malloc(numHilos * sizeof(ConvolucionArgs));
    
    if (!hilos || !args) {
        fprintf(stderr, "Error de memoria al asignar estructuras de hilos\n");
        // Limpiar memoria
        for (int y = 0; y < info->alto; y++) {
            for (int x = 0; x < info->ancho; x++) {
                free(pixelesResultado[y][x]);
            }
            free(pixelesResultado[y]);
        }
        free(pixelesResultado);
        liberarKernel(kernel, tamKernel);
        if (hilos) free(hilos);
        if (args) free(args);
        return 0;
    }
    
    int filasPorHilo = (int)ceil((double)info->alto / numHilos);
    
    // Crear y lanzar hilos
    for (int i = 0; i < numHilos; i++) {
        args[i].pixeles = info->pixeles;
        args[i].pixelesResultado = pixelesResultado;
        args[i].inicio = i * filasPorHilo;
        args[i].fin = (i + 1) * filasPorHilo < info->alto ? (i + 1) * filasPorHilo : info->alto;
        args[i].ancho = info->ancho;
        args[i].alto = info->alto;
        args[i].canales = info->canales;
        args[i].kernel = kernel;
        args[i].tamKernel = tamKernel;
        
        if (pthread_create(&hilos[i], NULL, convolucionHilo, &args[i]) != 0) {
            fprintf(stderr, "Error al crear hilo %d\n", i);
            // Limpiar memoria y salir
            for (int y = 0; y < info->alto; y++) {
                for (int x = 0; x < info->ancho; x++) {
                    free(pixelesResultado[y][x]);
                }
                free(pixelesResultado[y]);
            }
            free(pixelesResultado);
            liberarKernel(kernel, tamKernel);
            free(hilos);
            free(args);
            return 0;
        }
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numHilos; i++) {
        if (pthread_join(hilos[i], NULL) != 0) {
            fprintf(stderr, "Error al esperar hilo %d\n", i);
        }
    }
    
    // Reemplazar la imagen original con el resultado
    // Liberar imagen original
    for (int y = 0; y < info->alto; y++) {
        for (int x = 0; x < info->ancho; x++) {
            free(info->pixeles[y][x]);
        }
        free(info->pixeles[y]);
    }
    free(info->pixeles);
    
    // Asignar resultado
    info->pixeles = pixelesResultado;
    
    // Limpiar recursos
    liberarKernel(kernel, tamKernel);
    free(hilos);
    free(args);
    
    printf("Convolución Gaussiana aplicada con %d hilos (kernel %dx%d, sigma=%.2f, %s).\n", 
           numHilos, tamKernel, tamKernel, sigma, 
           info->canales == 1 ? "grises" : "RGB");
    
    return 1;
}
