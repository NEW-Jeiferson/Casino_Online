# Documentación: Módulos `ui/tablero_apuestas` y `ruleta/mouse_picking`

## 1. Resumen del módulo
Estos módulos en conjunto gestionan la interactividad y la representación gráfica del tablero de apuestas de la ruleta. Por un lado, `ui/tablero_apuestas` se encarga de dibujar el clásico tapete verde de la ruleta mediante primitivas 2D de OpenGL colocadas en un espacio tridimensional. Por otro, `ruleta/mouse_picking` implementa la lógica espacial (raycasting) que permite al usuario interactuar con la mesa en 3D usando el cursor 2D, traduciendo coordenadas de pantalla a coordenadas lógicas sobre el tablero virtual para iluminar zonas bajo el cursor (hover) y efectuar las apuestas.

## 2. Estructuras de datos clave
El comportamiento está fuertemente acoplado a las definiciones de la lógica del juego (provenientes de `core/estado_juego.h`):
- `Apuesta`: Estructura para registrar una apuesta activa, incluyendo el `tipo` de apuesta, su `valor` o identificador y el `monto` de dinero asignado.
- `TipoApuesta`: Enumeración de las formas de jugar en la ruleta (`APUESTA_NUMERO`, `APUESTA_DOCENA`, `APUESTA_MITAD`, `APUESTA_PAR_IMPAR`, `APUESTA_COLOR`).
- `ColorRuleta`: Enums para colores lógicos de casillas (`COLOR_ROJO`, `COLOR_NEGRO`, etc).
- `celda_hover_col` y `celda_hover_fila`: Variables globales del módulo usadas para guardar y pintar la casilla donde actualmente descansa el ratón.

## 3. Funciones públicas (Detalles)

### `dibujar_tablero_apuestas`
- **¿Para qué funciona/sirve?** Renderiza visualmente todo el tapete de apuestas, incluyendo las celdas numéricas, áreas de apuestas especiales, líneas divisorias, etiquetas, las fichas colocadas sobre la mesa y un área resaltada (efecto *hover*).
- **¿Por qué se diseñó de esta manera?** Se optó por utilizar una superposición de llamadas a primitivas 2D, a las cuales se les aplica una serie de traslaciones, rotaciones y escalados (transformaciones matriciales en OpenGL clásico). Este enfoque permite usar un sistema coordenado simple en píxeles para el diseño "plano" del tapete y proyectarlo fácilmente al 3D. Además, se deshabilita la iluminación (`glDisable(GL_LIGHTING)`) para que los colores del tablero se vean consistentes independientemente de las luces del escenario.
- **¿Cómo lo hace internamente?** Funciona de forma secuencial llamando a funciones estáticas que simulan capas: primero el fondo de cada casilla, luego el área de docenas e inferior; las trazas de líneas y texto; los discos correspondientes a las fichas apiladas según arreglos de entrada; y por último dibuja un rectángulo amarillo semitransparente que resalta la casilla actual donde está el cursor, si la hay.

### `obtener_celda_en_punto`
- **¿Para qué funciona/sirve?** Convierte una coordenada 3D en la mesa `(x, z)` a índices bidimensionales de fila y columna correspondientes al grid de números del 1 al 36. Devuelve un booleano (0 o 1) si la coordenada cayó en ese grid.
- **¿Por qué se diseñó de esta manera?** En lugar de hacer colisión por fuerza bruta caja por caja, se diseñó aplicando la matemática inversa al dibujado. Puesto que las celdas son regulares en el tapete, un cálculo aritmético directo determina rápidamente en O(1) los índices lógicos.
- **¿Cómo lo hace internamente?** Invierte el proceso de escala (`ESCALA_TABLERO`) y los offsets iniciales (`-2.9f`, `4.9f`) calculando los píxeles lógicos `u` y `v`. Luego, basándose en el ancho definido (`CELDA_PX`), divide `u` y `v` en rangos enteros y se asegura de que caigan dentro de los límites esperados (`TABLERO_COLUMNAS` y `TABLERO_FILAS`).

