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
#include "ui/tablero_apuestas.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

 /* Monto fijo de apuesta para el MVP (mas adelante se podria pedir por
    teclado un monto variable, pero por ahora simplificamos) */
#define MONTO_APUESTA_FIJO 50.0f

    /* Estado global de la aplicacion.                                     */
    /* TODO (Persona C): valorar si esto conviene encapsularse distinto     */
    /* (por ejemplo en un struct "Juego") a medida que crezca.              */
    
static Jugador      jugador;
static EstadoBolita  bolita;
static float         angulo_rueda = 0.0f;

/* --- Estado de apuesta (Persona C) ---
   Estas variables "recuerdan" que aposto el jugador mientras la bolita
   esta girando, para poder resolver el resultado cuando termine. */

   /* Cuanto dinero aposto el jugador en la ronda actual */
static float      monto_apuesta_actual = 0.0f;

/* A que color aposto (COLOR_ROJO o COLOR_NEGRO), definido en estado_juego.h */
static ColorRuleta color_apostado = COLOR_ROJO;

/* 1 si hay una apuesta activa esperando resultado, 0 si no hay ninguna
   (evita resolver una apuesta que no existe cuando la bolita se detiene) */
static int         hay_apuesta_pendiente = 0;

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
    dibujar_tablero_apuestas();

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

// Callback de reshape. Se ejecuta cuando cambia el tamano de ventana. 

void reshape(int ancho, int alto) {
    float aspecto;
    if (alto == 0) alto = 1; 
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


// Callback de teclado.                                                 

void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: /* ESC */
        exit(0);
        break;

    case 'r':
    case 'R':
        /* Apostar a ROJO. Solo se permite si estamos jugando, la bolita
           esta detenida (no se puede apostar a mitad de giro) y el
           jugador tiene saldo suficiente. */
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && jugador.saldo >= MONTO_APUESTA_FIJO) {
            color_apostado = COLOR_ROJO;
            monto_apuesta_actual = MONTO_APUESTA_FIJO;
            hay_apuesta_pendiente = 1;
            iniciar_giro_bolita(&bolita, 220.0f);
        }
        break;

    case 'n':
    case 'N':
        /* Apostar a NEGRO. Misma logica que ROJO. */
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && jugador.saldo >= MONTO_APUESTA_FIJO) {
            color_apostado = COLOR_NEGRO;
            monto_apuesta_actual = MONTO_APUESTA_FIJO;
            hay_apuesta_pendiente = 1;
            iniciar_giro_bolita(&bolita, 220.0f);
        }
        break;

    case 'p':
    case 'P':
        /* Pedir prestamo: solo tiene sentido en la pantalla de prestamo.
           Si la deuda acumulada se vuelve impagable, no hay vuelta atras
           y se pasa directo a Game Over. */
        if (estado_actual == ESTADO_PRESTAMO) {
            pedir_prestamo(&jugador, 200.0f, 0.20f);

            if (deuda_es_impagable(&jugador, 1000.0f)) {
                cambiar_estado(ESTADO_GAME_OVER);
            }
            else {
                cambiar_estado(ESTADO_JUGANDO);
            }
        }
        break;

    case 13: /* ENTER */
        /* Reiniciar partida desde la pantalla de Game Over: vuelve a
           poner el saldo inicial y limpia cualquier apuesta pendiente
           de la partida anterior. */
        if (estado_actual == ESTADO_GAME_OVER) {
            inicializar_jugador(&jugador, 1000.0f);
            hay_apuesta_pendiente = 0;
            cambiar_estado(ESTADO_JUGANDO);
        }
        break;

    default:
        break;
    }
    glutPostRedisplay();
}



// Callback idle. Actualiza animaciones y transiciones de estado.   

void idle(void) {
    /* TODO (Persona A/C): reemplazar este delta fijo por un calculo
       real basado en glutGet(GLUT_ELAPSED_TIME) para que la animacion
       no dependa de que tan rapido corra cada computadora. */
    const float delta_tiempo = 0.016f; /* ~60 FPS asumidos */

    /* Guardamos si la bolita estaba girando ANTES de actualizarla, para
       poder detectar el momento exacto en que se detiene (flanco de
       bajada: giraba -> ya no gira). */
    int estaba_girando = bolita.girando;

    if (bolita.girando) {
        angulo_rueda += 60.0f * delta_tiempo; /* velocidad de giro de la rueda */
        if (angulo_rueda >= 360.0f) angulo_rueda -= 360.0f;
    }

    actualizar_bolita(&bolita, delta_tiempo);

    //  Resolver resultado cuando la bolita se acaba de detener 
    if (estaba_girando && !bolita.girando && hay_apuesta_pendiente) {
        ColorRuleta color_ganador = calcular_color_ganador(bolita.angulo_actual);
        float ganancia;

        if (color_apostado == color_ganador) {
           
            ganancia = monto_apuesta_actual;
        }
        else {
            
            ganancia = -monto_apuesta_actual;
        }

        aplicar_resultado_apuesta(&jugador, monto_apuesta_actual, ganancia);
        hay_apuesta_pendiente = 0;

        if (jugador.saldo < MONTO_APUESTA_FIJO) {
            cambiar_estado(ESTADO_PRESTAMO); /* directo a la pantalla de prestamo */
        }
        
    }

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
    inicializar_jugador(&jugador, 50.0f); /* saldo inicial de ejemplo */
    inicializar_estado_juego();
    estado_actual = ESTADO_JUGANDO; /* fuerza estado de prueba, quitar cuando el menu funcione */
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