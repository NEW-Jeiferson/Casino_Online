# Trazabilidad de Temas del Curso: Computación Gráfica

Este documento presenta un análisis de extrema rigurosidad académica sobre los conceptos fundamentales abordados durante el curso de Computación Gráfica, detallando exhaustivamente su aplicación práctica y las justificaciones teóricas detrás de cada decisión arquitectónica en el proyecto **Casino Online**.

## 1. Fundamentos de C y Orden de Ejecución

* **Aplicación y Justificación Teórica:** El desarrollo se restringió rigurosamente al estándar C89. La elección de C puro sobre lenguajes de más alto nivel (con recolección de basura) se debe a la necesidad de mantener un control determinista sobre la memoria y minimizar la latencia en el ciclo de renderizado. Se implementó un patrón de inyección de dependencias mediante aritmética de punteros (por ejemplo, al pasar el estado mediante `&partida.jugador`), asegurando que las modificaciones al modelo de datos no generen copias redundantes en memoria.
* **Paradigma de Estado:** Se respetó estrictamente la naturaleza secuencial de la máquina de estados (*State Machine*) de OpenGL. Por diseño, los atributos del contexto (color, normales, coordenadas de textura) deben definirse antes de someter un vértice a la tubería gráfica.
* **Evidencia en Código:** Las instrucciones como `glColor3f(...)` siempre preceden a la emisión del vértice `glVertex3f(...)` dentro del bloque delimitado por `glBegin(...)` y `glEnd()`.

## 2. Contexto de OpenGL y Pipeline de Renderizado

* **Aplicación y Justificación Teórica:** La inicialización del contexto gráfico se delegó a GLUT, definiendo un espacio de trabajo con un búfer de profundidad y color RGB. Se optó explícitamente por utilizar la técnica de *Double Buffering* (`GLUT_DOUBLE`) en contraposición al búfer simple (`GLUT_SINGLE`).
* **¿Por qué Double Buffering?** Si se utilizara un solo búfer, el usuario percibiría el borrado y redibujado progresivo de los polígonos durante cada fotograma, lo que causa un efecto indeseable de *tearing* y parpadeo visual (*flickering*). El uso de un búfer trasero (donde se construye la escena de forma invisible) garantiza que la pantalla solo se actualice de manera atómica una vez que el fotograma está completamente renderizado, intercambiando los punteros de memoria.
* **Evidencia en Código:** La bandera `glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)` en la inicialización, seguida sistemáticamente por la llamada a `glutSwapBuffers()` al finalizar el callback de renderizado en `main.c`.

## 3. Callbacks y Primitivas Geométricas

* **Aplicación y Justificación Teórica:** El proyecto adopta una arquitectura puramente orientada a eventos (*Event-Driven Architecture*) a través de punteros a funciones (Callbacks) proporcionados por el sistema de ventanas.
* **¿Por qué Callbacks sobre un bucle principal activo (Polling)?** Un enfoque de *polling* consumiría el 100% de los ciclos de la CPU esperando entradas. Delegar esta responsabilidad al OS mediante `glutDisplayFunc`, `glutKeyboardFunc` y `glutIdleFunc` permite una utilización eficiente del procesador, disparando el recálculo de la física y la geometría solo cuando ocurre un evento o ha pasado el delta de tiempo necesario para el siguiente fotograma.
* **Primitivas Geométricas:** En lugar de importar mallas complejas (archivos `.obj`), se modelaron los elementos desde cero utilizando primitivas geométricas elementales, como `GL_QUADS` para los planos y `GL_TRIANGLE_FAN` para aproximar círculos (como los puntos numéricos de los dados), garantizando una topología óptima y controlada matemáticamente.

## 4. Proyección Ortogonal vs. Perspectiva

* **Aplicación y Justificación Teórica:** Se implementó un modelo de cámara dual dinámico. La escena tridimensional opera bajo una transformación de volumen de vista de tronco de pirámide (*Frustum*), calculada mediante la matriz de perspectiva. Sin embargo, para la Interfaz de Usuario (HUD, balances, notificaciones), el pipeline transiciona a una proyección ortogonal 2D.
* **¿Por qué alternar las proyecciones?** La proyección en perspectiva distorsiona los objetos según su distancia a la cámara (escorzo), lo cual es ideal para el volumen de los dados y la ruleta. Sin embargo, el texto y los menús del HUD deben evadir esta distorsión y permanecer invariables respecto a la profundidad. La matriz ortográfica desactiva el efecto del eje Z temporalmente, mapeando directamente coordenadas del espacio de mundo al espacio de la pantalla.
* **Evidencia en Código:** Las transiciones de matriz usando `glMatrixMode(GL_PROJECTION)`. Para el entorno 3D, se invoca `gluPerspective(45.0, aspect, 0.1, 100.0)`. Para superponer UI, se resetea la matriz y se usa `glOrtho(0, width, height, 0, -1, 1)`.

## 5. Transformaciones Geométricas

