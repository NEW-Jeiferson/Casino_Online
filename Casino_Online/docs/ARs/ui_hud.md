# Documentación del Módulo: HUD (Heads-Up Display)

## 1. Resumen del módulo
Los módulos de interfaz de usuario (`hud`, `hud_dados`, `hud_tragamonedas`) se encargan de renderizar componentes 2D superpuestos a la vista 3D del casino. Al tratarse de un "Serious Game" orientado a la concientización sobre la ludopatía, estos módulos no solo presentan información del estado del juego (saldo, apuestas, resultados), sino que incorporan fuertemente el componente de retroalimentación de riesgo y mensajes educativos para desmitificar falacias del jugador.

## 2. Estructuras de datos clave
La mayoría de los datos son consumidos desde las estructuras centrales como `Jugador` y `EstadoTragamonedas`. Sin embargo, a nivel de UI se define:

*   **`ZonaControlTragamonedas` (Enum)**: Definida en `hud_tragamonedas.h`. Identifica las regiones interactivas de la barra de control inferior (SPIN, BET_MENOS, BET_MAS, NINGUNA) para facilitar el mapeo de los clicks del mouse.

## 3. Funciones públicas (Detailed)

### `dibujar_hud`
*   **¿Para qué funciona/sirve?**: Renderiza la interfaz principal del jugador (ruleta), mostrando saldo, total apostado, préstamos activos, ficha seleccionada, cantidad de apuestas y el nivel de riesgo de la sesión. También muestra mensajes educativos durante los giros.
*   **¿Por qué se diseñó de esta manera?**: 
    *   *Retroalimentación visual de riesgo/deuda*: Los colores de los textos y barras cambian dinámicamente. El saldo cambia a un gradiente de verde a negro si cae bajo cierto umbral; el medidor de riesgo ambiental advierte desde "BAJO" en verde hasta "CONDUCTA COMPULSIVA" en rojo si el jugador acumula deudas y apuestas aceleradas.
    *   *Decisiones de diseño*: Emplea "glassmorphism" (fondos semi-transparentes) y bordes para modernizar la apariencia, albergando la información en un recuadro. 
    *   *Restricciones C89*: No hay soporte nativo en OpenGL fijo para texturas avanzadas en textos fácilmente, por lo que se utiliza `glutBitmapCharacter` para trazar los textos iterando por cada carácter.
*   **¿Cómo lo hace internamente?**: Altera temporalmente las matrices (usando `glPushMatrix` y cambiando a `glOrtho` 2D). Desactiva iluminación y prueba de profundidad, activa mezcla alpha (`GL_BLEND`) para dibujar polígonos semitransparentes. Tras calcular colores y textos (utilizando `sprintf_s`), dibuja los elementos y, finalmente, restaura el modo de proyección 3D y estados de OpenGL originales.

### `dibujar_hud_dados`
*   **¿Para qué funciona/sirve?**: Muestra un panel en la parte superior de la pantalla indicando las apuestas disponibles y sus respectivos pagos para el juego de dados (Craps).
*   **¿Por qué se diseñó de esta manera?**: El diseño consiste en un panel superior centrado y compacto para evitar distraer del tablero de dados 3D. Centraliza la información útil de multiplicadores de apuesta para guiar la decisión, limitando la UI a lo indispensable.
*   **¿Cómo lo hace internamente?**: Realiza la misma transición temporal a ortogonal 2D. Dibuja un rectángulo sombreado con borde y utiliza funciones de texto 2D fijadas en posiciones relativas a las medidas de ventana, centrando los contenidos horizontalmente.

