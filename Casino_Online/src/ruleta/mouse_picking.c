/*
* Implementacion de la conversion de clic de mouse a punto 3D sobre la
*/
#include <GL/glut.h>
#include "mouse_picking.h"


/* Sirve para obtener el punto en la mesa donde se hizo clic */
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


    /* Sirve para invertir la coordenada Y, ya que OpenGL mide Y desde abajo hacia arriba, mientras que GLUT entrega la posicion del mouse con Y desde arriba hacia abajo */
    winY = (GLdouble)(viewport[3] - mouse_y);


	/* Se obtiene el rayo que pasa por la camara y el punto del mouse en la pantalla */
    gluUnProject((GLdouble)mouse_x, winY, 0.0, modelview, projection, viewport,
                 &near_x, &near_y, &near_z);
    gluUnProject((GLdouble)mouse_x, winY, 1.0, modelview, projection, viewport,
                 &far_x, &far_y, &far_z);


	/* Calcula la direccion del rayo */
    dir_x = far_x - near_x;
    dir_y = far_y - near_y;
    dir_z = far_z - near_z;


	/* Calcula la interseccion del rayo con el plano de la mesa (y=0) */
    if (dir_y == 0.0) {
        return 0;
    }
    t = -near_y / dir_y;
    if (t < 0.0) {
        return 0;
    }


	/* Calcula el punto de interseccion que representa el clic en la mesa */
    *out_x = (float)(near_x + t * dir_x);
    *out_z = (float)(near_z + t * dir_z);
    return 1;
}
