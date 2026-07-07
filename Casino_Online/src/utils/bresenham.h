/*
 * bresenham.h
 * -----------------------------------------------------------------------
 * Implementacion propia del algoritmo de Bresenham para dibujar lineas,
 * usada en las lineas del tablero de apuestas (en vez de GL_LINES nativo)
 * como ejercicio de rasterizacion manual.
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef BRESENHAM_H
#define BRESENHAM_H

/* Dibuja una linea entre (x0,y0) y (x1,y1) usando Bresenham, pintando
   cada pixel como un punto OpenGL (glVertex2i dentro de GL_POINTS) */
void dibujar_linea_bresenham(int x0, int y0, int x1, int y1);

#endif /* BRESENHAM_H */
