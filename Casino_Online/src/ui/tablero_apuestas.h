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

/* Celda con hover activo (-1,-1 si ninguna). Dubenny puede incluir
   este header y leer estas dos variables extern para dibujar el
   resaltado visual de la celda seleccionada. */
extern int celda_hover_col;
extern int celda_hover_fila;
void fijar_celda_hover(int col, int fila);

#endif /* TABLERO_APUESTAS_H */