* **Aplicación y Justificación Teórica:** Se realizaron extensas manipulaciones del espacio euclidiano utilizando transformaciones afines: traslaciones, rotaciones y escalados, para lograr la cinemática de los objetos móviles.
* **Naturaleza Multiplicativa:** Dado que OpenGL aplica post-multiplicación de matrices a los vértices (multiplicación por la derecha), el orden de declaración en el código debe ser **inverso** al orden lógico en que se desean aplicar.
* **¿Por qué esta rigidez?** Para que un rodillo gire sobre su propio centro, primero debe rotarse y luego trasladarse a su posición final en el mundo. Si el orden fuera inverso, el rodillo rotaría alrededor del origen del mundo, describiendo una órbita.
* **Evidencia en Código:** Uso de `glTranslatef(pos_x, pos_y, pos_z)` seguido de `glRotatef(angulo, 1.0, 0.0, 0.0)` en las funciones de actualización de animaciones.

## 6. Pila de Matrices (Push/Pop) y Jerarquías

* **Aplicación y Justificación Teórica:** Para modelar entidades compuestas (como la máquina tragamonedas que contiene rodillos, o la mesa que sostiene los dados), se estructuró un grafo de escena (*Scene Graph*) implícito mediante la manipulación de la pila de matrices del estado de OpenGL.
* **¿Por qué la Pila de Matrices?** Al llamar a `glPushMatrix()`, se guarda el estado actual de la transformación espacial. Cualquier rotación o traslación subsecuente afecta únicamente al subnodo actual (por ejemplo, el rodillo girando). Al finalizar de dibujar esa parte, `glPopMatrix()` restaura el sistema de coordenadas de la entidad padre, evitando que las transformaciones de un elemento "contaminen" espacialmente a los demás.
* **Evidencia en Código:** El uso simétrico de `glPushMatrix()` y `glPopMatrix()` al aislar cada entidad renderizable (dados, fichas, rodillos).

## 7. Color RGBA y Blending

* **Aplicación y Justificación Teórica:** Más allá del sombreado opaco, se integró el concepto de alfa compositing (canal de transparencia) para la simulación de materiales translúcidos y transiciones suaves en la UI (*fade-ins/outs*).
* **Ecuación de Mezcla:** Se seleccionó el algoritmo de mezcla estándar de Porter-Duff, usando la ecuación de fuente contra el inverso del alfa de fuente.
* **¿Por qué esta configuración?** A diferencia de la mezcla aditiva (que suma colores saturando el brillo y es útil para fuego o partículas), la ecuación `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA` interpola correctamente el color del fragmento entrante con el color existente en el *framebuffer*, basándose en el porcentaje de opacidad. Esto resulta crucial para superponer paneles semitransparentes en la pantalla de Game Over sin perder legibilidad.
* **Evidencia en Código:** Inicialización mediante `glEnable(GL_BLEND)` y la definición `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` junto al uso de `glColor4f(r, g, b, alpha)`.

## 8. Modelo de Iluminación de Phong y Normales

* **Aplicación y Justificación Teórica:** Para romper la apariencia plana, se implementó el modelo de sombreado de Phong, que calcula la iluminación local interpolando las normales por cada píxel para producir reflejos especulares suaves.
* **¿Por qué Phong?** A diferencia del sombreado *Flat* (que evalúa la luz por cara) o *Gouraud* (que interpola intensidades de luz por vértice pero pierde reflejos agudos), Phong permite la simulación convincente de superficies de curvatura continua, como los bordes curvos del plato de la ruleta.
* **El Rol de las Normales:** Para que las matemáticas del producto punto del cálculo lumínico funcionen, se generaron analíticamente vectores normales perpendiculares a cada cara.
* **Evidencia en Código:** Activación de sombreado suave con `glShadeModel(GL_SMOOTH)` y la asignación paramétrica de `glNormal3f(nx, ny, nz)` durante la generación de geometría rotacional.

## 9. Fuentes de Luz y Materiales

* **Aplicación y Justificación Teórica:** La arquitectura desacopla estrictamente los componentes lumínicos del entorno (las luces) de las propiedades de la superficie de los objetos (materiales).
* **La Ecuación de Reflactancia:** La luz principal (`GL_LIGHT0`) actúa como una fuente posicional. Los objetos se renderizan respondiendo al modelo empírico de iluminación mediante la definición de sus coeficientes de reflexión difusa y especular.
* **¿Por qué definir Materiales?** El paño de la mesa (fieltro) requiere una absorción casi total de la luz (componente difusa alta, nula componente especular). Por otro lado, los detalles metálicos de la tragamonedas requieren una alta concentración especular (*shininess*) para imitar el cromo.
* **Evidencia en Código:** Activación global con `glEnable(GL_LIGHTING)`, definición de la luz con `glLightfv(GL_LIGHT0, GL_POSITION, pos)`, y definición de propiedades superficiales con `glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color)`.

## 10. Mapeo de Texturas y Coordenadas UV

