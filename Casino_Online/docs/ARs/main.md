# Documentación del Módulo `main.c`

## 1. Resumen del módulo
El archivo `main.c` es el punto de entrada principal y el corazón del proyecto "Casino Online", un 'Serious Game' desarrollado en C89 con OpenGL (GLUT). Este módulo orquesta la inicialización gráfica, la máquina de estados global (menús, juegos, encuestas, educación), la recolección de entradas del usuario (teclado, ratón) y la sincronización (loop de GLUT). Además, gestiona la lógica transversal como las reglas económicas del jugador y la reproducción de audio básico mediante la API MCI de Windows.

## 2. Estructuras de datos clave

### `EstadoPartida`
Es la estructura de datos más crítica definida en este archivo. Antes, las variables estaban dispersas como estáticas globales, pero fueron agrupadas en esta estructura (resolviendo un TODO propio del código). Esto permite manejar "el estado de una partida en curso" como una única unidad, facilitando la escalabilidad (ej. guardado/carga o reinicio de todo el juego).
Contiene:
- **Estados de los juegos**: `jugador` (datos del usuario), `bolita` (física de ruleta), estado de `tragamonedas` y `dados`.
- **Apuestas**: `apuestas_activas`, `num_apuestas_activas`, ficha seleccionada (`ficha_actual_index`, `monto_ficha_actual`).
- **Control de Aleatoriedad**: `numero_ganador_pendiente` (fija el resultado ANTES de animar para evitar errores de coma flotante).
- **Interfaz y Serious Game**: Paginación educativa (`pagina_educacion`), opción de menú (`opcion_menu`), mensajes reflexivos (`mensaje_reflexivo_actual`), sistema de checkpoints y quizzes (`indice_checkpoint_educativo`, `quiz_pregunta_actual`, etc.).

### `FICHAS`
Arreglo estático `const float FICHAS[4]` que define las denominaciones de fichas disponibles ($10, $25, $50, $100).

## 3. Funciones (Detalladas)

### `limpiar_audio(void)`
- **¿Para qué sirve?**: Cierra la sesión de la API MCI de Windows que reproduce la música de fondo.
- **¿Por qué se diseñó así?**: Se registra usando `atexit()` en `main` para garantizar que los recursos del sistema operativo (audio) se liberen correctamente al cerrar el juego de golpe (ej. presionando ESC), independientemente de dónde ocurra la salida.
- **¿Cómo lo hace?**: Llama a `mciSendStringA("close musica", ...)` directamente.

### `iniciar_musica_fondo(void)`
- **¿Para qué sirve?**: Carga y comienza a reproducir la música de fondo del casino.
- **¿Por qué se diseñó así?**: Las rutas relativas en C/Windows pueden ser frágiles dependiendo desde qué directorio se lanza el `.exe`. Se incluyó lógica de contingencia ("fallback") para probar tanto en el directorio actual (`musica\...`) como en el padre (`..\musica\...`).
- **¿Cómo lo hace?**: Usa `GetFileAttributesA` para verificar la existencia del `.wav`. Si existe, lo abre y lo reproduce con `mciSendStringA`.

### `monto_apostado_en_ronda(void)`
- **¿Para qué sirve?**: Calcula el monto total de dinero que el jugador ha puesto en la mesa en el transcurso de la ronda actual (antes de que gire la ruleta o los dados).
- **¿Por qué se diseñó así?**: Se introdujo como un "BUGFIX (integridad económica)". Antes, el saldo no se descontaba sino hasta el final de la ronda, lo que permitía al jugador seguir apostando más allá de sus fondos reales.
- **¿Cómo lo hace?**: Itera sobre `partida.apuestas_activas` sumando los montos.

### `saldo_alcanza_para_ficha(float monto_ficha)`
- **¿Para qué sirve?**: Valida si el jugador puede costear la ficha que intenta colocar.
- **¿Por qué se diseñó así?**: Utiliza `monto_apostado_en_ronda()` para comparar el saldo real disponible (Saldo total - Apuestas puestas).
- **¿Cómo lo hace?**: Retorna 1 si `(partida.jugador.saldo - monto_apostado_en_ronda()) >= monto_ficha`, 0 de lo contrario.

### `verificar_fondos_y_pedir_prestamo_si_hace_falta(void)`
- **¿Para qué sirve?**: Verifica si el jugador se ha quedado sin dinero y fuerza un cambio de estado hacia `ESTADO_PRESTAMO`.
- **¿Por qué se diseñó así?**: Para evitar duplicar el código en los distintos puntos donde el jugador podría quebrar (al resolver ronda o al cerrar un mensaje reflexivo). Prioriza mostrar notificaciones y mensajes antes de obligarlo a pedir el préstamo.
- **¿Cómo lo hace?**: Comprueba el saldo contra `partida.monto_ficha_actual` en los estados de juego y llama a `cambiar_estado(ESTADO_PRESTAMO)`.

