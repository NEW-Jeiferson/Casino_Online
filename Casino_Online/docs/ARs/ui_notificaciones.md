# Módulo UI / Notificaciones

## 1. Resumen del módulo
El módulo `ui/notificaciones` implementa un sistema de notificaciones flotantes transitorias (non-blocking toast notifications) para un "Serious Game" de casino en OpenGL, escrito en C89. Este sistema permite mostrar mensajes efímeros en la pantalla (como alertas o retroalimentación rápida) sin interrumpir el flujo principal del juego ni bloquear al usuario. Las notificaciones aparecen apiladas en la esquina superior derecha, cuentan con animaciones de desvanecimiento de entrada y salida (*fade in / fade out*), y desaparecen automáticamente tras un tiempo predefinido.

## 2. Estructuras de datos clave

El módulo se basa en la estructura interna `Notificacion`, la cual almacena la información individual de cada mensaje flotante:

```c
typedef struct {
    char texto[128];             /* Cadena de texto a mostrar (truncado a 128 chars) */
    float r;                     /* Componente rojo del color (0.0f a 1.0f) */
    float g;                     /* Componente verde del color (0.0f a 1.0f) */
    float b;                     /* Componente azul del color (0.0f a 1.0f) */
    unsigned int tiempo_inicio_ms; /* Timestamp en que se creó (para animación y tiempo de vida) */
    int activa;                  /* Bandera (booleano en C89) que indica si está en uso (1) o libre (0) */
} Notificacion;
```

También define el arreglo global estático `g_notificaciones` con un tamaño máximo definido por `MAX_NOTIFICACIONES` (actualmente configurado en `3`), limitando la cantidad de notificaciones simultáneas.

## 3. Funciones públicas

### `void inicializar_notificaciones(void)`
- **¿Para qué funciona/sirve?**
  Prepara el subsistema de notificaciones para su uso, marcando todas las ranuras del arreglo global de notificaciones como inactivas y vaciando sus cadenas de texto.
- **¿Por qué se diseñó de esta manera?**
  Dado que C89 no tiene inicialización garantizada implícita para variables dinámicas, esta función se requiere para dejar el arreglo en un estado determinista limpio al arrancar el motor del juego.
- **¿Cómo lo hace internamente?**
  Itera desde `0` hasta `MAX_NOTIFICACIONES - 1`. Por cada iteración, establece la bandera `activa` en `0` y coloca un carácter nulo (`'\0'`) en la primera posición del array `texto`.

### `void agregar_notificacion(const char* texto, float r, float g, float b)`
- **¿Para qué funciona/sirve?**
  Agrega un nuevo mensaje flotante a la cola para que se renderice en pantalla. Recibe el texto a mostrar y su color RGB.
- **¿Por qué se diseñó de esta manera?**
  Implementa una cola de retroalimentación efímera que evita desbordamientos. Al limitar el máximo a 3 (según macro), si llega una nueva notificación y la cola está llena, el sistema desplaza la cola descartando la más antigua, preservando siempre la información más reciente. Es compatible con el estándar C89 sin usar memoria dinámica (`malloc`), utilizando búferes estáticos, lo cual evita problemas de segmentación o fugas de memoria, algo crítico en Serious Games donde se requiere estabilidad continua.
- **¿Cómo lo hace internamente?**
  Primero, valida que la cadena no sea nula o vacía. Obtiene el tiempo actual mediante `glutGet(GLUT_ELAPSED_TIME)`. Busca una casilla inactiva (`activa == 0`) en el arreglo. Si la encuentra, la utiliza. Si no hay ninguna inactiva (cola llena), desplaza todas las notificaciones hacia arriba (descartando la del índice 0) y toma la última posición. Finalmente, copia la cadena (usando `strncpy_s` con `_TRUNCATE` para evitar desbordamientos en el buffer `texto` de 128 caracteres), guarda los componentes de color RGB, guarda el tiempo actual y marca la notificación como `activa = 1`.

### `void dibujar_notificaciones(void)`
- **¿Para qué funciona/sirve?**
  Se encarga de renderizar visualmente las notificaciones activas sobre el lienzo OpenGL, manejando las transformaciones ortogonales (2D) y la opacidad (*alpha blending*) para crear animaciones de transición.
- **¿Por qué se diseñó de esta manera?**
  En un sistema de retroalimentación en tiempo real (C89 + OpenGL fijo/inmediato), se requiere un método para superponer UI a la vista 3D. Se diseñó basándose en el tiempo (`glutGet(GLUT_ELAPSED_TIME)`) en lugar de frames, para que las animaciones de *fade in / fade out* duren lo mismo independientemente de la velocidad de fotogramas del usuario. Usa la matriz `GL_PROJECTION` para crear temporalmente una vista 2D independiente de la cámara 3D subyacente.
- **¿Cómo lo hace internamente?**
  Guarda las matrices actuales (`glPushMatrix`) de Proyección y Modelo, cambia la proyección a modo ortogonal (2D) desactivando luces y test de profundidad (`GL_DEPTH_TEST`), y activa la transparencia (`GL_BLEND`).
  Itera sobre las notificaciones. Para las que están activas, calcula el tiempo transcurrido desde su inicio. Si supera la duración total (3300 ms), las desactiva. Si están en el período de entrada (300 ms) o salida (600 ms), ajusta dinámicamente un multiplicador `alpha`.
  Dibuja las notificaciones visibles de arriba hacia abajo apiladas en la esquina superior derecha (X calculada dinámicamente según `glutGet(GLUT_WINDOW_WIDTH)` y ancho de la tarjeta). Dibuja un fondo semitransparente oscuro (`GL_QUADS`), un borde dorado (`GL_LINE_LOOP`), y por último dibuja la cadena carácter a carácter (usando función estática auxiliar `dibujar_texto_2d_notif` y `glutBitmapCharacter`).
  Restaura los estados originales de OpenGL (`glPopMatrix`, habilitar luces y depth test).

## 4. Dependencias
- **Librerías estándar (C89):** `<stdio.h>`, `<string.h>` (para el manejo del texto y `strncpy_s`).
- **OpenGL / GLUT:** `<GL/glut.h>` (Para `glutGet`, dibujo 2D inmediato con `GL_QUADS`/`GL_LINE_LOOP`, texto bitmap y manejo de matrices).
