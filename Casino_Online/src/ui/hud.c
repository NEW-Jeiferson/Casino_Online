/*
 * hud.c
 * Implementacion del HUD. Ver hud.h.
 */
#include <GL/glut.h>
#include <stdio.h>
#include "hud.h"

/* Funcion auxiliar para dibujar texto con glutBitmapCharacter */
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void dibujar_hud(const Jugador* jugador) {
    char buffer[128];

    /* TODO: cambiar a proyeccion ortografica 2D antes de dibujar
       (glMatrixMode(GL_PROJECTION) + glLoadIdentity + glOrtho),
       deshabilitar GL_LIGHTING y GL_DEPTH_TEST temporalmente,
       dibujar el texto, y restaurar el estado despues. */

    sprintf_s(buffer, sizeof(buffer), "Saldo: %.2f", jugador->saldo);
    dibujar_texto_2d(10.0f, 730.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d(10.0f, 705.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos activos: %d", jugador->prestamos_activos);
    dibujar_texto_2d(10.0f, 680.0f, buffer);

    /* TODO: si jugador->saldo es bajo, interpolar el color del texto
       de blanco a rojo (interpolacion de color) para llamar la atencion */
}
