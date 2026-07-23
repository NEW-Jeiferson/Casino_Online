/*
 * main.c
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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

/* Se encarga de almacenar los valores de las fichas disponibles */
static const float FICHAS[4] = { 10.0f, 25.0f, 50.0f, 100.0f };


/* Estruct encargada de almacenar el estado de la partida en curso */
typedef struct {
    Jugador      jugador;
    EstadoBolita bolita;
    float        angulo_rueda;

    Apuesta apuestas_activas[MAX_APUESTAS];
    int     num_apuestas_activas;

    int   ficha_actual_index;
    float monto_ficha_actual;

    int numero_ganador_pendiente;

    const char* mensaje_reflexivo_actual;
} EstadoPartida;


/* Variable global que mantiene el estado de la partida en curso */
static EstadoPartida partida;


/* Funcion que calcula el monto total apostado en la ronda actual */
static float monto_apostado_en_ronda(void) {
    float total = 0.0f;
    int i;
    for (i = 0; i < partida.num_apuestas_activas; i++) {
        total += partida.apuestas_activas[i].monto;
    }
    return total;
}


/* Funcion que verifica si el saldo alcanza para colocar una ficha */
static int saldo_alcanza_para_ficha(float monto_ficha) {
    return (partida.jugador.saldo - monto_apostado_en_ronda()) >= monto_ficha;
}



/* Funcion que verifica los fondos y pide un prestamo si es necesario */
static void verificar_fondos_y_pedir_prestamo_si_hace_falta(void) {
    if (estado_actual == ESTADO_JUGANDO && partida.jugador.saldo < partida.monto_ficha_actual) {
        cambiar_estado(ESTADO_PRESTAMO);
    }
}


/* Funcion que agrega una apuesta al arreglo activo si hay espacio y saldo suficiente */
static int agregar_apuesta(TipoApuesta tipo, int valor, float monto) {

    if (partida.num_apuestas_activas >= MAX_APUESTAS) return 0;
    partida.apuestas_activas[partida.num_apuestas_activas].tipo = tipo;
    partida.apuestas_activas[partida.num_apuestas_activas].valor = valor;
    partida.apuestas_activas[partida.num_apuestas_activas].monto = monto;
    partida.num_apuestas_activas++;

    registrar_apuesta(&partida.jugador, monto); /* suma al HUD ya */
    return 1;
}


/* Funcion que quita una apuesta del arreglo activo y ajusta el saldo del jugador */
static void quitar_apuesta(int indice) {
    int i;
    if (indice < 0 || indice >= partida.num_apuestas_activas) return;

    anular_apuesta(&partida.jugador, partida.apuestas_activas[indice].monto);

    for (i = indice; i < partida.num_apuestas_activas - 1; i++) {
        partida.apuestas_activas[i] = partida.apuestas_activas[i + 1];
    }
    partida.num_apuestas_activas--;
}


/* Funcion que quita la ultima apuesta de un tipo y valor especifico */
static void quitar_ultima_apuesta_tipo_valor(TipoApuesta tipo, int valor) {
    int i;
    for (i = partida.num_apuestas_activas - 1; i >= 0; i--) {
        if (partida.apuestas_activas[i].tipo == tipo && partida.apuestas_activas[i].valor == valor) {
            quitar_apuesta(i);
            break;
        }
    }
}


/* Funcion que dibuja la escena completa, incluyendo la mesa, la rueda, la bolita y el HUD */
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    gluLookAt(0.0, 15.0, 7.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0);

    glPushMatrix();
    dibujar_mesa();
    dibujar_tablero_apuestas(partida.apuestas_activas, partida.num_apuestas_activas);

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    glRotatef(partida.angulo_rueda, 0.0f, 1.0f, 0.0f);
    dibujar_rueda();
    dibujar_pista_numerada(); 

    glPushMatrix();
    dibujar_bolita(&partida.bolita);
    glPopMatrix();
    glPopMatrix();

    dibujar_vidrio_protector();
    glPopMatrix();

    dibujar_hud(&partida.jugador, partida.monto_ficha_actual, partida.num_apuestas_activas);
    dibujar_pantalla_segun_estado(estado_actual, &partida.jugador, partida.mensaje_reflexivo_actual);

    glutSwapBuffers();
}


