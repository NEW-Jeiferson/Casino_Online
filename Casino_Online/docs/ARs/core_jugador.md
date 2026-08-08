# Documentación del Módulo: `core/jugador`

## 1. Resumen del módulo
El módulo `core/jugador` (`jugador.c` y `jugador.h`) define y gestiona el estado económico y conductual del jugador dentro del casino. A diferencia de un sistema de jugador convencional, este módulo está diseñado específicamente para un "Serious Game", integrando un fuerte enfoque en la concientización sobre la ludopatía. Además de administrar saldo, préstamos y estadísticas de sesión, monitorea activamente el comportamiento del usuario para generar "reality checks" e identificar signos de juego problemático (como rachas de pérdidas o sobre-apuestas), disparando mensajes reflexivos y calculando un nivel de riesgo de conducta compulsiva.

## 2. Estructuras de datos clave

### `Jugador`
Estructura central que encapsula el estado completo de la sesión de un jugador. 
Contiene las siguientes categorías de campos:
- **Economía base:** `saldo`, `deuda`, `prestamos_activos`.
- **Estadísticas para el HUD:** `total_apostado`, `total_ganado`, `total_perdido`, `veces_sin_fondos`, `interes_acumulado`.
- **Concientización (Memoria interna de ludopatía):** Variables como `saldo_inicial`, `racha_perdidas_consecutivas`, `racha_ya_advertida`, `ultimo_hito_apostado_notificado`, `ultimo_veces_sin_fondos_notificado`, `mensajes_reflexivos_mostrados`, `rondas_jugadas` y `ultima_ronda_notificada`. Estos campos no se muestran en el HUD, sino que sirven para registrar hitos conductuales y evitar que los mensajes reflexivos se repitan innecesariamente ("spam").
- **Simulación de tiempo:** `tiempo_inicio_ms`, que almacena el tiempo exacto en el que inició la sesión (separado de la dependencia gráfica de `GLUT`).

## 3. Funciones públicas

### `inicializar_jugador(Jugador* j, float saldo_inicial, int tiempo_actual_ms)`
- **¿Para qué sirve?** Inicializa la estructura del jugador, estableciendo el saldo inicial y reseteando todas las estadísticas, deudas, contadores de ludopatía y el reloj de sesión.
- **¿Por qué se diseñó de esta manera?** En vez de usar librerías gráficas internas para tomar el tiempo actual, exige `tiempo_actual_ms` como parámetro. Esto desvincula la lógica base de dependencias externas (como `glutGet`), garantizando su testeabilidad y evitando que el juego inicie con un reloj desincronizado. 
- **¿Cómo lo hace internamente?** Establece los valores iniciales de cada campo en `0` o `0.0f` correspondientemente. Asigna el `saldo_inicial` tanto en el `saldo` actual como en el campo `saldo_inicial` (el cual es crucial como base de cálculo para determinar hitos de riesgo de ludopatía más adelante).

### `registrar_apuesta(Jugador* j, float monto)`
- **¿Para qué sirve?** Contabiliza de inmediato una apuesta apenas se coloca una ficha.
- **¿Por qué se diseñó de esta manera?** Se ejecuta en el momento del clic o la colocación de la ficha, *antes* de que el giro termine. Esto asegura que el HUD refleje instantáneamente cuánto dinero está en riesgo, mejorando la transparencia visual y forzando al jugador a ser consciente de su gasto en tiempo real.
- **¿Cómo lo hace internamente?** Suma directamente el `monto` a la variable `total_apostado`.

### `anular_apuesta(Jugador* j, float monto)`
- **¿Para qué sirve?** Resta un monto del total apostado si el jugador deshace una ficha antes de confirmar el giro.
- **¿Por qué se diseñó de esta manera?** Da flexibilidad para corregir el HUD sin afectar el `saldo`, ya que el dinero de la ficha no fue descontado hasta resolver la ronda, solo había sido contabilizado estadísticamente.
- **¿Cómo lo hace internamente?** Resta el `monto` de `total_apostado` e incluye una barrera de seguridad que garantiza que el total apostado nunca baje de `0.0f`.

### `aplicar_resultado_apuesta(Jugador* j, float ganancia)`
- **¿Para qué sirve?** Aplica las consecuencias financieras y conductuales cuando se resuelve la ronda.
- **¿Por qué se diseñó de esta manera?** Diseñado bajo mecánicas de riesgo, tiene una regla punitiva: la ganancia nunca va directo al bolsillo si hay deudas previas (una mecánica típica en casinos reales). Además, evalúa directamente si este resultado califica como "pérdida" para alimentar el sistema que previene el juego problemático ("perseguir las pérdidas").
- **¿Cómo lo hace internamente?**
  1. Registra independientemente la métrica en `total_ganado` o `total_perdido`.
  2. Si hubo ganancia y el jugador tiene `deuda`, desvía obligatoriamente los fondos para pagar la deuda primero. Solo el excedente (si lo hay) llega al `saldo`.
  3. Modifica el `saldo` disponible (con protección al llegar a cero, sumando una vez en `veces_sin_fondos`).
  4. Actualiza la `racha_perdidas_consecutivas`. Si hay pérdida neta, suma 1; si no, resetea la racha y habilita nuevas advertencias.
  5. Suma 1 a `rondas_jugadas` siempre, como métrica neutral.

