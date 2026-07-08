/*
 * tablero_apuestas.c
 * Implementacion del tablero de apuestas. Ver tablero_apuestas.h.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tablero_apuestas.h"
#include "../utils/bresenham.h"
#include "../core/estado_juego.h"

#define TABLERO_COLUMNAS 12   /* 12 columnas x 3 filas = 36 numeros */
#define TABLERO_FILAS     3
#define CELDA_PX         40   /* tamano de cada celda en "pixeles" locales */
#define ESCALA_TABLERO   0.012f /* convierte esos pixeles a unidades de mundo */
#define MARGEN_CASILLA    2   /* margen para que se vea el borde de Bresenham */

 /* Calcula que numero (1-36) corresponde a una celda del grid, siguiendo
    el orden real de una mesa de ruleta: columnas verticales de 3 numeros
    consecutivos (columna 0 = 1,2,3 - columna 1 = 4,5,6 - etc). */
static int numero_de_celda(int col, int fila) {
    return col * 3 + fila + 1;
}

/* Dibuja un numero centrado en el origen actual, con la fuente vectorial
   de GLUT (stroke), para que escale igual que el resto del tablero. */
static void dibujar_numero_centrado(int numero) {
    char texto[4];
    int i, len;
    float ancho_total, alto_total;
    float escala_x, escala_y;
    const float ANCHO_MAX = 34.0f; /* ancho disponible dentro de la celda (40px - margen) */

    snprintf(texto, sizeof(texto), "%d", numero);
    len = (int)strlen(texto);

    /* Altura fija para que todos los numeros se vean del mismo tamano,
       sin importar cuantos digitos tengan. */
    escala_y = 0.24f;
    alto_total = 119.05f * escala_y;

    /* El ancho parte de la misma escala que la altura, pero si con esa
       escala el numero (1 o 2 digitos) no entra en la celda, se reduce
       solo el ancho (escala_x) para que siempre quepa. Esto evita que
       los numeros de 2 digitos (10, 11, 25, 36, etc.) se salgan del
       recuadro, que era el bug reportado. */
    escala_x = escala_y;
    ancho_total = len * 104.76f * escala_x;
    if (ancho_total > ANCHO_MAX) {
        escala_x = ANCHO_MAX / (len * 104.76f);
        ancho_total = ANCHO_MAX;
    }

    glPushMatrix();
    glTranslatef(-ancho_total / 2.0f, -alto_total / 2.0f, 0.0f);
    glScalef(escala_x, escala_y, 1.0f);

    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }

    glPopMatrix();
}

/* Capa 1: casillas coloreadas (rojo/negro segun color_de_numero). */
static void dibujar_casillas(void) {
    int col, fila, numero;
    ColorRuleta color;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);
            color = color_de_numero(numero);

            if (color == COLOR_ROJO)
                glColor3f(0.75f, 0.08f, 0.08f);
            else
                glColor3f(0.08f, 0.08f, 0.08f); /* negro (el 0/verde no esta en este grid) */

            glBegin(GL_QUADS);
            glVertex2i(col * CELDA_PX + MARGEN_CASILLA, fila * CELDA_PX + MARGEN_CASILLA);
            glVertex2i((col + 1) * CELDA_PX - MARGEN_CASILLA, fila * CELDA_PX + MARGEN_CASILLA);
            glVertex2i((col + 1) * CELDA_PX - MARGEN_CASILLA, (fila + 1) * CELDA_PX - MARGEN_CASILLA);
            glVertex2i(col * CELDA_PX + MARGEN_CASILLA, (fila + 1) * CELDA_PX - MARGEN_CASILLA);
            glEnd();
        }
    }

    glPopMatrix();
}

/* Capa 2: lineas divisorias (igual que la Etapa 2, sin cambios). */
static void dibujar_lineas(void) {
    int col, fila;

    glPushMatrix();
    glTranslatef(-2.9f, 0.1f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);

    for (col = 0; col <= TABLERO_COLUMNAS; col++) {
        int x = col * CELDA_PX;
        dibujar_linea_bresenham(x, 0, x, TABLERO_FILAS * CELDA_PX);
    }
    for (fila = 0; fila <= TABLERO_FILAS; fila++) {
        int y = fila * CELDA_PX;
        dibujar_linea_bresenham(0, y, TABLERO_COLUMNAS * CELDA_PX, y);
    }

    glPopMatrix();
}

/* Capa 5: resaltado de la celda bajo el mouse (hover), usando
   celda_hover_col/celda_hover_fila (actualizadas desde mouse_mover()
   en main.c). Dibuja solo el borde, para no tapar el color rojo/negro
   de la casilla. */
