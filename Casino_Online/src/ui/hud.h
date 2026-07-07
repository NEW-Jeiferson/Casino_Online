/*
 * hud.h
 * -----------------------------------------------------------------------
 * Dibuja el HUD en pantalla: saldo actual, total apostado, numero de
 * prestamos y otras estadisticas de sesion, usando texto en pantalla y
 * colores RGBA (con interpolacion si el saldo es bajo).
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef HUD_H
#define HUD_H

#include "../core/jugador.h"

/* Dibuja el HUD con la informacion actual del jugador. Debe llamarse en
   modo de proyeccion ortografica 2D (superpuesto a la escena 3D) */
void dibujar_hud(const Jugador* jugador);

#endif /* HUD_H */
