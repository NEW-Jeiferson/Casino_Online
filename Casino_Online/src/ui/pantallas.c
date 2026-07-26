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

 /* Funcion auxiliar para dibujar texto (duplicada de hud.c a proposito,
    para no acoplar ambos archivos con un header compartido) */
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

static GLuint g_textura_carga = 0;
static GLuint g_textura_casino = 0;

void inicializar_texturas_pantallas(unsigned int tex_carga, unsigned int tex_casino) {
    g_textura_carga = (GLuint)tex_carga;
    g_textura_casino = (GLuint)tex_casino;
}

static void dibujar_icono_advertencia(float cx, float cy, float r) {
    glColor3f(1.0f, 0.85f, 0.0f); /* Dorado/Amarillo */
    glBegin(GL_TRIANGLES);
    glVertex2f(cx, cy + r);
    glVertex2f(cx - r, cy - r);
    glVertex2f(cx + r, cy - r);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f); /* Negro */
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(cx, cy + r * 0.4f);
    glVertex2f(cx, cy - r * 0.2f);
    glEnd();
    
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy - r * 0.6f);
    glEnd();
    glLineWidth(1.0f);
}

static void dibujar_icono_info(float cx, float cy, float r) {
    const int SEGMENTOS = 16;
    int k;

    glColor3f(0.55f, 0.75f, 1.0f); /* Azul info */
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (k = 0; k <= SEGMENTOS; k++) {
        float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
        glVertex2f(cx + cosf(ang) * r, cy + sinf(ang) * r);
    }
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(cx, cy + r * 0.2f);
    glVertex2f(cx, cy - r * 0.5f);
    glEnd();

    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy + r * 0.5f);
    glEnd();
    glLineWidth(1.0f);
}

/* Ancho promedio aproximado de un caracter en HELVETICA_18 (bitmap font
   de GLUT), usado solo para centrar texto -no hace falta mas precision
   que esta para mensajes cortos, y evita acoplar esto a la metrica
   exacta de la fuente (que GLUT no expone facil para fuentes bitmap). */
#define ANCHO_CHAR_APROX 10.5f

   /* Dibuja un bloque de texto separado por '\n', centrado horizontalmente
      en cx, una linea debajo de la otra empezando en y_inicio. Preserva
      lineas vacias ("\n\n") como espacio en blanco -no usa strtok(), que
      colapsa delimitadores consecutivos y se comeria ese espacio.
      Extraido de dibujar_pantalla_mensaje_reflexivo() para reusarlo en
      dibujar_pantalla_educacion(). */
static void dibujar_texto_multilinea_centrado(const char* texto, float cx, float y_inicio, float alto_linea) {
    char copia[900];
    char* inicio;
    float y = y_inicio;

    strncpy_s(copia, sizeof(copia), texto, _TRUNCATE);

    inicio = copia;
    for (;;) {
        char* fin = strchr(inicio, '\n');
        if (fin != NULL) *fin = '\0';

        if (inicio[0] != '\0') {
            dibujar_texto_2d(cx - (float)strlen(inicio) * ANCHO_CHAR_APROX / 2.0f, y, inicio);
        }
        y -= alto_linea;

        if (fin == NULL) break;
        inicio = fin + 1;
    }
}

/* Igual que la anterior, pero alineado a la izquierda en x_inicio en
   vez de centrado -para bloques con listas/vinetas, donde centrar cada
   linea se ve raro (los guiones de las vinetas quedarian a distinta
   distancia del borde en cada linea). */
static void dibujar_texto_multilinea_izquierda(const char* texto, float x_inicio, float y_inicio, float alto_linea) {
    char copia[900];
    char* inicio;
    float y = y_inicio;

    strncpy_s(copia, sizeof(copia), texto, _TRUNCATE);

    inicio = copia;
    for (;;) {
        char* fin = strchr(inicio, '\n');
        if (fin != NULL) *fin = '\0';

        if (inicio[0] != '\0') {
            dibujar_texto_2d(x_inicio, y, inicio);
        }
        y -= alto_linea;

        if (fin == NULL) break;
        inicio = fin + 1;
    }
}

