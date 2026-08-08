# Arquitectura del Sistema — Casino Online

> **Nota:** Este documento describe la arquitectura real del proyecto verificada contra el código fuente en `src/`. Todos los diagramas de dependencias se construyeron a partir de las directivas `#include` reales de cada archivo, no por suposición basada en la estructura de carpetas. Este análisis de diseño arquitectónico incorpora justificaciones y decisiones clave vinculadas a las restricciones ambientales (IEEE-830).

---

## 1. Estructura de archivos de `src/`

La estructura obedece al paradigma de separación de responsabilidades, vital para un sistema monolítico en C89.

```
src/
├── main.c                         ← Punto de entrada, loop GLUT, orquestación global
├── stb_image.h                    ← Librería header-only para carga de imágenes (PNG/JPG)
│
├── core/                          ← Lógica de negocio pura (sin dependencias gráficas)
│   ├── estado_juego.c / .h        ← Máquina de estados centralizada (transiciones de pantalla)
│   └── jugador.c / .h             ← Estado económico/conductual del jugador, sistema de riesgo
│
├── render/                        ← Subsistema de renderizado OpenGL de función fija
│   ├── iluminacion.c / .h         ← Configuración de luces (GL_LIGHT0..N, modelo Phong)
│   ├── materiales.c / .h          ← Materiales predefinidos (glMaterialfv) para objetos 3D
│   └── textura.c / .h             ← Carga de texturas PNG/JPG vía stb_image → glTexImage2D
│
├── ui/                            ← Interfaz de usuario 2D (HUD, pantallas, notificaciones)
│   ├── pantallas.c / .h           ← Todas las pantallas 2D (intro, menú, préstamo, game over, quiz)
│   ├── hud.c / .h                 ← HUD principal de la ruleta (saldo, deuda, medidor de riesgo)
│   ├── hud_dados.c / .h           ← HUD específico del minijuego de dados
│   ├── hud_tragamonedas.c / .h    ← HUD específico de la máquina tragamonedas
│   ├── notificaciones.c / .h      ← Sistema de notificaciones efímeras (cola FIFO con temporizador)
│   └── tablero_apuestas.c / .h    ← Tablero visual de apuestas de la ruleta (renderizado + lógica)
│
├── ruleta/                        ← Minijuego: Ruleta Europea
│   ├── ruleta_geometria.c / .h    ← Geometría 3D de la rueda (revolución de perfil Bézier)
│   ├── ruleta_animacion.c / .h    ← Física simulada de la bolita (desaceleración, rebote)
│   └── mouse_picking.c / .h      ← Selección de casillas por clic (color picking con glReadPixels)
│
├── dados/                         ← Minijuego: Dados
│   ├── dados_geometria.c / .h     ← Geometría 3D de los dados (cubos con caras numeradas)
│   ├── dados_animacion.c / .h     ← Animación de lanzamiento (rotación, asentamiento)
│   └── dados_logica.c / .h        ← Reglas de apuesta (Par/Impar, Suma=7, Extremos)
│
├── tragamonedas/                  ← Minijuego: Máquina Tragamonedas
│   ├── tragamonedas_geometria.c / .h  ← Geometría 3D de la máquina (carcasa, rodillos, palanca)
│   ├── tragamonedas_animacion.c / .h  ← Animación de rodillos (interpolación Bézier)
│   ├── tragamonedas_logica.c / .h     ← Tabla de pagos, resolución de rondas, RNG
│   ├── simbolos_tex.c / .h            ← Carga de texturas PNG para los símbolos de los rodillos
│   └── simbolos/                      ← Directorio con archivos PNG de símbolos individuales
│
├── utils/                         ← Utilidades matemáticas reutilizables
│   ├── bezier.c / .h              ← Evaluación de curvas de Bézier cúbicas (punto + derivada)
│   └── bresenham.c / .h           ← Algoritmo de Bresenham para rasterización de líneas
│
└── assets/
    └── tragamonedas_fondo.png     ← Textura de fondo para la escena de la tragamonedas
```

