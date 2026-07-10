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

void dibujar_hud(const Jugador* jugador, float monto_ficha_actual, int num_apuestas_activas) {
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

    /* --- Dibujo del HUD ---
       BUGFIX: antes las posiciones Y eran constantes absolutas
       (730, 705, 680...) calculadas a mano para una ventana de
       1024x768. pantallas.c ya calculaba todo en base al ancho/alto
       reales (glutGet), pero hud.c no -si la ventana se redimensionaba
       (reshape() lo permite sin restriccion), el HUD podia quedar
       cortado (ventana mas chica que 768 de alto) o "flotando" lejos
       de la esquina superior (ventana mas grande). Ahora las Y se
       anclan a 'alto', con el mismo espaciado vertical de 25px y el
       mismo margen superior de 38px que tenia el layout original a
       768 de alto (768 - 730 = 38, 768 - 705 = 63, etc.), asi que a
       1024x768 se ve identico a antes, y en cualquier otro tamano de
       ventana se mantiene pegado a la esquina superior izquierda. */
    sprintf_s(buffer, sizeof(buffer), "Saldo: %.2f", jugador->saldo);
    dibujar_texto_2d(10.0f, (float)alto - 38.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d(10.0f, (float)alto - 63.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos activos: %d", jugador->prestamos_activos);
    dibujar_texto_2d(10.0f, (float)alto - 88.0f, buffer);

    /* Ficha actualmente seleccionada (1-4), para que el jugador sepa
       cuanto esta a punto de apostar antes de hacer clic. */
    sprintf_s(buffer, sizeof(buffer), "Ficha actual: %.2f", monto_ficha_actual);
    dibujar_texto_2d(10.0f, (float)alto - 113.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Apuestas colocadas: %d", num_apuestas_activas);
    dibujar_texto_2d(10.0f, (float)alto - 138.0f, buffer);

    /* --- Restaurar estado 3D --- */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}