/*
 * ruleta_geometria.c
 * Implementacion de la geometria de la rueda. Ver ruleta_geometria.h.
 */
#include <GL/glut.h>
#include "ruleta_geometria.h"
#include "../utils/bezier.h"

void generar_perfil_bezier_rueda(void) {
    /* TODO: definir puntos de control y evaluar la curva de Bezier
       cubica con evaluar_bezier_cubica() de utils/bezier.h */
}

void construir_malla_rueda(void) {
    /* TODO: revolucionar el perfil generado alrededor del eje Y,
       calculando normales de vertice para cada punto de la malla */
}

void dibujar_mesa(void) {
    glPushMatrix();
    /* TODO: dibujar plano de la mesa con material fieltro */
    glPopMatrix();
}

void dibujar_rueda(float angulo_rotacion) {
    glPushMatrix();
    glRotatef(angulo_rotacion, 0.0f, 1.0f, 0.0f);
    /* TODO: dibujar la malla de la rueda construida */
    glPopMatrix();
}

void dibujar_vidrio_protector(void) {
    /* TODO: activar blending (glEnable(GL_BLEND)) y dibujar el domo
       semitransparente sobre la rueda */
}
