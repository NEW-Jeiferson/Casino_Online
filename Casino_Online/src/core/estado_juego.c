/*
 * estado_juego.c
 * Implementacion de la maquina de estados. Ver estado_juego.h.
 */

#include "estado_juego.h"
#include <stdio.h>


/* Estado global actual (definido en estado_juego.c) */
EstadoJuego estado_actual;


/* Metodo encargado de inicializar el estado del juego */
void inicializar_estado_juego(void) {
    estado_actual = ESTADO_MENU;
}


/* Metodo que devuelve el orden fijo de los numeros en la rueda de la ruleta europea */
const int ORDEN_RUEDA_EUROPEA[37] = {
    0, 32, 15, 19, 4, 21, 2, 25, 17, 34, 6, 27, 13, 36, 11, 30, 8, 23,
    10, 5, 24, 16, 33, 1, 20, 14, 31, 9, 22, 18, 29, 7, 28, 12, 35, 3, 26
};



/* Metodo que devuelve los numeros rojos en una ruleta europea real */
static const int NUMEROS_ROJOS[18] = {
    1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
};



/* Metodo que determina el color de un numero especifico */
ColorRuleta color_de_numero(int numero) {
    int i;
    if (numero == 0) return COLOR_VERDE;
    for (i = 0; i < 18; i++) {
        if (NUMEROS_ROJOS[i] == numero) return COLOR_ROJO;
    }
    return COLOR_NEGRO;
}


/* Metodo que determina si una transicion entre estados es valida */
static int es_transicion_valida(EstadoJuego actual, EstadoJuego nuevo) {
    if (actual == nuevo) return 1; /* quedarse en el mismo estado siempre es valido */


    /* Swicht encargado de manejar las transiciones validas */
    switch (actual) {
    case ESTADO_MENU:
        return nuevo == ESTADO_JUGANDO;

    case ESTADO_JUGANDO:
        return nuevo == ESTADO_PRESTAMO || nuevo == ESTADO_MENSAJE_REFLEXIVO;

    case ESTADO_MENSAJE_REFLEXIVO:

        return nuevo == ESTADO_JUGANDO || nuevo == ESTADO_SESION_TERMINADA;

    case ESTADO_PRESTAMO:

        return nuevo == ESTADO_JUGANDO || nuevo == ESTADO_GAME_OVER || nuevo == ESTADO_SESION_TERMINADA;

    case ESTADO_GAME_OVER:
        return nuevo == ESTADO_JUGANDO;

    case ESTADO_SESION_TERMINADA:
        return nuevo == ESTADO_JUGANDO;

    default:
        return 0;
    }
}


/* Metodo encargado de cambiar el estado del juego */
void cambiar_estado(EstadoJuego nuevo_estado) {
    if (!es_transicion_valida(estado_actual, nuevo_estado)) {

        fprintf(stderr,
            "cambiar_estado: transicion invalida (%d -> %d) ignorada\n",
            (int)estado_actual, (int)nuevo_estado);
        return;
    }
    estado_actual = nuevo_estado;
}


/* Metodo que determina la docena a la que pertenece un numero especifico */
int docena_de_numero(int numero) {
    if (numero <= 0 || numero > 36) return 0;
    if (numero <= 12) return 1;
    if (numero <= 24) return 2;
    return 3;
}


/* Metodo que determina si un numero es par */
int numero_es_par(int numero) {
    return (numero != 0) && (numero % 2 == 0);
}



/* Metodo que determina la mitad a la que pertenece un numero especifico */
int mitad_de_numero(int numero) {
    if (numero <= 0 || numero > 36) return 0;
    return (numero <= 18) ? 1 : 2;
}


/* Metodo que calcula la ganancia/perdida de una apuesta específica */
float calcular_ganancia_apuesta(const Apuesta* apuesta, int numero_ganador) {
    switch (apuesta->tipo) {
    case APUESTA_NUMERO:
        return (apuesta->valor == numero_ganador) ? apuesta->monto * 35.0f : -apuesta->monto;

    case APUESTA_COLOR:
        return (numero_ganador != 0 && apuesta->valor == (int)color_de_numero(numero_ganador))
            ? apuesta->monto : -apuesta->monto;

    case APUESTA_DOCENA:
        return (docena_de_numero(numero_ganador) == apuesta->valor)
            ? apuesta->monto * 2.0f : -apuesta->monto;

    case APUESTA_PAR_IMPAR:

        return (numero_ganador != 0 && numero_es_par(numero_ganador) != apuesta->valor)
            ? apuesta->monto : -apuesta->monto;

    case APUESTA_MITAD:
        return (mitad_de_numero(numero_ganador) == apuesta->valor)
            ? apuesta->monto : -apuesta->monto;

    default:
        return -apuesta->monto;
    }
}



/* Metodo que calcula la ganancia/perdida total de un conjunto de apuestas */
float calcular_ganancia_total(const Apuesta apuestas[], int cantidad, int numero_ganador) {
    float total = 0.0f;
    int i;
    for (i = 0; i < cantidad; i++) {
        total += calcular_ganancia_apuesta(&apuestas[i], numero_ganador);
    }
    return total;
}