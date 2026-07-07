/*
 * pantallas.h
 * -----------------------------------------------------------------------
 * Pantallas de transicion superpuestas a la escena 3D: pantalla de
 * prestamo (cuando el saldo llega a 0) y pantalla de Game Over (cuando
 * la deuda se vuelve impagable). Usan blending para el overlay y
 * interpolacion de color para el efecto de fundido (fade).
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef PANTALLAS_H
#define PANTALLAS_H

#include "../core/jugador.h"
#include "../core/estado_juego.h"

/* Dibuja el overlay y mensaje reflexivo de la pantalla de prestamo */
void dibujar_pantalla_prestamo(const Jugador* jugador);

/* Dibuja el resumen de estadisticas + mensaje de Game Over */
void dibujar_pantalla_game_over(const Jugador* jugador);

/* Despacha a la funcion de dibujo correspondiente segun el estado actual */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador);

#endif /* PANTALLAS_H */