---

## 2. Mapa de dependencias entre módulos y Justificación Arquitectónica

El diseño modular refleja las restricciones operativas impuestas por la adopción del estándar **C89** y el uso de **OpenGL 1.x Legacy**. Debido a la ausencia de Programación Orientada a Objetos en C, el diseño impone una arquitectura basada en **Descomposición Funcional**.

### 2.1. Módulos consumidos por todo el proyecto

Los dos módulos de `core/` encapsulan la lógica de dominio puro. Están diseñados intencionalmente para ser independientes del sistema de ventanas (GLUT) y del motor de renderizado (OpenGL). Esto permite aislar y verificar formalmente la lógica de apuestas sin iniciar el entorno gráfico.

| Módulo consumidor | Depende de `estado_juego.h` | Depende de `jugador.h` |
|---|---|---|
| `main.c` | ✅ | ✅ |
| `ui/pantallas.h` | ✅ | ✅ |
| `ui/hud.h` | — | ✅ |
| `ui/hud_dados.h` | — | — (vía `dados_logica.h`) |
| `ui/hud_tragamonedas.h` | ✅ | — |
| `ui/tablero_apuestas.h` | ✅ | — |
| `ui/tablero_apuestas.c` | ✅ (redundante) | — |
| `ruleta/ruleta_geometria.c` | ✅ | — |
| `dados/dados_logica.h` | — | ✅ |
| `tragamonedas/tragamonedas_logica.h` | — | ✅ |

### 2.2. Diagrama de dependencias simplificado

```
                    ┌─────────────────┐
                    │     main.c      │
                    │  (orquestador)  │
                    └────────┬────────┘
                             │ incluye todo
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
   ┌───────────┐     ┌─────────────┐     ┌──────────────┐
   │  core/    │     │   render/   │     │    utils/     │
   │ estado    │     │ iluminación │     │  bezier      │
   │ jugador   │     │ materiales  │     │  bresenham   │
   │ (sin GL)  │     │ textura     │     └──────┬───────┘
   └─────┬─────┘     └──────┬──────┘            │
         │                  │                   │
         │  consume         │ consume           │ consume
         ▼                  ▼                   ▼
   ┌─────────────────────────────────────────────────┐
   │                 Módulos de juego                │
   │  ruleta/    │    dados/    │   tragamonedas/    │
   └─────────────────────┬───────────────────────────┘
                         │
                         │ consume
                         ▼
                ┌─────────────────┐
                │      ui/        │
                │  pantallas      │
                │  hud (×3)       │
                │  notificaciones │
                │  tablero        │
                └─────────────────┘
```

> **Nota importante:** La dependencia cruzada `tragamonedas_geometria.c` → `ui/hud_tragamonedas.h` rompe la separación estricta entre módulos de juego y UI. Este acoplamiento es el resultado de un "trade-off" pragmático. Evita una explosión combinatoria de parámetros pasados por los componentes visuales de alto nivel.

---

## 3. Flujo de ejecución de `main.c`: Patrón Orquestador

En aplicaciones interactivas modernas se utiliza la Inversión de Control (IoC). Dado que usamos GLUT, cedemos el bucle infinito del programa (`glutMainLoop`).

### 3.1. Secuencia de Inicialización

La inicialización asegura el despliegue correcto del contexto gráfico, estado del juego y los recursos de memoria, de una sola vez.

