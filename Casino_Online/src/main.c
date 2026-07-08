/*
 * main.c
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "core/estado_juego.h"
#include "core/jugador.h"
#include "ruleta/ruleta_geometria.h"
#include "ruleta/ruleta_animacion.h"
#include "ruleta/mouse_picking.h"
#include "render/iluminacion.h"
#include "render/materiales.h"
#include "ui/hud.h"
#include "ui/pantallas.h"
#include "ui/tablero_apuestas.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

static Jugador      jugador;
static EstadoBolita bolita;
static float        angulo_rueda = 0.0f;

/* --- Sistema de apuestas multiples (Persona C) --- */
static Apuesta apuestas_activas[MAX_APUESTAS];
static int     num_apuestas_activas = 0;

/* Fichas seleccionables con teclas 1-4 */
static const float FICHAS[4] = { 10.0f, 25.0f, 50.0f, 100.0f };
static int   ficha_actual_index = 0;
static float monto_ficha_actual = 10.0f;

/* Agrega una apuesta al arreglo activo si hay espacio y saldo suficiente.
   Devuelve 1 si se agrego, 0 si no (arreglo lleno). */
static int agregar_apuesta(TipoApuesta tipo, int valor, float monto) {
    if (num_apuestas_activas >= MAX_APUESTAS) return 0;
    apuestas_activas[num_apuestas_activas].tipo = tipo;
    apuestas_activas[num_apuestas_activas].valor = valor;
    apuestas_activas[num_apuestas_activas].monto = monto;
    num_apuestas_activas++;

    registrar_apuesta(&jugador, monto); /* <-- nueva linea: suma al HUD YA */
    return 1;
}

/* Quita la apuesta en la posicion 'indice' del arreglo, recorriendo
   el resto un lugar hacia atras para no dejar huecos. Tambien
   descuenta ese monto de total_apostado (ver anular_apuesta). */
static void quitar_apuesta(int indice) {
    int i;
    if (indice < 0 || indice >= num_apuestas_activas) return;

    anular_apuesta(&jugador, apuestas_activas[indice].monto);

    for (i = indice; i < num_apuestas_activas - 1; i++) {
        apuestas_activas[i] = apuestas_activas[i + 1];
    }
    num_apuestas_activas--;
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 9.0, 11.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0);

    glPushMatrix();
    dibujar_mesa();
    dibujar_tablero_apuestas(apuestas_activas, num_apuestas_activas);

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    glRotatef(angulo_rueda, 0.0f, 1.0f, 0.0f);
    dibujar_rueda();

    glPushMatrix();
    dibujar_bolita(&bolita);
    glPopMatrix();
    glPopMatrix();

    dibujar_vidrio_protector();
    glPopMatrix();

    dibujar_hud(&jugador, monto_ficha_actual, num_apuestas_activas);
    dibujar_pantalla_segun_estado(estado_actual, &jugador);

    glutSwapBuffers();
}

void reshape(int ancho, int alto) {
    float aspecto;
    if (alto == 0) alto = 1;
    aspecto = (float)ancho / (float)alto;

    glViewport(0, 0, ancho, alto);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspecto, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: /* ESC */
        exit(0);
        break;

        /* --- Seleccion de monto de ficha --- */
    case '1': ficha_actual_index = 0; monto_ficha_actual = FICHAS[0]; break;
    case '2': ficha_actual_index = 1; monto_ficha_actual = FICHAS[1]; break;
    case '3': ficha_actual_index = 2; monto_ficha_actual = FICHAS[2]; break;
    case '4': ficha_actual_index = 3; monto_ficha_actual = FICHAS[3]; break;

    case 'r':
    case 'R':
        /* Apuesta a color ROJO: se agrega al arreglo de apuestas
           activas, pero NO dispara el giro (ver tecla ESPACIO). */
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && jugador.saldo >= monto_ficha_actual) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_ROJO, monto_ficha_actual);
        }
        break;

    case 'n':
    case 'N':
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && jugador.saldo >= monto_ficha_actual) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_NEGRO, monto_ficha_actual);
        }
        break;

    case ' ':
        /* Gira la bolita con todas las apuestas ya colocadas (mouse +
           teclado) en esta ronda. */
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && num_apuestas_activas > 0) {
            iniciar_giro_bolita(&bolita, 220.0f);
        }
        break;

    case 'p':
    case 'P':
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
    case 8: /* BACKSPACE: deshace la ultima ficha colocada (LIFO) */
        if (estado_actual == ESTADO_JUGANDO && !bolita.girando
            && num_apuestas_activas > 0) {
            quitar_apuesta(num_apuestas_activas - 1);
        }
        break;

    case 13: /* ENTER */
        if (estado_actual == ESTADO_MENU) {
            /* Minimo indispensable para poder salir del menu ahora que
               ya no se fuerza ESTADO_JUGANDO en main(). No es una
               pantalla de menu completa (fuera de este alcance). */
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_GAME_OVER) {
            inicializar_jugador(&jugador, 1000.0f);
            num_apuestas_activas = 0;
            cambiar_estado(ESTADO_JUGANDO);
        }
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

