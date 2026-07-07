/*
 * estado_juego.c
 * Implementacion de la maquina de estados. Ver estado_juego.h.
 */
#include "estado_juego.h"

EstadoJuego estado_actual;

void inicializar_estado_juego(void) {
    estado_actual = ESTADO_MENU;
}

void cambiar_estado(EstadoJuego nuevo_estado) {
    /* TODO: agregar logica de validacion de transiciones si se necesita,
       por ejemplo no permitir pasar de MENU directo a GAME_OVER */
    estado_actual = nuevo_estado;
}
