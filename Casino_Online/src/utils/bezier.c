/*
 * bezier.c
 * Implementacion de evaluacion de curvas de Bezier. Ver bezier.h.
 */
#include "bezier.h"

Punto3D evaluar_bezier_cuadratica(Punto3D p0, Punto3D p1, Punto3D p2, float t) {
    Punto3D resultado;
    float u = 1.0f - t;

    /* B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2 */
    resultado.x = u * u * p0.x + 2.0f * u * t * p1.x + t * t * p2.x;
    resultado.y = u * u * p0.y + 2.0f * u * t * p1.y + t * t * p2.y;
    resultado.z = u * u * p0.z + 2.0f * u * t * p1.z + t * t * p2.z;

    return resultado;
}

Punto3D evaluar_bezier_cubica(Punto3D p0, Punto3D p1, Punto3D p2, Punto3D p3, float t) {
    Punto3D resultado;
    float u = 1.0f - t;
    float uu = u * u;
    float tt = t * t;

    /* B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3 */
    resultado.x = uu * u * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + tt * t * p3.x;
    resultado.y = uu * u * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + tt * t * p3.y;
    resultado.z = uu * u * p0.z + 3.0f * uu * t * p1.z + 3.0f * u * tt * p2.z + tt * t * p3.z;

    return resultado;
}