### `dibujar_mensaje_giro_dados` y `dibujar_mensaje_giro_tragamonedas`
*   **¿Para qué funciona/sirve?**: Muestran un banner flotante temporal que expone la probabilidad real y desmiente falacias o invita a terminar la sesión mientras la simulación corre en el fondo.
*   **¿Por qué se diseñó de esta manera?**: Es el pilar del "Serious Game". Aprovecha los tiempos muertos y de alta dopamina (giros) para introducir cuestionamientos éticos y matemáticos (ej: "Los pagos altos son ilusiones"). Se diseñó truncando mensajes largos en múltiples líneas ya que `glutBitmapCharacter` no procesa saltos de línea automáticamente. 
*   **¿Cómo lo hace internamente?**: Extrae el mensaje de un arreglo estático usando el `índice`. Busca el separador `\n` particionando el arreglo (utilizando `strncpy_s` por seguridad), calcula los anchos máximos de los strings de GLUT y dibuja el banner de fondo semi-transparente en el centro exacto de la pantalla (agregando márgenes dinámicos para no pisar las interfaces laterales) y luego dibuja el texto.

### `dibujar_barra_control_2d`
*   **¿Para qué funciona/sirve?**: Dibuja el panel principal de interacción (Saldo, Modificador de Apuesta, Botón de Giro y Ganancias) para el módulo de tragamonedas.
*   **¿Por qué se diseñó de esta manera?**: 
    *   *Decisión visual*: Se rediseñó de un sistema expandido proporcionalmente (que quedaba mal en resoluciones grandes) a un panel compacto de 680px fijos centrado al fondo, con un diseño "octogonal" pulido simulando acabados físicos. Se utilizan botones con efectos "glossy".
    *   *Restricciones técnicas*: En lugar de dibujar este panel proyectado como textura 3D sobre la máquina (lo que requeriría "render to texture", fuera del scope C89 de este proyecto), se implementó como el _overlay_ 2D más inmersivo posible que emula la botonera física.
*   **¿Cómo lo hace internamente?**: Llama a `calcular_zonas_barra_control` para aislar las variables posicionales de pantalla. Aplica interpolaciones matemáticas (degradados) recorriendo 8 vértices (`dibujar_panel_2d_biselado`) y llama repetidamente a funciones de polígonos circulares translúcidos descentrados (`dibujar_boton_circular_glossy`) para generar brillos/luces asimétricas en los botones.

### `obtener_zona_control_2d`
*   **¿Para qué funciona/sirve?**: Realiza el mapeo inverso de interacciones (hit-testing). Identifica en qué componente del panel de tragamonedas se ha hecho click.
*   **¿Por qué se diseñó de esta manera?**: Para disociar la entrada de teclado tradicional e introducir una interacción más natural con botones visuales pseudo-físicos requeridos en la interfaz. GLUT tiene distintos sistemas de coordenadas de los de OpenGL, por lo que requirió esta traducción explícita.
*   **¿Cómo lo hace internamente?**: Recibe las coordenadas X/Y del ratón provistas por la interrupción de clic de GLUT. Como la `Y` en GLUT incrementa hacia abajo y en proyección OpenGL incrementa hacia arriba, invierte el eje Y. Luego vuelve a calcular los centros de las botoneras y utiliza el teorema de Pitágoras (`dx*dx + dy*dy <= r*r`) para validar matemáticamente la colisión con los botones circulares, retornando el enum correspondiente.

## 4. Dependencias
*   `GL/glut.h` o dependencias base de OpenGL para el dibujado (primitivas, `glOrtho`, colores, texto bitmap).
*   `math.h`: Utilizado fuertemente para calcular las curvas de los círculos (seno/coseno) en la UI y colisiones (Pitágoras).
*   Librerías estándar (`stdio.h`, `string.h`): Utilizadas para manipular textos (como conversiones de float a string y separaciones por salto de línea).
*   **Módulos Core del Proyecto**:
    *   `../core/jugador.h` (Utilizado por la interfaz de ruleta para el saldo e historial)
    *   `../core/estado_juego.h` / `../tragamonedas/tragamonedas_animacion.h` (Para mostrar si se está girando)
    *   `../dados/dados_logica.h` (Estructuras de estado en HUD Dados)
