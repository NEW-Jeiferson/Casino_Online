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