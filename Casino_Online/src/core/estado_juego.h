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
    COLOR_ROJO,
    COLOR_NEGRO,
    COLOR_VERDE
} ColorRuleta;

/* Calcula el color ganador segun el angulo final de la bolita (0-360),
   dividiendo la rueda en 37 sectores como una ruleta europea real */
ColorRuleta calcular_color_ganador(float angulo_final);
extern const int ORDEN_RUEDA_EUROPEA[37];

/* Determina el numero ganador (0-36) segun el angulo final de la bolita */
int calcular_numero_ganador(float angulo_final);

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
