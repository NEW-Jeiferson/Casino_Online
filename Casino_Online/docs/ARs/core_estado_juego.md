# Documentación del Módulo: core/estado_juego

## 1. Resumen del módulo
El módulo `estado_juego` (compuesto por `estado_juego.c` y `estado_juego.h`) es el núcleo lógico del flujo de este "Serious Game" tipo casino. Se encarga de dos responsabilidades principales:
1. **Máquina de estados global**: Gestiona en qué pantalla o fase se encuentra el juego en todo momento (menú, jugando, pantalla de préstamo, mensajes educativos, game over, etc.), imponiendo reglas estrictas sobre qué transiciones están permitidas.
2. **Sistema de apuestas y lógica de ruleta**: Provee las constantes de una ruleta europea real, funciones para deducir propiedades de un número ganador (color, docena, par/impar, mitad) y la evaluación matemática de las apuestas de los jugadores (cálculo de ganancias/pérdidas).

El diseño prioriza la robustez, evitando transiciones inválidas que podrían derivar en estados inconsistentes, y encapsula toda la lógica de validación matemática (C89).

## 2. Estructuras de datos clave

* **`ColorRuleta` (enum)**: Define los colores posibles de un número en la ruleta (`COLOR_ROJO`, `COLOR_NEGRO`, `COLOR_VERDE`).
* **`TipoApuesta` (enum)**: Clasifica el tipo de apuesta realizada (`APUESTA_NUMERO`, `APUESTA_COLOR`, `APUESTA_DOCENA`, `APUESTA_PAR_IMPAR`, `APUESTA_MITAD`).
* **`Apuesta` (struct)**: Almacena los datos de una apuesta individual:
  * `tipo` (TipoApuesta): La modalidad de la apuesta.
  * `valor` (int): El valor específico apostado (dependiendo del tipo, ej. 0-36, un ColorRuleta, 1-3 para docena, etc.).
  * `monto` (float): Cantidad apostada.
* **`EstadoJuego` (enum)**: Define todos los estados posibles en la máquina de estados. Incluye fases de carga (`ESTADO_CARGA`), de juego real (`ESTADO_JUGANDO`, `ESTADO_TRAGAMONEDAS_JUGANDO`, `ESTADO_DADOS_JUGANDO`), cierres de sesión (`ESTADO_GAME_OVER`, `ESTADO_SESION_TERMINADA`) y los vitales estados del propósito educativo del juego (`ESTADO_EDUCACION`, `ESTADO_PRESTAMO`, `ESTADO_MENSAJE_REFLEXIVO`, `ESTADO_CHECKPOINT_EDUCATIVO`, `ESTADO_QUIZ_EDUCATIVO`, `ESTADO_ADVERTENCIA`, etc.).

## 3. Funciones públicas

A continuación, se detalla cada función pública expuesta en `estado_juego.h`.

### `color_de_numero`
* **¿Para qué funciona/sirve?** Determina el color de un número específico (0 al 36) según el estándar de una ruleta europea.
* **¿Por qué se diseñó de esta manera?** En lugar de usar algoritmos complejos basados en posiciones alternadas, usa una tabla de búsqueda (lookup table) para los números rojos fijos. Esta decisión en C89 es altamente eficiente, clara, y minimiza errores lógicos.
* **¿Cómo lo hace internamente?** Comprueba primero si el número es 0 (devuelve `COLOR_VERDE`). Si no lo es, itera a través de un arreglo estático de números rojos (`NUMEROS_ROJOS`); si hay coincidencia, devuelve `COLOR_ROJO`, en caso contrario asume `COLOR_NEGRO`.

### `docena_de_numero`
* **¿Para qué funciona/sirve?** Devuelve a qué docena pertenece un número ganador (1: 1-12, 2: 13-24, 3: 25-36) o 0 si el número es 0.
* **¿Por qué se diseñó de esta manera?** Separa la validación de grupos de números en una función pura que no depende de ningún estado externo, cumpliendo con la necesidad de tener evaluaciones de apuestas deterministas.
* **¿Cómo lo hace internamente?** Usa simples comparaciones con condicionales `if` secuenciales: filtra el rango inválido y el 0, y luego verifica `<= 12` (primera), `<= 24` (segunda), cayendo por descarte en la tercera.

### `numero_es_par`
* **¿Para qué funciona/sirve?** Indica si un número dado es par (1) o impar (0). 
* **¿Por qué se diseñó de esta manera?** Mantiene aislada una regla clásica de la ruleta: el número 0 no cuenta ni como par ni como impar para efectos de las apuestas, requiriendo un manejo especial que es preferible encapsular.
* **¿Cómo lo hace internamente?** Evalúa una expresión booleana: el número debe ser diferente de 0 y su módulo entre 2 (`numero % 2`) debe ser 0.

