/*
 * estado_juego.h
 * -----------------------------------------------------------------------
 * Maquina de estados general del juego. Controla las transiciones entre
 * menu, juego activo, pantalla de prestamo y pantalla de game over.
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H

typedef enum {
    ESTADO_MENU,
    ESTADO_JUGANDO,
    ESTADO_SIN_FONDOS,
    ESTADO_PRESTAMO,
    ESTADO_GAME_OVER
} EstadoJuego;

/* Estado global actual (definido en estado_juego.c) */
extern EstadoJuego estado_actual;

/* Inicializa la maquina de estados en ESTADO_MENU */
void inicializar_estado_juego(void);

/* Cambia el estado actual, validando transiciones si hace falta */
void cambiar_estado(EstadoJuego nuevo_estado);

#endif /* ESTADO_JUEGO_H */
