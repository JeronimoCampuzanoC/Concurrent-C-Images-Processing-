# Implementación de Rotación de Imágenes Concurrente

## Resumen

Esta implementación extiende el programa base de procesamiento de imágenes con una nueva función de **rotación concurrente** que utiliza pthreads para paralelizar el procesamiento de transformaciones geométricas.

## Funcionalidades Implementadas

### 1. Rotación de Imagen Concurrente

- **QUÉ**: Rota una imagen en cualquier ángulo (0-360 grados) usando concurrencia
- **CÓMO**: Utiliza transformaciones matriciales con interpolación bilineal y divide el trabajo entre múltiples hilos
- **POR QUÉ**: Demuestra concurrencia aplicada a operaciones matriciales complejas y mejora el rendimiento

### Características Técnicas

#### Concurrencia

- **Número de hilos**: 4 hilos configurables
- **División del trabajo**: Por filas (cada hilo procesa un rango de filas)
- **Sincronización**: pthread_create() y pthread_join()
- **Prevención de race conditions**: Cada hilo trabaja en una región independiente de la matriz destino

#### Algoritmo de Rotación

- **Transformación inversa**: Mapea cada píxel destino al píxel origen correspondiente
- **Interpolación bilineal**: Suaviza la rotación usando los 4 píxeles vecinos más cercanos
- **Cálculo de dimensiones**: Determina automáticamente el tamaño de la imagen rotada
- **Centrado**: Rota alrededor del centro de la imagen

#### Fórmulas Matemáticas

```
Transformación directa:
x' = x*cos(θ) - y*sin(θ)
y' = x*sin(θ) + y*cos(θ)

Transformación inversa (usada en la implementación):
x_orig = x_dest*cos(θ) + y_dest*sin(θ)
y_orig = -x_dest*sin(θ) + y_dest*cos(θ)
```

#### Interpolación Bilineal

```
valor = (1-dx)*(1-dy)*P(x1,y1) + dx*(1-dy)*P(x2,y1) +
        (1-dx)*dy*P(x1,y2) + dx*dy*P(x2,y2)
```

## Estructura de Archivos

### `functions/rotation.h`

- Declaraciones de funciones y estructuras
- Estructura `RotacionArgs` para pasar datos a hilos
- Prototipos de funciones públicas

### `functions/rotation.c`

- Implementación completa de la rotación concurrente
- Funciones auxiliares para manejo de memoria
- Lógica de hilos y transformaciones matemáticas

### Modificaciones en `img_base.c`

- Integración en el menú principal (opción 5)
- Inclusión del header de rotación
- Manejo de entrada del usuario para ángulos

## Uso del Programa

### Compilación

```bash
gcc -o img img_base.c functions/rotation.c -pthread -lm
```

### Ejecución

```bash
./img [imagen.png]
```

### Menú de Opciones

1. Cargar imagen PNG
2. Mostrar matriz de píxeles
3. Guardar como PNG
4. Ajustar brillo (+/- valor) concurrentemente
5. **Rotar imagen (ángulo en grados)** ← NUEVA FUNCIÓN
6. Salir

## Ejemplos de Uso

### Rotación de 90 grados

```
Opción: 5
Ángulo de rotación en grados (0-360): 90
Rotando imagen 90.0 grados...
Dimensiones: 800x600 → 600x800
Imagen rotada exitosamente usando 4 hilos (RGB).
```

### Rotación de 45 grados

```
Opción: 5
Ángulo de rotación en grados (0-360): 45
Rotando imagen 45.0 grados...
Dimensiones: 800x600 → 1131x1131
Imagen rotada exitosamente usando 4 hilos (RGB).
```

## Características de Rendimiento

### Paralelización

- **División por filas**: Cada hilo procesa aproximadamente alto/numHilos filas
- **Balanceo de carga**: Distribución uniforme del trabajo
- **Escalabilidad**: Número de hilos configurable (actualmente 4)

### Optimizaciones

- **Precálculo de trigonometría**: sin() y cos() calculados una sola vez
- **Interpolación eficiente**: Cálculos de punto flotante optimizados
- **Gestión de memoria**: Asignación y liberación eficiente de matrices 3D

## Compatibilidad

### Formatos de Imagen

- ✅ Escala de grises (1 canal)
- ✅ RGB (3 canales)
- ✅ Dimensiones variables

### Ángulos Soportados

- ✅ Cualquier ángulo de 0-360 grados
- ✅ Múltiplos de 90° (rotaciones exactas)
- ✅ Ángulos arbitrarios (con interpolación)

## Manejo de Errores

### Gestión de Memoria

- Verificación de malloc() en cada asignación
- Liberación correcta en caso de error
- Prevención de fugas de memoria

### Validación de Entrada

- Verificación de imagen cargada
- Normalización de ángulos (0-360°)
- Manejo de coordenadas fuera de límites

### Errores de Concurrencia

- Verificación de pthread_create()
- Sincronización correcta con pthread_join()
- Manejo de fallos en creación de hilos

## Ventajas de la Implementación

1. **Concurrencia Real**: Utiliza múltiples núcleos del procesador
2. **Flexibilidad**: Soporta cualquier ángulo de rotación
3. **Calidad Visual**: Interpolación bilineal para rotaciones suaves
4. **Robustez**: Manejo completo de errores y memoria
5. **Escalabilidad**: Fácil modificación del número de hilos
6. **Compatibilidad**: Funciona con grises y RGB sin modificaciones

## Extensiones Futuras Posibles

1. **Configuración dinámica de hilos**: Permitir que el usuario especifique el número de hilos
2. **Rotación en lotes**: Procesar múltiples imágenes simultáneamente
3. **Diferentes algoritmos de interpolación**: Bicúbica, Lanczos, etc.
4. **Optimización SIMD**: Usar instrucciones vectoriales para mayor rendimiento
5. **Rotación en lugar**: Optimizar memoria para rotaciones de 90°, 180°, 270°

## Conclusión

La implementación de rotación concurrente demuestra exitosamente:

- Aplicación práctica de pthreads en procesamiento de imágenes
- Manejo eficiente de matrices 3D complejas
- Implementación robusta de transformaciones geométricas
- Integración limpia con el programa base existente

Esta función sirve como excelente ejemplo de cómo la concurrencia puede mejorar significativamente el rendimiento en operaciones intensivas sobre matrices de datos.