### `mitad_de_numero`
* **¿Para qué funciona/sirve?** Determina en qué mitad del tablero se encuentra un número (1: 1-18, 2: 19-36). Si es el 0, devuelve 0.
* **¿Por qué se diseñó de esta manera?** Proporciona la evaluación para la apuesta conocida como "Falta / Pasa" (Low/High), manteniéndose al igual que el resto como una función pura y aislada.
* **¿Cómo lo hace internamente?** Primero descarta números inválidos y el 0 devolviendo 0. Luego, mediante un operador ternario, si es menor o igual a 18 devuelve 1; en caso contrario, devuelve 2.

### `calcular_ganancia_apuesta`
* **¿Para qué funciona/sirve?** Calcula la ganancia o pérdida neta que genera una apuesta específica frente a un número ganador.
* **¿Por qué se diseñó de esta manera?** En el diseño antiguo o básico se solían descontar y sumar fondos en diferentes pasos; aquí, la función devuelve directamente la "variación neta" del saldo. Si gana, devuelve `monto * payout`; si pierde, `-monto`. Además, corrige bugs antiguos (como la comprobación par/impar) centralizando aquí los multiplicadores (payouts) para cada modalidad en un simple bloque de evaluación.
* **¿Cómo lo hace internamente?** Realiza un bloque `switch` basado en el `tipo` de la apuesta. En cada caso (`APUESTA_NUMERO`, `APUESTA_COLOR`, etc.), compara el `valor` apostado con las funciones derivadas (`color_de_numero`, `numero_es_par`, etc.). Un operador ternario decide si sumar la multiplicación correspondiente (e.g. x35 para pleno) o devolver el monto en negativo.

### `calcular_ganancia_total`
* **¿Para qué funciona/sirve?** Recibe un arreglo de apuestas activas y suma las variaciones (ganancias o pérdidas) de todas ellas, arrojando el balance final de la ronda de un jugador.
* **¿Por qué se diseñó de esta manera?** Evita realizar múltiples actualizaciones o "parpadeos" en el saldo de un jugador y permite manejar un número máximo de apuestas (`MAX_APUESTAS`) de forma unificada. 
* **¿Cómo lo hace internamente?** Declara una variable acumuladora (iniciada en 0.0f) e itera usando un bucle `for` convencional de C89 a través del array de `Apuesta`, invocando a `calcular_ganancia_apuesta` por cada índice y sumando el resultado.

### `inicializar_estado_juego`
* **¿Para qué funciona/sirve?** Establece el estado de arranque inicial del sistema cuando comienza la ejecución.
* **¿Por qué se diseñó de esta manera?** Es una práctica estándar obligar al juego a iniciar de forma determinista para arrancar limpiamente, en este caso en la pantalla de carga inicial.
* **¿Cómo lo hace internamente?** Simplemente asigna el valor `ESTADO_CARGA` a la variable global externa `estado_actual`.

### `cambiar_estado`
* **¿Para qué funciona/sirve?** Es la única forma permitida para modificar el estado global del juego (transición de un estado a otro).
* **¿Por qué se diseñó de esta manera (máquina de estados estricta y prevención de transiciones inválidas)?** Al tratarse de un "Serious Game" (donde entran en juego interrupciones obligatorias como mensajes de concientización, quizes educativos o préstamos simulados para ilustrar ludopatía), saltar arbitrariamente a pantallas incorrectas destruiría la narrativa o dejaría el juego en un estado inestable. La función actúa como un guardián de la integridad estructural y del diseño lógico del proyecto.
* **¿Cómo lo hace internamente?** Llama internamente a una función estática y encapsulada `es_transicion_valida(actual, nuevo)`. Esa función evalúa mediante un gran `switch` todas las transiciones permitidas (ej. no se puede ir a `GAME_OVER` sin pasar por la fase educativa pertinente). Si `es_transicion_valida` devuelve 0 (falso), `cambiar_estado` imprime un mensaje de error por error estándar utilizando `fprintf(stderr, ...)` y simplemente interrumpe la ejecución con un `return`, protegiendo la variable `estado_actual` de cambios ilegales.

## 4. Dependencias

* **Internas al Módulo:** El archivo `estado_juego.c` depende íntimamente de la definición de las estructuras, constantes y enumeraciones (como `EstadoJuego`, `TipoApuesta`, etc.) que él mismo expone en su cabecera `estado_juego.h`.
* **Standard C Library:** Incluye únicamente `<stdio.h>` para poder emitir mensajes de error o advertencia en caso de presentarse comportamientos inesperados (por ejemplo, mediante la función `fprintf(stderr, ...)` para transiciones de estados que no son válidas).
* **Compatibilidad Estándar:** Todo el código implementado adhiere de forma estricta a los parámetros del estándar ANSI C (C89/C90). Esto se evidencia en la declaración de variables al principio de cada bloque delimitado, en el uso extensivo de constantes globales para configuraciones sin estado dinámico y arreglos tipados fijos, contribuyendo a la estabilidad y portabilidad del proyecto.