### `obtener_numero_en_punto`
- **¿Para qué funciona/sirve?** Valida e indica si un punto espacial en la mesa pertenece específicamente a una apuesta numérica (0 al 36).
- **¿Por qué se diseñó de esta manera?** A diferencia de los números del 1 al 36, el número "0" tiene un tamaño asimétrico en la interfaz (es un rectángulo vertical largo a la izquierda). Aislar esto garantiza un diseño limpio de cálculo aritmético sobre el tapete común y permite verificar el cero con una condición física distinta.
- **¿Cómo lo hace internamente?** Primero calcula los `u` y `v` normalizados y revisa si corresponden al rectángulo del cero. Si es falso, delega los valores a `obtener_celda_en_punto()`. Si esta última acierta, mapea matemáticamente la fila y columna al valor del 1 al 36 con la expresión `(col * 3 + fila + 1)`.

### `obtener_zona_especial_en_punto`
- **¿Para qué funciona/sirve?** Revisa si un punto tridimensional corresponde a las cajas de juego especiales del tapete, es decir, apuestas a docenas, mitades, paridad o color.
- **¿Por qué se diseñó de esta manera?** Las áreas de apuestas especiales están construidas como una extensión física debajo del grid de números. Mantener este cálculo en una función separada centraliza la lógica de selección de zonas de apuesta sin entrelazarla con los números.
- **¿Cómo lo hace internamente?** Normaliza las coordenadas al plano 2D. Verifica si el punto pertenece al rango vertical "medio" (`Y_MID` a `Y_TOP`) correspondiente a docenas, y según el `x` (ancho modificado de las docenas) determina a qué tercio pertenece. Si está en el rango vertical "inferior", hace una distinción entre las seis cajas (1-18, pares, rojo, negro, impares, 19-36), actualizando los punteros de salida `tipo_out` y `valor_out` con los enums respectivos y sus identificadores numéricos.

### `fijar_celda_hover`
- **¿Para qué funciona/sirve?** Actualiza las variables globales que registran qué casilla o zona señala el cursor para que pueda mostrarse iluminada en pantalla.
- **¿Por qué se diseñó de esta manera?** Separa el manejo de eventos (input del ratón) de la lógica de renderizado. Sirve como puente de comunicación rápido; un manejador pasivo almacena los resultados de la detección (mouse picking) y luego `dibujar_hover()` consume estos datos temporalmente.
- **¿Cómo lo hace internamente?** Sobrescribe el valor de las dos variables globales locales `celda_hover_col` y `celda_hover_fila` con los parámetros dados.

### `obtener_punto_clic_en_mesa`
- **¿Para qué funciona/sirve?** Es la piedra angular de la interacción espacial; convierte el click 2D recibido por la ventana de GLUT a un punto cartesiano tridimensional `(x, 0, z)` interceptado en la "mesa".
- **¿Por qué se diseñó de esta manera (raycasting/picking 3D a 2D en OpenGL legacy)?** En versiones obsoletas y tradicionales de OpenGL no existen sistemas de física nativos. Para saber en qué geometría tridimensional impacta un ratón, se usa "Mouse Picking" basado en la técnica de Raycasting. Un rayo nace desde la cámara simulada, pasando por las coordenadas de la ventana, hasta el horizonte. Dado que se asume que el tapete de apuestas se encuentra horizontalmente en la elevación fija de `Y = 0`, basta realizar la intersección del rayo con ese plano usando álgebra básica sin requerir mallas poligonales colisionables complejas.
- **¿Cómo lo hace internamente?** 
  1. Obtiene de OpenGL el *viewport* y las matrices actuales de ModelView y Proyección.
  2. Ajusta la coordenada `Y` del ratón (resta de arriba a abajo en GLUT contra el sistema de abajo a arriba de OpenGL).
  3. Llama a `gluUnProject` dos veces: una con *z* = 0.0 (near, inicio del rayo) y otra con *z* = 1.0 (far, final del rayo).
  4. Calcula el vector director restando los dos puntos obtenidos `(far - near)`.
  5. Encuentra la intersección del vector con el plano `Y = 0` igualando a cero el eje vertical y despejando el factor `t` de alcance de línea (`t = -near_y / dir_y`). Finalmente, sustituye `t` en los ejes X y Z, entregando esos dos valores flotantes por medio de punteros.

## 4. Dependencias
- Gráficos y Eventos: `GL/glut.h`.
- Matemáticas y Utilidades C: `<stdio.h>`, `<string.h>`, `<math.h>`.
- Librerías internas: 
  - `../utils/bresenham.h`: Utilizada para el renderizado eficiente de las líneas blancas del tapete.
  - `../core/estado_juego.h`: Da soporte a las estructuras relativas a apuestas y enum de tipos, crucial para identificar los montos, colores y fichas que se colocan sobre la tabla.
