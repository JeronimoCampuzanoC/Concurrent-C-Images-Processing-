# Concurrent-C-Images-Processing-

## Descripción

Programa avanzado de procesamiento de imágenes PNG en C que utiliza concurrencia con pthreads para acelerar operaciones matriciales complejas. Incluye funcionalidades base como carga/guardado de imágenes y nuevas implementaciones de rotación concurrente.

## Características Principales

### Funcionalidades Base

- ✅ Carga y guardado de imágenes PNG (escala de grises y RGB)
- ✅ Visualización de matrices de píxeles
- ✅ Ajuste de brillo concurrente

### Nueva Funcionalidad Implementada

- 🆕 **Rotación de Imagen Concurrente**: Rota imágenes en cualquier ángulo usando múltiples hilos

## Compilación

```bash
gcc -o img img_base.c functions/rotation.c -pthread -lm
```

## Uso

```bash
./img [imagen.png]
```

## Menú Interactivo

1. Cargar imagen PNG
2. Mostrar matriz de píxeles
3. Guardar como PNG
4. Ajustar brillo (+/- valor) concurrentemente
5. **Rotar imagen (ángulo en grados)** ← NUEVO
6. Salir

## Implementación de Rotación

### Características Técnicas

- **Concurrencia**: 4 hilos configurables
- **Algoritmo**: Transformación inversa con interpolación bilineal
- **Compatibilidad**: Escala de grises y RGB
- **Ángulos**: Cualquier valor de 0-360 grados

### Rendimiento

- Paralelización por filas para optimal balanceo de carga
- Precálculo de operaciones trigonométricas
- Gestión eficiente de memoria para matrices 3D

## Estructura del Proyecto

```
├── img_base.c              # Programa principal
├── functions/
│   ├── rotation.h          # Header de rotación
│   ├── rotation.c          # Implementación de rotación
│   ├── border.h/.c         # (Para futuras implementaciones)
│   ├── convolution.h/.c    # (Para futuras implementaciones)
│   └── resize.h/.c         # (Para futuras implementaciones)
├── stb_image.h             # Biblioteca para carga de imágenes
├── stb_image_write.h       # Biblioteca para guardado de imágenes
├── test_rotation.sh        # Script de pruebas
├── ROTATION_DOCS.md        # Documentación detallada
└── README.md               # Este archivo
```

## Ejemplo de Uso - Rotación

```bash
./img profesionalPhoto.png
# Seleccionar opción 5
# Ingresar ángulo: 45
# La imagen se rota 45 grados usando 4 hilos
# Guardar resultado con opción 3
```

## Dependencias

- `pthread` - Para concurrencia
- `math.h` - Para operaciones trigonométricas
- `stb_image.h` y `stb_image_write.h` - Para manejo de PNG

## Documentación Adicional

Ver `ROTATION_DOCS.md` para documentación técnica detallada de la implementación de rotación concurrente.

## Requisitos Cumplidos

- ✅ Mantiene estructura base sin modificar funciones existentes
- ✅ Compilación con `gcc -o img img_base.c -pthread -lm`
- ✅ Manejo de imágenes en escala de grises y RGB
- ✅ Gestión dinámica de memoria sin fugas
- ✅ Implementación de rotación con concurrencia (mínimo 2 hilos)
- ✅ Operaciones matriciales complejas con transformaciones geométricas
- ✅ Integración en menú interactivo
- ✅ Manejo robusto de errores
