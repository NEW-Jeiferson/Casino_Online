/*
 * main.c
 * -----------------------------------------------------------------------
 * Punto de entrada del programa. Aqui se configura la ventana de freeGLUT,
 * se registran los callbacks del ciclo de vida (display, reshape, keyboard,
 * idle) y se inicializan los subsistemas (jugador, estado del juego,
 * geometria de la ruleta, iluminacion).
 *
 * Responsable sugerido: Persona C (sistema de juego y UI), en coordinacion
 * con A y B para las llamadas de inicializacion de sus modulos.
 * -----------------------------------------------------------------------
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/estado_juego.h"
#include "core/jugador.h"
#include "ruleta/ruleta_geometria.h"
#include "ruleta/ruleta_animacion.h"
#include "render/iluminacion.h"
#include "render/materiales.h"
#include "ui/hud.h"
#include "ui/pantallas.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

/* ------------------------------------------------------------------- */
/* Callback de dibujo. Se ejecuta cada frame.                          */
/* ------------------------------------------------------------------- */
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* TODO: posicionar camara (gluLookAt) */

    /* TODO (Persona A): dibujar mesa + rueda + bolita usando pila de matrices */
    /* dibujar_mesa(); */
    /* dibujar_rueda(); */
    /* dibujar_bolita(); */

    /* TODO (Persona C): dibujar HUD y pantallas de transicion segun el estado */
    /* dibujar_hud(&jugador); */
    /* dibujar_pantalla_segun_estado(estado_actual); */

    glutSwapBuffers();
}

/* ------------------------------------------------------------------- */
/* Callback de reshape. Se ejecuta cuando cambia el tamano de ventana. */
/* ------------------------------------------------------------------- */
void reshape(int ancho, int alto) {
    glViewport(0, 0, ancho, alto);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* TODO: gluPerspective o glOrtho segun se defina en el AR correspondiente */

    glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------- */
/* Callback de teclado.                                                 */
/* ------------------------------------------------------------------- */
void teclado(unsigned char tecla, int x, int y) {
    switch (tecla) {
        case 27: /* ESC */
            exit(0);
            break;
        /* TODO (Persona C): apostar, girar, aceptar/rechazar prestamo, reiniciar */
        default:
            break;
    }
    glutPostRedisplay();
}

/* ------------------------------------------------------------------- */
/* Callback idle. Actualiza animaciones y transiciones de estado.     */
/* ------------------------------------------------------------------- */
void idle(void) {
    /* TODO (Persona A): actualizar giro de rueda / bolita */
    /* TODO (Persona C): actualizar maquina de estados        */
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    /* GLUT_MULTISAMPLE habilita MSAA */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta (MVP)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    /* TODO: inicializar iluminacion, materiales, geometria, jugador */
    /* inicializar_iluminacion(); */
    /* inicializar_jugador(&jugador); */
    /* inicializar_estado_juego(); */

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
