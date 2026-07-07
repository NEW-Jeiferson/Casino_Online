/*
 * ruleta_geometria.h
 * -----------------------------------------------------------------------
 * Generacion de la geometria de la mesa y la rueda de ruleta a partir de
 * un perfil de Bezier revolucionado (superficie de revolucion), y su
 * dibujo usando la pila de matrices para la jerarquia mesa -> rueda ->
 * casillas -> bolita.
 *
 * Responsable sugerido: Persona A
 * -----------------------------------------------------------------------
 */
#ifndef RULETA_GEOMETRIA_H
#define RULETA_GEOMETRIA_H

/* Genera los puntos del perfil de la rueda usando una curva de Bezier
   cubica, y los guarda para usarse en la superficie de revolucion */
void generar_perfil_bezier_rueda(void);

/* Construye la malla 3D de la rueda (superficie de revolucion) a partir
   del perfil generado, calculando normales de vertice */
void construir_malla_rueda(void);

/* Dibuja la mesa (plano/paño con material fieltro) */
void dibujar_mesa(void);

/* Dibuja la rueda ya construida, aplicando su rotacion actual */
void dibujar_rueda(float angulo_rotacion);

/* Dibuja el vidrio protector semitransparente sobre la rueda (blending) */
void dibujar_vidrio_protector(void);

#endif /* RULETA_GEOMETRIA_H */
