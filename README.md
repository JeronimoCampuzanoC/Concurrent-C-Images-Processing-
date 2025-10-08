# Concurrent-C-Images-Processing-

## Descripción

Programa avanzado de procesamiento de imágenes PNG en C que utiliza concurrencia con pthreads para acelerar operaciones matriciales complejas. Incluye funcionalidades base como carga/guardado de imágenes y nuevas implementaciones de rotación concurrente.

```bash
gcc -o img.out img_base.c functions/rotation.c functions/resize.c functions/border.c functions/convolution.c  -pthread -lm
```

## Uso

```bash
./img.out
```

## Menú Interactivo

1. Cargar imagen PNG
2. Mostrar matriz de píxeles
3. Guardar como PNG
4. Ajustar brillo (+/- valor) concurrentemente
5. Convolucion(Desenfoque Gaussino)
6. Rotar imagen (ángulo en grados)
7. Deteccion de bordes
8. Redimensionar imagen(bilineal)
9. Salir

## Ejemplo de Uso - convolucion

Tamaño del kernel (Debe ser impar):3
Valor de sigma (1.0, 2.0, 3.0, 4.0, 5.0): 2.0
Número de hilos (1-4): 4

## Ejemplo de Uso - Rotación

Seleccion opcion 6
Ingresar Angulo de rotacion: 90
Guardar imagen con opcion 3

## Ejemplo de Uso - Bordes

Seleccion opcion 7
Ingresar cantidad de hilos de trabajo
Guardar imagen con opcion 3

## Ejemplo de Uso - Resize

Nuevo ancho: 500
Nuevo alto: 500
Número de hilos (1-4): 4


# Sistema utilizado
<img width="968" height="517" alt="image" src="https://github.com/user-attachments/assets/376e5cf9-7c45-4d14-b26e-cfc119662add" />

