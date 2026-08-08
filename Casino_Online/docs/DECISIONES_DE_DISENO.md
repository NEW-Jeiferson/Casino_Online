# Documento de Decisiones de Diseño (Architecture and Design Decisions)

Este documento detalla de manera rigurosa y académica las decisiones fundamentales de arquitectura de software, metodologías de trabajo, e ingeniería de requisitos adoptadas durante el desarrollo de "Casino Online".

---

## 1. Justificación de la Arquitectura: Orquestador Centralizado
En contraposición a un modelo distribuido con múltiples ejecutables independientes, el proyecto adopta un patrón de orquestador central (un único punto de entrada a través de la función `main()`). Esta decisión minimiza la sobrecarga (overhead) computacional asociada a la inicialización, destrucción y gestión de múltiples contextos de renderizado OpenGL. Un ciclo de vida unificado garantiza una gestión determinista de los recursos en memoria (como texturas, búferes y contextos de audio) y permite un Game Loop continuo, ofreciendo un rendimiento predecible y robusto.

---

## 2. Patrón de Diseño: Máquina de Estados Finitos (FSM) en C89
El núcleo de la lógica de navegación y ejecución de la aplicación se ha abstraído a través de una Máquina de Estados Finitos (FSM, por sus siglas en inglés). Esta arquitectura fue seleccionada por su comprobada eficacia en el desarrollo de videojuegos y sistemas embebidos, garantizando transiciones deterministas y evitando la proliferación de lógica condicional acoplada (código espagueti).

La implementación se ha realizado estrictamente bajo el estándar **ANSI C (C89)**, asegurando la máxima portabilidad. A continuación, se presenta un extracto representativo del diseño modular implementado en `core/estado_juego.c`:

```c
/* Fragmento de core/estado_juego.h */
typedef enum {
    ESTADO_INIT,
    ESTADO_INTRO,
    ESTADO_MENU_PRINCIPAL,
    ESTADO_MINIJUEGO_RULETA,
    ESTADO_MINIJUEGO_TRAGAMONEDAS,
    ESTADO_MINIJUEGO_DADOS,
    ESTADO_GAME_OVER
} TipoEstadoJuego;

/* Fragmento de core/estado_juego.c */
#include "estado_juego.h"

static TipoEstadoJuego estado_actual = ESTADO_INIT;

void cambiar_estado(TipoEstadoJuego nuevo_estado) {
    /* Lógica de limpieza (teardown) del estado anterior si fuese necesaria */
    estado_actual = nuevo_estado;
}

void actualizar_y_renderizar_estado(void) {
    switch (estado_actual) {
        case ESTADO_INTRO:
            renderizar_pantalla_intro();
            break;
        case ESTADO_MENU_PRINCIPAL:
            actualizar_menu();
            renderizar_menu();
            break;
        case ESTADO_MINIJUEGO_RULETA:
            actualizar_ruleta();
            renderizar_ruleta();
            break;
        case ESTADO_GAME_OVER:
            renderizar_game_over();
            break;
        default:
            /* Manejo de errores por estados indefinidos */
            break;
    }
}
```
Al delegar la responsabilidad del flujo a la FSM, la arquitectura respeta el principio de Responsabilidad Única (SRP). Las llamadas a rutinas de OpenGL y la actualización de lógica de negocio se restringen exclusivamente al contexto del estado activo.

---

## 3. Reorientación a "Serious Game" y Requisitos No Funcionales (IEEE-830)
Durante el ciclo de desarrollo temprano, el proyecto fue conceptualizado como un simulador de casino tradicional. No obstante, mediante un análisis ético y pedagógico, el equipo decidió pivotar hacia un paradigma de **"Serious Game"** (Juego Serio). Este cambio de enfoque redefinió sustancialmente los Requisitos No Funcionales (RNF) del sistema, alineándolos con las directrices de la especificación IEEE-830:

*   **RNF-Etica (Propósito Pedagógico):** En lugar de glorificar las apuestas, el software funciona como una herramienta de simulación para ilustrar matemáticamente la premisa de que "la casa siempre gana".
*   **RNF-Confiabilidad (Determinismo Matemático):** El sistema debe garantizar estadísticamente que, a largo plazo, la esperanza matemática para el jugador es negativa, simulando con precisión el riesgo de bancarrota asociado a la ludopatía.

Este enfoque transformó un proyecto de entretenimiento lúdico en un artefacto académico de concientización social.

---

## 4. Diseño de Experiencia de Usuario (UX): Usabilidad y Psicología (IEEE-830)
El diseño de la interfaz y la usabilidad (definida bajo los parámetros de calidad del estándar IEEE-830) fueron adaptados deliberadamente para soportar la narrativa del Serious Game, empleando técnicas de psicología cognitiva:

1.  **Lectura Forzada (Contextualización):** Se impuso la obligatoriedad de lectura en las pantallas introductorias. La omisión de mecánicas de "skip" rápido garantiza que el usuario asimile las advertencias sobre el comportamiento adictivo antes de interactuar con estímulos de azar.
2.  **Fricción Intencional y Supresión del Bucle de Dopamina:** En un diseño lúdico tradicional, la retención del usuario se maximiza facilitando el reinicio ("Jugar de Nuevo"). En contraposición, el requisito de usabilidad para la pantalla de bancarrota (`ESTADO_GAME_OVER`) prohíbe explícitamente cualquier botón de reinicio. El usuario está forzado a cerrar la ventana del sistema operativo (Hard Exit). Esta *fricción intencional* busca interrumpir abruptamente el bucle neuroquímico de la compulsión, obligando al usuario a confrontar de forma reflexiva la pérdida total del patrimonio virtual.

---

## 5. Estrategia de Control de Versiones e Integración de Módulos
La arquitectura en un proyecto colaborativo demanda estrategias rigurosas de control de versiones. El equipo, conformado por **Jeiferson**, **Dubenny** y **Luis**, enfrentó inicialmente desafíos críticos de acoplamiento (colisiones en variables globales y gestión de contextos gráficos) al desarrollar los minijuegos (Ruleta, Tragaperras, Dados) de forma aislada.

Para resolver este desafío, se adoptó una arquitectura de **"Bajo Acoplamiento" y "Fuente Única de Verdad"**. Se definieron dos módulos core:
*   `core/estado_juego.c`: Administrador exclusivo del flujo de la interfaz (FSM).
*   `core/jugador.c`: Gestor centralizado del patrimonio (créditos y apuestas) del usuario.

Esta refactorización viabilizó una **estrategia de branching (ramas) de Git** altamente eficiente:
*   **Jeiferson:** Asumió el liderazgo del orquestador (FSM), la lógica de la Ruleta y la integración principal.
*   **Dubenny:** Se enfocó en la lógica probabilística y matemática de los pagos de las Tragamonedas en su respectiva rama (feature branch).
*   **Luis:** Trabajó de forma aislada en la compleja geometría, animaciones de OpenGL de las Tragamonedas y los Dados.

Cualquier evento de transacción financiera era despachado a la API de `core/jugador`, mientras que las transiciones de pantalla se delegaban a `core/estado_juego`. Esta estricta separación de responsabilidades a nivel de interfaces de código permitió que, al realizar los *Pull Requests* hacia la rama `main`, la incidencia de *merge conflicts* (conflictos de fusión) se redujera a cero, demostrando una madurez notable en las prácticas de Ingeniería de Software del equipo.
