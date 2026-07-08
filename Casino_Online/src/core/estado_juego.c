/*
 * estado_juego.c
 * Implementacion de la maquina de estados. Ver estado_juego.h.
 */
#include "estado_juego.h"
#include <math.h>

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

void cambiar_estado(EstadoJuego nuevo_estado) {
    /* TODO: agregar logica de validacion de transiciones si se necesita,
       por ejemplo no permitir pasar de MENU directo a GAME_OVER */
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