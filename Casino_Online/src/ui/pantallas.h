/*
 * pantallas.h
 * Clase encargada de dibujar las pantallas del juego segun el estado actual
 */

#ifndef PANTALLAS_H
#define PANTALLAS_H

#include "../core/jugador.h"
#include "../core/estado_juego.h"


/* Metodo que dibuja la pantalla de menu */
void dibujar_pantalla_menu(void);


/* Metodo que dibuja la pantalla de prestamo */
void dibujar_pantalla_prestamo(const Jugador* jugador);


/* Metodo que dibuja la pantalla de game over */
void dibujar_pantalla_game_over(const Jugador* jugador);


/* Metodo que dibuja la pantalla de sesion terminada */
void dibujar_pantalla_sesion_terminada(const Jugador* jugador);


/* Metodo que dibuja la pantalla de mensaje reflexivo */
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje, const Jugador* jugador);


/* Metodo que dibuja la pantalla de juego */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const char* mensaje_reflexivo);

#endif