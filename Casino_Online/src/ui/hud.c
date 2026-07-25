/** Implementacion del HUD (Heads-Up Display).
 * Contiene las funciones para renderizar la interfaz 2D sobrepuesta al juego 3D,
 * mostrando informacion en tiempo real del jugador como saldo, apuestas y configuracion actual.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include "hud.h"

 /* Funcion auxiliar para dibujar una cadena de texto en pantalla en 2D.
    Utiliza la fuente  de 18 puntos de GLUT y renderiza caracter por caracter
    empezando en las coordenadas especificadas (x, y). */
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

/* Dibuja en pantalla toda la informacion de la interfaz del usuario (HUD).
   Maneja internamente la transicion temporal a modo ortogonal (2D), renderiza
   los datos del jugador y restaura el estado 3D al finalizar. */

void dibujar_hud(const Jugador* jugador, float monto_ficha_actual, int num_apuestas_activas) {
    char buffer[128];
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* Fondo semitransparente (Glassmorphism) */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* Fondo semitransparente (Glassmorphism) */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(5.0f, (float)alto - 150.0f);
    glVertex2f(270.0f, (float)alto - 150.0f);
    glVertex2f(270.0f, (float)alto - 10.0f);
    glVertex2f(5.0f, (float)alto - 10.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde dorado */
    glColor3f(1.0f, 0.85f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(5.0f, (float)alto - 150.0f);
    glVertex2f(270.0f, (float)alto - 150.0f);
    glVertex2f(270.0f, (float)alto - 10.0f);
    glVertex2f(5.0f, (float)alto - 10.0f);
    glEnd();
    glLineWidth(1.0f);

    /* 1. Saldo */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 38.0f, "Saldo: ");
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
    sprintf_s(buffer, sizeof(buffer), "%.2f", jugador->saldo);
    dibujar_texto_2d(15.0f + 65.0f, (float)alto - 38.0f, buffer);

    /* 2. Total apostado */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 63.0f, "Total apostado: ");
    if (jugador->total_apostado > 0.0f) {
        glColor3f(1.0f, 0.6f, 0.2f);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    sprintf_s(buffer, sizeof(buffer), "%.2f", jugador->total_apostado);
    dibujar_texto_2d(15.0f + 140.0f, (float)alto - 63.0f, buffer);

    /* 3. Prestamos */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 88.0f, "Prestamos: ");
    if (jugador->prestamos_activos > 0) {
        glColor3f(1.0f, 0.3f, 0.3f);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    sprintf_s(buffer, sizeof(buffer), "%d", jugador->prestamos_activos);
    dibujar_texto_2d(15.0f + 100.0f, (float)alto - 88.0f, buffer);

    /* 4. Ficha seleccionada y circulo de color */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 113.0f, "Ficha: ");
    if (monto_ficha_actual <= 10.0f) {
        glColor3f(1.0f, 0.9f, 0.0f); /* Amarillo */
    } else if (monto_ficha_actual <= 25.0f) {
        glColor3f(0.0f, 0.7f, 0.1f); /* Verde */
    } else if (monto_ficha_actual <= 50.0f) {
        glColor3f(0.1f, 0.4f, 0.9f); /* Azul */
    } else {
        glColor3f(0.6f, 0.1f, 0.7f); /* Morado */
    }
    sprintf_s(buffer, sizeof(buffer), "%.2f", monto_ficha_actual);
    dibujar_texto_2d(15.0f + 60.0f, (float)alto - 113.0f, buffer);

    /* Circulo indicador al lado */
    {
        const int SEGMENTOS = 16;
        int k;
        float cx = 155.0f;
        float cy = (float)alto - 108.0f;
        float rad = 8.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (k = 0; k <= SEGMENTOS; k++) {
            float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
            glVertex2f(cx + cosf(ang) * rad, cy + sinf(ang) * rad);
        }
        glEnd();
        
        /* Borde interior blanco del circulo */
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        for (k = 0; k <= SEGMENTOS; k++) {
            float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
            glVertex2f(cx + cosf(ang) * (rad - 2.0f), cy + sinf(ang) * (rad - 2.0f));
        }
        glEnd();
    }

    /* 5. Cantidad apuestas */
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf_s(buffer, sizeof(buffer), "Apuestas: %d", num_apuestas_activas);
    dibujar_texto_2d(15.0f, (float)alto - 138.0f, buffer);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}