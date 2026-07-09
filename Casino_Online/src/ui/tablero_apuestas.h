/*
 * tablero_apuestas.h
 * -----------------------------------------------------------------------
 * Dibuja la cuadricula de apuestas (numeros 0-36) plana sobre la mesa,
 * usando el algoritmo de Bresenham para las lineas divisorias.
 * -----------------------------------------------------------------------
 */
#ifndef TABLERO_APUESTAS_H
#define TABLERO_APUESTAS_H

#include "../core/estado_juego.h"  /* agregar este include, para Apuesta */

void dibujar_tablero_apuestas(const Apuesta apuestas_activas[], int num_apuestas);


int obtener_celda_en_punto(float x, float z, int* col_out, int* fila_out);
int obtener_numero_en_punto(float x, float z, int* numero_out);

/* Zonas de apuesta adicionales dibujadas debajo del grid de numeros:
   docena (1ra 12 / 2da 12 / 3ra 12), mitad (1-18 / 19-36) y par/impar,
   al estilo de un tablero de casino real. Devuelve 1 y escribe el tipo
   y valor de apuesta correspondientes (mismo convenio que Apuesta.valor
   en estado_juego.h) si el punto (x, z) cae dentro de alguna de esas
   zonas; 0 si no. */
int obtener_zona_especial_en_punto(float x, float z, TipoApuesta* tipo_out, int* valor_out);

/* Celda con hover activo (-1,-1 si ninguna). Dubenny puede incluir
   este header y leer estas dos variables extern para dibujar el
   resaltado visual de la celda seleccionada. */
extern int celda_hover_col;
extern int celda_hover_fila;
void fijar_celda_hover(int col, int fila);

#endif /* TABLERO_APUESTAS_H */