# Documentación del Módulo: `ui/pantallas`

## 1. Resumen del módulo
El módulo `ui/pantallas` (`src/ui/pantallas.h` y `src/ui/pantallas.c`) es el encargado de renderizar todas las interfaces de usuario 2D superpuestas (overlays) que actúan como transiciones, menús, o puntos de fricción educativa dentro del flujo del "Serious Game". Estas pantallas interrumpen temporalmente el renderizado 3D de los juegos de casino (ruleta, tragamonedas, dados) para presentar información crítica, advertencias sobre ludopatía, opciones financieras (préstamos) y elementos interactivos educativos (quizzes). 

El diseño de este módulo es fundamental para los objetivos del proyecto, ya que implementa la "fricción intencional" y rompe la inmersión típica de los juegos de apuestas para forzar la reflexión del usuario.

## 2. Estructuras de datos clave

### `InfoPantalla`
```c
typedef struct {
    const char* mensaje_reflexivo;
    int pagina_educacion;         
    int opcion_menu;              
    float progreso_carga;         
    int indice_checkpoint;        
    int quiz_pregunta_idx;
    int quiz_fase;
    int quiz_respuesta_elegida;
    int quiz_fue_correcta;
    int juego_activo;             
} InfoPantalla;
```
**¿Por qué se diseñó así?**
Actúa como un contenedor de parámetros transitorios. En lugar de crear múltiples variables globales o modificar la estructura principal del juego con datos que solo son relevantes durante un parpadeo de la UI (ej. en qué fase de un quiz estamos), se empaquetan en `InfoPantalla`. Esto mantiene el estado limpio y permite que la función principal de renderizado (`dibujar_pantalla_segun_estado`) reciba un único puntero de configuración uniforme, ignorando los campos que no aplican al estado actual.

## 3. Funciones públicas (Detalle)

### `inicializar_texturas_pantallas`
- **¿Para qué sirve?** Almacena las IDs de las texturas cargadas por el motor (ej. fondos, logos) para ser usadas en estas pantallas.
- **¿Por qué se diseñó así?** Para mantener un desacoplamiento entre el módulo de UI y el cargador de assets (debido a las restricciones de C89 y OpenGL clásico, es más limpio pasar simplemente los `GLuint`).
- **¿Cómo lo hace internamente?** Copia los IDs de texturas en variables estáticas locales (`g_textura_carga`, etc.) que luego son mapeadas en los `glBegin(GL_QUADS)` pertinentes.

### `dibujar_pantalla_carga`
- **¿Para qué sirve?** Muestra una barra de progreso inicial mientras se cargan los modelos 3D y texturas.
- **¿Cómo lo hace?** Cambia temporalmente la matriz de proyección a ortográfica (2D) y dibuja rectángulos escalados en base al parámetro `progreso`.

### `dibujar_pantalla_menu`
- **¿Para qué sirve?** Dibuja el menú principal para seleccionar entre Ruleta, Tragamonedas o Dados.
- **¿Por qué se diseñó así?** Utiliza un estilo de diseño que imita el "glassmorphism" (paneles semitransparentes con bordes sólidos) para dar un aspecto moderno. Introduce inmediatamente advertencias sobre la naturaleza del juego ("Simulador educativo sobre ludopatía") para fijar el tono.
- **¿Cómo lo hace?** Usa `glBlendFunc` para la transparencia sobre el fondo oscuro y funciones de dibujado de texto bit a bit (GLUT Helvetica).

### `dibujar_pantalla_prestamo`
- **¿Para qué sirve?** Se invoca cuando el jugador se queda sin saldo y busca seguir apostando.
- **¿Por qué se diseñó así?** Implementa **fricción intencional**. En un juego de casino normal, pedir crédito es fácil (para que no dejes de jugar). Aquí, la pantalla se vuelve agresiva y reflexiva. Evalúa cuántos préstamos llevas y adapta el mensaje: desde una advertencia inicial, hasta indicarte que estás por caer en bancarrota irreparable. Ofrece explícitamente la opción de abandonar.
- **¿Cómo lo hace?** Compara la deuda potencial del `Jugador` con límites establecidos (ej. `LIMITE_DEUDA_IMPAGABLE`) para seleccionar uno de 5 niveles de severidad en el texto, centrándolo en pantalla junto a un ícono de advertencia generado procedimentalmente mediante primitivas GL.

