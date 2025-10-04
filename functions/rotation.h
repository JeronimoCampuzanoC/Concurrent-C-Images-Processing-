#ifndef ROTATION_H
#define ROTATION_H

// QUÉ: Estructura para almacenar información de la imagen.
// CÓMO: Contiene dimensiones, canales y matriz de píxeles 3D.
// POR QUÉ: Necesitamos la definición completa para las funciones.
#ifndef IMAGENINFO_DEFINED
#define IMAGENINFO_DEFINED
typedef struct
{
    int ancho;                // Ancho de la imagen en píxeles
    int alto;                 // Alto de la imagen en píxeles
    int canales;              // 1 (escala de grises) o 3 (RGB)
    unsigned char ***pixeles; // Matriz 3D: [alto][ancho][canales]
} ImagenInfo;
#endif

// QUÉ: Estructura para pasar argumentos a los hilos de rotación.
// CÓMO: Contiene matrices origen/destino, dimensiones, ángulo y rango de trabajo.
// POR QUÉ: Los hilos necesitan datos específicos para procesar en paralelo.
typedef struct
{
    unsigned char ***origen;  // Matriz de píxeles original
    unsigned char ***destino; // Matriz de píxeles rotada
    int anchoOrigen;          // Ancho de la imagen original
    int altoOrigen;           // Alto de la imagen original
    int anchoDestino;         // Ancho de la imagen rotada
    int altoDestino;          // Alto de la imagen rotada
    int canales;              // Número de canales
    float angulo;             // Ángulo de rotación en radianes
    int inicioY;              // Fila inicial para este hilo
    int finY;                 // Fila final para este hilo
    float cosAngulo;          // Coseno del ángulo (precalculado)
    float sinAngulo;          // Seno del ángulo (precalculado)
    int centroXOrigen;        // Centro X de la imagen original
    int centroYOrigen;        // Centro Y de la imagen original
    int centroXDestino;       // Centro X de la imagen rotada
    int centroYDestino;       // Centro Y de la imagen rotada
} RotacionArgs;

// QUÉ: Rotar imagen en múltiplos de 90 grados usando concurrencia.
// CÓMO: Divide el trabajo entre hilos y usa transformaciones matriciales.
// POR QUÉ: Demuestra concurrencia aplicada a transformaciones geométricas.
void rotarImagenConcurrente(ImagenInfo *info, float angulo);

// QUÉ: Función ejecutada por cada hilo para procesar parte de la rotación.
// CÓMO: Calcula nuevas coordenadas usando matrices de transformación.
// POR QUÉ: Permite paralelizar el cálculo de píxeles rotados.
void *rotarImagenHilo(void *args);

// QUÉ: Asignar memoria para una nueva matriz de píxeles.
// CÓMO: Crea estructura 3D dinámicamente con dimensiones dadas.
// POR QUÉ: Necesaria para crear la imagen rotada con nuevas dimensiones.
unsigned char ***asignarMatriz(int alto, int ancho, int canales);

// QUÉ: Liberar memoria de una matriz de píxeles.
// CÓMO: Libera cada nivel de la estructura 3D recursivamente.
// POR QUÉ: Evita fugas de memoria al cambiar dimensiones de imagen.
void liberarMatriz(unsigned char ***matriz, int alto, int ancho);

#endif // ROTATION_H
