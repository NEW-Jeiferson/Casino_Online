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

 /* Dibuja la pantalla de bienvenida (ESTADO_MENU): titulo, aviso de que
	se juega con saldo virtual, y la lista de controles del juego. */
void dibujar_pantalla_menu(void);

/* Dibuja el overlay y mensaje reflexivo de la pantalla de prestamo */
void dibujar_pantalla_prestamo(const Jugador* jugador);

/* Dibuja el resumen de estadisticas + mensaje de Game Over */
void dibujar_pantalla_game_over(const Jugador* jugador);

/* Dibuja un mensaje de concientizacion sobre ludopatia (ver
   core/jugador.h, verificar_mensaje_reflexivo). 'mensaje' puede tener
   saltos de linea ('\n') para forzar cortes de linea especificos; el
   texto tambien se envuelve automaticamente si una linea es mas ancha
   que la pantalla. */
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje);

/* Despacha a la funcion de dibujo correspondiente segun el estado
   actual. 'mensaje_reflexivo' solo se usa (y puede ser NULL en
   cualquier otro estado) cuando estado == ESTADO_MENSAJE_REFLEXIVO. */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const char* mensaje_reflexivo);

#endif /* PANTALLAS_H */