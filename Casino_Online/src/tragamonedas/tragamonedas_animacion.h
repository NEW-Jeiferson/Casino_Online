/*
 * tragamonedas_animacion.h
 * -----------------------------------------------------------------------
 * Estado y animacion del giro de los rodillos. Mismo enfoque que
 * ruleta_animacion.h (EstadoBolita, easing de desaceleracion con
 * Bezier) pero para los 3 rodillos del tragamonedas.
 *
 * A CARGO DE: Luis (parte visual, junto con tragamonedas_geometria.c/h).
 *
 * ESTADO ACTUAL: PASO 4 (animacion real) implementado. Cada rodillo
 * gira con desaceleracion tipo Bezier hasta detenerse exactamente en
 * el simbolo decidido por la logica (ya implementada en logica.c).
 *
 * ACTUALIZACION: EstadoTragamonedas extendido con campos de resultado
 * (resultado_actual, hay_ganancia, ganancia_ultima, tiempo_desde_parada,
 * monto_apuesta) para la animacion de victoria y la futura integracion
 * con core/jugador.h. Este header incluye tragamonedas_logica.h para
 * poder usar SimboloTragamonedas en esos campos.
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_ANIMACION_H
#define TRAGAMONEDAS_ANIMACION_H

#include "tragamonedas_logica.h"
typedef struct {
    /* Posicion del rodillo en "unidades de simbolo" -mismo significado
       que espera obtener_simbolo_en_posicion() en tragamonedas_logica.h
       (0.0 = primer simbolo de la tira centrado, 1.0 = el siguiente,
       ciclando con modulo). Este es el contrato con la parte de logica:
       la parte visual solo avanza este numero, nunca decide que simbolo
       hay -eso se pregunta. */
    float posicion_actual;
    float velocidad;
    int   girando;

    /* Estado interno del easing (mismo patron que EstadoBolita en
       ruleta_animacion.h) */
    float velocidad_inicial;
    float duracion_total;
    float tiempo_transcurrido;

    /* Posicion absoluta (no ciclada) a la que el rodillo tiene que
       llegar exacto cuando termine de girar -se calcula una sola vez
       en iniciar_giro_tragamonedas() y se usa para "clavar" el valor
       final sin arrastre de error de punto flotante. */
    float posicion_objetivo;
} EstadoRodillo;

typedef struct {
    EstadoRodillo rodillos[NUM_RODILLOS];

    /* --- Resultado de la tirada ---
       Se llenan en iniciar_giro_tragamonedas() antes de animar, con
       decidir_resultado_tragamonedas() de tragamonedas_logica.c. La
       geometria puede leerlos para saber que simbolos mostrar al
       detenerse, y para efectos visuales de victoria. */
    SimboloTragamonedas resultado_actual[NUM_RODILLOS];

    /* --- Estado post-parada ---
       todos_detenidos pasa a 1 UNA SOLA VEZ por giro, el frame en que
       el ultimo rodillo se detiene. Se resetea a 0 al iniciar un nuevo
       giro. Permite a la geometria saber el exacto momento en que
       mostrar el resultado final y arrancar la animacion de premio. */
    int   todos_detenidos;

    /* Resultado economico de la ultima tirada, calculado al detenerse
       todos los rodillos via calcular_ganancia_tragamonedas(). */
    int   hay_ganancia;         /* 1 si la tirada pago algo, 0 si no */
    float ganancia_ultima;      /* ganancia neta: positiva o -monto si perdio */
    float monto_apuesta;        /* monto apostado en esta tirada */

    /* Segundos transcurridos desde que el ultimo rodillo se detuvo.
       La geometria lo usa para efectos con duracion definida (ej.
       parpadeo de luces de victoria durante 5 segundos). */
    float tiempo_desde_parada;
} EstadoTragamonedas;

/* Deja todos los rodillos detenidos, en la posicion inicial */
void inicializar_tragamonedas_animacion(EstadoTragamonedas* estado);

/* Avanza la animacion segun el tiempo real transcurrido (delta_tiempo en
   segundos, mismo patron que actualizar_bolita() en ruleta_animacion.c).
   Detecta automaticamente cuando todos los rodillos se detienen. */
void actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo);

/* Inicia el giro de los 3 rodillos hacia el resultado decidido para
   esta ronda. El resultado se decide ANTES de animar con
   decidir_resultado_tragamonedas() (ya implementada en logica.c).
   Si algun rodillo ya esta girando, no hace nada. */
void iniciar_giro_tragamonedas(EstadoTragamonedas* estado);

#endif /* TRAGAMONEDAS_ANIMACION_H */
