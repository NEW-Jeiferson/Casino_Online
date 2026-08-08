# Módulo Utils (Bezier y Bresenham)

## 1. Resumen del módulo
Este módulo proporciona utilidades matemáticas y algoritmos fundamentales para gráficos generados desde cero. Consta de dos subcomponentes: utilidades para curvas de Bezier (`bezier.h`, `bezier.c`), orientadas a la creación de formas redondeadas y curvas suaves (como el plato de la ruleta), y el algoritmo de Bresenham (`bresenham.h`, `bresenham.c`), utilizado para la rasterización eficiente y dibujo de líneas rectas píxel por píxel en pantalla.

## 2. Estructuras de datos clave

### `Punto3D`
- **Descripción**: Estructura básica que representa una posición exacta en el mundo 3D usando coordenadas `x`, `y`, `z` de tipo `float`.
- **Uso**: Es fundamental en las funciones de cálculo de las curvas de Bezier para definir los puntos de control de las curvas (los puntos guía) y el resultado, que es el punto evaluado en la curva a lo largo del tiempo o el progreso.

## 3. Funciones públicas

### `evaluar_bezier_cuadratica(Punto3D p0, Punto3D p1, Punto3D p2, float t)`
- **¿Para qué funciona/sirve?**: Calcula un punto exacto en una curva de Bezier cuadrática (guiada por 3 puntos: inicio `p0`, control `p1`, y fin `p2`). La variable `t` representa el avance en la curva desde `0.0` (inicio) hasta `1.0` (final). Sirve para crear trayectorias y curvas suaves.
- **¿Por qué se implementó?**: 
  El proyecto impone restricciones estrictas al utilizar el estándar C89 en un entorno de gráficos base (OpenGL primitivo). En lugar de depender de librerías modernas de animación o geometría de alto nivel, implementar el cálculo matemático directo de Bezier asegura una total compatibilidad con C89, control minucioso del rendimiento sin añadir dependencias pesadas y permite generar las geometrías redondeadas requeridas (como partes de la ruleta) de forma algorítmica y pura, adaptándose a las necesidades del entorno primitivo.
- **¿Cómo lo hace internamente?**: 
  Aplica la fórmula matemática de la interpolación de la curva de Bezier cuadrática: `P(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2`. Calcula el valor del complemento `u = 1 - t` y evalúa algebraicamente los componentes `x`, `y`, `z` ponderando cada punto de control con base en los polinomios de Bernstein.

### `evaluar_bezier_cubica(Punto3D p0, Punto3D p1, Punto3D p2, Punto3D p3, float t)`
- **¿Para qué funciona/sirve?**: Calcula un punto en una curva de Bezier cúbica (usando 4 puntos guía). Permite modelar formas mucho más complejas y de mayor grado de suavidad en comparación con la versión cuadrática. Igual que la cuadrática, `t` va de `0.0` a `1.0`.
- **¿Por qué se implementó?**: 
  Por las mismas razones que la cuadrática (restricciones de C89 y control sobre la geometría). Al usar matemáticas puras, el desarrollador tiene flexibilidad absoluta para crear superficies más sofisticadas para los modelos 3D y elementos de interfaz de juego sin requerir librerías modernas complejas, logrando una ejecución determinista y ligera que es fácil de portar y compilar bajo los estándares más antiguos.
- **¿Cómo lo hace internamente?**: 
  Emplea la fórmula de la curva de Bezier cúbica: `P(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)t^2*P2 + t^3*P3`. Internamente, calcula `u = 1 - t`, y los cuadrados de `u` y `t` (`uu` y `tt`), para evitar recalcular estas multiplicaciones en cada eje. Luego se evalúa polinómicamente el resultado independiente en `x`, `y`, y `z`.

### `dibujar_linea_bresenham(int x0, int y0, int x1, int y1)`
- **¿Para qué funciona/sirve?**: Dibuja una línea recta perfecta píxel por píxel en la pantalla conectando el punto inicial `(x0, y0)` con el final `(x1, y1)`.
- **¿Por qué se implementó?**: 
  Aunque OpenGL posee la capacidad nativa de dibujar líneas (`GL_LINES`), muchas veces es útil, pedagógico, o un requerimiento estricto controlar exactamente la rasterización y saber qué píxeles precisos encienden las líneas (por ejemplo, para interfaces personalizadas tipo píxel-art o lógicas estables de nivel inferior). Al implementarlo manualmente respetando C89, se evita el uso de operaciones con coma flotante para dibujar líneas en 2D, resultando en un método matemáticamente óptimo con enteros sin depender de funciones adicionales de rasterizado moderno.
- **¿Cómo lo hace internamente?**: 
  Implementa el clásico algoritmo de Bresenham utilizando solo aritmética entera.
  1. Calcula las diferencias absolutas `dx` y `dy` entre los puntos.
  2. Determina la dirección del avance `sx` y `sy` (1 o -1).
  3. Inicializa una variable de `error` que ayuda a determinar cuándo desplazar la coordenada perpendicular para mantener la trayectoria recta óptima.
  4. Inicia un bloque `glBegin(GL_POINTS)`.
  5. Entra en un ciclo infinito donde en cada iteración se enciende el píxel actual usando `glVertex2i(x0, y0)`, y se evalúa si ya se alcanzó el destino. Si no, se ajusta el error y se avanza en `x` o `y`, o ambos, para el siguiente píxel, hasta llegar al destino donde el bucle termina, finalizando con `glEnd()`.

## 4. Dependencias
- Las utilidades de Bezier (`bezier.h`, `bezier.c`) son componentes matemáticos puros y **no tienen dependencias externas**.
- El algoritmo de Bresenham (`bresenham.h`, `bresenham.c`) depende de:
  - `<GL/glut.h>`: para realizar el renderizado real de los píxeles en pantalla a través de llamadas de OpenGL (`glBegin`, `glVertex2i`, `glEnd`).
  - `<stdlib.h>`: para utilizar la función matemática `abs()` requerida para obtener distancias absolutas con números enteros.