/* Funcion que se llama cuando la ventana se redimensiona, ajustando la proyeccion y el viewport */
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


/* Funcion que maneja la entrada del teclado, incluyendo la seleccion de fichas, apuestas y giro de la bolita */
void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: 
        exit(0);
        break;


		/* Seleccion de ficha actual: 1-4 */
    case '1': partida.ficha_actual_index = 0; partida.monto_ficha_actual = FICHAS[0]; break;
    case '2': partida.ficha_actual_index = 1; partida.monto_ficha_actual = FICHAS[1]; break;
    case '3': partida.ficha_actual_index = 2; partida.monto_ficha_actual = FICHAS[2]; break;
    case '4': partida.ficha_actual_index = 3; partida.monto_ficha_actual = FICHAS[3]; break;

    case 'r':
    case 'R':
        
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_ROJO, partida.monto_ficha_actual);
        }
        break;

    case 'n':
    case 'N':
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_NEGRO, partida.monto_ficha_actual);
        }
        break;

    case ' ':

        /* Giro de la bolita */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.num_apuestas_activas > 0) {

            int sector_ganador = rand() % 37;
            int numero_ganador = ORDEN_RUEDA_EUROPEA[sector_ganador];
            float angulo_sector_centro = ((float)sector_ganador + 0.5f) * (360.0f / 37.0f);

            partida.numero_ganador_pendiente = numero_ganador;

            iniciar_giro_bolita_hacia_absoluto(&partida.bolita, angulo_sector_centro, 1 /* vuelta extra visual */);
        }
        break;

    case 'p':
    case 'P':
        if (estado_actual == ESTADO_PRESTAMO) {
            pedir_prestamo(&partida.jugador, 200.0f, 0.20f);

            if (deuda_es_impagable(&partida.jugador, 1000.0f)) {
                cambiar_estado(ESTADO_GAME_OVER);
            }
            else {
                cambiar_estado(ESTADO_JUGANDO);
            }
        }
        break;

    case 's':
    case 'S':
		/* Salir de la sesion actual y volver al menu principal */
        if (estado_actual == ESTADO_MENSAJE_REFLEXIVO) {
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_SESION_TERMINADA);
        }
        else if (estado_actual == ESTADO_PRESTAMO) {
            cambiar_estado(ESTADO_SESION_TERMINADA);
        }
        break;
    case 8: 
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.num_apuestas_activas > 0) {
            quitar_apuesta(partida.num_apuestas_activas - 1);
        }
        break;

    case 13: 
        if (estado_actual == ESTADO_MENU) {
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_GAME_OVER) {
            inicializar_jugador(&partida.jugador, 1000.0f);
            partida.num_apuestas_activas = 0;
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_SESION_TERMINADA) {
            inicializar_jugador(&partida.jugador, 1000.0f);
            partida.num_apuestas_activas = 0;
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_MENSAJE_REFLEXIVO) {

            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
            verificar_fondos_y_pedir_prestamo_si_hace_falta();
        }
        break;

    default:
        break;
    }
    glutPostRedisplay();
}


