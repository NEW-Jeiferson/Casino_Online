# Sistema de Concientización y Consecuencias: Un Enfoque Académico en el Diseño de Serious Games

## 1. Fundamentos Teóricos y Alineación con el Requisito No Funcional Primario (RNF-1)

El presente documento expone la arquitectura conceptual y matemática del "Sistema de Concientización y Consecuencias" implementado en el Serious Game de Casino Online (desarrollado en ANSI C89 y OpenGL). Este módulo se subordina rigurosamente al Requisito No Funcional primario de la aplicación: **servir como una herramienta educativa e intervencionista para combatir la ludopatía (juego patológico)**. 

A diferencia de los videojuegos comerciales orientados a maximizar el tiempo de retención y la monetización mediante ciclos de dopamina, este sistema invierte deliberadamente estas prácticas. Aplica técnicas de fricción cognitiva, refuerzo negativo y deconstrucción de la falacia del jugador para simular la degradación psicológica y financiera intrínseca a la adicción al juego.

## 2. Modelado Matemático de la Degeneración Financiera y Psicológica

Para representar fielmente la espiral destructiva del juego compulsivo, se ha implementado un sistema determinista de evaluación de riesgo y acumulación de deuda, diseñado explícitamente para garantizar la insostenibilidad a largo plazo.

### 2.1. Sistema Discreto de Evaluación de Riesgo (Risk Meter)

El estado de vulnerabilidad del jugador no se modela como una función continua, sino mediante un sistema estricto de acumulación discreta de puntos de riesgo basado en umbrales absolutos. La función `calcular_nivel_riesgo()` (ubicada en `core/jugador.c`, línea 98) evalúa iterativamente el estado de la estructura `Jugador` sumando puntos fijos:

*   **Rachas Negativas:** Se suma 1 punto si `racha_perdidas_consecutivas >= 3`, y 1 punto adicional si `racha_perdidas_consecutivas >= 5`.
*   **Agotamiento de Liquidez:** Quedarse sin fondos (`veces_sin_fondos >= 1`) suma 2 puntos, y reincidir (`veces_sin_fondos >= 2`) suma 2 adicionales.
*   **Dependencia Financiera:** Mantener `prestamos_activos >= 1` suma 2 puntos, escalando con 2 puntos extra si son `>= 2`. Tener cualquier `deuda > 0.0f` aporta 1 punto.
*   **Compulsión al Riesgo (Volumen Apostado):** Si el `total_apostado` supera el 150% del saldo inicial, suma 1 punto; si supera el 300%, suma 1 punto adicional.
*   **Fijación Temporal:** Jugar ininterrumpidamente durante 15 minutos o más (evaluado vía `glutGet(GLUT_ELAPSED_TIME)`) suma 1 punto.
*   **Pérdida Severa de Capital:** Si el saldo cae por debajo del 40% del `saldo_inicial`, suma 2 puntos; caer por debajo del 10% suma 2 puntos extra.

El sumatorio total clasifica al jugador en 4 estratos inflexibles de intervención:
- **0 puntos:** Nivel 0 (Sin riesgo detectable).
- **1 a 2 puntos:** Nivel 1 (Riesgo Bajo).
- **3 a 4 puntos:** Nivel 2 (Riesgo Moderado).
- **5 o más puntos:** Nivel 3 (Riesgo Alto/Compulsivo), desencadenando la máxima fricción en el sistema.

### 2.2. Límite de Ruina Matemática y Deuda Estructural

El sistema simula el crédito usurero al que recurren los ludópatas. Contrario a la asunción de un interés compuesto gradual, la aplicación impone un choque de deuda inmediato y lineal en cada transacción (ver función `pedir_prestamo` en `core/jugador.c`, línea 86).

Cada vez que el jugador carece de fondos para cubrir una apuesta, el orquestador (`verificar_fondos_y_pedir_prestamo_si_hace_falta` en `main.c`) inyecta un préstamo fijo definido por las constantes de `core/jugador.h` (línea 16):

$$ D_{nueva} = D_{actual} + (MONTO\_PRESTAMO) + (MONTO\_PRESTAMO \times TASA\_INTERES\_PRESTAMO) $$

Con $MONTO\_PRESTAMO = 200.0f$ y $TASA\_INTERES\_PRESTAMO = 0.20f$ (20% de interés de usura fijo por evento).

El **Límite de Ruina Absoluta** está hardcodeado incondicionalmente en el macro `LIMITE_DEUDA_IMPAGABLE 1000.0f`. Una vez que la deuda acumulada alcanza o supera este umbral (evaluado por la función `deuda_es_impagable`), el sistema detecta la asíntota financiera y activa el `ESTADO_GAME_OVER` definitivo.

## 3. Conclusión Analítica

La arquitectura algorítmica del "Sistema de Concientización y Consecuencias" no busca el entretenimiento, sino la deconstrucción empírica de la ludopatía. Mediante la aplicación de un estricto medidor de riesgo y la inyección lineal e inmediata de deuda usurera sin posibilidad de recuperación gradual, el software cumple su objetivo educativo primario. Se demuestra matemáticamente a través de los límites duros del sistema que se trata de un entorno no ergódico y determinista hacia la ruina, donde la única decisión racional (el óptimo global) es cesar el juego antes de alcanzar el estado de bancarrota irrecuperable.
