/*
 * mouse_picking.c
 * Implementacion de la conversion de clic de mouse a punto 3D sobre la
 * mesa. Ver mouse_picking.h.
 */
#include <GL/glut.h>
#include "mouse_picking.h"

int obtener_punto_clic_en_mesa(int mouse_x, int mouse_y, float* out_x, float* out_z) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble near_x, near_y, near_z;
    GLdouble far_x, far_y, far_z;
    double dir_x, dir_y, dir_z;
    double t;
    GLdouble winY;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    /* OpenGL mide Y desde abajo hacia arriba; GLUT entrega la posicion
       del mouse con Y desde arriba hacia abajo. Hay que invertir. */
    winY = (GLdouble)(viewport[3] - mouse_y);

    /* Se "des-proyectan" dos puntos del clic: uno en el near plane
       (z=0.0) y otro en el far plane (z=1.0). La resta de ambos da la
       direccion del rayo que sale de la camara hacia el mundo. */
    gluUnProject((GLdouble)mouse_x, winY, 0.0, modelview, projection, viewport,
                 &near_x, &near_y, &near_z);
    gluUnProject((GLdouble)mouse_x, winY, 1.0, modelview, projection, viewport,
                 &far_x, &far_y, &far_z);

    dir_x = far_x - near_x;
    dir_y = far_y - near_y;
    dir_z = far_z - near_z;

    /* La mesa esta en el plano Y = 0. Se busca t tal que
       near_y + t*dir_y = 0 (interseccion rayo-plano). */
    if (dir_y == 0.0) {
        return 0; /* rayo paralelo al plano de la mesa: no hay interseccion */
    }
    t = -near_y / dir_y;
    if (t < 0.0) {
        return 0; /* la interseccion queda "detras" de la camara: invalida */
    }

    *out_x = (float)(near_x + t * dir_x);
    *out_z = (float)(near_z + t * dir_z);
    return 1;
}