/* Funcion que maneja los clics del mouse, agregando o quitando apuestas segun el boton presionado */
void mouse_click(int boton, int estado_boton, int x, int y) {
    float wx, wz;
    int numero;
    TipoApuesta tipo_zona;
    int valor_zona;

    if (estado_boton != GLUT_DOWN) return;
    if (estado_actual != ESTADO_JUGANDO || partida.bolita.girando) return;

    if (boton == GLUT_LEFT_BUTTON) {
        /* Clic izquierdo: agregar ficha */
        if (!saldo_alcanza_para_ficha(partida.monto_ficha_actual)) return;
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;

        if (obtener_numero_en_punto(wx, wz, &numero)) {
            agregar_apuesta(APUESTA_NUMERO, numero, partida.monto_ficha_actual);
            glutPostRedisplay();
        }
        else if (obtener_zona_especial_en_punto(wx, wz, &tipo_zona, &valor_zona)) {
            agregar_apuesta(tipo_zona, valor_zona, partida.monto_ficha_actual);
            glutPostRedisplay();
        }
    }
    else if (boton == GLUT_RIGHT_BUTTON) {
        /* Clic derecho: quitar la ultima ficha puesta en ese numero o zona */
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;

        if (obtener_numero_en_punto(wx, wz, &numero)) {
            quitar_ultima_apuesta_tipo_valor(APUESTA_NUMERO, numero);
            glutPostRedisplay();
        }
        else if (obtener_zona_especial_en_punto(wx, wz, &tipo_zona, &valor_zona)) {
            quitar_ultima_apuesta_tipo_valor(tipo_zona, valor_zona);
            glutPostRedisplay();
        }
    }
}


/* Funcion que maneja el movimiento del mouse, actualizando la celda hover en el tablero de apuestas */
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


/* Funcion que se llama en cada frame, actualizando la animacion de la bolita y resolviendo el resultado cuando se detiene */
void idle(void) {

    static int tiempo_anterior_ms = -1;
    int   tiempo_actual_ms = glutGet(GLUT_ELAPSED_TIME);
    float delta_tiempo;
    int estaba_girando = partida.bolita.girando;

    if (tiempo_anterior_ms < 0) tiempo_anterior_ms = tiempo_actual_ms;
    delta_tiempo = (float)(tiempo_actual_ms - tiempo_anterior_ms) / 1000.0f;
    tiempo_anterior_ms = tiempo_actual_ms;
    if (delta_tiempo < 0.0f) delta_tiempo = 0.0f;   /* por si el contador diera un valor raro */
    if (delta_tiempo > 0.1f) delta_tiempo = 0.1f;   /* clamp anti-salto */

    if (partida.bolita.girando) {

        partida.angulo_rueda += VELOCIDAD_RUEDA_DURANTE_GIRO * delta_tiempo;
        if (partida.angulo_rueda >= 360.0f) partida.angulo_rueda -= 360.0f;
    }

    actualizar_bolita(&partida.bolita, delta_tiempo);


	/* Si la bolita estaba girando y ahora se detuvo, y hay apuestas activas, se calcula el resultado */
    if (estaba_girando && !partida.bolita.girando && partida.num_apuestas_activas > 0) {

        int   numero_ganador = partida.numero_ganador_pendiente;
        float ganancia_total = calcular_ganancia_total(partida.apuestas_activas, partida.num_apuestas_activas, numero_ganador);


#ifdef _DEBUG
        {
            const char* color_texto = (color_de_numero(numero_ganador) == COLOR_ROJO) ? "ROJO"
                : (color_de_numero(numero_ganador) == COLOR_NEGRO) ? "NEGRO" : "VERDE";
            printf("[RESULTADO REAL] numero=%d color=%s ganancia=%.2f\n", numero_ganador, color_texto, ganancia_total);
        }
#endif

        aplicar_resultado_apuesta(&partida.jugador, ganancia_total);
        partida.num_apuestas_activas = 0;

        partida.mensaje_reflexivo_actual = verificar_mensaje_reflexivo(&partida.jugador);
        if (partida.mensaje_reflexivo_actual != NULL) {
            cambiar_estado(ESTADO_MENSAJE_REFLEXIVO);
        }
        else {
            verificar_fondos_y_pedir_prestamo_si_hace_falta();
        }
    }

    glutPostRedisplay();
}


/* Funcion principal del programa */
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    srand((unsigned int)time(NULL));

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta (MVP)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_jugador(&partida.jugador, 1000.0f); 
    inicializar_estado_juego();                     
    inicializar_bolita(&partida.bolita);
    partida.angulo_rueda = 0.0f;
    partida.num_apuestas_activas = 0;
    partida.ficha_actual_index = 0;
    partida.monto_ficha_actual = FICHAS[0];
    partida.numero_ganador_pendiente = -1;
    partida.mensaje_reflexivo_actual = NULL;

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