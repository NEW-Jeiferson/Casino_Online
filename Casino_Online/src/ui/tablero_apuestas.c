/*/*
 * tablero_apuestas.c
 * Implementacion del tablero de apuestas. Ver tablero_apuestas.h.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
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

void dibujar_tablero_apuestas(void) 
{
    /* El tablero se pinta con colores planos via glColor3f (rojo/negro
       en las casillas, blanco en lineas y numeros). Con GL_LIGHTING
       activado y sin GL_COLOR_MATERIAL, esos glColor3f no tienen efecto:
       todo sale con el ultimo material aplicado (MATERIAL_FIELTRO,
       verde, seteado por dibujar_mesa() justo antes). Por eso apagamos
       la iluminacion mientras se dibuja el tablero, igual que hace
       hud.c/pantallas.c con su contenido 2D. */
    glDisable(GL_LIGHTING);

    dibujar_casillas();
    dibujar_lineas();
    dibujar_numeros();

    glEnable(GL_LIGHTING);
}