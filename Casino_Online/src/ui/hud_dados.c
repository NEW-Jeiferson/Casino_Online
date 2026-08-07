#include "hud_dados.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

static void dibujar_texto_2d(float x, float y, const char* texto) {
    glRasterPos2f(x, y);
    while (*texto) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *texto);
        texto++;
    }
}

void dibujar_hud_dados(int ancho_ventana, int alto_ventana, TipoApuestaDados ultima_apuesta, int ultimo_resultado, int girando, int d1, int d2) {
    const float ALTO_PANEL = 110.0f;
    const float ANCHO_PANEL = 680.0f;
    float panel_x0 = (ancho_ventana - ANCHO_PANEL) / 2.0f;
    float panel_x1 = panel_x0 + ANCHO_PANEL;
    float panel_y0 = 20.0f;
    float panel_y1 = panel_y0 + ALTO_PANEL;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, ancho_ventana, 0.0, alto_ventana, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    /* Fondo del panel */
    glColor4f(0.1f, 0.1f, 0.1f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(panel_x0, panel_y0);
    glVertex2f(panel_x1, panel_y0);
    glVertex2f(panel_x1, panel_y1);
    glVertex2f(panel_x0, panel_y1);
    glEnd();
    
    /* Borde */
    glColor3f(0.8f, 0.6f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panel_x0, panel_y0);
    glVertex2f(panel_x1, panel_y0);
    glVertex2f(panel_x1, panel_y1);
    glVertex2f(panel_x0, panel_y1);
    glEnd();
    
    /* Texto de apuestas disponibles */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(panel_x0 + 20.0f, panel_y1 - 30.0f, "Apuestas Disponibles:");
    glColor3f(0.8f, 0.8f, 0.8f);
    dibujar_texto_2d(panel_x0 + 20.0f, panel_y1 - 60.0f, "[P] Par / [I] Impar   (Paga 0.9x)");
    dibujar_texto_2d(panel_x0 + 20.0f, panel_y1 - 90.0f, "[7] Suma Siete        (Paga 4.5x)");
    dibujar_texto_2d(panel_x0 + 320.0f, panel_y1 - 60.0f, "[E] Extremos (2 o 12) (Paga 13.0x)");
    
    /* Ya no dibujamos Ultimo resultado fijo aqui; se maneja por notificaciones flotantes en main.c */
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
static const char* MENSAJES_GIRO_DADOS[8] = {
    "Tiradas totalmente independientes.\nSaberlo ayuda a saber cuando detenerse.",
    "Toda apuesta favorece al casino a largo plazo.\nParar ahora es una opcion valida.",
    "Mayor pago implica la peor probabilidad.\nBuena razon para terminar por hoy.",
    "Aun con altas chances, el casino lleva ventaja.\nLevantar la sesion esta en tus manos.",
    "Apostaste mas de lo que tenias planeado?\nQuizas sea momento de hacer una pausa.",
    "Los pagos altos son ilusiones para retenerte.\nReconocerlo ayuda a poder decidir.",
    "Con dinero real, afectarias tu patrimonio.\nRetirarse a tiempo es ganar.",
    "De verdad necesitas ver este resultado,\no podrias terminar la sesion aca?"
};

void dibujar_mensaje_giro_dados(int indice) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    int idx = indice % 8;
    const char* msg_prob;
    char linea1[256] = {0};
    char linea2[256] = {0};
    char* salto;
    int len1, len2, max_len;
    float banner_cx, banner_y, half_w;

    if (idx < 0) idx = 0;
    msg_prob = MENSAJES_GIRO_DADOS[idx];

    /* Separar en dos lineas por el \n */
    strncpy_s(linea1, sizeof(linea1), msg_prob, _TRUNCATE);
    salto = strchr(linea1, '\n');
    if (salto) {
        *salto = '\0';
        strncpy_s(linea2, sizeof(linea2), salto + 1, _TRUNCATE);
    }

    len1 = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)linea1);
    len2 = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)linea2);
    max_len = (len1 > len2) ? len1 : len2;

    banner_cx = (float)ancho / 2.0f;
    banner_y = (float)alto - 60.0f; /* Un poco mas bajo porque son 2 lineas */
    half_w = (float)max_len / 2.0f + 16.0f;

    /* Margen de seguridad: evitar pisar UI lateral si la hubiera */
    {
        float margen_hud = 15.0f + (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"[S] Terminar la sesion en cualquier momento") + 20.0f;
        if (banner_cx - half_w < margen_hud) {
            banner_cx = margen_hud + half_w;
        }
    }
    if (banner_cx + half_w > (float)ancho - 10.0f) {
        banner_cx = (float)ancho - 10.0f - half_w;
    }

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
    glColor4f(0.02f, 0.08f, 0.18f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(banner_cx - half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y + 24.0f);
    glVertex2f(banner_cx - half_w, banner_y + 24.0f);
    glEnd();

    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(banner_cx - half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y + 24.0f);
    glVertex2f(banner_cx - half_w, banner_y + 24.0f);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(banner_cx - ((float)len1 / 2.0f), banner_y + 4.0f, linea1);
    if (linea2[0] != '\0') {
        dibujar_texto_2d(banner_cx - ((float)len2 / 2.0f), banner_y - 18.0f, linea2);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
