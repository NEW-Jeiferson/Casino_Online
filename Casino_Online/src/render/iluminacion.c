/*
 * iluminacion.c
 * Implementacion de la configuracion de luces. Ver iluminacion.h.
 */
#include <GL/glut.h>
#include "iluminacion.h"

void inicializar_iluminacion(void) {
    /* LUZ PRINCIPAL (LIGHT0): recalibrada para la camara ACTUAL de
       display() en main.c -gluLookAt(0,15,7 -> 0,0,1)-, mucho mas
       cenital (65 grados de elevacion) que la que se uso para calibrar
       esto originalmente (comentario historico: "camara en (0,9,11)",
       ~39 grados, que ya no existe en el codigo). Con una camara tan
       de-arriba-hacia-abajo, una luz alta y casi centrada (2,10,6, en
       vez de la (3,6,5) anterior) deja el highlight especular de la
       rueda/bolita visible cerca del centro de la escena en vez de
       correrse hacia un borde que la camara nueva ya no encuadra tan
       de lado. Si se vuelve a mover la camara, revisar esto de nuevo:
       la posicion "correcta" de esta luz depende del angulo de camara,
       no es un valor fijo para siempre. */
    GLfloat luz0_ambiental[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    GLfloat luz0_difusa[] = { 0.85f, 0.85f, 0.85f, 1.0f };
    GLfloat luz0_especular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat luz0_posicion[] = { 2.0f, 10.0f, 6.0f, 1.0f };

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

/* NOTA: aqui existia actualizar_posicion_luz(x,y,z), sin ningun
   llamador en el codigo actual (la luz nunca se mueve en tiempo de
   ejecucion). Se elimino junto con su declaracion en iluminacion.h;
   si en el futuro se anima la luz, se puede volver a agregar. */