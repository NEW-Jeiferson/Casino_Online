/*
 * pantallas.c
 * Implementacion de las pantallas de transicion. Ver pantallas.h.
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pantallas.h"


/* Metodo encargado de dibujar texto en 2D */
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}


/* Metodo encargado de dibujar la pantalla de menu */
void dibujar_pantalla_menu(void) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;



    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();


    glDisable(GL_BLEND);


    glColor3f(1.0f, 0.85f, 0.0f);


    dibujar_texto_2d(cx - 190.0f, cy + 190.0f, "CASINO ONLINE - SIMULADOR DE RULETA");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(cx - 220.0f, cy + 150.0f,
        "Juegas con saldo virtual. Nunca dinero real.");

    /* Controles */
    dibujar_texto_2d(cx - 220.0f, cy + 105.0f, "Controles:");
    dibujar_texto_2d(cx - 220.0f, cy + 78.0f, "1 - 4            Elegir monto de ficha");
    dibujar_texto_2d(cx - 220.0f, cy + 51.0f, "Clic izquierdo   Apostar en la celda senalada");
    dibujar_texto_2d(cx - 220.0f, cy + 24.0f, "Clic derecho     Quitar una ficha de esa celda");
    dibujar_texto_2d(cx - 220.0f, cy - 3.0f, "BACKSPACE        Deshacer la ultima ficha");
    dibujar_texto_2d(cx - 220.0f, cy - 30.0f, "ESPACIO          Girar la ruleta");
    dibujar_texto_2d(cx - 220.0f, cy - 57.0f, "P                Pedir prestamo (sin saldo)");
    dibujar_texto_2d(cx - 220.0f, cy - 84.0f, "S                Terminar la sesion (si se ofrece la opcion)");


    /* Opciones de salida */
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - 150.0f, cy - 110.0f, "[ENTER] Comenzar     [ESC] Salir");


    /* Se encarga de restaurar el estado 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Metodo encargado de dibujar la pantalla de prestamo */
