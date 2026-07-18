/* Implementacion de las pantallas de transicion.
  Contiene las funciones para renderizar las pantallas superpuestas (menu,
  prestamo, game over y mensajes reflexivos
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pantallas.h"

 /* Funcion auxiliar para dibujar una cadena de texto en pantalla en 2D. */
    
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Dibuja la pantalla de menu principal inicial.
   
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


    dibujar_texto_2d(cx - 220.0f, cy + 105.0f, "Controles:");
    dibujar_texto_2d(cx - 220.0f, cy + 78.0f, "1 - 4            Elegir monto de ficha");
    dibujar_texto_2d(cx - 220.0f, cy + 51.0f, "Clic izquierdo   Apostar en la celda senalada");
    dibujar_texto_2d(cx - 220.0f, cy + 24.0f, "Clic derecho     Quitar una ficha de esa celda");
    dibujar_texto_2d(cx - 220.0f, cy - 3.0f, "BACKSPACE        Deshacer la ultima ficha");
    dibujar_texto_2d(cx - 220.0f, cy - 30.0f, "ESPACIO          Girar la ruleta");
    dibujar_texto_2d(cx - 220.0f, cy - 57.0f, "P                Pedir prestamo (sin saldo)");

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - 150.0f, cy - 110.0f, "[ENTER] Comenzar     [ESC] Salir");

  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Dibuja la pantalla de advertencia cuando el jugador se queda sin saldo.
   
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

    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 50.0f,
        "[P] Pedir prestamo y seguir     [ESC] Salir");

    /* Restaurar estado 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Dibuja una pantalla de pausa con un mensaje reflexivo progresivo//
   
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;


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

    
    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* titulo = "UN MOMENTO...";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 130.0f, titulo);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    {
        char copia[512];
        char* inicio;
        float y = cy + 80.0f;
        const float ALTO_LINEA = 24.0f;

        
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

    
    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* instruccion = "[ENTER] Continuar jugando";
        dibujar_texto_2d(cx - (float)strlen(instruccion) * ANCHO_CHAR_APROX / 2.0f, cy - 150.0f, instruccion);
    }

   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Dibuja la pantalla final de "Game Over" cuando la deuda supera el limite tolerado.//
   
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
        float pulso = (sinf(tiempo_ms * 0.002f) + 1.0f) / 2.0f;
        float r = 0.3f + pulso * 0.5f;

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


    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 30.0f,
        "GAME OVER: la casa siempre gana.");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 55.0f,
        "[ESC] Salir     [ENTER] Reiniciar");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* Enrutador principal que decide que pantalla superpuesta dibujar
   en funcion del estado actual del juego. Si el estado es ESTADO_JUGANDO,*/

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
    case ESTADO_MENSAJE_REFLEXIVO:
        if (mensaje_reflexivo != NULL) dibujar_pantalla_mensaje_reflexivo(mensaje_reflexivo);
        break;
    default:
        break;
    }
}