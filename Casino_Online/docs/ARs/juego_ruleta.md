# Documentación del Módulo: Juego Ruleta

## 1. Resumen del módulo
El módulo `ruleta` es el núcleo visual y físico del simulador de ruleta del casino. Su objetivo es gestionar la generación de la geometría tridimensional tanto de la mesa como de la rueda, así como la simulación y animación del movimiento de la bolita cayendo en la casilla ganadora. Debido a que el proyecto está escrito en C89 y utiliza OpenGL clásico (Fixed Function Pipeline) sin depender de un motor físico externo, este módulo resuelve de manera matemática y artesanal:
- La construcción de mallas 3D complejas (superficies de revolución a partir de curvas de Bézier).
- La interpolación de movimiento de la bolita aplicando un freno simulado mediante curvas de amortiguación (easing).
- El cálculo preciso de iluminación y alturas para posicionar elementos sobre las superficies curvas.

## 2. Estructuras de datos clave

### `EstadoBolita`
Esta estructura, definida en `ruleta_animacion.h`, mantiene el estado completo de la bolita durante su trayectoria de giro:
```c
typedef struct {
    float angulo_actual;      /* El ángulo actual de la bolita en su órbita */
    float velocidad;          /* Velocidad instantánea en grados por segundo */
    float velocidad_inicial;  /* Velocidad al momento del lanzamiento */
    float duracion_total;     /* Tiempo calculado que durará toda la animación */
    int   girando;            /* Bandera booleana (1 o 0) que indica si está en movimiento */
    float tiempo_transcurrido;/* Segundos transcurridos desde que empezó el giro */
} EstadoBolita;
```
Permite desacoplar el estado lógico temporal de la representación gráfica pura, haciendo fácil el avance con `delta_tiempo`.

## 3. Funciones públicas

A continuación se detallan las funciones públicas del módulo, separadas por su subdominio.

### Geometría (`ruleta_geometria.h`)

#### `generar_perfil_bezier_rueda(void)`
- **¿Para qué sirve?**: Genera los puntos bidimensionales del perfil radial (la silueta desde el centro hacia afuera) del plato de la rueda.
- **¿Por qué se diseñó así?**: Para crear una superficie cóncava realista para la ruleta sin necesidad de importar modelos 3D externos (OBJ, etc). Las curvas de Bézier permiten formas suaves parametrizables usando poca memoria.
- **¿Cómo lo hace internamente?**: Define cuatro puntos de control fijos que representan el hundimiento del plato hacia el centro. Utiliza las utilidades del módulo `bezier` para evaluar segmentos a lo largo de este perfil y almacena las coordenadas resultantes (X=radio, Y=altura) en un arreglo estático `perfil_puntos`.

#### `construir_malla_rueda(void)`
- **¿Para qué sirve?**: Construye los vértices 3D y normales definitivos de la superficie del plato de la ruleta.
- **¿Por qué se diseñó así?**: OpenGL clásico requiere que las normales a las superficies sean proveídas manualmente para que la iluminación calcule bien los reflejos. Como no hay shaders, esto debe precalcularse eficientemente para luego solo iterar al dibujar.
- **¿Cómo lo hace internamente?**: Toma los puntos del perfil radial 2D calculado previamente y los revoluciona en 360 grados usando funciones trigonométricas. Por cada vértice tridimensional generado, calcula el vector tangente al perfil y el tangente al círculo de revolución. Luego, usa el producto cruz de ambos para obtener y normalizar la normal de esa cara, guardando resultados en matrices estáticas 2D.

#### `altura_superficie_en_radio(float radio)`
- **¿Para qué sirve?**: Devuelve la altura `Y` exacta de la superficie de la ruleta dado una distancia `radio` desde el centro.
- **¿Por qué se diseñó así?**: Es fundamental para que la bolita pueda orbitar a la altura correcta sin atravesar la malla 3D ni flotar en el aire.
- **¿Cómo lo hace internamente?**: Recorre el arreglo de puntos del perfil evaluado y busca el segmento en el que encaja el parámetro `radio`. Una vez encontrado el segmento entre dos puntos, aplica interpolación lineal para averiguar el valor exacto de `Y`.

