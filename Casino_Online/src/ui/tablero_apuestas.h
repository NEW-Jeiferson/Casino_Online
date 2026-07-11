/** Interfaz del tablero de apuestas.
 * Tiene las funciones para dibujar el tapete y saber donde hace clic el jugador.
 */
#ifndef TABLERO_APUESTAS_H
#define TABLERO_APUESTAS_H

#include "../core/estado_juego.h"  

 /* Dibuja el tablero en pantalla con sus números, zonas especiales y las fichas apostadas. */
void dibujar_tablero_apuestas(const Apuesta apuestas_activas[], int num_apuestas);

/* Convierte el clic del raton en 3D a la columna y fila del tapete.
   Devuelve 1 si tocaste el tablero, o 0 si hiciste clic afuera. */
int obtener_celda_en_punto(float x, float z, int* col_out, int* fila_out);

/* Averigua si el clic cayo exactamente sobre un numero del 0 al 36.
   Devuelve 1 si acerto en un numero, o 0 si no. */
int obtener_numero_en_punto(float x, float z, int* numero_out);

/* Averigua si el clic cayo en zonas como docenas, rojo/negro o par/impar.
   Guarda que tipo de apuesta es y devuelve 1 si acerto. */
int obtener_zona_especial_en_punto(float x, float z, TipoApuesta* tipo_out, int* valor_out);

/* Variables para recordar que casilla esta señalando el raton en este momento (hover). */
extern int celda_hover_col;
extern int celda_hover_fila;

/* Guarda la posicion de la casilla que el raton esta apuntando ahora mismo
   para poder iluminarla en pantalla. */
void fijar_celda_hover(int col, int fila);

#endif