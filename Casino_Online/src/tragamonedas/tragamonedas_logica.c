/*
 * tragamonedas_logica.c
 * Implementacion de las reglas del tragamonedas. Ver tragamonedas_logica.h.
 * A CARGO DE: Dubenny.
 */
#include <math.h>
#include "tragamonedas_logica.h"

/* Orden de simbolos en la tira de cada rodillo (se repite en ciclo).
   Orden arbitrario de partida -cambiar si conviene para ajustar
   probabilidades (un simbolo que aparece mas veces en la tira es mas
   probable que caiga). */
const SimboloTragamonedas ORDEN_TIRA_RODILLO[NUM_SIMBOLOS] = {
    SIMBOLO_CEREZA,
    SIMBOLO_CAMPANA,
    SIMBOLO_HERRADURA,
    SIMBOLO_DIAMANTE,
    SIMBOLO_BARRA,
    SIMBOLO_SIETE
};

SimboloTragamonedas obtener_simbolo_en_posicion(int rodillo, float posicion) {
    float posicion_ciclada;
    int indice;

    (void)rodillo; /* no se usa todavia -los 3 rodillos comparten tira por ahora, ver comentario en el .h */

    /* fmodf puede devolver negativo si 'posicion' es negativa; se
       corrige sumando NUM_SIMBOLOS antes del modulo final para que el
       indice siempre caiga en [0, NUM_SIMBOLOS). */
    posicion_ciclada = fmodf(posicion, (float)NUM_SIMBOLOS);
    if (posicion_ciclada < 0.0f) posicion_ciclada += (float)NUM_SIMBOLOS;

    indice = (int)posicion_ciclada;
    if (indice >= NUM_SIMBOLOS) indice = NUM_SIMBOLOS - 1; /* proteccion por redondeo */

    return ORDEN_TIRA_RODILLO[indice];
}

/* TODO (Dubenny): decidir_resultado_tragamonedas() y
   calcular_ganancia_tragamonedas() -ver las firmas sugeridas en
   tragamonedas_logica.h. */
