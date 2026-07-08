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
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);

    /* --- Guardar estado 3D y entrar en modo 2D --- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* --- Interpolacion de color segun saldo bajo --- */
    {
        const float UMBRAL_SALDO_BAJO = 200.0f;
        float t = jugador->saldo / UMBRAL_SALDO_BAJO;
        float r, g, b;

        if (t > 1.0f) t = 1.0f;
        if (t < 0.0f) t = 0.0f;

        r = 1.0f;
        g = t;
        b = t;

        glColor3f(r, g, b);
    }

    /* --- Dibujo del HUD --- */
    sprintf_s(buffer, sizeof(buffer), "Saldo: %.2f", jugador->saldo);
    dibujar_texto_2d(10.0f, 730.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d(10.0f, 705.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos activos: %d", jugador->prestamos_activos);
    dibujar_texto_2d(10.0f, 680.0f, buffer);

    /* --- Restaurar estado 3D --- */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}