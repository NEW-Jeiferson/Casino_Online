/*
 * bezier.h
 * -----------------------------------------------------------------------
 * Funciones utilitarias para evaluar curvas de Bezier cuadraticas y
 * cubicas. Usadas tanto para geometria (perfil de revolucion de la
 * rueda) como para animacion (curva de desaceleracion de la bolita).
 *
 * Responsable sugerido: Persona A (geometria) / Persona A o C (animacion)
 * -----------------------------------------------------------------------
 */
#ifndef BEZIER_H
#define BEZIER_H

typedef struct {
    float x, y, z;
} Punto3D;

/* Evalua una curva de Bezier cuadratica (3 puntos de control) en t [0,1] */
Punto3D evaluar_bezier_cuadratica(Punto3D p0, Punto3D p1, Punto3D p2, float t);

/* Evalua una curva de Bezier cubica (4 puntos de control) en t [0,1] */
Punto3D evaluar_bezier_cubica(Punto3D p0, Punto3D p1, Punto3D p2, Punto3D p3, float t);

#endif /* BEZIER_H */
