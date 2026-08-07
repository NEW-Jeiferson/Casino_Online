/*
 * tragamonedas_logica.c
 * Implementacion de las reglas del tragamonedas. Ver tragamonedas_logica.h.
 * A CARGO DE: Dubenny.
 */
#include <math.h>
#include <stdlib.h>
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

void decidir_resultado_tragamonedas(SimboloTragamonedas resultado[NUM_RODILLOS]) {
    int i;

    /* Cada rodillo tira independiente -por ahora los 3 tienen la misma
       probabilidad por simbolo (equivale a ORDEN_TIRA_RODILLO con un
       solo elemento por simbolo). Si mas adelante se quiere una tira
       con simbolos repetidos para pesar probabilidades, este for tiene
       que cambiar para elegir un INDICE DE LA TIRA en vez de un
       SimboloTragamonedas directo -ver comentario en la tira en el .h. */
    for (i = 0; i < NUM_RODILLOS; i++) {
        resultado[i] = (SimboloTragamonedas)(rand() % NUM_SIMBOLOS);
    }
}

float calcular_ganancia_tragamonedas(const SimboloTragamonedas resultado[NUM_RODILLOS], float monto) {
    /* Multiplicador de trio por simbolo, indexado con el propio enum
       SimboloTragamonedas -progresion agresiva de 2x (CEREZA, el
       simbolo mas comun en la tira) a 50x (SIETE, el menos comun). Si
       se agrega/saca un simbolo del enum, este array tiene que crecer
       o achicarse junto con el (por eso el tamano es NUM_SIMBOLOS, no
       un numero a mano). */
    static const float MULTIPLICADOR_TRIO[NUM_SIMBOLOS] = {
        2.0f,  /* SIMBOLO_CEREZA */
        5.0f,  /* SIMBOLO_CAMPANA */
        10.0f, /* SIMBOLO_HERRADURA */
        20.0f, /* SIMBOLO_DIAMANTE */
        30.0f, /* SIMBOLO_BARRA */
        50.0f  /* SIMBOLO_SIETE */
    };
    /* Pago fijo de par: no depende de que simbolo se repita (pagar poco
       y parejo evita que el jugador aprenda a "buscar" un par de un
       simbolo especifico -el trio ya es el premio grande). */
    const float MULTIPLICADOR_PAR = 0.5f;

    int conteo[NUM_SIMBOLOS];
    int i;
    int simbolo_trio;
    int hay_par;

    for (i = 0; i < NUM_SIMBOLOS; i++) {
        conteo[i] = 0;
    }
    for (i = 0; i < NUM_RODILLOS; i++) {
        conteo[(int)resultado[i]]++;
    }

    /* Trio: algun simbolo salio en los NUM_RODILLOS rodillos. Nota: esta
       deteccion (conteo == NUM_RODILLOS para trio, == 2 para par, mas
       abajo) asume NUM_RODILLOS == 3 -si en algun momento se cambia la
       cantidad de rodillos, esta funcion hay que revisarla junto con
       eso. */
    simbolo_trio = -1;
    for (i = 0; i < NUM_SIMBOLOS; i++) {
        if (conteo[i] == NUM_RODILLOS) {
            simbolo_trio = i;
            break;
        }
    }
    if (simbolo_trio != -1) {
        return monto * (MULTIPLICADOR_TRIO[simbolo_trio] - 1.0f);
    }

    /* Par: algun simbolo salio exactamente 2 veces */
    hay_par = 0;
    for (i = 0; i < NUM_SIMBOLOS; i++) {
        if (conteo[i] == 2) {
            hay_par = 1;
            break;
        }
    }
    if (hay_par) {
        return monto * (MULTIPLICADOR_PAR - 1.0f);
    }

    /* Sin combinacion: se pierde el monto entero */
    return -monto;
}

const char* resolver_ronda_tragamonedas(Jugador* jugador, const SimboloTragamonedas resultado[NUM_RODILLOS], float monto) {
    float ganancia = calcular_ganancia_tragamonedas(resultado, monto);
    aplicar_resultado_apuesta(jugador, ganancia);
    return verificar_mensaje_reflexivo(jugador);
}