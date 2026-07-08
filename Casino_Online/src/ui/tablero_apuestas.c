/*
 * tablero_apuestas.c
 * Implementacion del tablero de apuestas. Ver tablero_apuestas.h.
 */
#include <GL/glut.h>
#include "tablero_apuestas.h"
#include "../utils/bresenham.h"

#define TABLERO_COLUMNAS 12   /* 12 columnas x 3 filas = 36 numeros */
#define TABLERO_FILAS     3
#define CELDA_PX         40   /* tamano de cada celda en "pixeles" locales */
#define ESCALA_TABLERO   0.012f /* convierte esos pixeles a unidades de mundo */

void dibujar_tablero_apuestas(void) {
    int col, fila;

    glPushMatrix();

    /* Posicion del tablero: enfrente de la rueda, sobre el paño verde.
       Y=0.02 para evitar z-fighting (parpadeo) con la superficie de la mesa. */
    glTranslatef(-2.9f, 0.1f, 4.9f);

    /* Bresenham dibuja en el plano XY (glVertex2i implica z=0). Rotamos
       -90 grados en X para "acostar" ese plano sobre la mesa (plano XZ
       del mundo), en vez de dejarlo parado de frente a la camara. */
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    /* Convertimos las coordenadas enteras de Bresenham a un tamano
       razonable en el mundo 3D (sin este scale, el tablero seria enorme). */
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);

    /* Lineas verticales (separan las 12 columnas) */
    for (col = 0; col <= TABLERO_COLUMNAS; col++) {
        int x = col * CELDA_PX;
        dibujar_linea_bresenham(x, 0, x, TABLERO_FILAS * CELDA_PX);
    }

    /* Lineas horizontales (separan las 3 filas) */
    for (fila = 0; fila <= TABLERO_FILAS; fila++) {
        int y = fila * CELDA_PX;
        dibujar_linea_bresenham(0, y, TABLERO_COLUMNAS * CELDA_PX, y);
    }

    glPopMatrix();
}