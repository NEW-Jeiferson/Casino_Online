# Especificación de Requisitos de Software (ERS) — Casino Online

**Conforme a la estructura de IEEE Std 830-1998** (Recommended Practice for Software Requirements Specifications)

| | |
|---|---|
| **Proyecto** | Casino Online — Simulador de concientización sobre ludopatía |
| **Versión del documento** | 1.3 — agregado el grupo de requisitos RF-INTRO-01 a 05 (secuencia de arranque: pantalla de carga con mensajes rotativos y las 3 pantallas de contexto real, ausentes hasta esta versión); RF-DAD-03 corregido para reflejar el reemplazo real de la animación de dados (ya no usa Bézier, usa un esquema propio de 2 fases); ver también correcciones relacionadas en ADR-003 y ADR-005 |
| **Estado** | Borrador para revisión del equipo/docente — el contenido está completo y verificado contra el código; el único paso pendiente es la aprobación formal del equipo/docente, que este documento no puede autootorgarse |
| **Elaborado a partir de** | Código fuente real del proyecto (`src/`), README, `docs/analisis-previo.md` y `docs/ARs/` |

---

## Nota metodológica

Esta ERS se redactó de forma **retrospectiva**: el sistema ya está implementado (los tres minijuegos —Ruleta, Tragamonedas, Dados— están terminados según el estado del proyecto), y este documento describe los requisitos que el código real satisface hoy, no requisitos especulativos previos a la construcción. Esto es una diferencia deliberada respecto del uso "canónico" de IEEE 830 (que asume que el SRS precede a la implementación), pero es una práctica académica legítima y común cuando se documenta un sistema ya construido: cada requisito funcional de la sección 3.2 está trazado a la función o archivo real que lo satisface (ver [Apéndice A](#apéndice-a-matriz-de-trazabilidad)), de forma que el documento sea verificable contra el código, no solo aspiracional.

---

## Tabla de contenido

1. [Introducción](#1-introducción)
   1.1. [Propósito](#11-propósito)
   1.2. [Alcance](#12-alcance)
   1.3. [Definiciones, acrónimos y abreviaturas](#13-definiciones-acrónimos-y-abreviaturas)
   1.4. [Referencias](#14-referencias)
   1.5. [Visión general del documento](#15-visión-general-del-documento)
2. [Descripción general](#2-descripción-general)
   2.1. [Perspectiva del producto](#21-perspectiva-del-producto)
   2.2. [Funciones del producto](#22-funciones-del-producto)
   2.3. [Características de los usuarios](#23-características-de-los-usuarios)
   2.4. [Restricciones](#24-restricciones)
   2.5. [Supuestos y dependencias](#25-supuestos-y-dependencias)
3. [Requisitos específicos](#3-requisitos-específicos)
   3.1. [Requisitos de interfaces externas](#31-requisitos-de-interfaces-externas)
   3.2. [Requisitos funcionales](#32-requisitos-funcionales)
   3.3. [Requisitos de desempeño](#33-requisitos-de-desempeño)
   3.4. [Restricciones de diseño](#34-restricciones-de-diseño)
   3.5. [Atributos del sistema](#35-atributos-del-sistema)
   3.6. [Otros requisitos](#36-otros-requisitos)
4. [Apéndice A: Matriz de trazabilidad](#apéndice-a-matriz-de-trazabilidad)

---

## 1. Introducción

### 1.1. Propósito

Este documento especifica, de forma completa y verificable, los requisitos funcionales y no funcionales del sistema **Casino Online**. Su audiencia es el equipo de desarrollo (para verificar que la implementación cumple lo especificado), el docente evaluador de la materia de Computación Gráfica (para auditar la cobertura de requisitos del entregable) y cualquier integrante futuro del equipo que necesite entender qué debe hacer el sistema sin tener que inferirlo únicamente leyendo el código fuente.

Este documento **no** reemplaza a `docs/analisis-previo.md` (que justifica *por qué* existe el proyecto y qué antecedentes lo respaldan) ni a `docs/ARs/` (que documenta *por qué* se tomó cada decisión técnica). Los tres documentos son complementarios: la ERS dice **qué** debe cumplir el sistema, los ADR dicen **por qué** se construyó de una forma específica, y el análisis previo dice **por qué vale la pena que exista**.

### 1.2. Alcance

El sistema a especificar es **Casino Online**, una aplicación de escritorio para Windows que simula tres juegos de casino (Ruleta, Tragamonedas, Dados) con saldo virtual, con el propósito de concientizar sobre la ludopatía mediante la exposición controlada a mecánicas de juego reales y un sistema de retroalimentación educativa.

**Dentro del alcance:**
- Los tres minijuegos completos: Ruleta, Tragamonedas, Dados.
- La economía compartida de jugador (saldo, deuda, préstamos, estadísticas de sesión).
- El sistema de concientización en cuatro capas (ambiental, liviana, intermedia, seria — ver [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md)).
- La máquina de estados que orquesta la navegación entre pantallas.
- El renderizado 3D de toda la geometría mediante primitivas y curvas de Bézier, sin modelos importados.

**Fuera del alcance (explícitamente):**
- Cualquier módulo de **Póker**: se mencionó como roadmap conceptual en una versión temprana del proyecto, pero fue descartado y reemplazado por el módulo de Dados; no existe código de Póker en el proyecto actual.
- Dinero real, pasarelas de pago, cuentas de usuario persistentes o conexión a internet: el saldo es siempre virtual y la sesión no se guarda entre ejecuciones (no hay sistema de guardado/carga de partida).
- Multijugador o cualquier forma de interacción en red.
- Plataformas distintas de Windows x86 (no hay build para Linux/macOS ni para arquitecturas de 64 bits, ver [3.4](#34-restricciones-de-diseño)).
- Diagnóstico clínico real de ludopatía: el medidor de riesgo es una simulación educativa, nunca una herramienta clínica (ver [RF-CONC-01](#32-requisitos-funcionales)).

### 1.3. Definiciones, acrónimos y abreviaturas

| Término | Significado |
|---|---|
| **ERS / SRS** | Especificación de Requisitos de Software / Software Requirements Specification |
| **RF** | Requisito Funcional |
| **RNF** | Requisito No Funcional |
| **ADR** | Architecture Decision Record (Registro de Decisión de Arquitectura) |
| **HUD** | Head-Up Display — panel de información siempre visible en pantalla (saldo, riesgo, estadísticas) |
| **GLUT** | OpenGL Utility Toolkit — librería de ventana/entrada usada en su variante clásica (Win32, no freeGLUT) |
| **MCI** | Media Control Interface — API nativa de Windows usada para reproducir audio (`winmm.lib`) |
| **C89** | Estándar ANSI C de 1989; en este proyecto, exige declarar variables al inicio de cada bloque |
| **Near-miss** | "Casi acierto": resultado de una tragamonedas que visualmente sugiere una combinación ganadora a un símbolo de distancia, sin serlo |
| **LDW (Losses Disguised as Wins)** | "Pérdidas disfrazadas de victorias": pago menor al monto apostado, presentado con la misma celebración visual/sonora que una victoria real |
| **Falacia del jugador** | Sesgo cognitivo por el cual se cree que una racha de resultados influye en el próximo resultado independiente |
| **Ventaja de la casa** | Porcentaje esperado de pérdida del jugador a largo plazo sobre el monto apostado, para un tipo de apuesta determinado |
| **Ludopatía / trastorno de juego** | Trastorno conductual reconocido en el DSM-5, caracterizado por la incapacidad de controlar el impulso de apostar pese a sus consecuencias negativas |
| **Serious game** | Videojuego cuyo propósito primario no es el entretenimiento, sino educar, entrenar o generar un cambio de comportamiento |
| **Reality check** | Disparador neutral del sistema de concientización que aparece cada cierto número de rondas jugadas, independientemente de si el jugador va ganando o perdiendo |

### 1.4. Referencias

- IEEE Std 830-1998, *IEEE Recommended Practice for Software Requirements Specifications*.
- [`docs/analisis-previo.md`](analisis-previo.md) — antecedentes académicos y justificación del proyecto (SPARX, Re-Mission, The Amazing Château/McGill, PlayForward, investigación VR sobre near-miss).
- [`docs/ARs/`](ARs/) — ADR-001 a ADR-005 y AR-audio-mci, AR-texturas-stb-image.
- [`docs/guia-documentacion-codigo.md`](guia-documentacion-codigo.md) — estándar de comentarios del código fuente.
- [`README.md`](../../README.md) — visión general del proyecto y guía de compilación.
- Código fuente citado directamente en este documento: `src/core/jugador.c/h`, `src/core/estado_juego.c/h`, `src/main.c`, `src/ruleta/`, `src/tragamonedas/`, `src/dados/`.

### 1.5. Visión general del documento

La sección 2 describe el sistema en términos generales (qué es, quién lo usa, bajo qué restricciones y supuestos), sin entrar en el detalle de cada requisito individual. La sección 3 contiene los requisitos específicos, verificables y numerados (interfaces externas, requisitos funcionales agrupados por módulo, desempeño, restricciones de diseño y atributos de calidad). El Apéndice A traza cada requisito funcional a la función o archivo de código que lo implementa, cerrando el vacío de trazabilidad que el proyecto tenía pendiente.

---

## 2. Descripción general

### 2.1. Perspectiva del producto

Casino Online es un producto **independiente y autocontenido**: no se integra con, ni depende de, ningún otro sistema externo (no hay backend, no hay servicios web, no hay cuentas de usuario). Es un ejecutable de Windows (`.exe`, x86) construido sobre el pipeline de función fija de OpenGL con GLUT clásico, cuya única interacción con el sistema operativo host es el acceso a la tarjeta gráfica (vía OpenGL), la reproducción de audio (vía MCI de Windows) y la carga de archivos de imagen locales (vía `stb_image.h`).

Es, al mismo tiempo, un **artefacto académico**: existe como entregable de la materia de Computación Gráfica, por lo que su perspectiva de producto incluye una dimensión pedagógica doble — debe funcionar como simulador de concientización (para el usuario final) y como evidencia de dominio de los temas del curso (para el docente evaluador): transformaciones, jerarquía de matrices, iluminación de Phong, blending, curvas de Bézier, Bresenham, culling y z-buffer.

### 2.2. Funciones del producto

A alto nivel, el sistema ofrece las siguientes funciones (cada una se descompone en requisitos funcionales específicos en la sección 3.2):

1. Permitir al usuario jugar a la Ruleta, el Tragamonedas o los Dados con saldo virtual compartido entre los tres.
2. Simular con fidelidad los mecanismos psicológicos reales de cada juego (falacia del jugador, near-miss/LDW, variación de ventaja de casa).
3. Registrar el comportamiento de juego del usuario dentro de la sesión (racha de pérdidas, monto apostado, tiempo jugado, veces sin fondos) para alimentar el sistema de concientización.
4. Presentar contenido educativo en capas de intensidad creciente, sin bloquear el juego salvo cuando el comportamiento del usuario lo justifica.
5. Ofrecer al usuario la posibilidad de terminar la sesión en cualquier momento.
6. Simular la escalada de una deuda de juego (préstamo con interés, límite de deuda impagable, Game Over) como consecuencia visible de seguir jugando sin fondos.
7. Ofrecer una pantalla de información no interactiva con contenido educativo general sobre ludopatía, navegable independientemente de estar jugando.
8. Exponer al usuario, antes de poder jugar por primera vez en cada ejecución, contexto real sobre la escala de la industria del juego y su impacto humano (regional, nacional y de deuda), con fuentes citadas en pantalla, de forma obligatoria y no salteable.

### 2.3. Características de los usuarios

| Clase de usuario | Descripción | Nivel técnico esperado |
|---|---|---|
| **Jugador (usuario final)** | Estudiante o joven que interactúa con el simulador, típicamente en un contexto educativo o de concientización guiada | Ninguno — controles de teclado simples (letras y números), sin necesidad de experiencia previa en videojuegos ni en juegos de casino reales |
| **Evaluador docente** | Revisa el proyecto como entregable académico | Conocimiento de Computación Gráfica (OpenGL, GLUT); no necesariamente conocimiento del dominio de prevención de ludopatía |
| **Integrante del equipo de desarrollo** | Extiende o corrige el sistema | Conocimiento de C89 y del pipeline fijo de OpenGL; se apoya en [`docs/guia-documentacion-codigo.md`](guia-documentacion-codigo.md) y en este documento para entender el comportamiento esperado antes de modificar código |

El sistema **no** asume que el jugador conozca las reglas de un casino real: cada minijuego presenta sus controles y reglas de apuesta en pantalla (HUD y tablero de apuestas) al momento de jugar.

### 2.4. Restricciones

(Ver también 3.4, donde se detallan como restricciones de diseño formales.) A nivel general: el proyecto está acotado por requisitos curriculares no negociables de la materia — C89 estricto, GLUT clásico (no freeGLUT), sin importar modelos 3D externos — y por una restricción técnica derivada de esas dos: texto en pantalla en ASCII plano estricto. Estas restricciones están documentadas como decisiones de arquitectura en [ADR-001](ARs/ADR-001-c89-estricto-y-glut-clasico.md), [ADR-002](ARs/ADR-002-ascii-plano-estricto.md) y [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md).

### 2.5. Supuestos y dependencias

- Se asume que el sistema operativo host es **Windows** (32 bits o 64 bits con soporte de subsistema de 32 bits), dado que GLUT clásico (`glutdlls37beta`) es una librería específica de Win32.
- Se asume que **Visual Studio 2022** (o compatible) con la carga de trabajo de desarrollo de escritorio en C++ está instalado, y que la plataforma del proyecto está configurada en **x86**.
- Se asume que `glut.h`, `glut32.lib` y `glut32.dll` fueron instalados manualmente por quien compila (no viajan con el repositorio, ver README).
- Se depende de la API nativa de Windows Multimedia (`mmsystem.h`/`winmm.lib`) para audio, y de `stb_image.h` (incluido en el repositorio, dominio público) para decodificar texturas PNG.
- Se asume que la tarjeta gráfica del equipo que ejecuta el programa soporta el pipeline de función fija de OpenGL (no se requiere soporte de shaders programables, ya que el proyecto no los usa por decisión curricular).
- No se depende de ningún servicio externo, base de datos, ni conexión a internet.

---

## 3. Requisitos específicos

### 3.1. Requisitos de interfaces externas

#### 3.1.1. Interfaz de usuario

La única interfaz de entrada es el **teclado** (no hay soporte de mando/gamepad; el mouse se usa únicamente para selección de números en el tablero de la ruleta, ver `ruleta/mouse_picking.c`). Los controles reales, verificados contra `main.c::teclado()`, son:

| Tecla | Efecto | Estado(s) donde aplica |
|---|---|---|
| `ESC` | Cierra el programa inmediatamente (`exit(0)`) | Cualquier estado, sin excepción |
| `ENTER` | Confirma / avanza (continuar bajo responsabilidad, confirmar inicio de juego) | Menú, confirmación de juego, interrupciones educativas |
| `N` | Cancela / rechaza la confirmación de juego; también coloca ficha en NEGRO en Ruleta | Confirmación de juego / Ruleta jugando |
| `S` | Termina la sesión | Sin condición en cualquier interrupción (Préstamo, Mensaje reflexivo, Checkpoint, Quiz); en estado jugando, solo si el minijuego activo no está animando en ese instante — ver [RF-EST-03](#32-requisitos-funcionales) |
| `1`–`4` | Selecciona el monto de la ficha actual | Cualquier minijuego en curso |
| `R` | Apuesta a color ROJO | Ruleta jugando |
| `ESPACIO` | Dispara el giro/tiro con las apuestas activas; en Tragamonedas, gira los rodillos; en la pantalla de Información, avanza de página | Ruleta jugando / Tragamonedas jugando / Educación |
| `P` | Apuesta Par en Dados; pide un préstamo en la pantalla de Préstamo | Dados jugando / Préstamo |
| `I` | Apuesta Impar en Dados; abre la pantalla de Información | Dados jugando / Menú, Game Over, Sesión Terminada |
| `7` | Apuesta a Suma=7 en Dados | Dados jugando |
| `E` | Apuesta a Extremos (2 o 12) en Dados | Dados jugando |
| `A` / `B` / `C` | Selecciona la opción de respuesta en el Quiz educativo | Quiz educativo |

Toda la salida visual se renderiza en una única ventana GLUT de **1024×768 píxeles** (`glutInitWindowSize`), sin soporte de redimensionado dinámico del layout de HUD más allá del manejo estándar de `glutReshapeFunc`.

#### 3.1.2. Interfaz de hardware

- **Tarjeta gráfica** con soporte de OpenGL de función fija (sin requerimiento de shaders programables).
- **Dispositivo de audio** estándar de Windows, accedido vía MCI para la reproducción de música de fondo en loop.
- No se requiere hardware adicional (sin soporte de VR, sin periféricos especiales).

#### 3.1.3. Interfaz de software

- **OpenGL** (pipeline de función fija) + **GLU** para primitivas de utilidad.
- **GLUT clásico** (Win32, `glutdlls37beta`) para ventana, entrada y bucle de eventos — no freeGLUT.
- **Windows Multimedia API** (`winmm.lib`, `mciSendString`) para audio (ver [AR-audio-mci](ARs/AR-audio-mci.md)).
- **`stb_image.h`** (biblioteca de un solo header, dominio público) para decodificación de texturas PNG (ver [AR-texturas-stb-image](ARs/AR-texturas-stb-image.md)).
- No hay interfaz de red, de base de datos, ni de sistema de archivos más allá de la carga de assets estáticos (texturas, música) empaquetados con el ejecutable.

### 3.2. Requisitos funcionales

Los requisitos se agrupan por módulo. Cada uno tiene un identificador único (`RF-<módulo>-<número>`) usado en la matriz de trazabilidad (Apéndice A).

#### Economía y jugador compartido (`core/jugador.c`)

- **RF-ECO-01**: El sistema debe mantener un único saldo de jugador (`Jugador.saldo`), compartido entre los tres minijuegos — nunca un saldo independiente por juego.
- **RF-ECO-02**: El sistema debe inicializar cada sesión nueva con un saldo virtual de **1000.00** unidades (`inicializar_jugador`, `main.c`), nunca dinero real.
- **RF-ECO-03**: Al colocar una apuesta, el sistema debe registrar el monto en `total_apostado` de inmediato (`registrar_apuesta`), antes de conocerse el resultado de la ronda.
- **RF-ECO-04**: Al resolverse una ronda, el sistema debe aplicar la ganancia o pérdida neta al saldo (`aplicar_resultado_apuesta`), capturando el valor original de la ganancia/pérdida para las estadísticas de sesión (`total_ganado`/`total_perdido`) **antes** de que el pago automático de deuda pendiente lo modifique.
- **RF-ECO-05**: Si el jugador tiene deuda pendiente y la ronda resuelta en curso arroja ganancia neta positiva, el sistema debe abonar esa ganancia a la deuda (hasta saldarla) antes de sumar cualquier excedente al saldo disponible.
- **RF-ECO-06**: Si el saldo del jugador llega a 0 o menos, el sistema debe fijarlo en 0.00 e incrementar el contador `veces_sin_fondos`.
- **RF-ECO-07**: El sistema debe ofrecer un préstamo de monto fijo **200.00** con una tasa de interés fija del **20%** (`MONTO_PRESTAMO`, `TASA_INTERES_PRESTAMO`), acreditado al saldo y sumado a la deuda junto con su interés.
- **RF-ECO-08**: Si la deuda del jugador alcanza o supera el límite de **1000.00** (`LIMITE_DEUDA_IMPAGABLE`), el sistema debe transicionar al estado de Game Over.
- **RF-ECO-09**: El sistema debe llevar una racha de pérdidas consecutivas, incrementándola en cada ronda con resultado neto negativo y reiniciándola a 0 en cualquier ronda sin pérdida neta.

#### Máquina de estados (`core/estado_juego.c`)

- **RF-EST-01**: El sistema debe centralizar toda transición de pantalla en una única función (`cambiar_estado`), que solo aplica el cambio si la transición está contemplada en la tabla de transiciones válidas (`es_transicion_valida`); toda transición no contemplada debe ignorarse y registrarse como error de diagnóstico.
- **RF-EST-02**: La tecla `ESC` debe cerrar el programa (`exit(0)`) desde **cualquier** estado del sistema, sin pasar por la validación de transiciones.
- **RF-EST-03**: La tecla `S` (terminar sesión) debe estar disponible en cualquier estado de interrupción (Préstamo, Mensaje reflexivo, Checkpoint educativo, Quiz educativo) sin ninguna condición adicional, y en cualquier estado jugable (Ruleta, Tragamonedas, Dados) siempre que la animación del minijuego activo no esté en curso (`!bolita.girando`, `!alguno_girando` de los rodillos, `!dados.girando` respectivamente) — restricción idéntica en los tres juegos, para no permitir cortar una animación a mitad de camino. En ningún caso `S` depende de que el sistema haya detectado una condición de riesgo.
- **RF-EST-04**: El sistema debe recordar, mediante `partida.juego_activo`, a qué minijuego regresar tras resolver cualquier interrupción (Préstamo, Mensaje reflexivo, Checkpoint, Quiz).
- **RF-EST-05**: El sistema debe requerir una pantalla de confirmación explícita ("¿seguís bajo tu responsabilidad?", `ENTER`/`N`) antes de iniciar o reiniciar cualquier minijuego, incluyendo el reinicio tras Game Over o Sesión Terminada.

#### Secuencia de arranque e introducción (`main.c`, `ui/pantallas.c`, `core/estado_juego.c`)

- **RF-INTRO-01**: El sistema debe mostrar, al arrancar, una pantalla de carga de duración fija (`DURACION_CARGA_SEGUNDOS` = 16.0 segundos) antes de continuar a la secuencia de advertencia, independiente del tiempo real que tome cargar los recursos.
- **RF-INTRO-02**: Durante la pantalla de carga, el sistema debe rotar 4 mensajes de concientización en orden fijo según el tiempo transcurrido (no aleatorio), de forma que las 4 se muestren exactamente una vez por cada carga completa.
- **RF-INTRO-03**: Tras la Advertencia y el Propósito, el sistema debe mostrar 3 pantallas adicionales de contexto real antes de habilitar el Menú: "El juego en Latinoamérica" (escala de la industria regional), "El caso de República Dominicana" (bancas de apuestas registradas/estimadas y prevalencia local) y "El costo humano" (cifras de suicidio vinculado a ludopatía y pensamiento suicida asociado a deudas de juego) — cada una citando su fuente directamente en pantalla, no solo en la documentación.
- **RF-INTRO-04**: La secuencia completa de arranque (Carga → Advertencia → Propósito → las 3 pantallas de contexto → Menú) debe ser estrictamente lineal y no salteable por ningún medio salvo cerrar el programa (`ESC`); no existe ninguna transición en `es_transicion_valida()` que permita omitir una pantalla de la secuencia.
- **RF-INTRO-05**: La pantalla "El costo humano", por tratar contenido sobre pensamiento suicida asociado a deudas de juego, debe incluir en el mismo cuerpo del texto (no en una pantalla separada ni solo en la documentación) un puntero explícito a la tecla `I` y a los recursos de ayuda reales disponibles en el país.

#### Ruleta (`ruleta/`, `core/estado_juego.c`)

- **RF-RUL-01**: El sistema debe generar el resultado de cada giro mediante `rand() % 37` sobre `ORDEN_RUEDA_EUROPEA` (37 números, orden de ruleta europea real), de forma verdaderamente independiente entre giros.
- **RF-RUL-02**: El sistema debe permitir apuestas de tipo número pleno (paga 35:1), color (paga 1:1), docena (paga 2:1), paridad (paga 1:1) y mitad (paga 1:1), calculadas por `calcular_ganancia_apuesta`.
- **RF-RUL-03**: El sistema debe permitir seleccionar el monto de la ficha activa entre 4 valores predefinidos (teclas `1`–`4`) antes de colocar una apuesta.
- **RF-RUL-04**: El sistema debe impedir colocar una apuesta si el saldo disponible es menor al monto de la ficha seleccionada.
- **RF-RUL-05**: El sistema debe animar la bolita con una curva de desaceleración (easing, basada en Bézier) hasta detenerse en el sector correspondiente al número ganador ya decidido.
- **RF-RUL-06**: Mientras la bolita gira, el sistema debe mostrar un mensaje educativo no bloqueante, elegido al azar de un banco de 12 mensajes (`MENSAJES_PROBABILIDAD`), enfocado en la falacia del jugador.

#### Tragamonedas (`tragamonedas/`, `core/jugador.c`)

- **RF-TRA-01**: El sistema debe decidir el resultado de los 3 rodillos (`decidir_resultado_tragamonedas`) antes de iniciar la animación de giro, entre 6 símbolos posibles con probabilidad uniforme por rodillo.
- **RF-TRA-02**: El sistema debe pagar un trío (3 símbolos iguales) con un multiplicador que escala de **2x** (Cereza) a **50x** (Siete) según el símbolo (`MULTIPLICADOR_TRIO`), y un par (2 de 3 símbolos iguales, cualquier símbolo) con un multiplicador fijo de **0.5x** — un pago menor al monto apostado, deliberadamente diseñado para exhibir el mecanismo de losses disguised as wins.
- **RF-TRA-03**: Si no hay trío ni par, el sistema debe descontar el monto apostado completo.
- **RF-TRA-04**: El sistema debe impedir iniciar un nuevo giro mientras cualquiera de los 3 rodillos siga en movimiento.
- **RF-TRA-05**: Mientras los rodillos giran, el sistema debe mostrar un mensaje educativo no bloqueante, elegido al azar de un banco de 8 mensajes, enfocado en near-miss y LDW.

#### Dados (`dados/`)

- **RF-DAD-01**: El sistema debe decidir el resultado de ambos dados (`decidir_resultado_dados`, 1 a 6 cada uno) antes de iniciar la animación de lanzamiento.
- **RF-DAD-02**: El sistema debe ofrecer 3 tipos de apuesta con ventaja de casa deliberadamente distinta entre sí: Par/Impar (paga 0.9x neto, ~5% de ventaja), Suma=7 (paga 4.5x neto, ~8.3% de ventaja) y Extremos 2 o 12 (paga 13.0x neto, ~22.2% de ventaja) — calculadas por `calcular_ganancia_dados`.
- **RF-DAD-03**: El sistema debe animar el lanzamiento de ambos dados en dos fases dentro del tiempo total de tiro (`TIEMPO_TIRO` = 4.0 segundos): una fase de giro libre (primer 70% del tiempo) con velocidad angular en decaimiento cuadrático suave, y una fase de asentamiento (30% final) que interpola con ease-out cuadrático desde el ángulo alcanzado al inicio de esa fase hasta el ángulo exacto de la cara ganadora ya decidida, de forma que quede orientada hacia arriba (`+Y`, hacia la cámara) al finalizar (ángulo calculado por un único eje de rotación por valor, 1 a 6, `calcular_angulos_finales`), ajustando el objetivo al camino angular más corto para evitar vueltas de más. Esta convergencia continua reemplaza una versión anterior que fijaba el ángulo final de golpe recién al terminar la animación, generando un salto visual perceptible en vez de un asentamiento natural.
- **RF-DAD-04**: El sistema debe permitir colocar la apuesta mediante las teclas dedicadas (`P` Par, `I` Impar, `7` Suma=7, `E` Extremos) antes de lanzar los dados.

#### Sistema de concientización (`core/jugador.c`, `ui/`)

- **RF-CONC-01**: El sistema debe calcular, en cada frame, un nivel de riesgo (`calcular_nivel_riesgo`) en 4 niveles (0=Bajo, 1=Moderado, 2=Alto, 3=Riesgo de conducta compulsiva), agnóstico del minijuego activo, y mostrarlo siempre visible en el HUD con la aclaración explícita "(simulación educativa, no un diagnóstico real)".
- **RF-CONC-02**: El sistema debe evaluar, después de cada ronda resuelta, hasta 4 disparadores de mensaje reflexivo bloqueante (`verificar_mensaje_reflexivo`): (a) racha de pérdidas consecutivas ≥ 5; (b) monto total apostado, evaluado como hito recurrente cada 2 veces adicionales el saldo inicial de la sesión (se repite en 2x, 4x, 6x..., no solo una vez); (c) el jugador se quedó sin fondos 2 o más veces; y (d) un "reality check" cada 5 rondas jugadas, independiente del resultado, que alterna entre un recordatorio personal de rondas jugadas (ocurrencias impares) y un dato educativo general tomado de un banco de 8 datos específico del minijuego activo (ocurrencias pares) — nunca el mismo banco genérico compartido entre los tres juegos.
- **RF-CONC-03**: El sistema debe mostrar como máximo **un** mensaje reflexivo por ronda resuelta, incluso si más de un disparador se cumple simultáneamente; el disparador no mostrado debe permanecer pendiente para la próxima ronda en que se cumpla.
- **RF-CONC-04**: Cada mensaje reflexivo debe insertar cifras reales de la sesión del jugador (monto apostado, deuda acumulada, número de rondas) en vez de texto genérico.
- **RF-CONC-05**: El sistema debe interrumpir el juego cada 3 rondas jugadas o cada 5 minutos reales de sesión (lo que ocurra primero) con una pantalla de Checkpoint educativo o de Quiz interactivo, alternando entre ambos.
- **RF-CONC-06**: El Quiz debe presentar una pregunta de opción múltiple (A/B/C) con retroalimentación inmediata de correcto/incorrecto y una explicación, extraída de un banco específico por minijuego (12 preguntas en Ruleta, 8 en Tragamonedas, 8 en Dados).
- **RF-CONC-07**: El sistema debe ofrecer una pantalla de Información no interactiva, accesible con la tecla `I` desde el Menú, Game Over o Sesión Terminada, con 8 páginas navegables mediante `ESPACIO`, cubriendo qué es la ludopatía, señales de alerta, mitos por cada minijuego, consecuencias, recursos de ayuda y prevención.
- **RF-CONC-08**: El contenido de cada capa educativa (mensajes de giro, checkpoint, quiz, datos generales) debe ser específico del minijuego activo (`juego_activo`) que lo disparó, nunca genérico compartido entre los tres.
- **RF-CONC-09**: Todo el contenido educativo debe redactarse en formato de autoevaluación (pregunta o afirmación dirigida al jugador), nunca como instrucción u orden directa.

### 3.3. Requisitos de desempeño

- **RNF-PERF-01**: La animación de la bolita de la ruleta, los rodillos del tragamonedas y el lanzamiento de los dados debe percibirse fluida en tiempo real sobre hardware gráfico estándar con soporte de pipeline fijo de OpenGL.
- **RNF-PERF-02**: El bucle de actualización (`idle`) debe calcular el delta de tiempo entre frames para que las animaciones basadas en velocidad angular y easing sean independientes de la tasa de refresco real del hardware.

> **Nota de auditoría — único punto de esta ERS que sigue abierto:** el código actual no define un objetivo explícito de FPS ni un presupuesto de tiempo por frame (no hay limitador de FPS ni V-Sync forzado en el código revisado). No se fabrica aquí un número no respaldado por el código (contradiría la metodología retrospectiva de este documento, ver la nota al inicio); es una **decisión que le corresponde al equipo/docente**, no algo que este documento pueda resolver por sí solo. Si la materia exige una métrica cuantitativa, la recomendación es fijar un mínimo razonable (p. ej. 30 FPS en hardware de referencia) y agregarlo aquí como RNF-PERF-03 una vez ratificado.

### 3.4. Restricciones de diseño

Ya documentadas en detalle como decisiones de arquitectura; se listan aquí como restricciones formales de este documento:

- **RNF-DIS-01**: Todo el código fuente debe cumplir C89 estricto (declaraciones al inicio de bloque, sin `//`, sin VLA) — ver [ADR-001](ARs/ADR-001-c89-estricto-y-glut-clasico.md).
- **RNF-DIS-02**: La única librería de ventana/entrada permitida es GLUT clásico (Win32); no se permite freeGLUT ni librerías equivalentes modernas (SDL, GLFW) — ver [ADR-001](ARs/ADR-001-c89-estricto-y-glut-clasico.md).
- **RNF-DIS-03**: Todo texto renderizado en pantalla con `glutBitmapCharacter` debe ser ASCII plano estricto (7 bits): sin tildes, sin `ñ`, sin `¡¿` — ver [ADR-002](ARs/ADR-002-ascii-plano-estricto.md).
- **RNF-DIS-04**: No se permite importar modelos 3D externos en ningún formato; toda la geometría se construye con primitivas de OpenGL/GLU y curvas de Bézier propias (`utils/bezier.c`); solo se permiten texturas 2D vía `stb_image.h` — ver [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md).
- **RNF-DIS-05**: La plataforma de compilación y ejecución debe ser Windows, x86, con Visual Studio 2022 o compatible.

### 3.5. Atributos del sistema

- **RNF-CAL-01 (Confiabilidad):** toda transición de estado no contemplada por la máquina de estados debe rechazarse de forma segura (sin aplicar el cambio) y registrarse como diagnóstico por `stderr`, en vez de dejar el sistema en un estado visual inconsistente.
- **RNF-CAL-02 (Mantenibilidad):** el código fuente debe seguir el estándar de comentarios documentado en [`guia-documentacion-codigo.md`](guia-documentacion-codigo.md) — comentar el "por qué", no el "qué", con énfasis en invariantes no evidentes (orden de operaciones económicas, composición de rotaciones OpenGL, convenciones de codificación de apuestas).
- **RNF-CAL-03 (Portabilidad):** el sistema es intencionalmente **no portable** fuera de Windows x86, como consecuencia directa de RNF-DIS-02 y RNF-DIS-05 — no se considera un defecto, es una restricción curricular aceptada (ver [ADR-001](ARs/ADR-001-c89-estricto-y-glut-clasico.md)).
- **RNF-CAL-04 (Seguridad):** el sistema no debe tener ninguna ruta de código que conecte el saldo del jugador con dinero real, medios de pago, o cuentas externas; no se recopila ni transmite información personal del usuario; no hay conexión de red.
- **RNF-CAL-05 (Usabilidad):** la salida de la sesión (tecla `S`) debe permanecer accesible sin pasar por ningún menú ni flujo de confirmación adicional, en cualquier estado de interrupción sin excepción, y en cualquier estado jugable tan pronto termina la animación en curso del minijuego activo — nunca oculta detrás de una decisión previa del sistema (ver RF-EST-03).
- **RNF-CAL-06 (Consistencia interna del contenido):** cualquier par de arrays paralelos usados por el sistema de concientización (por ejemplo, títulos y cuerpos de la pantalla de Información) debe mantenerse sincronizado en tamaño; una desincronización no produce error de compilación, produce lectura de memoria inválida en tiempo de ejecución.

### 3.6. Otros requisitos

- **RNF-ETI-01 (Requisito ético/de diseño responsable):** el sistema no debe incluir sistemas de logros, coleccionables, ni ninguna forma de auto-apuesta o auto-giro, por replicar mecanismos de enganche que el proyecto busca desmontar, no reforzar (ver `docs/analisis-previo.md`, sección 1.5).
- **RNF-ETI-02:** el medidor de riesgo debe aclarar siempre, de forma visible, que es una simulación educativa y no un diagnóstico clínico real.
- **RNF-ETI-03:** la pantalla de recursos de ayuda (`ui/pantallas.c`, líneas 878, 881 y 885) debe mostrar información de contacto **verificada contra una fuente directa**, no solo contra un resultado de búsqueda genérico. Estado de verificación de esta revisión (agosto 2026):

  | Organización | Número en el código | Verificación |
  |---|---|---|
  | Fundación Fénix | 809-542-4759 | ✅ Confirmado contra el sitio oficial [fenix.org.do](https://fenix.org.do/) — coincide, y la fundación trata explícitamente ludopatía/juego problemático |
  | Clínica Conductual Volver | 849-856-3789 | ✅ Confirmado contra el sitio oficial [volver.com.do](https://volver.com.do/) — coincide (el sitio también lista un segundo número, 809-534-6002, que no está en el juego; no es un error, solo una línea adicional no incluida) |
  | CAIDEP (Centro de Atención Integral a las Dependencias) | 809-684-2300 | ⚠️ **Sin confirmar.** Se verificó que CAIDEP existe como unidad pública del Servicio Nacional de Salud, operando en el hospital Francisco Moscoso Puello, pero ninguna fuente consultada confirma este número específico. Pendiente: confirmar directamente con el SNS o el Consejo Nacional de Drogas (`consejodedrogasrd.gob.do`) antes de la entrega final |

  Mostrar un número de ayuda incorrecto en un producto de concientización sobre una adicción es un riesgo de daño real, no solo un error de datos — por eso este ítem no se cierra como "hecho" hasta que el tercer número también quede confirmado.

---

## Apéndice A: Matriz de trazabilidad

| Requisito | Función/archivo real | Documento de diseño relacionado |
|---|---|---|
| RF-ECO-01 a RF-ECO-06 | `core/jugador.c` (`inicializar_jugador`, `registrar_apuesta`, `aplicar_resultado_apuesta`) | — |
| RF-ECO-07, RF-ECO-08 | `core/jugador.c` (`pedir_prestamo`, `deuda_es_impagable`), `core/jugador.h` (`MONTO_PRESTAMO`, `TASA_INTERES_PRESTAMO`, `LIMITE_DEUDA_IMPAGABLE`) | — |
| RF-ECO-09 | `core/jugador.c` (`aplicar_resultado_apuesta`, campo `racha_perdidas_consecutivas`) | — |
| RF-EST-01 | `core/estado_juego.c` (`cambiar_estado`, `es_transicion_valida`) | [ADR-004](ARs/ADR-004-maquina-de-estados-centralizada.md) |
| RF-EST-02, RF-EST-03 | `main.c` (`teclado()`, casos `27` y `'s'/'S'`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-EST-04 | `main.c` (campo `partida.juego_activo`) | [ADR-004](ARs/ADR-004-maquina-de-estados-centralizada.md) |
| RF-EST-05 | `core/estado_juego.c` (`ESTADO_CONFIRMACION_JUEGO`) | [ADR-004](ARs/ADR-004-maquina-de-estados-centralizada.md) |
| RF-INTRO-01, RF-INTRO-02 | `main.c` (`idle()`, `DURACION_CARGA_SEGUNDOS`), `ui/pantallas.c` (`dibujar_pantalla_carga`, array `MENSAJES_CARGA[4]`) | — |
| RF-INTRO-03 | `ui/pantallas.c` (`dibujar_pantalla_intro_latinoamerica`, `dibujar_pantalla_intro_rd`, `dibujar_pantalla_intro_costo_humano`) | — |
| RF-INTRO-04 | `core/estado_juego.c` (`es_transicion_valida`, casos `ESTADO_PROPOSITO` a `ESTADO_INTRO_COSTO_HUMANO`), `main.c` (`teclado()`, cadena de `ENTER`) | [ADR-004](ARs/ADR-004-maquina-de-estados-centralizada.md) |
| RF-INTRO-05 | `ui/pantallas.c` (`dibujar_pantalla_intro_costo_humano`, línea con puntero a `[I]`) | — |
| RF-RUL-01, RF-RUL-02 | `core/estado_juego.c` (`ORDEN_RUEDA_EUROPEA`, `calcular_ganancia_apuesta`, `calcular_ganancia_total`) | [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md) |
| RF-RUL-03, RF-RUL-04 | `main.c` (`teclado()`, `saldo_alcanza_para_ficha`) | — |
| RF-RUL-05 | `ruleta/ruleta_animacion.c`, `utils/bezier.c` | [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md) |
| RF-RUL-06 | `ui/hud.c` (`MENSAJES_PROBABILIDAD`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-TRA-01 a RF-TRA-03 | `tragamonedas/tragamonedas_logica.c` (`decidir_resultado_tragamonedas`, `calcular_ganancia_tragamonedas`) | `docs/analisis-previo.md` §1.3 |
| RF-TRA-04 | `main.c` (bloque `ESTADO_TRAGAMONEDAS_JUGANDO` en `teclado()`) | — |
| RF-TRA-05 | `ui/hud_tragamonedas.c:262` (`MENSAJES_GIRO_TRAGAMONEDAS[8]`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-DAD-01, RF-DAD-04 | `dados/dados_logica.c` (`decidir_resultado_dados`), `main.c` (`hacer_apuesta_dados`) | — |
| RF-DAD-02 | `dados/dados_logica.c` (`calcular_ganancia_dados`) | `docs/analisis-previo.md` §1.3 |
| RF-DAD-03 | `dados/dados_animacion.c` (`calcular_angulos_finales`, `actualizar_dados`, campos `fase_asentamiento`/`rot_snap_x/y/z`) | [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md) §Contexto (reemplazo de Bézier por easing propio de 2 fases), `guia-documentacion-codigo.md` §5.2 |
| RF-CONC-01 | `core/jugador.c` (`calcular_nivel_riesgo`) | `docs/analisis-previo.md` §1.5 |
| RF-CONC-02 a RF-CONC-04 | `core/jugador.c` (`verificar_mensaje_reflexivo`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-CONC-05 | `ui/pantallas.c:1306` (`MENSAJES_CHECKPOINT[8]`), `:1340` (`MENSAJES_CHECKPOINT_DADOS[8]`), `:1382` (`MENSAJES_CHECKPOINT_TRAGAMONEDAS[8]`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-CONC-06 | `ui/pantallas.c:1503` (`QUIZ_PREGUNTAS[12]`, `NUM_PREGUNTAS_QUIZ` en `pantallas.h:74`), `:1621` (`QUIZ_PREGUNTAS_DADOS[8]`), `:1688` (`QUIZ_PREGUNTAS_TRAGAMONEDAS[8]`) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-CONC-07 | `ui/pantallas.c:778` (`EDUCACION_TITULOS`), `:789` (`EDUCACION_CUERPOS`), `EDUCACION_NUM_PAGINAS` = 8 en `pantallas.h:54` | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RF-CONC-08, RF-CONC-09 | `core/jugador.c` (parámetro `juego_activo` en `verificar_mensaje_reflexivo`), `ui/pantallas.c`, `ui/hud_dados.c`, `ui/hud_tragamonedas.c` (bancos separados por juego) | [ADR-005](ARs/ADR-005-sistema-concientizacion-en-capas.md) |
| RNF-DIS-01 a RNF-DIS-05 | Transversal a todo `src/` | [ADR-001](ARs/ADR-001-c89-estricto-y-glut-clasico.md), [ADR-002](ARs/ADR-002-ascii-plano-estricto.md), [ADR-003](ARs/ADR-003-modelado-procedural-primitivas-y-bezier.md) |
| RNF-CAL-02 | Transversal a todo `src/` | `guia-documentacion-codigo.md` |
| RNF-ETI-01, RNF-ETI-02 | `docs/analisis-previo.md` §1.5 | `docs/analisis-previo.md` |
| RNF-ETI-03 | `ui/pantallas.c:878,881,885` (números de teléfono) — verificación de fuente directa en la sección 3.6 | — |

Esta matriz se completó y verificó línea por línea contra el código real (`ui/pantallas.c`, `ui/hud_dados.c`, `ui/hud_tragamonedas.c`, `pantallas.h`) — no quedan referencias sin confirmar. El único punto de la ERS que sigue abierto es el objetivo de desempeño (3.3) y la confirmación del teléfono de CAIDEP (3.6, RNF-ETI-03), ambos por ser decisiones que exceden lo que este documento puede resolver por sí solo.
