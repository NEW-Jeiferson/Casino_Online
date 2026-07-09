/*
 * estado_juego.c
 * Implementacion de la maquina de estados. Ver estado_juego.h.
 */
#include "estado_juego.h"
#include <math.h>
#include <stdio.h>

EstadoJuego estado_actual;

void inicializar_estado_juego(void) {
    estado_actual = ESTADO_MENU;
}

const int ORDEN_RUEDA_EUROPEA[37] = {
    0, 32, 15, 19, 4, 21, 2, 25, 17, 34, 6, 27, 13, 36, 11, 30, 8, 23,
    10, 5, 24, 16, 33, 1, 20, 14, 31, 9, 22, 18, 29, 7, 28, 12, 35, 3, 26
};

/* Numeros rojos en una ruleta europea real (fijos, no alternan por sector) */
static const int NUMEROS_ROJOS[18] = {
    1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
};

ColorRuleta color_de_numero(int numero) {
    int i;
    if (numero == 0) return COLOR_VERDE;
    for (i = 0; i < 18; i++) {
        if (NUMEROS_ROJOS[i] == numero) return COLOR_ROJO;
    }
    return COLOR_NEGRO;
}

/* Tabla de transiciones permitidas. Solo se listan los flujos que el
   juego realmente usa (ver main.c/idle.c):
     MENU      -> JUGANDO    (ENTER en la pantalla de bienvenida)
     JUGANDO   -> PRESTAMO   (el saldo no alcanza para la ficha minima)
     PRESTAMO  -> JUGANDO    (se acepta el prestamo y la deuda es pagable)
     PRESTAMO  -> GAME_OVER  (se acepta el prestamo pero la deuda ya es
                              impagable)
     GAME_OVER -> JUGANDO    (ENTER reinicia la partida)
   Cualquier otra transicion (por ejemplo MENU -> GAME_OVER directo, o
   JUGANDO -> MENU) se considera invalida y se ignora. */
static int es_transicion_valida(EstadoJuego actual, EstadoJuego nuevo) {
    if (actual == nuevo) return 1; /* quedarse en el mismo estado siempre es valido */

    switch (actual) {
    case ESTADO_MENU:
        return nuevo == ESTADO_JUGANDO;

    case ESTADO_JUGANDO:
        return nuevo == ESTADO_PRESTAMO;

    case ESTADO_PRESTAMO:
        return nuevo == ESTADO_JUGANDO || nuevo == ESTADO_GAME_OVER;

    case ESTADO_GAME_OVER:
        return nuevo == ESTADO_JUGANDO;

    default:
        return 0;
    }
}

void cambiar_estado(EstadoJuego nuevo_estado) {
    if (!es_transicion_valida(estado_actual, nuevo_estado)) {
        /* Transicion no contemplada por el diseno del juego: se ignora
           en vez de aplicarla a ciegas, para no dejar el juego en un
           estado inconsistente (por ejemplo, ver la pantalla de
           Game Over sin haber pasado por Prestamo). */
        fprintf(stderr,
            "cambiar_estado: transicion invalida (%d -> %d) ignorada\n",
            (int)estado_actual, (int)nuevo_estado);
        return;
    }
    estado_actual = nuevo_estado;
}

ColorRuleta calcular_color_ganador(float angulo_final) {
    float angulo_normalizado = fmodf(angulo_final, 360.0f);
    int sector;
    if (angulo_normalizado < 0.0f) angulo_normalizado += 360.0f;

    sector = (int)(angulo_normalizado / (360.0f / 37.0f)); /* 0..36 */

    if (sector == 0) return COLOR_VERDE;
    return (sector % 2 == 0) ? COLOR_NEGRO : COLOR_ROJO;
}

int calcular_numero_ganador(float angulo_final) {
    float angulo_normalizado = fmodf(angulo_final, 360.0f);
    int sector;
    if (angulo_normalizado < 0.0f) angulo_normalizado += 360.0f;

    sector = (int)(angulo_normalizado / (360.0f / 37.0f)); /* 0..36 */
    if (sector > 36) sector = 36; /* proteccion por redondeo */

    return ORDEN_RUEDA_EUROPEA[sector];
}

int docena_de_numero(int numero) {
    if (numero <= 0 || numero > 36) return 0;
    if (numero <= 12) return 1;
    if (numero <= 24) return 2;
    return 3;
}

int numero_es_par(int numero) {
    return (numero != 0) && (numero % 2 == 0);
}

int mitad_de_numero(int numero) {
    if (numero <= 0 || numero > 36) return 0;
    return (numero <= 18) ? 1 : 2;
}

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
        return (numero_ganador != 0 && numero_es_par(numero_ganador) == apuesta->valor)
            ? apuesta->monto : -apuesta->monto;

    case APUESTA_MITAD:
        return (mitad_de_numero(numero_ganador) == apuesta->valor)
            ? apuesta->monto : -apuesta->monto;

    default:
        return -apuesta->monto;
    }
}

float calcular_ganancia_total(const Apuesta apuestas[], int cantidad, int numero_ganador) {
    float total = 0.0f;
    int i;
    for (i = 0; i < cantidad; i++) {
        total += calcular_ganancia_apuesta(&apuestas[i], numero_ganador);
    }
    return total;
}