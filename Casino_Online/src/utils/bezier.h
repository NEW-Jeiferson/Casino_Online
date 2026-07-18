// Interfaz de utilidades matemáticas para curvas de Bezier.
// Sirve para crear formas redondeadas  el plato de la ruleta
 
#ifndef BEZIER_H
#define BEZIER_H

 /* Representa una posición exacta en el mundo 3D usando coordenadas X, Y, Z. */
typedef struct {
    float x, y, z;
} Punto3D;

// Calcula un punto en una curva suave usando 3 puntos guía.
Punto3D evaluar_bezier_cuadratica(Punto3D p0, Punto3D p1, Punto3D p2, float t);

// Calcula un punto en una curva usando 4 puntos guía para hacer formas más complejas.
    
Punto3D evaluar_bezier_cubica(Punto3D p0, Punto3D p1, Punto3D p2, Punto3D p3, float t);

#endif