```c
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    srand((unsigned int)time(NULL));

    /* Modo de display Legacy (Double buffering, RGBA, Depth, Multisample) */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GL_MULTISAMPLE);

    glutGameModeString("1024x768:32@60");
    if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
        glutEnterGameMode();
    } else {
        glutCreateWindow("Casino Online - Serious Game");
        glutFullScreen();
    }

    /* Z-buffer, Culling y Anti-Aliasing */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_notificaciones();
    inicializar_jugador(&partida.jugador, 1000.0f, glutGet(GLUT_ELAPSED_TIME));
    inicializar_estado_juego();
    inicializar_bolita(&partida.bolita);

    /* ... inicialización de geometría ... */

    /* Registro de Callbacks (Delegación) */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutSpecialFunc(teclas_especiales);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutPassiveMotionFunc(mouse_mover);

    /* Cede el control a GLUT */
    glutMainLoop(); 
    return 0;
}
```

### 3.2. Trade-offs Arquitectónicos: Ejecutable Único vs. Múltiples

Se ha optado deliberadamente por empaquetar toda la experiencia de "Casino Online" en un **único archivo ejecutable (monolito)**, en lugar de dividir cada minijuego en procesos separados.

**Ventajas del modelo único:**
1. **Compartición de estado en memoria:** `estado_juego` y `jugador` persisten fácilmente entre los tres juegos (Ruleta, Dados, Tragamonedas). Un esquema multiproceso requeriría IPC (Inter-Process Communication) complejo.
2. **Contexto de OpenGL Compartido:** Un único loop `glutMainLoop` permite transicionar instantáneamente de la ruleta al menú sin requerir desmontar la ventana gráfica (y recargar el contexto del driver).
3. **Restricción C89:** El estándar C89 no ofrece librerías estándar nativas para la orquestación multiproceso cruzada.

**Desventajas del modelo único:**
1. **Acoplamiento de compilación:** Cambiar la lógica de los dados fuerza a recompilar el main, la ruleta y la UI.
2. **Uso de memoria:** Los recursos de la tragamonedas (geometría, texturas) están cargados incluso cuando el jugador juega dados.

### 3.3. El Patrón de Delegación por Callbacks

Una vez cedido el control a `glutMainLoop()`, la arquitectura opera de manera orientada a eventos. `main.c` se convierte en el "Director de Orquesta":

- **`display()`**: Ejecutado al solicitar un repintado (`glutPostRedisplay`). Determina el `estado_actual` en un macro `switch` y delega a la capa visual pertinente (e.g. `ESTADO_JUGANDO_RULETA` renderiza la geometría 3D y el overlay 2D de su HUD).
- **`idle()`**: Motor de simulación en tiempo real. En cada ciclo computa el delta de tiempo, delega los avances físicos a las rutinas de animación (`actualizar_bolita`, `actualizar_animacion_dados`), y valida de forma asíncrona notificaciones.
- **`teclado()` / `mouse_click()`**: Intercepta comandos de usuario (I/O). En función del estado actual (`core/estado_juego`), la entrada se rutea al minijuego correspondiente o al menú.

---

## 4. Restricciones Ambientales (Cumplimiento IEEE-830)

| Restricción / Librería | Uso e Impacto Arquitectónico |
|---|---|
| **C89 Estricto** | Fuerza una estructura procedural monolítica. Las variables se declaran al principio del bloque (sin VLA), lo que promueve el uso de un solo `struct EstadoPartida` para simular objetos. |
| **OpenGL 1.x Legacy** | Todo el renderizado es por tubería de función fija (`glBegin`/`glEnd`). Impide el uso de VBOs/Shaders, forzando la computación pesada de geometría (como mallas de Bézier) en la CPU, lo que afecta el diseño de los módulos `*_geometria`. |
| `GL/glut.h` | Controlador de ciclo de vida. Impide el polimorfismo activo de bucles; obliga al uso masivo de callbacks globales y variables de estado globales en `main.c`. |
| `stb_image.h` | Inclusión header-only elegida en sustitución de librerías DLL dinámicas para evitar problemas de dependencias en Windows. |
| `<windows.h>` + MCI | Reproducción de audio sin librerías externas dependientes. Introduce una mínima condicionalidad a plataforma Win32. |