static void dibujar_hover(void) {
    if (celda_hover_col < 0 || celda_hover_fila < 0) return; /* sin hover activo */

    glPushMatrix();
    glTranslatef(-2.9f, 0.13f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f); /* blanco brillante, bien visible sobre rojo/negro */
    glLineWidth(3.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glEnd();

    glLineWidth(1.0f); /* restaurar grosor por defecto, para no afectar otras lineas */

    glPopMatrix();
}

/* Capa 3: numeros en blanco, centrados sobre cada casilla. */
static void dibujar_numeros(void) {
    int col, fila, numero;

    glPushMatrix();
    glTranslatef(-2.9f, 0.12f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);

            glPushMatrix();
            glTranslatef(col * CELDA_PX + CELDA_PX / 2.0f,
                fila * CELDA_PX + CELDA_PX / 2.0f,
                0.0f);
            dibujar_numero_centrado(numero);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

/* Capa 4: fichas doradas sobre los numeros donde hay apuesta activa. */
static void dibujar_fichas_apostadas(const Apuesta apuestas[], int cantidad) {
    int col, fila, numero, i, hay_ficha;

    glPushMatrix();
    glTranslatef(-2.9f, 0.14f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 0.85f, 0.0f); /* dorado, bien visible sobre rojo/negro */

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);
            hay_ficha = 0;
            for (i = 0; i < cantidad; i++) {
                if (apuestas[i].tipo == APUESTA_NUMERO && apuestas[i].valor == numero) {
                    hay_ficha = 1;
                    break;
                }
            }
            if (!hay_ficha) continue;

            glPushMatrix();
            glTranslatef(col * CELDA_PX + CELDA_PX / 2.0f,
                fila * CELDA_PX + CELDA_PX / 2.0f, 0.0f);
            {
                const int SEGMENTOS = 16;
                const float RADIO = 12.0f;
                int k;
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(0.0f, 0.0f);
                for (k = 0; k <= SEGMENTOS; k++) {
                    float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
                    glVertex2f(cosf(ang) * RADIO, sinf(ang) * RADIO);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }

    glPopMatrix();
}

void dibujar_tablero_apuestas(const Apuesta apuestas_activas[], int num_apuestas) {
    glDisable(GL_LIGHTING);

    dibujar_casillas();
    dibujar_lineas();
    dibujar_numeros();
    dibujar_fichas_apostadas(apuestas_activas, num_apuestas);
    dibujar_hover();   /* <-- nueva linea */

    glEnable(GL_LIGHTING);
}

/* Celda actualmente resaltada (hover del mouse), compartida con el
   modulo de render (Dubenny) para que pueda dibujar el highlight
   visual correspondiente. -1 significa "ninguna celda". */
int celda_hover_col = -1;
int celda_hover_fila = -1;

void fijar_celda_hover(int col, int fila) {
    celda_hover_col = col;
    celda_hover_fila = fila;
}

/* Inversa de numero_de_celda(): dado un punto (x, z) del mundo (el que
   entrega obtener_punto_clic_en_mesa de Jeiferson), calcula a que
   columna/fila del grid corresponde.
   Es la transformacion inversa exacta de la que usa dibujar_casillas():
   ese glTranslatef(-2.9, 0.08, 4.9) + glRotatef(-90, X) + glScalef
   mapea un punto local (u, v) del grid a world = (u*ESCALA - 2.9, 0.08, -v*ESCALA + 4.9).
   Aqui despejamos u y v a partir de world (x, z). */
int obtener_celda_en_punto(float x, float z, int* col_out, int* fila_out) {
    float u = (x - (-2.9f)) / ESCALA_TABLERO;
    float v = (4.9f - z) / ESCALA_TABLERO;
    int col, fila;

    if (u < 0.0f || u >= (float)(TABLERO_COLUMNAS * CELDA_PX)) return 0;
    if (v < 0.0f || v >= (float)(TABLERO_FILAS * CELDA_PX)) return 0;

    col = (int)(u / CELDA_PX);
    fila = (int)(v / CELDA_PX);
    if (col >= TABLERO_COLUMNAS) col = TABLERO_COLUMNAS - 1;
    if (fila >= TABLERO_FILAS)   fila = TABLERO_FILAS - 1;

    *col_out = col;
    *fila_out = fila;
    return 1;
}

int obtener_numero_en_punto(float x, float z, int* numero_out) {
    int col, fila;
    if (!obtener_celda_en_punto(x, z, &col, &fila)) return 0;
    *numero_out = numero_de_celda(col, fila);
    return 1;
}