* **Aplicación y Justificación Teórica:** Para incrementar la densidad de detalle visual sin saturar el procesamiento de vértices geométricos, se implementó un sistema de mapeo de texturas.
* **El Problema del Mapeo:** Cargar una imagen bidimensional plana sobre un modelo tridimensional curvo (como los rodillos cilíndricos de la máquina) requiere una parametrización de Coordenadas UV.
* **¿Por qué usar stb_image en lugar de dependencias externas pesadas?** Se seleccionó la biblioteca de un solo encabezado `stb_image.h` porque elimina la necesidad de enlazadores complejos (como los que requiere `libpng`), manteniendo la portabilidad extrema del proyecto escrito en C.
* **Evidencia en Código:** El uso de `glTexImage2D(...)` para transferir la matriz de píxeles a la VRAM, y la asignación interpolada de vértices mediante `glTexCoord2f(u, v)`.

## 11. Rasterización: Algoritmo de Bresenham

* **Aplicación y Justificación Teórica:** La cuadrícula de apuestas de la mesa se dibuja de forma procedural, implementando matemáticamente el algoritmo del punto medio (Bresenham) para la generación de líneas.
* **¿Por qué Bresenham en lugar de mapear una textura estática de la tabla?** Las texturas bidimensionales sufren de filtrado bilineal (*aliasing* y borrosidad al realizar *zoom-in*) o requieren costosos mapas MIP para verse correctamente. Al rasterizar el tablero vectorial calculando el error acumulado píxel a píxel, nos aseguramos que las líneas delimitadoras sean perfectamente nítidas independientemente de la resolución de la ventana o del factor de escala del tapete.
* **Evidencia en Código:** La implementación de los acumuladores de error con las variables del algoritmo: `dx = abs(x1 - x0)`, `dy = abs(y1 - y0)`, evaluando si el píxel incrementa en $X$ o $Y$ reduciendo el uso de aritmética de punto flotante en el trazado de la interfaz.

## 12. Back-face Culling y Z-Buffer (Optimización)

* **Aplicación y Justificación Teórica:** Como estrategia de optimización para reducir la carga en el rasterizador (*fill rate*), se implementaron dos técnicas fundamentales de exclusión visual.
* **Z-Buffer (Depth Test):** Impide que objetos lejanos sobrescriban a los objetos cercanos en la pantalla. Esto soluciona de forma intrínseca el Problema del Pintor, asegurando la oclusión correcta de un dado apoyado sobre la mesa.
* **¿Por qué Back-face Culling?** Los modelos sólidos (dados, cilindros) están compuestos por polígonos, la mitad de los cuales siempre miran hacia el interior de la malla geométrica y nunca serán visibles. Descartando polígonos cuyo orden de enrollamiento (*winding order*) indique que están de espaldas a la cámara antes del paso de fragmento, casi se duplica el rendimiento computacional.
* **Evidencia en Código:** `glEnable(GL_DEPTH_TEST)`, la limpieza por fotograma `glClear(GL_DEPTH_BUFFER_BIT)`, seguido de la activación de `glEnable(GL_CULL_FACE)` y `glCullFace(GL_BACK)`.

## 13. Modelado: Curvas de Bézier

* **Aplicación y Justificación Teórica:** En lugar de depender de primitivas de revolución estáticas (como `gluSphere` o `gluCylinder`), se desarrolló una arquitectura matemática orientada a interpolaciones polinómicas de grado cúbico (Curvas de Bézier).
* **¿Por qué generar formas mediante polinomios de Bernstein?** 
   1. **Geometría Paramétrica:** Permite definir la silueta curvada y elegante de los platos concéntricos de la ruleta rusa con tan solo 4 puntos de control, escalando algorítmicamente el nivel de detalle sin consumir memoria extra.
   2. **Cinemática Física:** La curva de Bézier se adaptó al dominio del tiempo ($T$) para simular un *easing* o interpolación no lineal suave en el frenado mecánico de los rodillos, dotando a la animación de inercia y fricción en vez de un movimiento lineal robótico.
* **Evidencia en Código:** La implementación en un módulo dedicado (`utils/bezier.c`) y llamadas continuas a las funciones matemáticas como `evaluateBezier(t, p0, p1, p2, p3)`.

## 14. Navegación y Detección de Colisiones

* **Aplicación y Justificación Teórica:** La interacción fundamental del usuario se gestionó a través de algoritmos espaciales para el motor de captura de interacciones (*Mouse Picking*) y el confinamiento de objetos.
* **¿Por qué usar AABB en lugar de Ray Casting de malla completa?** Proyectar un rayo a través de la matriz inversa y probar la intersección triángulo por triángulo es un proceso de alto coste temporal $O(n)$. Para este caso de estudio (fichas rectangulares, botones UI, y delimitaciones del tablero), se implementaron Cajas Delimitadoras Alineadas a los Ejes (AABB - *Axis-Aligned Bounding Boxes*). Esta solución matemática simple y extremadamente rápida determina solapamientos comprobando límites numéricos mínimos y máximos en las coordenadas proyectadas.
* **Evidencia en Código:** Las evaluaciones lógicas empleadas en la interfaz de usuario: `checkCollisionAABB(mouse_x, mouse_y, box.x_min, box.x_max, box.y_min, box.y_max)`, garantizando que el usuario apunte e interactúe sin sobrecarga algorítmica inútil.
