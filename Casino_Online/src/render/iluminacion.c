/*
 * iluminacion.c
 * Implementacion de la configuracion de luces. Ver iluminacion.h.
 */
#include <GL/glut.h>
#include "iluminacion.h"

 /*
  * iluminacion.c
  * Implementacion de la configuracion de luces. Ver iluminacion.h.
  */
#include <GL/glut.h>
#include "iluminacion.h"

void inicializar_iluminacion(void) {
    /* LUZ PRINCIPAL (LIGHT0): antes en (0,5,0), directamente arriba del
       centro. Con la camara en (0,9,11) mirando al origen, un brillo
       especular desde ahi cae en un punto casi invisible desde el
       angulo de camara. Movida hacia (3,6,5) -arriba y hacia la camara-
       para que el highlight de la rueda/bolita caiga donde realmente
       se ve. Diffuse subido levemente (0.8 a 0.85) para compensar que
       ya no esta perfectamente perpendicular a la mesa. */
    GLfloat luz0_ambiental[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    GLfloat luz0_difusa[] = { 0.85f, 0.85f, 0.85f, 1.0f };
    GLfloat luz0_especular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat luz0_posicion[] = { 3.0f, 6.0f, 5.0f, 1.0f };

    /* LUZ DE RELLENO (LIGHT1): tenue, del lado opuesto a LIGHT0, sin
       componente specular (para no crear un segundo highlight que
       compita con el de LIGHT0). Solo evita que el lado de la rueda
       que no mira a la luz principal se pierda a negro plano. */
    GLfloat luz1_ambiental[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat luz1_difusa[] = { 0.22f, 0.22f, 0.25f, 1.0f }; /* levemente fria, para diferenciarla de la calida principal */
    GLfloat luz1_especular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat luz1_posicion[] = { -4.0f, 3.0f, -3.0f, 1.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    glLightfv(GL_LIGHT0, GL_AMBIENT, luz0_ambiental);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luz0_difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luz0_especular);
    glLightfv(GL_LIGHT0, GL_POSITION, luz0_posicion);

    glLightfv(GL_LIGHT1, GL_AMBIENT, luz1_ambiental);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, luz1_difusa);
    glLightfv(GL_LIGHT1, GL_SPECULAR, luz1_especular);
    glLightfv(GL_LIGHT1, GL_POSITION, luz1_posicion);
}


void actualizar_posicion_luz(float x, float y, float z) {
    GLfloat posicion_luz[] = { x, y, z, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicion_luz);
}