void dibujar_pantalla_menu(int opcion_seleccionada) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;

    /* --- Entrar en modo 2D --- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* --- Overlay semitransparente sobre toda la pantalla --- */
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

    /* --- Titulo y aviso de proposito (concientizacion, saldo virtual) --- */
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - 200.0f, cy + 220.0f, "CASINO ONLINE - SELECCION DE JUEGO");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(cx - 210.0f, cy + 180.0f, "Usa las flechas [ARRIBA/ABAJO] para elegir un juego.");

    /* --- Selector Card (Glassmorphism style panel) --- */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.15f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 250.0f, cy - 30.0f);
    glVertex2f(cx + 250.0f, cy - 30.0f);
    glVertex2f(cx + 250.0f, cy + 150.0f);
    glVertex2f(cx - 250.0f, cy + 150.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Border of the selector card
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 250.0f, cy - 30.0f);
    glVertex2f(cx + 250.0f, cy - 30.0f);
    glVertex2f(cx + 250.0f, cy + 150.0f);
    glVertex2f(cx - 250.0f, cy + 150.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Opcion 1: Ruleta */
    if (opcion_seleccionada == 0) {
        glColor3f(1.0f, 0.85f, 0.0f); /* Amarillo brillante */
        dibujar_texto_2d(cx - 200.0f, cy + 100.0f, "->  1. RULETA EUROPEA  (Disponible)");
        glColor3f(1.0f, 1.0f, 1.0f);
        dibujar_texto_2d(cx - 160.0f, cy + 75.0f, "Simulador 3D completo con concientizacion.");
    } else {
        glColor3f(0.5f, 0.5f, 0.5f); /* Gris oscuro */
        dibujar_texto_2d(cx - 200.0f, cy + 100.0f, "    1. RULETA EUROPEA  (Disponible)");
        dibujar_texto_2d(cx - 160.0f, cy + 75.0f, "Simulador 3D completo con concientizacion.");
    }

    /* Opcion 2: Tragamonedas */
    if (opcion_seleccionada == 1) {
        glColor3f(1.0f, 0.85f, 0.0f); /* Amarillo brillante */
        dibujar_texto_2d(cx - 200.0f, cy + 20.0f, "->  2. TRAGAMONEDAS  (Proximamente)");
        glColor3f(1.0f, 1.0f, 1.0f);
        dibujar_texto_2d(cx - 160.0f, cy - 5.0f, "Modulo en construccion por otro desarrollador.");
    } else {
        glColor3f(0.5f, 0.5f, 0.5f); /* Gris oscuro */
        dibujar_texto_2d(cx - 200.0f, cy + 20.0f, "    2. TRAGAMONEDAS  (Proximamente)");
        dibujar_texto_2d(cx - 160.0f, cy - 5.0f, "Modulo en construccion por otro desarrollador.");
    }

    /* --- Accion --- */
    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d(cx - 180.0f, cy - 65.0f, "Presione [ENTER] para confirmar seleccion.");
    dibujar_texto_2d(cx - 150.0f, cy - 90.0f, "Presione [I] para ver Informacion.");

    /* --- Controles de ayuda en el Menu --- */
    glColor3f(0.6f, 0.6f, 0.6f);
    dibujar_texto_2d(cx - 240.0f, cy - 140.0f, "Nota: Saldo inicial virtual de 1,000.00 creditos.");
    dibujar_texto_2d(cx - 240.0f, cy - 165.0f, "Este software es para concientizacion sobre ludopatia.");

    /* --- Restaurar estado 3D --- */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_prestamo(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
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

    /* Overlay semitransparente oscuro */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    /* Panel de Glassmorphism */
    glColor4f(0.05f, 0.05f, 0.1f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 260.0f, cy - 130.0f);
    glVertex2f(cx + 260.0f, cy - 130.0f);
    glVertex2f(cx + 260.0f, cy + 200.0f);
    glVertex2f(cx - 260.0f, cy + 200.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde dorado */
    glColor3f(1.0f, 0.85f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 260.0f, cy - 130.0f);
    glVertex2f(cx + 260.0f, cy - 130.0f);
    glVertex2f(cx + 260.0f, cy + 200.0f);
    glVertex2f(cx - 260.0f, cy + 200.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono advertencia */
    dibujar_icono_advertencia(cx, cy + 150.0f, 22.0f);

    /* Mensajes */
    glColor3f(1.0f, 0.85f, 0.0f);
    {
        const char* title = "SOLICITUD DE PRESTAMO";
        dibujar_texto_2d(cx - (float)strlen(title) * ANCHO_CHAR_APROX / 2.0f, cy + 100.0f, title);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    {
        const char* msg1 = "Ya perdiste tu saldo inicial.";
        const char* msg2 = "En la vida real, este seria el momento de parar.";
        dibujar_texto_2d(cx - (float)strlen(msg1) * ANCHO_CHAR_APROX / 2.0f, cy + 45.0f, msg1);
        dibujar_texto_2d(cx - (float)strlen(msg2) * ANCHO_CHAR_APROX / 2.0f, cy + 20.0f, msg2);
    }

    /* Linea separadora */
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(cx - 220.0f, cy - 5.0f);
    glVertex2f(cx + 220.0f, cy - 5.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Deuda acumulada: %.2f", jugador->deuda);
    dibujar_texto_2d(cx - (float)strlen(buffer) * ANCHO_CHAR_APROX / 2.0f, cy - 30.0f, buffer);

    /* Opciones */
    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* op1 = "[P] Pedir prestamo y seguir jugando";
        dibujar_texto_2d(cx - (float)strlen(op1) * ANCHO_CHAR_APROX / 2.0f, cy - 70.0f, op1);
    }
    glColor3f(1.0f, 0.3f, 0.3f);
    {
        const char* op2 = "[S] Terminar la sesion aqui, sin mas deuda";
        dibujar_texto_2d(cx - (float)strlen(op2) * ANCHO_CHAR_APROX / 2.0f, cy - 95.0f, op2);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_mensaje_reflexivo(const char* mensaje, const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
    int es_repeticion = (jugador != NULL && jugador->mensajes_reflexivos_mostrados >= 2);

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

    /* Panel de dialogo */
    glColor4f(0.05f, 0.08f, 0.2f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 320.0f, cy - 210.0f);
    glVertex2f(cx + 320.0f, cy - 210.0f);
    glVertex2f(cx + 320.0f, cy + 200.0f);
    glVertex2f(cx - 320.0f, cy + 200.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde del panel */
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 320.0f, cy - 210.0f);
    glVertex2f(cx + 320.0f, cy - 210.0f);
    glVertex2f(cx + 320.0f, cy + 200.0f);
    glVertex2f(cx - 320.0f, cy + 200.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono informativo o de advertencia */
    if (!es_repeticion) {
        dibujar_icono_info(cx, cy + 150.0f, 22.0f);
    } else {
        dibujar_icono_advertencia(cx, cy + 150.0f, 22.0f);
    }

    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* titulo = "REALITY CHECK";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 100.0f, titulo);
    }

    /* Mensaje multilinea */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(mensaje, cx, cy + 60.0f, 24.0f);

    /* Linea separadora */
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(cx - 260.0f, cy - 100.0f);
    glVertex2f(cx + 260.0f, cy - 100.0f);
    glEnd();

    /* Opciones */
    {
        const char* op_seguir = "[ENTER] Continuar jugando";
        const char* op_terminar = "[S] Terminar la sesion aqui";
        float y_prominente = cy - 140.0f;
        float y_secundaria = cy - 170.0f;

        if (!es_repeticion) {
            glColor3f(0.55f, 0.75f, 1.0f);
            dibujar_texto_2d(cx - (float)strlen(op_seguir) * ANCHO_CHAR_APROX / 2.0f, y_prominente, op_seguir);
            glColor3f(0.5f, 0.5f, 0.5f);
            dibujar_texto_2d(cx - (float)strlen(op_terminar) * ANCHO_CHAR_APROX / 2.0f, y_secundaria, op_terminar);
        }
        else {
            glColor3f(1.0f, 0.85f, 0.0f);
            dibujar_texto_2d(cx - (float)strlen(op_terminar) * ANCHO_CHAR_APROX / 2.0f, y_prominente, op_terminar);
            glColor3f(0.5f, 0.5f, 0.5f);
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

void dibujar_pantalla_game_over(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
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

    /* Pulso rojo de fondo */
    {
        float tiempo_ms = (float)glutGet(GLUT_ELAPSED_TIME);
        float pulso = (sinf(tiempo_ms * 0.002f) + 1.0f) / 2.0f;
        float r = 0.2f + pulso * 0.4f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(r, 0.0f, 0.0f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((float)ancho, 0.0f);
        glVertex2f((float)ancho, (float)alto);
        glVertex2f(0.0f, (float)alto);
        glEnd();
    }

    /* Dialog Panel */
    glColor4f(0.1f, 0.0f, 0.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy + 210.0f);
    glVertex2f(cx - 300.0f, cy + 210.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde Rojo */
    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy + 210.0f);
    glVertex2f(cx - 300.0f, cy + 210.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono de advertencia */
    dibujar_icono_advertencia(cx, cy + 160.0f, 22.0f);

    glColor3f(1.0f, 0.2f, 0.2f);
    {
        const char* title = "=== JUEGO TERMINADO ===";
        dibujar_texto_2d(cx - (float)strlen(title) * ANCHO_CHAR_APROX / 2.0f, cy + 110.0f, title);
    }

    /* Resumen */
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d(cx - 180.0f, cy + 75.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos solicitados: %d", jugador->prestamos_activos);
    dibujar_texto_2d(cx - 180.0f, cy + 50.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Interes acumulado: %.2f", jugador->interes_acumulado);
    dibujar_texto_2d(cx - 180.0f, cy + 25.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Deuda final impagable: %.2f", jugador->deuda);
    dibujar_texto_2d(cx - 180.0f, cy, buffer);

    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_2d(cx - 180.0f, cy - 25.0f, buffer);

    /* Separador */
    glColor3f(0.5f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(cx - 240.0f, cy - 40.0f);
    glVertex2f(cx + 240.0f, cy - 40.0f);
    glEnd();

    glColor3f(1.0f, 0.6f, 0.5f);
    dibujar_texto_2d(cx - 250.0f, cy - 65.0f, "Ese tiempo y esa deuda son el mismo costo que,");
    dibujar_texto_2d(cx - 250.0f, cy - 90.0f, "en la vida real, se le resta a la familia o estudios.");

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Pediste %d prestamo(s) para seguir jugando,", jugador->prestamos_activos);
    dibujar_texto_2d(cx - 230.0f, cy - 130.0f, buffer);
    dibujar_texto_2d(cx - 180.0f, cy - 155.0f, "y la deuda crecio de forma descontrolada.");

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - 170.0f, cy - 195.0f, "[ESC] Salir     [ENTER] Reiniciar");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_sesion_terminada(const Jugador* jugador) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
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

    /* Panel de dialogo */
    glColor4f(0.05f, 0.08f, 0.2f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy + 210.0f);
    glVertex2f(cx - 300.0f, cy + 210.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde Azul */
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy - 220.0f);
    glVertex2f(cx + 300.0f, cy + 210.0f);
    glVertex2f(cx - 300.0f, cy + 210.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono informativo */
    dibujar_icono_info(cx, cy + 160.0f, 22.0f);

    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* title = "SESION FINALIZADA";
        dibujar_texto_2d(cx - (float)strlen(title) * ANCHO_CHAR_APROX / 2.0f, cy + 110.0f, title);
    }

    /* Resumen */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(cx - 200.0f, cy + 75.0f, "Se eligio parar antes de agravar deudas.");
    
    sprintf_s(buffer, sizeof(buffer), "Total apostado: %.2f", jugador->total_apostado);
    dibujar_texto_2d(cx - 200.0f, cy + 40.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Prestamos solicitados: %d", jugador->prestamos_activos);
    dibujar_texto_2d(cx - 200.0f, cy + 15.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Deuda final con la que termina: %.2f", jugador->deuda);
    dibujar_texto_2d(cx - 200.0f, cy - 10.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Saldo final obtenido: %.2f", jugador->saldo);
    dibujar_texto_2d(cx - 200.0f, cy - 35.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_2d(cx - 200.0f, cy - 60.0f, buffer);

    /* Separador */
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(cx - 240.0f, cy - 75.0f);
    glVertex2f(cx + 240.0f, cy - 75.0f);
    glEnd();

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d(cx - 210.0f, cy - 105.0f, "Es un buen momento para reflexionar en que");
    dibujar_texto_2d(cx - 210.0f, cy - 130.0f, "mas podrias haber invertido este tiempo.");
    dibujar_texto_2d(cx - 225.0f, cy - 160.0f, "Parar a tiempo tambien es una forma de ganar.");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(cx - 180.0f, cy - 195.0f, "[ESC] Salir     [ENTER] Jugar de nuevo");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* Contenido de las 6 paginas de dibujar_pantalla_educacion(). Fundamentado
   en DSM-5/CIE-11, Mayo Clinic, y fuentes citadas en
   docs/analisis-ludopatia.md -no son afirmaciones inventadas para el
   proyecto. Los recursos de ayuda de la pagina 5 son reales y
   especificos de Republica Dominicana (verificar telefonos antes de
   una entrega final, pueden cambiar con el tiempo). */
static const char* EDUCACION_TITULOS[EDUCACION_NUM_PAGINAS] = {
    "QUE ES LA LUDOPATIA",
    "SENALES DE ALERTA",
    "MITOS SOBRE LAS APUESTAS",
    "CONSECUENCIAS",
    "RECURSOS DE AYUDA (REP. DOMINICANA)",
    "PREVENCION"
};

static const char* EDUCACION_CUERPOS[EDUCACION_NUM_PAGINAS] = {
    "La ludopatia (trastorno por juego de apuestas)\n"
    "es una adiccion reconocida clinicamente, no\n"
    "una falta de fuerza de voluntad.\n"
    "\n"
    "El DSM-5 (el manual de diagnostico psiquiatrico\n"
    "mas usado) la describe como un patron\n"
    "persistente de juego que causa un deterioro\n"
    "real en la vida de la persona, aunque sea\n"
    "consciente de las consecuencias.\n"
    "\n"
    "Es una adiccion comportamental: no hay una\n"
    "sustancia de por medio, pero el mismo circuito\n"
    "cerebral de recompensa esta involucrado, igual\n"
    "que en otras adicciones.",

    "- Apostar cantidades cada vez mayores para\n"
    "  sentir la misma emocion (tolerancia)\n"
    "- Irritabilidad al intentar dejar de jugar\n"
    "- Intentos fallidos y repetidos de parar\n"
    "- Jugar para escapar del estres o la ansiedad\n"
    "- Volver a jugar para recuperar lo perdido\n"
    "- Mentir sobre cuanto se juega o se pierde\n"
    "- Pedir dinero prestado para seguir jugando\n"
    "\n"
    "No hace falta tener todas estas senales a la\n"
    "vez: unas pocas ya son motivo suficiente para\n"
    "buscar una evaluacion profesional.",

    "Mito: \"perdi varias veces seguidas, ya me\n"
    "toca ganar\".\n"
    "\n"
    "Realidad: se llama la falacia del jugador. En\n"
    "la ruleta cada giro es independiente del\n"
    "anterior, la bolita no recuerda nada. Este\n"
    "mismo simulador decide el numero ganador al\n"
    "azar en cada ronda, sin importar que paso\n"
    "antes.\n"
    "\n"
    "Mito: \"hay sistemas o estrategias infalibles\n"
    "para ganar\".\n"
    "\n"
    "Realidad: ningun sistema de apuestas cambia\n"
    "la probabilidad matematica del juego ni la\n"
    "ventaja de la casa a largo plazo.",

    "El costo mas visible es el dinero: deudas,\n"
    "prestamos, interes que se acumula. Ya viste\n"
    "eso reflejado en tus propias partidas.\n"
    "\n"
    "Pero en la vida real, el costo no termina ahi.\n"
    "El tiempo dedicado al juego es tiempo que se\n"
    "le resta a la familia, los estudios o el\n"
    "trabajo. Y sostener la adiccion en secreto\n"
    "suele generar ansiedad, culpa y aislamiento:\n"
    "un costo real para la salud mental, aunque no\n"
    "se pueda medir con un numero como el dinero.\n"
    "\n"
    "Este simulador no puede medir esas otras\n"
    "partes de la vida de alguien, pero son tan\n"
    "reales como el saldo que se ve en pantalla.",

    "Fundacion Fenix: 809-542-4759\n"
    "  (fenix.org.do)\n"
    "\n"
    "Clinica Conductual Volver: 849-856-3789\n"
    "  (volver.com.do)\n"
    "\n"
    "Centro de Atencion Integral a las\n"
    "Dependencias (CAIDEP): 809-684-2300\n"
    "\n"
    "Jugadores Anonimos Republica Dominicana\n"
    "\n"
    "Linea Salud Mental (Ministerio de Salud\n"
    "Publica): linea nacional gratuita\n"
    "\n"
    "Mas info: casinos.gob.do/juego-responsable",

    "- Definir un presupuesto ANTES de jugar, y\n"
    "  no cruzarlo pase lo que pase\n"
    "- Definir tambien un limite de tiempo, no\n"
    "  solo de dinero\n"
    "- El juego es entretenimiento, no una forma\n"
    "  de generar ingresos\n"
    "- Nunca jugar para escapar del estres, la\n"
    "  tristeza o el aburrimiento\n"
    "- Nunca jugar para \"recuperar\" lo perdido\n"
    "- Tomarse pausas reales durante la sesion\n"
    "- Si sentis que no podes parar, esa es la\n"
    "  senal mas importante de todas: pedi ayuda"
};

void dibujar_pantalla_educacion(int pagina) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
    char buffer[64];

    if (pagina < 0 || pagina >= EDUCACION_NUM_PAGINAS) pagina = 0; /* indice invalido: no revienta, cae a la primera pagina */

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

    /* Mismo azul calmo que el mensaje reflexivo y la sesion terminada:
       esta pantalla es informativa, no punitiva. */
    glColor4f(0.02f, 0.05f, 0.12f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glDisable(GL_BLEND);

    /* --- Titulo de la pagina --- */
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - (float)strlen(EDUCACION_TITULOS[pagina]) * ANCHO_CHAR_APROX / 2.0f, cy + 200.0f, EDUCACION_TITULOS[pagina]);

    /* --- Cuerpo, alineado a la izquierda (bloques y vinetas ordenados) --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_izquierda(EDUCACION_CUERPOS[pagina], cx - 280.0f, cy + 150.0f, 22.0f);

    /* --- Indicador de pagina y navegacion --- */
    glColor3f(0.6f, 0.6f, 0.6f);
    sprintf_s(buffer, sizeof(buffer), "Pagina %d de %d", pagina + 1, EDUCACION_NUM_PAGINAS);
    dibujar_texto_2d(cx - (float)strlen(buffer) * ANCHO_CHAR_APROX / 2.0f, (float)alto / 2.0f - 250.0f, buffer);

    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* instruccion = "[ESPACIO] Siguiente pagina     [ENTER] Volver al menu";
        dibujar_texto_2d(cx - (float)strlen(instruccion) * ANCHO_CHAR_APROX / 2.0f, (float)alto / 2.0f - 280.0f, instruccion);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_carga(float progreso) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
    float barra_ancho = 400.0f;
    float barra_alto = 18.0f;
    float barra_x = cx - barra_ancho / 2.0f;
    float barra_y = cy - 100.0f;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* Fondo con textura de carga (si existe) */
    if (g_textura_carga != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_textura_carga);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f((float)ancho, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f((float)ancho, (float)alto);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, (float)alto);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    } else {
        /* Fondo degradado oscuro si no hay textura */
        glBegin(GL_QUADS);
        glColor3f(0.05f, 0.05f, 0.15f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((float)ancho, 0.0f);
        glColor3f(0.1f, 0.05f, 0.2f);
        glVertex2f((float)ancho, (float)alto);
        glVertex2f(0.0f, (float)alto);
        glEnd();
    }

    /* Titulo del juego (centrado horizontalmente) */
    glColor3f(1.0f, 0.85f, 0.0f);
    {
        const char* titulo = "CASINO ONLINE";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 60.0f, titulo);
    }
    glColor3f(0.7f, 0.7f, 0.7f);
    {
        const char* subtitulo = "Simulador de concientizacion sobre ludopatia";
        dibujar_texto_2d(cx - (float)strlen(subtitulo) * ANCHO_CHAR_APROX / 2.0f, cy + 30.0f, subtitulo);
    }

    /* Texto de carga y porcentaje (centrados horizontalmente) */
    glColor3f(1.0f, 1.0f, 1.0f);
    {
        char msg_carga[128];
        int pct = (int)(progreso * 100.0f);
        if (pct > 100) pct = 100;
        sprintf_s(msg_carga, sizeof(msg_carga), "Cargando recursos... %d%%", pct);
        dibujar_texto_2d(cx - (float)strlen(msg_carga) * ANCHO_CHAR_APROX / 2.0f, barra_y + 30.0f, msg_carga);
    }

    /* Borde de la barra de progreso (centrada horizontalmente) */
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barra_x, barra_y);
    glVertex2f(barra_x + barra_ancho, barra_y);
    glVertex2f(barra_x + barra_ancho, barra_y + barra_alto);
    glVertex2f(barra_x, barra_y + barra_alto);
    glEnd();

    /* Relleno de la barra de progreso con degradado */
    {
        float fill = progreso * barra_ancho;
        if (fill < 0.0f) fill = 0.0f;
        if (fill > barra_ancho) fill = barra_ancho;

        glBegin(GL_QUADS);
        glColor3f(0.2f, 0.5f, 1.0f);
        glVertex2f(barra_x, barra_y);
        glColor3f(0.4f, 0.7f, 1.0f);
        glVertex2f(barra_x + fill, barra_y);
        glVertex2f(barra_x + fill, barra_y + barra_alto);
        glColor3f(0.2f, 0.5f, 1.0f);
        glVertex2f(barra_x, barra_y + barra_alto);
        glEnd();
    }

    /* Texto pie de pagina (centrado horizontalmente) */
    glColor3f(0.5f, 0.5f, 0.5f);
    {
        const char* footer = "Practica de Computacion Grafica - OpenGL";
        dibujar_texto_2d(cx - (float)strlen(footer) * ANCHO_CHAR_APROX / 2.0f, 30.0f, footer);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_tragamonedas_placeholder(void) {
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

    /* Overlay oscuro */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    /* Panel central */
    glColor4f(0.08f, 0.1f, 0.2f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 280.0f, cy - 120.0f);
    glVertex2f(cx + 280.0f, cy - 120.0f);
    glVertex2f(cx + 280.0f, cy + 120.0f);
    glVertex2f(cx - 280.0f, cy + 120.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde dorado */
    glColor3f(1.0f, 0.85f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 280.0f, cy - 120.0f);
    glVertex2f(cx + 280.0f, cy - 120.0f);
    glVertex2f(cx + 280.0f, cy + 120.0f);
    glVertex2f(cx - 280.0f, cy + 120.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono de info */
    dibujar_icono_info(cx, cy + 60.0f, 22.0f);

    /* Mensajes */
    glColor3f(1.0f, 0.85f, 0.0f);
    {
        const char* titulo = "TRAGAMONEDAS";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 15.0f, titulo);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    {
        const char* msg1 = "Este modulo se encuentra en construccion";
        const char* msg2 = "por otro desarrollador del equipo.";
        dibujar_texto_2d(cx - (float)strlen(msg1) * ANCHO_CHAR_APROX / 2.0f, cy - 20.0f, msg1);
        dibujar_texto_2d(cx - (float)strlen(msg2) * ANCHO_CHAR_APROX / 2.0f, cy - 45.0f, msg2);
    }

    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* action = "[ENTER] Volver al menu principal";
        dibujar_texto_2d(cx - (float)strlen(action) * ANCHO_CHAR_APROX / 2.0f, cy - 90.0f, action);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const InfoPantalla* info) {
    switch (estado) {
    case ESTADO_CARGA:
        dibujar_pantalla_carga(info->progreso_carga);
        break;
    case ESTADO_MENU:
        dibujar_pantalla_menu(info->opcion_menu);
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
        if (info->mensaje_reflexivo != NULL) dibujar_pantalla_mensaje_reflexivo(info->mensaje_reflexivo, jugador);
        break;
    case ESTADO_EDUCACION:
        dibujar_pantalla_educacion(info->pagina_educacion);
        break;
    case ESTADO_TRAGAMONEDAS_PLACEHOLDER:
        dibujar_pantalla_tragamonedas_placeholder();
        break;
    default:
        break; /* ESTADO_JUGANDO no requiere overlay */
    }
}