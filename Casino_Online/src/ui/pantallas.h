/** Definicion de la interfaz de las pantallas de transicion.
 * Expone las funciones necesarias para renderizar las distintas interfaces
 * 2D superpuestas (overlays) que cambian dependiendo del estado del juego.
 */
#ifndef PANTALLAS_H
#define PANTALLAS_H

#include "../core/jugador.h"
#include "../core/estado_juego.h"

 // Dibuja la pantalla principal de menu.
	
void dibujar_pantalla_menu(void);

// Dibuja la pantalla de advertencia cuando el jugador agota su saldo.
   
void dibujar_pantalla_prestamo(const Jugador* jugador);

// Dibuja la pantalla final de derrota (Game Over). 
  
void dibujar_pantalla_game_over(const Jugador* jugador);

// Dibuja una pantalla de pausa con un mensaje 
   
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje);

/* Funcion principal que actua como enrutador visual.
   Evalua el estado actual del juego y llama internamente a la funcion de
   dibujado correspondiente (menu, prestamo, game over o reflexion). */
  
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const char* mensaje_reflexivo);

#endif