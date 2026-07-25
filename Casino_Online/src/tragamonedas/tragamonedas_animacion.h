/*
 * tragamonedas_animacion.h
 * -----------------------------------------------------------------------
 * Estado y animacion del giro de los rodillos. Misma idea que
 * ruleta_animacion.h (EstadoBolita, easing de desaceleracion) pero para
 * los rodillos del tragamonedas.
 *
 * A CARGO DE: Luis (parte visual, junto con tragamonedas_geometria.c/h).
 *
 * ESTADO ACTUAL: esqueleto minimo, con 3 rodillos como punto de partida
 * (numero ajustable, ver NUM_RODILLOS abajo -no es una decision final,
 * es solo para tener algo concreto sobre lo que iterar). La logica real
 * de easing/desaceleracion todavia no esta escrita -ver los comentarios
 * TODO en tragamonedas_animacion.c. Se puede reusar bastante del
 * enfoque de ruleta_animacion.c (duracion minima/maxima, velocidad
 * inicial calculada para llegar exacto a un simbolo objetivo, no a un
 * angulo cualquiera).
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_ANIMACION_H
#define TRAGAMONEDAS_ANIMACION_H

#define NUM_RODILLOS 3 /* ajustable; 3 es lo mas comun en un tragamonedas simple */

typedef struct {
    /* Posicion del rodillo en "unidades de simbolo" -mismo significado
       que espera obtener_simbolo_en_posicion() en tragamonedas_logica.h
       (0.0 = primer simbolo de la tira centrado, 1.0 = el siguiente,
       ciclando con modulo). Esto es el contrato con la parte de logica
       (Dubenny): la parte visual solo avanza este numero, nunca decide
       que simbolo hay -eso se pregunta. */
    float posicion_actual;
    float velocidad;
    int   girando;
    /* TODO: sumar lo que haga falta para el easing real (duracion total,
       tiempo transcurrido), mismo patron que EstadoBolita en
       ruleta_animacion.h */
} EstadoRodillo;

typedef struct {
    EstadoRodillo rodillos[NUM_RODILLOS];
} EstadoTragamonedas;

/* Deja todos los rodillos detenidos, en la posicion inicial */
void inicializar_tragamonedas_animacion(EstadoTragamonedas* estado);

/* Avanza la animacion segun el tiempo real transcurrido (delta_tiempo en
   segundos, mismo patron que actualizar_bolita() en ruleta_animacion.c
   -medir tiempo real con glutGet(GLUT_ELAPSED_TIME) en el idle() de
   main.c, no asumir una tasa de frames fija). TODO: implementar el
   easing real; por ahora no hace nada. */
void actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo);

/* Inicia el giro de los 3 rodillos hacia el resultado ya decidido por
   decidir_resultado_tragamonedas() (tragamonedas_logica.h) -mismo
   criterio que la ruleta: el resultado se decide ANTES de animar, la
   animacion solo se ajusta despues para caer visualmente ahi. */
void iniciar_giro_tragamonedas(EstadoTragamonedas* estado);

#endif /* TRAGAMONEDAS_ANIMACION_H */