#### Funciones de Renderizado (`dibujar_mesa`, `dibujar_rueda`, `dibujar_vidrio_protector`, `dibujar_pista_numerada`, `dibujar_emblema_central`)
- **¿Para qué sirven?**: Representan visualmente cada subcomponente de la ruleta y de la mesa en el mundo tridimensional.
- **¿Por qué se diseñaron así?**: Separar la representación gráfica permite aplicar de manera limpia los distintos materiales, configuraciones de opacidad (blending) y tipos de sombreado necesarios para cada parte sin crear dependencias cruzadas visuales.
- **¿Cómo lo hacen internamente?**: Configuran matrices (`glPushMatrix`, `glTranslatef`, `glRotatef`), aplican propiedades de material llamando al sistema de `materiales` (como roble o dorado) y finalmente utilizan las primitivas de OpenGL (`GL_QUADS`, `GL_TRIANGLE_STRIP`) alimentando los vértices precalculados. Especial mención a `dibujar_pista_numerada`, que hace uso de `glutStrokeCharacter` escalado a la medida justa para dibujar los números en cada casilla según la enumeración de la ruleta europea.

### Animación (`ruleta_animacion.h`)

#### `inicializar_bolita(EstadoBolita* bolita)`
- **¿Para qué sirve?**: Establece o reinicia los parámetros de animación a su estado base de inactividad.
- **¿Por qué se diseñó así?**: En C89 la inicialización no es implícita. Esto permite resetear el estado rápidamente para una nueva tirada de ruleta garantizando limpieza.
- **¿Cómo lo hace internamente?**: Pone en cero la posición, velocidades y contadores de tiempo, desactivando la bandera `girando`.

#### `iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float angulo_sector_centro, int vueltas_extra)`
- **¿Para qué sirve?**: Inicia la animación del lanzamiento, configurándola para que termine su curso descansando sobre el ángulo del número ganador pre-seleccionado lógicamente por el juego.
- **¿Por qué se diseñó así?**: No es una simulación física real ciega; el motor del juego dictamina en qué número cae. Por tanto, el frenado debe orquestarse matemáticamente hacia atrás (resolución analítica). Además incluye topes para impedir que los giros sean antinaturalmente rápidos o lentos.
- **¿Cómo lo hace internamente?**: Toma el ángulo actual y halla el desplazamiento relativo que debe cubrir la bolita para coincidir con el ángulo meta, sumando las vueltas extra pedidas. Luego, sabiendo que la curva de frenado es predecible, calcula la duración del giro usando la "velocidad típica" y el factor del área bajo la curva del *easing*. Ajusta la velocidad inicial si la duración viola los límites mínimos/máximos.

#### `actualizar_bolita(EstadoBolita* bolita, float delta_tiempo)`
- **¿Para qué sirve?**: Avanza la posición angular de la bolita basado en el tiempo que ha pasado del frame anterior (`delta_tiempo`).
- **¿Por qué se diseñó así?**: La simulación de físicas sin motor especializado se resuelve usando integraciones de Euler simples e impulsos cinemáticos, donde la curva de amortiguación dictamina el decaimiento de la velocidad por la "fricción".
- **¿Cómo lo hace internamente?**: Incrementa el tiempo transcurrido lógico. Evalúa en una curva cúbica de Bézier de la función de Easing el progreso temporal (0 a 1). El valor resultante de "Y" dictamina qué porcentaje de la `velocidad_inicial` sobrevive en este frame. Aplica este progreso angular a `angulo_actual`.

#### `dibujar_bolita(const EstadoBolita* bolita)`
- **¿Para qué sirve?**: Muestra visualmente la bola rotando sobre la pista en el *game loop*.
- **¿Por qué se diseñó así?**: Aisla la matemática gráfica del sistema de dibujado general.
- **¿Cómo lo hace internamente?**: Consulta `altura_superficie_en_radio` para obtener el piso exacto de la órbita. Añade su propio radio como margen. Rota el sistema de coordenadas según `angulo_actual`, traslada sobre el radio de su órbita y renderiza mediante `glutSolidSphere` en un material metálico.

## 4. Dependencias
El funcionamiento de este módulo hace uso estricto de las siguientes librerías y componentes:
- **Librerías estándar C89**: `<math.h>` para matemática trigonométrica, raíz cuadrada y funciones `fmod` y flotantes; `<stdio.h>` y `<string.h>` para generación y medida de los strings de los números de la ruleta.
- **API Gráfica**: `<GL/glut.h>` (incluyendo implícitamente OpenGL `gl.h` y `glu.h` por Fixed Pipeline, para dibujado de mallas, esferas, iluminación, stroke texts, etc).
- **Módulos Internos de Proyecto**:
  - `../render/materiales.h`: Para configurar los materiales como `MATERIAL_METAL`, `MATERIAL_VIDRIO`, `MATERIAL_MADERA_OSCURA`, etc.
  - `../utils/bezier.h`: Proveedor de la función `evaluar_bezier_cubica`, crucial tanto para la geometría como para el easing del frenado.
  - `../core/estado_juego.h`: Para dependencias lógicas, como determinar los colores ganadores y órdenes en `ORDEN_RUEDA_EUROPEA`.
