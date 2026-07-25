// Definicion de la interfaz del HUD//
 
#ifndef HUD_H
#define HUD_H

#include "../core/jugador.h"

 /* Dibuja en pantalla toda la informacion de la interfaz del usuario y
	Recibe el estado actual del jugador, el valor de la ficha seleccionada
	y la cantidad de apuestas en la mesa */
void dibujar_hud(const Jugador* jugador, float monto_ficha_actual, int num_apuestas_activas);

#endif