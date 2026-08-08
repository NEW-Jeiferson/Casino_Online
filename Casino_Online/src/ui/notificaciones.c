/*
 * notificaciones.c
 * Implementacion del sistema de notificaciones flotantes transitorias.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include "notificaciones.h"

typedef struct {
    char texto[128];
    float r;
    float g;
    float b;
    unsigned int tiempo_inicio_ms;
    int activa;
} Notificacion;

static Notificacion g_notificaciones[MAX_NOTIFICACIONES];

void inicializar_notificaciones(void) {
    int i;
    for (i = 0; i < MAX_NOTIFICACIONES; i++) {
        g_notificaciones[i].activa = 0;
        g_notificaciones[i].texto[0] = '\0';
    }
}

void agregar_notificacion(const char* texto, float r, float g, float b) {
    int i;
    unsigned int tiempo_actual;
    int idx_destino;

    if (texto == NULL || texto[0] == '\0') return;

    tiempo_actual = (unsigned int)glutGet(GLUT_ELAPSED_TIME);
    idx_destino = -1;

    /* Buscar una casilla inactiva o la mas antigua */
    for (i = 0; i < MAX_NOTIFICACIONES; i++) {
        if (!g_notificaciones[i].activa) {
            idx_destino = i;
            break;
        }
    }

    if (idx_destino == -1) {
        /* Si todas estan ocupadas, desplazar hacia arriba y usar la ultima */
        for (i = 0; i < MAX_NOTIFICACIONES - 1; i++) {
            g_notificaciones[i] = g_notificaciones[i + 1];
        }
        idx_destino = MAX_NOTIFICACIONES - 1;
    }

    strncpy_s(g_notificaciones[idx_destino].texto, sizeof(g_notificaciones[idx_destino].texto), texto, _TRUNCATE);
    g_notificaciones[idx_destino].r = r;
    g_notificaciones[idx_destino].g = g;
    g_notificaciones[idx_destino].b = b;
    g_notificaciones[idx_destino].tiempo_inicio_ms = tiempo_actual;
    g_notificaciones[idx_destino].activa = 1;
}

static void dibujar_texto_2d_notif(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void dibujar_notificaciones(void) {
    int i, n_visibles;
    unsigned int tiempo_actual;
    int ancho, alto;
    const float DURACION_TOTAL = 3300.0f; /* ms */
    const float FADE_IN_MS = 300.0f;
    const float FADE_OUT_MS = 600.0f;
    float card_w, card_h, margin_top, start_x;

    ancho = glutGet(GLUT_WINDOW_WIDTH);
    alto = glutGet(GLUT_WINDOW_HEIGHT);
    tiempo_actual = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

    card_w = 320.0f;
    card_h = 42.0f;
    margin_top = 15.0f;
    start_x = (float)ancho - card_w - 15.0f;

    n_visibles = 0;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (i = 0; i < MAX_NOTIFICACIONES; i++) {
        if (g_notificaciones[i].activa) {
            float elapsed = (float)(tiempo_actual - g_notificaciones[i].tiempo_inicio_ms);
            float alpha = 1.0f;
            float y_top, y_bottom;

            if (elapsed >= DURACION_TOTAL) {
                g_notificaciones[i].activa = 0;
                continue;
            }

            if (elapsed < FADE_IN_MS) {
                alpha = elapsed / FADE_IN_MS;
            } else if (elapsed > (DURACION_TOTAL - FADE_OUT_MS)) {
                alpha = (DURACION_TOTAL - elapsed) / FADE_OUT_MS;
            }

            if (alpha < 0.0f) alpha = 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;

            y_top = (float)alto - margin_top - (float)n_visibles * (card_h + 8.0f);
            y_bottom = y_top - card_h;

            /* Fondo semitransparente oscuro */
            glColor4f(0.02f, 0.04f, 0.08f, 0.78f * alpha);
            glBegin(GL_QUADS);
            glVertex2f(start_x, y_bottom);
            glVertex2f(start_x + card_w, y_bottom);
            glVertex2f(start_x + card_w, y_top);
            glVertex2f(start_x, y_top);
            glEnd();

            /* Borde dorado */
            glColor4f(1.0f, 0.84f, 0.0f, 0.85f * alpha);
            glLineWidth(1.4f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(start_x, y_bottom);
            glVertex2f(start_x + card_w, y_bottom);
            glVertex2f(start_x + card_w, y_top);
            glVertex2f(start_x, y_top);
            glEnd();
            glLineWidth(1.0f);

            /* Texto de la notificacion */
            glColor4f(g_notificaciones[i].r, g_notificaciones[i].g, g_notificaciones[i].b, alpha);
            dibujar_texto_2d_notif(start_x + 12.0f, y_bottom + 14.0f, g_notificaciones[i].texto);

            n_visibles++;
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
