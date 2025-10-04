#ifndef IMAGEN_INFO_H
#define IMAGEN_INFO_H

// QUÉ: Estructura para almacenar la imagen (ancho, alto, canales, píxeles).
// CÓMO: Usa matriz 3D para píxeles (alto x ancho x canales), donde canales es
// 1 (grises) o 3 (RGB). Píxeles son unsigned char (0-255).
// POR QUÉ: Permite manejar tanto grises como color, con memoria dinámica para
// flexibilidad y evitar desperdicio.
typedef struct
{
    int ancho;                // Ancho de la imagen en píxeles
    int alto;                 // Alto de la imagen en píxeles
    int canales;              // 1 (escala de grises) o 3 (RGB)
    unsigned char ***pixeles; // Matriz 3D: [alto][ancho][canales]
} ImagenInfo;

#endif // IMAGEN_INFO_H