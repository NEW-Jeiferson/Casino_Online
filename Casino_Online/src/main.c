/*
 * main.c
 * -----------------------------------------------------------------------
 * Punto de entrada del programa. Aqui se configura la ventana de GLUT,
 * se registran los callbacks del ciclo de vida (display, reshape, keyboard,
 * idle) y se inicializan los subsistemas (jugador, estado del juego,
 * geometria de la ruleta, iluminacion).
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
 /* Estado global de la aplicacion.                                     */
 /* TODO (Persona C): valorar si esto conviene encapsularse distinto     */
 /* (por ejemplo en un struct "Juego") a medida que crezca.              */
 /* ------------------------------------------------------------------- */
static Jugador      jugador;
static EstadoBolita  bolita;
static float         angulo_rueda = 0.0f;

/* ------------------------------------------------------------------- */
/* Callback de dibujo. Se ejecuta cada frame.                          */
/* ------------------------------------------------------------------- */
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Camara fija mirando hacia la mesa desde arriba/al frente.
       TODO (Persona A/C): ajustar posicion si se agrega control de
       camara con teclado o mouse mas adelante. */
    gluLookAt(0.0, 9.0, 11.0,   /* posicion del ojo */
        0.0, 0.0, 0.0,    /* punto al que mira */
        0.0, 1.0, 0.0);  /* vector "arriba" */

    /* Jerarquia real de la escena con pila de matrices:
       mesa (raiz) -> rueda (hijo: traslacion Y + rotacion) -> bolita
       (nieto: hereda la transformacion de la rueda). El vidrio
       protector es hijo de mesa, hermano de rueda (no gira). */
    glPushMatrix();
    dibujar_mesa();

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    glRotatef(angulo_rueda, 0.0f, 1.0f, 0.0f);
    dibujar_rueda();

    /* La bolita se dibuja DENTRO de este bloque para heredar
       la traslacion/rotacion de la rueda (jerarquia real). */
    glPushMatrix();
    dibujar_bolita(&bolita);
    glPopMatrix();
    glPopMatrix();

    dibujar_vidrio_protector();
    glPopMatrix();

    /* HUD y pantallas de transicion se dibujan en 2D superpuestos a la
       escena 3D (cada funcion se encarga de cambiar a proyeccion
       ortografica internamente, ver TODO en hud.c/pantallas.c). */
    dibujar_hud(&jugador);
    dibujar_pantalla_segun_estado(estado_actual, &jugador);

    glutSwapBuffers();
}

/* ------------------------------------------------------------------- */
/* Callback de reshape. Se ejecuta cuando cambia el tamano de ventana. */
/* ------------------------------------------------------------------- */
void reshape(int ancho, int alto) {
    float aspecto;
    if (alto == 0) alto = 1; /* evitar division por cero */
    aspecto = (float)ancho / (float)alto;

    glViewport(0, 0, ancho, alto);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* Proyeccion en perspectiva: se eligio sobre glOrtho porque la
       escena es una mesa 3D con profundidad real (rueda, bolita,
       vidrio a distintas alturas), no un dibujo plano tipo HUD.
       TODO: si se decide documentar esto formalmente, agregar el AR
       correspondiente sobre la eleccion gluPerspective vs glOrtho. */
    gluPerspective(45.0, aspecto, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------- */
/* Callback de teclado.                                                 */
/* ------------------------------------------------------------------- */
void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: /* ESC */
        exit(0);
        break;
    case ' ': /* ESPACIO: demo temporal para girar la bolita */
        /* TODO (Persona C): reemplazar por la logica real de
           "apostar y girar", conectada a la maquina de estados
           (ESTADO_JUGANDO -> validar apuesta -> iniciar giro). */
        if (!bolita.girando) {
            angulo_rueda += 0.0f; /* la rueda podria acelerar tambien, ver TODO abajo */
            iniciar_giro_bolita(&bolita, 220.0f);
        }
        break;
        /* TODO (Persona C): apostar, aceptar/rechazar prestamo, reiniciar */
    default:
        break;
    }
    glutPostRedisplay();
}

/* ------------------------------------------------------------------- */
/* Callback idle. Actualiza animaciones y transiciones de estado.     */
/* ------------------------------------------------------------------- */
void idle(void) {
    /* TODO (Persona A/C): reemplazar este delta fijo por un calculo
       real basado en glutGet(GLUT_ELAPSED_TIME) para que la animacion
       no dependa de que tan rapido corra cada computadora. */
    const float delta_tiempo = 0.016f; /* ~60 FPS asumidos */

    angulo_rueda += 15.0f * delta_tiempo; /* giro visual continuo de la rueda */
    if (angulo_rueda >= 360.0f) angulo_rueda -= 360.0f;

    actualizar_bolita(&bolita, delta_tiempo);

    /* TODO (Persona C): actualizar maquina de estados (por ejemplo,
       revisar si jugador.saldo llego a 0 y hacer cambiar_estado(...)) */

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

    /* Inicializacion de cada subsistema */
    inicializar_iluminacion();
    inicializar_jugador(&jugador, 1000.0f); /* saldo inicial de ejemplo */
    inicializar_estado_juego();
    inicializar_bolita(&bolita);
    generar_perfil_bezier_rueda();  /* no-op en el placeholder actual */
    construir_malla_rueda();        /* no-op en el placeholder actual */

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}