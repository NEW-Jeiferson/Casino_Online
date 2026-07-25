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
 * gira con desaceleracion tipo Bezier (mismo criterio que la bolita de
 * la ruleta) hasta detenerse exactamente en el simbolo decidido por la
 * logica (Dubenny) -ver el comentario TEMPORAL en
 * tragamonedas_animacion.c sobre decidir_resultado_tragamonedas().
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_ANIMACION_H
#define TRAGAMONEDAS_ANIMACION_H

#define NUM_RODILLOS 3 /* ajustable; 3 es lo mas comun en un tragamonedas simple */

typedef struct {
    /* Posicion del rodillo en "unidades de simbolo" -mismo significado
       que espera obtener_simbolo_en_posicion() en tragamonedas_logica.h
       (0.0 = primer simbolo de la tira centrado, 1.0 = el siguiente,
       ciclando con modulo). Este es el contrato con la parte de logica
       (Dubenny): la parte visual solo avanza este numero, nunca decide
       que simbolo hay -eso se pregunta. */
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
} EstadoTragamonedas;

/* Deja todos los rodillos detenidos, en la posicion inicial */
void inicializar_tragamonedas_animacion(EstadoTragamonedas* estado);

/* Avanza la animacion segun el tiempo real transcurrido (delta_tiempo en
   segundos, mismo patron que actualizar_bolita() en ruleta_animacion.c
   -medir tiempo real con glutGet(GLUT_ELAPSED_TIME) en el idle() de
   main.c, no asumir una tasa de frames fija). */
void actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo);

/* Inicia el giro de los 3 rodillos hacia el resultado decidido para
   esta ronda -mismo criterio que la ruleta: el resultado se decide
   ANTES de animar, la animacion solo se ajusta despues para caer
   visualmente ahi. Si algun rodillo ya esta girando, no hace nada
   (evita reiniciar un giro a mitad de camino). */
void iniciar_giro_tragamonedas(EstadoTragamonedas* estado);

#endif /* TRAGAMONEDAS_ANIMACION_H */
