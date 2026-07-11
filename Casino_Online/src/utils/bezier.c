/** Funciones matemáticas para curvas de Bezier.
 * Sirve para calcular posiciones a lo largo de una curva 
 */

#include "bezier.h"

 /* Calcula un punto exacto en una curva de Bezier guiada por 3 puntos .
    La variable 't' es el avance del trayecto: va de 0.0 (inicio) a 1.0 (final). */
Punto3D evaluar_bezier_cuadratica(Punto3D p0, Punto3D p1, Punto3D p2, float t) {
    Punto3D resultado;
    float u = 1.0f - t;

    
    resultado.x = u * u * p0.x + 2.0f * u * t * p1.x + t * t * p2.x;
    resultado.y = u * u * p0.y + 2.0f * u * t * p1.y + t * t * p2.y;
    resultado.z = u * u * p0.z + 2.0f * u * t * p1.z + t * t * p2.z;

    return resultado;
}

/* Calcula un punto exacto en una curva de Bezier guiada por 4 puntos .
   Permite formas más complejas. 't' también va de 0.0 (inicio) a 1.0 (final). */
Punto3D evaluar_bezier_cubica(Punto3D p0, Punto3D p1, Punto3D p2, Punto3D p3, float t) {
    Punto3D resultado;
    float u = 1.0f - t;
    float uu = u * u;
    float tt = t * t;

   
    resultado.x = uu * u * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + tt * t * p3.x;
    resultado.y = uu * u * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + tt * t * p3.y;
    resultado.z = uu * u * p0.z + 3.0f * uu * t * p1.z + 3.0f * u * tt * p2.z + tt * t * p3.z;

    return resultado;
}