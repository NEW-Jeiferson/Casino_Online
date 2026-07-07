/*
 * bresenham.c
 * Implementacion del algoritmo de Bresenham. Ver bresenham.h.
 */
#include <GL/glut.h>
#include <stdlib.h>
#include "bresenham.h"

void dibujar_linea_bresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int error = dx + dy;
    int e2;

    glBegin(GL_POINTS);
    for (;;) {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1) break;
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            if (y0 == y1) break;
            error += dx;
            y0 += sy;
        }
    }
    glEnd();
}