/* Clic izquierdo sobre el tablero: agrega una apuesta a NUMERO exacto
   con la ficha actualmente seleccionada. */
void mouse_click(int boton, int estado_boton, int x, int y) {
    float wx, wz;
    int numero;

    if (estado_boton != GLUT_DOWN) return;
    if (estado_actual != ESTADO_JUGANDO || bolita.girando) return;

    if (boton == GLUT_LEFT_BUTTON) {
        /* Clic izquierdo: agregar ficha */
        if (jugador.saldo < monto_ficha_actual) return;
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;
        if (!obtener_numero_en_punto(wx, wz, &numero)) return;

        agregar_apuesta(APUESTA_NUMERO, numero, monto_ficha_actual);
        glutPostRedisplay();
    }
    else if (boton == GLUT_RIGHT_BUTTON) {
        /* Clic derecho: quitar la ultima ficha puesta en ese numero */
        int i;
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;
        if (!obtener_numero_en_punto(wx, wz, &numero)) return;

        for (i = num_apuestas_activas - 1; i >= 0; i--) {
            if (apuestas_activas[i].tipo == APUESTA_NUMERO && apuestas_activas[i].valor == numero) {
                quitar_apuesta(i);
                break;
            }
        }
        glutPostRedisplay();
    }
}
/* Movimiento pasivo del mouse: actualiza que celda esta en hover, para
   que Dubenny pueda resaltarla visualmente (ver celda_hover_col/fila
   en tablero_apuestas.h). */
void mouse_mover(int x, int y) {
    float wx, wz;
    int col, fila;

    if (obtener_punto_clic_en_mesa(x, y, &wx, &wz)
        && obtener_celda_en_punto(wx, wz, &col, &fila)) {
        fijar_celda_hover(col, fila);
    }
    else {
        fijar_celda_hover(-1, -1);
    }
    glutPostRedisplay();
}

void idle(void) {
    const float delta_tiempo = 0.016f;
    int estaba_girando = bolita.girando;

    if (bolita.girando) {
        angulo_rueda += 60.0f * delta_tiempo;
        if (angulo_rueda >= 360.0f) angulo_rueda -= 360.0f;
    }

    actualizar_bolita(&bolita, delta_tiempo);

    /* --- Resolver resultado cuando la bolita se acaba de detener --- */
    if (estaba_girando && !bolita.girando && num_apuestas_activas > 0) {
        /* BUGFIX (ticket Luis #1): el angulo de la bolita por si solo
           es relativo a la rueda. Para saber en que casilla ABSOLUTA
           del mundo quedo, hay que sumarle el angulo actual de la
           rueda (angulo_rueda) antes de normalizar con fmodf. Sin
           esto, el numero/color ganador no correspondia a la posicion
           real de la bolita en pantalla. */
        float angulo_absoluto = fmodf(angulo_rueda + bolita.angulo_actual, 360.0f);
        int   numero_ganador;
        float ganancia_total;

        if (angulo_absoluto < 0.0f) angulo_absoluto += 360.0f;

        numero_ganador = calcular_numero_ganador(angulo_absoluto);
        ganancia_total = calcular_ganancia_total(apuestas_activas, num_apuestas_activas, numero_ganador);

        /* El monto ya se sumo a total_apostado en el momento de
           apostar (ver registrar_apuesta() dentro de agregar_apuesta),
           asi que aca solo se ajusta el saldo con la ganancia neta. */
        aplicar_resultado_apuesta(&jugador, ganancia_total);
        num_apuestas_activas = 0;

        if (jugador.saldo < monto_ficha_actual) {
            cambiar_estado(ESTADO_PRESTAMO);
        }
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta (MVP)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_jugador(&jugador, 1000.0f); /* saldo inicial real, ya no 50.0f de prueba */
    inicializar_estado_juego();             /* arranca en ESTADO_MENU, ya no se fuerza */
    inicializar_bolita(&bolita);
    generar_perfil_bezier_rueda();
    construir_malla_rueda();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutPassiveMotionFunc(mouse_mover);

    glutMainLoop();
    return 0;
}