void dibujar_pantalla_prestamo(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    char buffer[128];


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glDisable(GL_BLEND);



    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f + 40.0f,
        "Ya perdiste tu saldo inicial.");
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f + 15.0f,
        "En la vida real, este seria el momento de parar.");

    sprintf_s(buffer, sizeof(buffer), "Deuda actual: %.2f", jugador->deuda);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 15.0f, buffer);


	/* Opciones de salida */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 50.0f,
        "[P] Pedir prestamo y seguir");
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 75.0f,
        "[S] Terminar la sesion aqui, sin pedir prestamo");

    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Metodo encargado de dibujar la pantalla de mensaje reflexivo */
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje, const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;


	/* Se encarga de restaurar el estado 3D */
    int es_repeticion = (jugador != NULL && jugador->mensajes_reflexivos_mostrados >= 2);


    const float ANCHO_CHAR_APROX = 10.5f;


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.02f, 0.05f, 0.12f, 0.82f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glDisable(GL_BLEND);

    
    /* Se dibuja el titulo */
    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* titulo = "UN MOMENTO...";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 130.0f, titulo);
    }


    /* Cuerpo del Mensaje */
    glColor3f(1.0f, 1.0f, 1.0f);
    {
        char copia[700];
        char* inicio;
        float y = cy + 80.0f;
        const float ALTO_LINEA = 24.0f;


		/* Se hace una copia del mensaje para poder modificarlo sin alterar el original */
        strncpy_s(copia, sizeof(copia), mensaje, _TRUNCATE);

        inicio = copia;
        for (;;) {
            char* fin = strchr(inicio, '\n');
            if (fin != NULL) *fin = '\0';

            if (inicio[0] != '\0') {
                dibujar_texto_2d(cx - (float)strlen(inicio) * ANCHO_CHAR_APROX / 2.0f, y, inicio);
            }
            y -= ALTO_LINEA;

            if (fin == NULL) break;
            inicio = fin + 1;
        }
    }


    
	/* Opciones de salida */
    {
        const char* op_seguir = "[ENTER] Seguir jugando";
        const char* op_terminar = "[S] Terminar la sesion aqui";
        float y_prominente = cy - 150.0f;
        float y_secundaria = cy - 180.0f;

        if (!es_repeticion) {
            glColor3f(0.55f, 0.75f, 1.0f);
            dibujar_texto_2d(cx - (float)strlen(op_seguir) * ANCHO_CHAR_APROX / 2.0f, y_prominente, op_seguir);
            glColor3f(0.45f, 0.45f, 0.45f);
            dibujar_texto_2d(cx - (float)strlen(op_terminar) * ANCHO_CHAR_APROX / 2.0f, y_secundaria, op_terminar);
        }
        else {
            glColor3f(1.0f, 0.85f, 0.0f);
            dibujar_texto_2d(cx - (float)strlen(op_terminar) * ANCHO_CHAR_APROX / 2.0f, y_prominente, op_terminar);
            glColor3f(0.45f, 0.45f, 0.45f);
            dibujar_texto_2d(cx - (float)strlen(op_seguir) * ANCHO_CHAR_APROX / 2.0f, y_secundaria, op_seguir);
        }
    }


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Metodo encargado de dibujar la pantalla de game over */
void dibujar_pantalla_game_over(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    char buffer[128];


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);


    {
        float tiempo_ms = (float)glutGet(GLUT_ELAPSED_TIME);
        float pulso = (sinf(tiempo_ms * 0.002f) + 1.0f) / 2.0f; /* 0..1 */
        float r = 0.3f + pulso * 0.5f; /* oscila entre 0.3 y 0.8 de rojo */

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(r, 0.0f, 0.0f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((float)ancho, 0.0f);
        glVertex2f((float)ancho, (float)alto);
        glVertex2f(0.0f, (float)alto);
        glEnd();

        glDisable(GL_BLEND);
    }


    /* Se encaja el texto en el centro de la pantalla */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 120.0f,
        "=== RECIBO FINAL ===");

    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 90.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos solicitados: %d", jugador->prestamos_activos);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 65.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Interes acumulado: %.2f", jugador->interes_acumulado);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 40.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Deuda final: %.2f", jugador->deuda);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 15.0f, buffer);


    /* Mensaje final */
    sprintf_s(buffer, sizeof(buffer), "Pediste %d prestamo(s) para poder seguir jugando,", jugador->prestamos_activos);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 30.0f, buffer);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 55.0f,
        "y la deuda crecio hasta ser impagable.");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 90.0f,
        "[ESC] Salir     [ENTER] Reiniciar");


	/* Se encarga de restaurar el estado 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Metodo encargado de dibujar la pantalla de sesion terminada */
void dibujar_pantalla_sesion_terminada(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    char buffer[128];


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.02f, 0.05f, 0.12f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glDisable(GL_BLEND);


    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 150.0f, (float)alto / 2.0f + 150.0f,
        "SESION TERMINADA POR VOS");


    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 100.0f,
        "Elegiste parar antes de que la situacion");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 76.0f,
        "empeorara. Este es el resumen de la sesion:");

    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 40.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos solicitados: %d", jugador->prestamos_activos);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 15.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Deuda con la que termina la sesion: %.2f", jugador->deuda);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 10.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Saldo final: %.2f", jugador->saldo);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 35.0f, buffer);


    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 80.0f,
        "Parar a tiempo tambien es una forma de jugar bien.");
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 115.0f,
        "[ESC] Salir     [ENTER] Jugar una sesion nueva");


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Metodo encargado de dibujar la pantalla segun el estado del juego */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const char* mensaje_reflexivo) {
    switch (estado) {
    case ESTADO_MENU:
        dibujar_pantalla_menu();
        break;
    case ESTADO_PRESTAMO:
        dibujar_pantalla_prestamo(jugador);
        break;
    case ESTADO_GAME_OVER:
        dibujar_pantalla_game_over(jugador);
        break;
    case ESTADO_SESION_TERMINADA:
        dibujar_pantalla_sesion_terminada(jugador);
        break;
    case ESTADO_MENSAJE_REFLEXIVO:
        if (mensaje_reflexivo != NULL) dibujar_pantalla_mensaje_reflexivo(mensaje_reflexivo, jugador);
        break;
    default:
        break; /* ESTADO_JUGANDO no requiere overlay */
    }
}