### `pedir_prestamo(Jugador* j, float monto, float tasa_interes)`
- **¿Para qué sirve?** Otorga fondos de emergencia al jugador a cambio de incrementar su deuda con intereses usureros.
- **¿Por qué se diseñó de esta manera?** Es una trampa narrativa y mecánica intencional del "Serious Game". Sirve para simular cómo los jugadores intentan recuperar el dinero prestado, escalando sus problemas hasta llegar a un estado de deuda impagable.
- **¿Cómo lo hace internamente?** Calcula el interés (`monto * tasa_interes`). Suma `monto` al `saldo` disponible, pero incrementa `deuda` y el historial de `interes_acumulado`. Finalmente suma +1 a `prestamos_activos`.

### `deuda_es_impagable(const Jugador* j, float limite_deuda)`
- **¿Para qué sirve?** Evalúa si el jugador alcanzó el punto de "Game Over" económico.
- **¿Por qué se diseñó de esta manera?** Para forzar una condición de derrota si el jugador asume un comportamiento financiero insostenible, enseñando el riesgo destructivo de la escalada de deudas.
- **¿Cómo lo hace internamente?** Devuelve `1` (verdadero) si la deuda es mayor o igual al límite configurado; en caso contrario, `0`.

### `verificar_mensaje_reflexivo(Jugador* j, int juego_activo)`
- **¿Para qué sirve?** Es el núcleo de concientización. Analiza el comportamiento del jugador en cada ronda para disparar advertencias dinámicas basadas en patrones de riesgo.
- **¿Por qué se diseñó de esta manera?** Siguiendo evidencia de investigaciones (McGivern et al. 2019, Wohl et al.), este sistema evita los mensajes genéricos. Usa los datos *reales* y *personalizados* de la sesión del jugador (cuánto apostó en relación a lo que trajo, cuánto debe, cuántas rondas seguidas perdió). Además, está en formato de autoevaluación (preguntas dirigidas) y cuenta con un disparador neutral cada 5 rondas para hacer un "Reality Check" intercalado con trivias clínicas del juego para evitar fatiga. Todo esto busca generar fricción cognitiva ("romper la hipnosis de la pantalla").
- **¿Cómo lo hace internamente?** 
  Evalúa condiciones en orden de prioridad. Si una se cumple (y no había sido ya advertida en esa misma racha o tramo):
  1. Si lleva >= 5 derrotas seguidas (persiguiendo pérdidas).
  2. Si se quedó sin fondos 2 o más veces.
  3. Si ya apostó un múltiplo superior de su `saldo_inicial` original.
  4. Disparador neutral: Cada 5 rondas (Reality check), alterna entre una alerta neutral de conteo de rondas jugadas y un dato educativo clínico sobre la ludopatía o falacias matemáticas específicas para el tipo de `juego_activo` actual (Ruleta, Tragamonedas, Dados).
  Formatea las cadenas con los datos reales usando `sprintf_s` sobre un buffer estático de memoria y devuelve un puntero a este mensaje. Devuelve `NULL` si no hay alerta por mostrar.

### `calcular_nivel_riesgo(const Jugador* j, int tiempo_actual_ms)`
- **¿Para qué sirve?** Calcula un indicador entre 0 y 3 (Bajo, Moderado, Alto, Riesgo de Conducta Compulsiva) sobre el nivel de riesgo del comportamiento actual del usuario.
- **¿Por qué se diseñó de esta manera?** Es una métrica viva diseñada para poder acoplarse visualmente en un HUD (como un termómetro). Funciona pesando numéricamente múltiples factores de riesgo clínicos en un sistema de puntaje aditivo.
- **¿Cómo lo hace internamente?** 
  Suma "puntos" de penalización por comportamientos de alerta:
  - Rachas de pérdidas repetitivas (+1, +1).
  - Quedarse en bancarrota múltiples veces (+2, +2).
  - Pedir préstamos múltiples veces (+2, +2).
  - Mantener deudas activas (+1).
  - Exceder múltiplos grandes de tiempo (>= 15 mins) o dinero (1.5x o 3x saldo original).
  - Haber perdido porcentajes masivos del capital (60% o 90% del inicial) (+2, +2).
  Convierte este total en niveles: <=2 (Nivel 1), <=4 (Nivel 2), >4 (Nivel 3).

### `tiempo_jugado_minutos(const Jugador* j, int tiempo_actual_ms)`
- **¿Para qué sirve?** Retorna la cantidad de tiempo en minutos fraccionales desde el inicio de sesión.
- **¿Por qué se diseñó de esta manera?** De nuevo, evita la dependencia a librerías de tiempo externas recibiendo `tiempo_actual_ms` como argumento.
- **¿Cómo lo hace internamente?** Realiza la resta entre `tiempo_actual_ms` y `j->tiempo_inicio_ms`, divide por `60000.0f` y retorna el valor en formato flotante (evitando números negativos).

## 4. Dependencias
Este módulo de núcleo lógico es deliberadamente independiente de subsistemas gráficos o motores (como `GLUT` o `OpenGL`). Sus dependencias exclusivas son:
- `<stddef.h>`: Utilizado para la constante `NULL` de retornos vacíos en advertencias reflexivas.
- `<stdio.h>`: Específicamente usado para la función `sprintf_s`, vital para inyectar dinámicamente cifras reales en los strings del buffer de advertencias.
- Las constantes de diseño económico `MONTO_PRESTAMO`, `TASA_INTERES_PRESTAMO` y `LIMITE_DEUDA_IMPAGABLE` están definidas en la propia cabecera (`jugador.h`).
