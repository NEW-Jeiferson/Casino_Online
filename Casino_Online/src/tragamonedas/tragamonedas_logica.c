/*
 * tragamonedas_logica.c
 * Implementacion de las reglas del tragamonedas. Ver tragamonedas_logica.h.
 * A CARGO DE: Dubenny (logica).
 *
 * IMPLEMENTACION COMPLETA:
 * - obtener_simbolo_en_posicion(): ya existia, sin cambios.
 * - decidir_resultado_tragamonedas(): seleccion ponderada de simbolo
 *   por rodillo (CEREZA el mas frecuente, SIETE el jackpot mas raro).
 * - calcular_ganancia_tragamonedas(): tabla de pagos clasica de
 *   tragamonedas de 3 rodillos (ver la tabla en tragamonedas_logica.h).
 */
#include <math.h>
#include <stdlib.h>
#include "tragamonedas_logica.h"

/* Orden de simbolos en la tira del rodillo (se repite en ciclo).
   Contiene exactamente uno de cada simbolo distinto; la probabilidad
   real de aparicion se controla con los pesos de abajo, no repitiendo
   simbolos aqui -eso desacoplaria la logica de la animacion, que
   necesita que cada simbolo aparezca exactamente una vez en la tira
   para poder calcular la posicion objetivo al aterrizar. */
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

    (void)rodillo; /* los 3 rodillos comparten tira por ahora, ver comentario en el .h */

    /* fmodf puede devolver negativo si 'posicion' es negativa; se
       corrige sumando NUM_SIMBOLOS antes del modulo final para que el
       indice siempre caiga en [0, NUM_SIMBOLOS). */
    posicion_ciclada = fmodf(posicion, (float)NUM_SIMBOLOS);
    if (posicion_ciclada < 0.0f) posicion_ciclada += (float)NUM_SIMBOLOS;

    indice = (int)posicion_ciclada;
    if (indice >= NUM_SIMBOLOS) indice = NUM_SIMBOLOS - 1; /* proteccion por redondeo */

    return ORDEN_TIRA_RODILLO[indice];
}


/* -----------------------------------------------------------------------
 * Tabla de probabilidades ponderadas.
 *
 * Los pesos son proporcionales (no hace falta que sumen 100, pero es
 * mas facil razonar sobre probabilidades si se hace). La mecanica de
 * "premios frecuentes pero pequenos + jackpot raro" es la base del
 * diseno clasico de tragamonedas (refuerzo de razon variable -
 * Skinner, 1938; Griffiths, 1993). Se documenta aqui para que quede
 * explicita la intencion pedagogica del serious game: mostrar como
 * la distribucion esta deliberadamente sesgada a favor de la casa.
 *
 * Probabilidad aproximada de cada simbolo por rodillo:
 *   CEREZA    35 %  (el mas comun, da premios chicos frecuentes)
 *   CAMPANA   22 %
 *   HERRADURA 20 %
 *   DIAMANTE  12 %
 *   BARRA      8 %
 *   SIETE      3 %  (jackpot, muy raro - tension y expectativa)
 *
 * Probabilidad de jackpot (3x SIETE): 0.03^3 ~ 0.003 % por tirada.
 * ----------------------------------------------------------------------- */
static const int PESO_SIMBOLO[NUM_SIMBOLOS] = {
    35, /* SIMBOLO_CEREZA    */
    22, /* SIMBOLO_CAMPANA   */
    20, /* SIMBOLO_HERRADURA */
    12, /* SIMBOLO_DIAMANTE  */
     8, /* SIMBOLO_BARRA     */
     3  /* SIMBOLO_SIETE     - jackpot */
    /* Suma = 100 */
};

void decidir_resultado_tragamonedas(SimboloTragamonedas resultado[NUM_RODILLOS]) {
    int i, s, r, acum;
    for (i = 0; i < NUM_RODILLOS; i++) {
        r    = rand() % 100;
        acum = 0;
        resultado[i] = SIMBOLO_CEREZA; /* valor por defecto; no deberia quedar sin asignar */
        for (s = 0; s < NUM_SIMBOLOS; s++) {
            acum += PESO_SIMBOLO[s];
            if (r < acum) {
                resultado[i] = (SimboloTragamonedas)s;
                break;
            }
        }
    }
}

float calcular_ganancia_tragamonedas(const SimboloTragamonedas resultado[NUM_RODILLOS], float monto) {

    /* --- Tres iguales: jackpot o premio mayor --- */
    if (resultado[0] == resultado[1] && resultado[1] == resultado[2]) {
        switch (resultado[0]) {
        case SIMBOLO_SIETE:     return monto * 100.0f;
        case SIMBOLO_DIAMANTE:  return monto * 50.0f;
        case SIMBOLO_BARRA:     return monto * 20.0f;
        case SIMBOLO_CAMPANA:   return monto * 15.0f;
        case SIMBOLO_HERRADURA: return monto * 10.0f;
        case SIMBOLO_CEREZA:    return monto * 5.0f;
        default:                break;
        }
    }

    /* --- Cereza en rodillos 0 y 1 (rodillo 2 libre) --- */
    if (resultado[0] == SIMBOLO_CEREZA && resultado[1] == SIMBOLO_CEREZA) {
        return monto * 2.0f;
    }

    /* --- Cereza solo en rodillo 0 (R1 y R2 sin cereza) --- */
    if (resultado[0] == SIMBOLO_CEREZA) {
        return monto * 1.0f;
    }

    /* --- Sin combinacion pagadora: se pierde el monto apostado --- */
    return -monto;
}