### `dibujar_pantalla_game_over`
- **¿Para qué sirve?** Aparece de forma ineludible cuando la deuda sobrepasa la capacidad de pago.
- **¿Por qué se diseñó así?** Está pensada para castigar psicológicamente el sobreendeudamiento, no como un simple "has perdido", sino como un fracaso financiero severo. Explica de forma cruda que los intereses (dinero que nunca se vio) son los que ahogaron al jugador, haciendo paralelismos con prestamistas informales reales.
- **¿Cómo lo hace internamente?** Emplea `glutGet(GLUT_ELAPSED_TIME)` y funciones trigonométricas (`sinf`) para generar un **pulso rojo ominoso** de fondo, desactivando la iluminación y profundidad para escribir el resumen financiero final.

### `dibujar_pantalla_sesion_terminada`
- **¿Para qué sirve?** Es el contraste del Game Over; se muestra cuando el jugador **elige** retirarse voluntariamente.
- **¿Por qué se diseñó así?** Su tono es calmado y reforzador ("Parar a tiempo también es una forma de ganar"). Busca premiar la decisión de romper el ciclo de apuestas. Se usan paletas azules y celestes en lugar de rojo.
- **¿Cómo lo hace?** De forma similar al Game Over en cuanto al layout de las estadísticas, pero con un renderizado estático y amigable sin pulso temporal.

### `dibujar_pantalla_mensaje_reflexivo`
- **¿Para qué sirve?** Un "Reality Check" que aparece durante la partida (ej. tras cierto tiempo o pérdidas) interrumpiendo abruptamente el juego.
- **¿Por qué se diseñó así?** Para combatir la "disociación" (la zona) de la ludopatía. Si el jugador descarta estos mensajes repetidamente (`mensajes_reflexivos_mostrados >= 2`), la UI aplica **fricción visual progresiva**: invierte la posición de las opciones (lo que requiere leer antes de pulsar instintivamente ENTER) y cambia el color a señales de peligro.
- **¿Cómo lo hace?** Lee el historial de advertencias en la estructura `Jugador` e invoca condicionalmente colores dorados o azules para los textos, forzando la atención del usuario.

### `dibujar_pantalla_educacion`
- **¿Para qué sirve?** Muestra el Pilar 4 del Serious Game: un carrusel de páginas con información técnica, síntomas (DSM-5) y ayuda para República Dominicana.
- **¿Por qué se diseñó así?** Proporciona la capa didáctica estática requerida en este tipo de software. 
- **¿Cómo lo hace?** Basado en el parámetro de página suministrado, renderiza cadenas estáticas grandes separadas por saltos de línea utilizando `dibujar_texto_multilinea_izquierda`.

### `dibujar_pantalla_quiz` y `quiz_evaluar_respuesta`
- **¿Para qué sirve?** Renderiza una interfaz de opción múltiple y evalúa la corrección.
- **¿Por qué se diseñó así?** Gamifica el aprendizaje (Pilar 5), exigiendo que el usuario demuestre comprensión de las mecánicas engañosas (falacia del jugador, RNG) para continuar.
- **¿Cómo lo hace?** Dependiendo de la `fase` (pregunta, evaluación de correcto/incorrecto) usa condicionales para pintar la opción elegida en verde (acierto) o rojo (fallo), y revela el porqué del resultado.

### `dibujar_pantalla_segun_estado`
- **¿Para qué sirve?** Función router/despachadora principal.
- **¿Por qué se diseñó así?** Evita un `switch-case` masivo dentro del bucle principal de renderizado (`display()` en `main.c`), manteniendo limpio el motor central.
- **¿Cómo lo hace?** Recibe el `EstadoJuego`, un puntero al `Jugador` y el `InfoPantalla`, y deriva la ejecución a la función específica de dibujado pertinente usando una simple estructura condicional.

## 4. Dependencias
- **OpenGL/GLUT (`<GL/glut.h>`)**: Indispensable para las llamadas gráficas a primitivas (`GL_QUADS`, `GL_LINES`), manipulación de la matriz de proyección (ortográfica para 2D) y dibujado de fuentes de bits (`glutBitmapCharacter`).
- **Core del Juego (`../core/jugador.h`, `../core/estado_juego.h`)**: Necesario para leer el estado del flujo del programa y el historial de comportamiento y financiero del usuario para adaptar dinámicamente las pantallas.
- **Librería Estándar de C89 (`<stdio.h>`, `<stdlib.h>`, `<math.h>`, `<string.h>`)**: Usadas para el formateo de strings (`sprintf_s`), matemática básica de pulsos y animaciones (`sinf`, `fabsf`), y copiado/truncado de cadenas multilínea.