### `agregar_apuesta(...)`
- **¿Para qué sirve?**: Inserta una nueva apuesta en la ronda activa de la ruleta.
- **¿Por qué se diseñó así?**: Se limita a un máximo (`MAX_APUESTAS`) estático por restricciones de memoria/diseño en C89. Dispara notificaciones visuales inmediatamente para feedback al usuario.
- **¿Cómo lo hace?**: Comprueba si hay espacio. Si lo hay, asigna tipo, valor y monto al arreglo de la partida. Llama a `registrar_apuesta` e invoca al sistema de UI con `agregar_notificacion` utilizando `sprintf_s`.

### `quitar_apuesta(int indice)` / `quitar_ultima_apuesta_tipo_valor(...)`
- **¿Para qué sirve?**: Permiten deshacer apuestas. `quitar_ultima_apuesta_tipo_valor` busca de atrás hacia adelante para remover la última apuesta hecha en una casilla (útil para el clic derecho).
- **¿Por qué se diseñó así?**: Da libertad al jugador de corregir errores antes de girar. `quitar_apuesta` compacta el arreglo desplazando los elementos hacia atrás para evitar "huecos", clásico en arreglos estáticos de C.

### `hacer_apuesta_dados(TipoApuestaDados tipo)`
- **¿Para qué sirve?**: Centraliza el flujo para apostar en el minijuego de Dados.
- **¿Por qué se diseñó así?**: Garantiza que el tiro se procese de manera determinista. El resultado final del dado se decide antes de animarse visualmente, asegurando sincronía.
- **¿Cómo lo hace?**: Valida saldo, registra la apuesta, usa `decidir_resultado_dados`, e inicia la física/animación (`iniciar_tiro_dados`).

### Callbacks de GLUT (`display`, `reshape`, `teclado`, `teclas_especiales`, `mouse_click`, `mouse_mover`, `idle`)
- **`display`**: Es el bucle de renderizado principal.
  - **Decisiones**: Utiliza `glPushMatrix` / `glPopMatrix` para aislar transformaciones, dibuja fondos texturizados (2D usando modo ortográfico) antes que los objetos 3D. Renderiza diferentes cámaras (`gluLookAt`) dependiendo de `partida.juego_activo`.
- **`idle`**: Motor lógico y físico.
  - **Fricción Intencional y Diseño**: Se introdujo un BUGFIX vital: el uso de tiempo real (`glutGet(GLUT_ELAPSED_TIME)`) en lugar de saltos fijos. GLUT no limita los FPS, por lo que una animación basada en "frames" volaba en PCs modernas. Se calcula `delta_tiempo` (clamp a máx 0.1s para evitar fallas físicas) asegurando animaciones suaves y predecibles. Resuelve si se gana/pierde al finalizar rotaciones, y controla la aparición de encuestas (Quiz) y advertencias de adicción tras "x" minutos o rondas.
  - Además implementa un bucle casero para reiniciar la música con MCI.
- **`teclado` y `mouse_click`**: Enrutadores masivos mediante máquinas de estado (`switch` y `if/else`). Cada tecla y clic cambia de comportamiento drásticamente dependiendo de `estado_actual` (menú, advertencia, jugando ruleta, etc.). El Raycasting del mouse (matemática para saber dónde clica en el plano 3D) se hace mediante unciones delegadas (`obtener_punto_clic_en_mesa`).

### `main(int argc, char** argv)`
- **¿Para qué sirve?**: Inicialización global de la aplicación.
- **¿Por qué se diseñó así?**: Utiliza `glutFullScreen()` (fricción inmersiva intencional, típica de casinos simulados o Serious Games para captar total atención). Habilita antialiasing (`GLUT_MULTISAMPLE`). Pasa el control absoluto a `glutMainLoop()`.
- **¿Cómo lo hace?**: Prepara estados, compila geometrías (rueda Bézier), carga texturas (carga y menú), establece callbacks de GLUT y arranca.

## 4. Dependencias
- **Bibliotecas del Sistema / Estandar**: `<GL/glut.h>`, `<stdio.h>`, `<stdlib.h>`, `<math.h>`, `<time.h>`.
- **Específicas de Windows**: `<windows.h>`, `<mmsystem.h>` y `#pragma comment(lib, "winmm.lib")` (Utilizadas exclusivamente para la API multimedia MCI de fondo musical).
- **Módulos Internos (Proyecto)**:
  - `render/`: `textura.h`, `iluminacion.h`, `materiales.h`.
  - `core/`: `estado_juego.h`, `jugador.h`.
  - `ruleta/`, `dados/`, `tragamonedas/`: Módulos de lógica, animación y geometría para cada minijuego.
  - `ui/`: `hud.h`, `notificaciones.h`, `pantallas.h`, `tablero_apuestas.h`, etc. (Manejo de UI).
