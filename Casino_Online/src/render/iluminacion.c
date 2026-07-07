/*
 * iluminacion.c
 * Implementacion de la configuracion de luces. Ver iluminacion.h.
 */
#include <GL/glut.h>
#include "iluminacion.h"

void inicializar_iluminacion(void) {
    GLfloat luz_ambiental[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat luz_difusa[]     = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat luz_especular[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat posicion_luz[]   = { 0.0f, 5.0f, 0.0f, 1.0f }; /* w=1 -> point light */

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    glLightfv(GL_LIGHT0, GL_AMBIENT, luz_ambiental);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luz_difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luz_especular);
    glLightfv(GL_LIGHT0, GL_POSITION, posicion_luz);
}

void actualizar_posicion_luz(float x, float y, float z) {
    GLfloat posicion_luz[] = { x, y, z, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicion_luz);
}
