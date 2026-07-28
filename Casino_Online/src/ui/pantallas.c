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

#define ANCHO_CHAR_APROX 10.5f

/* Dibuja un texto en 2D centrado horizontalmente en cx usando la metrica exacta
   de GLUT bitmap HELVETICA_18 (sin aproximaciones inexactas por caracter). */
static void dibujar_texto_centrado(float cx, float y, const char* texto) {
    int ancho_px;
    if (texto == NULL || texto[0] == '\0') return;
    ancho_px = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)texto);
    dibujar_texto_2d(cx - (float)ancho_px / 2.0f, y, texto);
}

/* Dibuja un bloque de texto separado por '\n', centrado horizontalmente
   en cx, una linea debajo de la otra empezando en y_inicio. Preserva
   lineas vacias ("\n\n") como espacio en blanco. */
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
            dibujar_texto_centrado(cx, y, inicio);
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
    dibujar_texto_centrado(cx, cy + 220.0f, "CASINO ONLINE - SELECCION DE JUEGO");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_centrado(cx, cy + 180.0f, "Usa las flechas [ARRIBA/ABAJO] para elegir un juego.");

    /* --- Selector Card (Glassmorphism style panel) --- */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.1f, 0.15f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 260.0f, cy - 30.0f);
    glVertex2f(cx + 260.0f, cy - 30.0f);
    glVertex2f(cx + 260.0f, cy + 150.0f);
    glVertex2f(cx - 260.0f, cy + 150.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Border of the selector card
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 260.0f, cy - 30.0f);
    glVertex2f(cx + 260.0f, cy - 30.0f);
    glVertex2f(cx + 260.0f, cy + 150.0f);
    glVertex2f(cx - 260.0f, cy + 150.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Opcion 1: Ruleta */
    if (opcion_seleccionada == 0) {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy + 100.0f, "->  1. RULETA EUROPEA  (Disponible)");
        glColor3f(1.0f, 1.0f, 1.0f);
        dibujar_texto_centrado(cx, cy + 75.0f, "Simulador 3D completo con concientizacion.");
    } else {
        glColor3f(0.5f, 0.5f, 0.5f);
        dibujar_texto_centrado(cx, cy + 100.0f, "    1. RULETA EUROPEA  (Disponible)");
        dibujar_texto_centrado(cx, cy + 75.0f, "Simulador 3D completo con concientizacion.");
    }

    /* Opcion 2: Tragamonedas */
    if (opcion_seleccionada == 1) {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy + 20.0f, "->  2. TRAGAMONEDAS  (Disponible)");
        glColor3f(1.0f, 1.0f, 1.0f);
        dibujar_texto_centrado(cx, cy - 5.0f, "Simulador de tragamonedas con concientizacion.");
    } else {
        glColor3f(0.5f, 0.5f, 0.5f);
        dibujar_texto_centrado(cx, cy + 20.0f, "    2. TRAGAMONEDAS  (Disponible)");
        dibujar_texto_centrado(cx, cy - 5.0f, "Simulador de tragamonedas con concientizacion.");
    }

    /* --- Accion --- */
    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy - 65.0f, "Presione [ENTER] para confirmar seleccion.");
    dibujar_texto_centrado(cx, cy - 90.0f, "Presione [I] para ver Informacion.");

    /* --- Controles de ayuda en el Menu --- */
    glColor3f(0.6f, 0.6f, 0.6f);
    dibujar_texto_centrado(cx, cy - 140.0f, "Nota: Saldo inicial virtual de 1,000.00 creditos.");

    /* Banner permanente de Serious Game */
    glColor3f(1.0f, 0.75f, 0.0f);
    dibujar_texto_centrado(cx, cy - 175.0f, "Simulador educativo sobre ludopatia - No incentiva el juego con dinero real");

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
    dibujar_texto_centrado(cx, cy + 100.0f, "SOLICITUD DE PRESTAMO");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_centrado(cx, cy + 45.0f, "Ya perdiste tu saldo inicial.");
    dibujar_texto_centrado(cx, cy + 20.0f, "En la vida real, este seria el momento de parar.");

    /* Linea separadora */
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(cx - 220.0f, cy - 5.0f);
    glVertex2f(cx + 220.0f, cy - 5.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Deuda acumulada: %.2f", jugador->deuda);
    dibujar_texto_centrado(cx, cy - 30.0f, buffer);

    /* Opciones */
    glColor3f(0.5f, 0.5f, 0.5f);
    dibujar_texto_centrado(cx, cy - 70.0f, "[P] Pedir prestamo y seguir jugando");
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 95.0f, "[S] Terminar la sesion aqui, sin mas deuda");

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
    glColor4f(0.05f, 0.08f, 0.2f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 330.0f, cy - 240.0f);
    glVertex2f(cx + 330.0f, cy - 240.0f);
    glVertex2f(cx + 330.0f, cy + 220.0f);
    glVertex2f(cx - 330.0f, cy + 220.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde del panel */
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 330.0f, cy - 240.0f);
    glVertex2f(cx + 330.0f, cy - 240.0f);
    glVertex2f(cx + 330.0f, cy + 220.0f);
    glVertex2f(cx - 330.0f, cy + 220.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono informativo o de advertencia */
    if (!es_repeticion) {
        dibujar_icono_info(cx, cy + 175.0f, 22.0f);
    } else {
        dibujar_icono_advertencia(cx, cy + 175.0f, 22.0f);
    }

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy + 135.0f, "REALITY CHECK");

    /* Mensaje multilinea */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(mensaje, cx, cy + 90.0f, 21.0f);

    /* Linea separadora */
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(cx - 270.0f, cy - 120.0f);
    glVertex2f(cx + 270.0f, cy - 120.0f);
    glEnd();

    /* Opciones */
    {
        const char* op_seguir = "[ENTER] Continuar jugando";
        const char* op_terminar = "[S] Terminar la sesion aqui";
        float y_prominente = cy - 155.0f;
        float y_secundaria = cy - 190.0f;

        if (!es_repeticion) {
            glColor3f(0.5f, 0.5f, 0.5f);
            dibujar_texto_centrado(cx, y_prominente, op_seguir);
            glColor3f(1.0f, 0.85f, 0.0f);
            dibujar_texto_centrado(cx, y_secundaria, op_terminar);
        }
        else {
            glColor3f(1.0f, 0.85f, 0.0f);
            dibujar_texto_centrado(cx, y_prominente, op_terminar);
            glColor3f(0.5f, 0.5f, 0.5f);
            dibujar_texto_centrado(cx, y_secundaria, op_seguir);
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
    glColor4f(0.1f, 0.0f, 0.0f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy + 240.0f);
    glVertex2f(cx - 330.0f, cy + 240.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde Rojo */
    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy + 240.0f);
    glVertex2f(cx - 330.0f, cy + 240.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono de advertencia */
    dibujar_icono_advertencia(cx, cy + 195.0f, 22.0f);

    glColor3f(1.0f, 0.2f, 0.2f);
    dibujar_texto_centrado(cx, cy + 155.0f, "=== JUEGO TERMINADO ===");

    /* Resumen Financiero Desglosado */
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Total apostado: $%.2f", jugador->total_apostado);
    dibujar_texto_centrado(cx, cy + 120.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Ganaste en total: $%.2f", jugador->total_ganado);
    dibujar_texto_centrado(cx, cy + 98.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Perdiste en total: $%.2f", jugador->total_perdido);
    dibujar_texto_centrado(cx, cy + 76.0f, buffer);

    {
        float neto = jugador->total_ganado - jugador->total_perdido;
        if (neto >= 0.0f) {
            sprintf_s(buffer, sizeof(buffer), "Resultado neto de la sesion: +$%.2f", neto);
            glColor3f(0.3f, 1.0f, 0.3f);
        } else {
            sprintf_s(buffer, sizeof(buffer), "Resultado neto de la sesion: -$%.2f", fabsf(neto));
            glColor3f(1.0f, 0.35f, 0.35f);
        }
        dibujar_texto_centrado(cx, cy + 54.0f, buffer);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Prestamos: %d  |  Interes: $%.2f  |  Deuda impagable: $%.2f",
        jugador->prestamos_activos, jugador->interes_acumulado, jugador->deuda);
    dibujar_texto_centrado(cx, cy + 30.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_centrado(cx, cy + 8.0f, buffer);

    /* Separador */
    glColor3f(0.5f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(cx - 270.0f, cy - 10.0f);
    glVertex2f(cx + 270.0f, cy - 10.0f);
    glEnd();

    if (jugador->total_ganado - jugador->total_perdido > 0.0f) {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy - 35.0f, "Esta vez ganaste, pero no es el resultado comun en las apuestas.");
        dibujar_texto_centrado(cx, cy - 57.0f, "Tuviste suerte, pero la proxima vez podria no ser lo mismo.");
    } else {
        glColor3f(1.0f, 0.75f, 0.3f);
        dibujar_texto_centrado(cx, cy - 35.0f, "Este es el resultado comun: a largo plazo las perdidas");
        dibujar_texto_centrado(cx, cy - 57.0f, "siempre superan a las ganancias.");
    }

    glColor3f(1.0f, 0.6f, 0.5f);
    dibujar_texto_centrado(cx, cy - 90.0f, "Al final, las personas siempre terminan perdiendo algo");
    dibujar_texto_centrado(cx, cy - 112.0f, "(dinero, familia o amistades), aun cuando ganan dinero.");

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy - 150.0f, "Si te reconoces en algo de esta sesion, la tecla [I]");
    dibujar_texto_centrado(cx, cy - 172.0f, "desde el menu tiene mas informacion sobre signos de alerta.");

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 215.0f, "[ESC] Salir");
    glColor3f(0.5f, 0.5f, 0.5f);
    dibujar_texto_centrado(cx, cy - 235.0f, "    [ENTER] Reiniciar");

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
    glColor4f(0.05f, 0.08f, 0.2f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy + 240.0f);
    glVertex2f(cx - 330.0f, cy + 240.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde Azul */
    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy - 275.0f);
    glVertex2f(cx + 330.0f, cy + 240.0f);
    glVertex2f(cx - 330.0f, cy + 240.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Icono informativo */
    dibujar_icono_info(cx, cy + 195.0f, 22.0f);

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy + 155.0f, "SESION FINALIZADA");

    /* Resumen */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_centrado(cx, cy + 125.0f, "Se eligio parar antes de agravar deudas.");

    sprintf_s(buffer, sizeof(buffer), "Total apostado: $%.2f", jugador->total_apostado);
    dibujar_texto_centrado(cx, cy + 98.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Ganaste en total: $%.2f", jugador->total_ganado);
    dibujar_texto_centrado(cx, cy + 76.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Perdiste en total: $%.2f", jugador->total_perdido);
    dibujar_texto_centrado(cx, cy + 54.0f, buffer);

    {
        float neto = jugador->total_ganado - jugador->total_perdido;
        if (neto >= 0.0f) {
            sprintf_s(buffer, sizeof(buffer), "Resultado neto de la sesion: +$%.2f", neto);
            glColor3f(0.3f, 1.0f, 0.3f);
        } else {
            sprintf_s(buffer, sizeof(buffer), "Resultado neto de la sesion: -$%.2f", fabsf(neto));
            glColor3f(1.0f, 0.35f, 0.35f);
        }
        dibujar_texto_centrado(cx, cy + 32.0f, buffer);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Prestamos: %d  |  Deuda final: $%.2f  |  Saldo final: $%.2f",
        jugador->prestamos_activos, jugador->deuda, jugador->saldo);
    dibujar_texto_centrado(cx, cy + 10.0f, buffer);

    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_centrado(cx, cy - 12.0f, buffer);

    /* Separador */
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_LINES);
    glVertex2f(cx - 270.0f, cy - 28.0f);
    glVertex2f(cx + 270.0f, cy - 28.0f);
    glEnd();

    if (jugador->total_ganado - jugador->total_perdido > 0.0f) {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy - 52.0f, "Esta vez ganaste, pero no es el resultado comun en las apuestas.");
        dibujar_texto_centrado(cx, cy - 74.0f, "Tuviste suerte, pero la proxima vez podria no ser lo mismo.");
    } else {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy - 52.0f, "Este es el resultado comun: a largo plazo las perdidas");
        dibujar_texto_centrado(cx, cy - 74.0f, "siempre superan a las ganancias.");
    }

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy - 104.0f, "Al final, las personas siempre terminan perdiendo algo");
    dibujar_texto_centrado(cx, cy - 126.0f, "(dinero, familia o amistades), aun cuando ganan dinero.");
    dibujar_texto_centrado(cx, cy - 148.0f, "Parar a tiempo tambien es una forma de ganar.");

    glColor3f(0.7f, 0.85f, 1.0f);
    dibujar_texto_centrado(cx, cy - 180.0f, "Si te reconoces en algo de esta sesion, la tecla [I]");
    dibujar_texto_centrado(cx, cy - 200.0f, "desde el menu tiene mas informacion sobre signos de alerta.");

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 228.0f, "[ESC] Salir");
    glColor3f(0.5f, 0.5f, 0.5f);
    dibujar_texto_centrado(cx, cy - 248.0f, "    [ENTER] Jugar de nuevo");

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
    "SIGNOS DE ALERTA",
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
    "No hace falta presentar todos estos signos a la\n"
    "vez: unos pocos ya son motivo suficiente para\n"
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

    /* --- Titulo de la pagina activa --- */
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy + 200.0f, EDUCACION_TITULOS[pagina]);

    /* --- Cuerpo, alineado a la izquierda (bloques y vinetas ordenados) --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_izquierda(EDUCACION_CUERPOS[pagina], cx - 280.0f, cy + 150.0f, 22.0f);

    /* --- Indicador de pagina y navegacion --- */
    glColor3f(0.6f, 0.6f, 0.6f);
    sprintf_s(buffer, sizeof(buffer), "Pagina %d de %d", pagina + 1, EDUCACION_NUM_PAGINAS);
    dibujar_texto_centrado(cx, (float)alto / 2.0f - 250.0f, buffer);

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, (float)alto / 2.0f - 280.0f, "[ESPACIO] Siguiente pagina     [ENTER] Volver al menu");

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
    dibujar_texto_centrado(cx, cy + 60.0f, "CASINO ONLINE");
    glColor3f(0.7f, 0.7f, 0.7f);
    dibujar_texto_centrado(cx, cy + 30.0f, "Simulador de concientizacion sobre ludopatia");

    /* Texto de carga y porcentaje (centrados horizontalmente) */
    glColor3f(1.0f, 1.0f, 1.0f);
    {
        char msg_carga[128];
        int pct = (int)(progreso * 100.0f);
        if (pct > 100) pct = 100;
        sprintf_s(msg_carga, sizeof(msg_carga), "Cargando recursos... %d%%", pct);
        dibujar_texto_centrado(cx, barra_y + 30.0f, msg_carga);
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
    dibujar_texto_centrado(cx, 30.0f, "Practica de Computacion Grafica - OpenGL");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}



void dibujar_pantalla_advertencia(void) {
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

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.02f, 0.02f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy + 210.0f);
    glVertex2f(cx - 330.0f, cy + 210.0f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.3f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy + 210.0f);
    glVertex2f(cx - 330.0f, cy + 210.0f);
    glEnd();
    glLineWidth(1.0f);

    dibujar_icono_advertencia(cx, cy + 155.0f, 22.0f);

    glColor3f(1.0f, 0.3f, 0.3f);
    dibujar_texto_centrado(cx, cy + 115.0f, "ADVERTENCIA");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(
        "La ludopatia es una adiccion real que puede afectar\n"
        "la salud mental, la economia, y las relaciones\n"
        "personales de quien la padece.\n\n"
        "Este programa tiene fines EXCLUSIVAMENTE educativos.\n"
        "No busca promover ni incentivar el juego de apuestas\n"
        "con dinero real.",
        cx, cy + 65.0f, 22.0f);

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 170.0f, "[ENTER] Continuar");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_proposito(void) {
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

    glColor3f(0.03f, 0.05f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.09f, 0.2f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy + 210.0f);
    glVertex2f(cx - 330.0f, cy + 210.0f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy - 210.0f);
    glVertex2f(cx + 330.0f, cy + 210.0f);
    glVertex2f(cx - 330.0f, cy + 210.0f);
    glEnd();
    glLineWidth(1.0f);

    dibujar_icono_info(cx, cy + 155.0f, 22.0f);

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_centrado(cx, cy + 115.0f, "QUE ES ESTE PROYECTO");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(
        "Este es un simulador educativo (serious game) sobre\n"
        "ludopatia, desarrollado para una practica de\n"
        "Computacion Grafica.\n\n"
        "No busca incentivar las apuestas. Su objetivo es\n"
        "generar conciencia sobre los riesgos del juego\n"
        "problematico, ayudar a identificar sus signos de\n"
        "alerta, y promover la prevencion y la busqueda de\n"
        "ayuda profesional.",
        cx, cy + 65.0f, 22.0f);

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 170.0f, "[ENTER] Continuar");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void dibujar_pantalla_confirmacion_juego(void) {
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glColor4f(0.12f, 0.08f, 0.02f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 340.0f, cy - 210.0f);
    glVertex2f(cx + 340.0f, cy - 210.0f);
    glVertex2f(cx + 340.0f, cy + 210.0f);
    glVertex2f(cx - 340.0f, cy + 210.0f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.75f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 340.0f, cy - 210.0f);
    glVertex2f(cx + 340.0f, cy - 210.0f);
    glVertex2f(cx + 340.0f, cy + 210.0f);
    glVertex2f(cx - 340.0f, cy + 210.0f);
    glEnd();
    glLineWidth(1.0f);

    dibujar_icono_advertencia(cx, cy + 155.0f, 22.0f);

    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy + 115.0f, "REFLEXIONA ANTES DE JUGAR");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(
        "Estas a punto de iniciar una simulacion de apuestas.\n\n"
        "Recorda los riesgos asociados al juego: perdida de\n"
        "dinero, dependencia emocional, y el riesgo real de\n"
        "desarrollar un patron de juego problematico.\n\n"
        "Luego de leer esta informacion, deseas continuar\n"
        "bajo tu propia responsabilidad?",
        cx, cy + 65.0f, 22.0f);

    {
        const char* op_si = "[ENTER] Si, continuar";
        const char* op_no = "[N] No, volver al menu";
        float w_si = (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)op_si);
        float w_no = (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)op_no);
        float gap = 40.0f;
        float total_w = w_si + gap + w_no;
        float x_si = cx - total_w / 2.0f;
        float x_no = x_si + w_si + gap;

        glColor3f(0.5f, 0.5f, 0.5f);
        dibujar_texto_2d(x_si, cy - 170.0f, op_si);
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_2d(x_no, cy - 170.0f, op_no);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static const char* MENSAJES_CHECKPOINT[8] = {
    "Recorda: cada giro de la ruleta es un evento\n"
    "independiente. Ninguna racha, buena o mala,\n"
    "predice el resultado del proximo giro.",

    "Necesitar apostar montos cada vez mayores para\n"
    "sentir la misma emocion es un signo de alerta\n"
    "reconocido del juego problematico.",

    "El autocontrol -definir un limite de tiempo y\n"
    "dinero antes de jugar, y respetarlo- es la\n"
    "herramienta mas efectiva contra la ludopatia.",

    "Si sentis que no podes parar de jugar, ese es\n"
    "el indicio mas importante de todos. Buscar ayuda\n"
    "profesional a tiempo hace la diferencia.",

    "Ocultar o mentir sobre tus perdidas a familiares\n"
    "o amigos es un signo clasico de descontrol\n"
    "en el habito de juego (DSM-5).",

    "Apostar para escapar de problemas emocionales\n"
    "o financieros suele agravar la situacion\n"
    "en lugar de resolverla.",

    "Ninguna estrategia matematica puede vencer\n"
    "la ventaja fija del 2.7 por ciento que\n"
    "la casa mantiene en cada tiro.",

    "Tomarte una pausa regular te permite evaluar\n"
    "con claridad si estas jugando por diversion\n"
    "o por impulso compulsivo."
};

void dibujar_pantalla_checkpoint_educativo(int indice_mensaje) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;
    int idx;

    idx = indice_mensaje % 8;
    if (idx < 0) idx = 0;

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
    glColor4f(0.05f, 0.02f, 0.0f, 0.80f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glColor4f(0.18f, 0.08f, 0.01f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 320.0f, cy - 190.0f);
    glVertex2f(cx + 320.0f, cy - 190.0f);
    glVertex2f(cx + 320.0f, cy + 190.0f);
    glVertex2f(cx - 320.0f, cy + 190.0f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.55f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - 320.0f, cy - 190.0f);
    glVertex2f(cx + 320.0f, cy - 190.0f);
    glVertex2f(cx + 320.0f, cy + 190.0f);
    glVertex2f(cx - 320.0f, cy + 190.0f);
    glEnd();
    glLineWidth(1.0f);

    dibujar_icono_info(cx, cy + 135.0f, 22.0f);

    glColor3f(1.0f, 0.65f, 0.0f);
    dibujar_texto_centrado(cx, cy + 95.0f, "RECORDATORIO");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(MENSAJES_CHECKPOINT[idx], cx, cy + 40.0f, 24.0f);

    glColor3f(0.5f, 0.5f, 0.5f);
    dibujar_texto_centrado(cx, cy - 140.0f, "[ENTER] Continuar bajo tu responsabilidad");
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_centrado(cx, cy - 170.0f, "[S] Terminar la sesion aqui");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

typedef struct {
    const char* pregunta;
    const char* opcion_a;
    const char* opcion_b;
    const char* opcion_c;
    int correcta;           /* 0=A, 1=B, 2=C */
    const char* explicacion;
} PreguntaQuiz;

static const PreguntaQuiz QUIZ_PREGUNTAS[NUM_PREGUNTAS_QUIZ] = {
    {
        "Que es la ludopatia?",
        "Una adiccion real, reconocida clinicamente",
        "Un pasatiempo sin ningun riesgo",
        "Un problema exclusivo de personas con poco dinero",
        0,
        "La ludopatia es una adiccion comportamental reconocida\n"
        "clinicamente, no una falta de fuerza de voluntad ni un problema\n"
        "exclusivo de nadie en particular."
    },
    {
        "Cual de estos es un indicio reconocido del juego\nproblematico?",
        "Dormir mas de lo normal",
        "Comer mas frutas y verduras",
        "Necesitar apostar cantidades cada vez mayores",
        2,
        "Necesitar apostar montos cada vez mayores para sentir\n"
        "la misma emocion se llama tolerancia, y es un indicio real\n"
        "reconocido del juego problematico."
    },
    {
        "Si perdiste varias veces seguidas en la ruleta, que es\ncierto sobre el proximo giro?",
        "Es mas probable que ganes, porque ya toca",
        "La probabilidad es exactamente la misma que en cualquier otro giro",
        "Es menos probable que ganes",
        1,
        "Cada giro es independiente del anterior. Pensar que ya\n"
        "toca ganar se llama la falacia del jugador."
    },
    {
        "Que es perseguir las perdidas?",
        "Seguir apostando, muchas veces con montos mayores, para\nintentar recuperar lo perdido",
        "Anotar cuanto perdiste para aprender de eso",
        "Dejar de jugar despues de perder",
        0,
        "Perseguir perdidas es uno de los indicios mas comunes\n"
        "del juego problematico, y suele generar perdidas aun mayores, no\n"
        "recuperar nada."
    },
    {
        "Cual de estas es una practica de juego responsable?",
        "Aumentar la apuesta despues de cada perdida",
        "Jugar para escapar del estres o el aburrimiento",
        "Definir un limite de tiempo y dinero antes de jugar, y respetarlo",
        2,
        "El autocontrol -definir limites antes de jugar y\n"
        "sostenerlos pase lo que pase- es la herramienta mas efectiva contra\n"
        "el juego problematico."
    },
    {
        "Un sistema o estrategia de apuestas...",
        "Puede garantizar ganancias si se sigue bien",
        "No cambia la probabilidad matematica del juego",
        "Funciona mejor cuanto mas dinero se invierte",
        1,
        "Ningun sistema de apuestas cambia la probabilidad\n"
        "matematica ni la ventaja de la casa a largo plazo."
    },
    {
        "Que deberias hacer si sentis que no podes dejar de jugar?",
        "Buscar ayuda profesional o hablarlo con alguien de confianza",
        "Seguir jugando hasta que se te pase",
        "Cambiar de juego",
        0,
        "Si sentis que no podes parar, ese es el indicio mas\n"
        "importante de todos. Buscar ayuda a tiempo hace la diferencia."
    },
    {
        "Ocultarle a otras personas cuanto tiempo o dinero se\napuesta es...",
        "Algo normal que no dice nada en particular",
        "Una forma de jugar responsablemente",
        "Un indicio de alerta del juego problematico",
        2,
        "Mentir u ocultar cuanto se juega o se pierde es un\n"
        "indicio de alerta reconocido -si hace falta esconderlo, vale la\n"
        "pena preguntarse por que."
    },
    {
        "Apostar para escapar del estres, el aburrimiento o un mal\nmomento emocional es...",
        "Una forma valida de manejar esas emociones",
        "Un patron de riesgo, no una forma sana de lidiar con ellas",
        "Algo que solo le pasa a los jugadores profesionales",
        1,
        "Usar el juego para escapar de emociones dificiles es\n"
        "un patron de riesgo reconocido, no una solucion real al problema de\n"
        "fondo."
    },
    {
        "Pedir dinero prestado para seguir jugando es...",
        "Una forma de escalada real de la deuda, no una solucion",
        "Una solucion temporal sin consecuencias",
        "Algo recomendable si estas seguro de que vas a ganar",
        0,
        "Pedir prestamos para seguir apostando es exactamente\n"
        "como una deuda de juego real empieza a escalar."
    },
    {
        "Ademas del dinero, la ludopatia puede afectar...",
        "Solo la economia de la persona, nada mas",
        "Unicamente a personas que ya tienen otros problemas",
        "Las relaciones personales, el tiempo, y el bienestar emocional",
        2,
        "La ludopatia puede afectar el tiempo, las relaciones\n"
        "familiares y personales, y el bienestar emocional -no solo el\n"
        "dinero."
    },
    {
        "Llevar un control real del tiempo que se pasa jugando es\nutil porque...",
        "No sirve para nada, el tiempo no importa en las apuestas",
        "Ayuda a notar si se esta jugando mas de lo que se tenia pensado",
        "Solo importa si se esta perdiendo dinero",
        1,
        "El tiempo jugado es un indicio tan importante como el\n"
        "dinero -perder la nocion del tiempo es, en si, una advertencia real."
    }
};

int quiz_evaluar_respuesta(int pregunta_idx, int respuesta) {
    if (pregunta_idx < 0 || pregunta_idx >= NUM_PREGUNTAS_QUIZ) return 0;
    return QUIZ_PREGUNTAS[pregunta_idx].correcta == respuesta;
}

void dibujar_pantalla_quiz(int pregunta_idx, int fase, int respuesta_elegida, int fue_correcta) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = w / 2.0f;
    float cy = h / 2.0f;

    if (pregunta_idx < 0 || pregunta_idx >= NUM_PREGUNTAS_QUIZ) return;

    /* --- Entrar en modo 2D --- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(w, 0);
    glVertex2i(w, h);
    glVertex2i(0, h);
    glEnd();

    glColor3f(1.0f, 0.6f, 0.0f);
    dibujar_texto_centrado(cx, cy + 120.0f, "QUIZ EDUCATIVO");
    
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(QUIZ_PREGUNTAS[pregunta_idx].pregunta, cx, cy + 60.0f, 24.0f);

    if (fase == 0) {
        /* Fase de pregunta */
        glColor3f(0.9f, 0.9f, 0.9f);
        dibujar_texto_centrado(cx, cy - 20.0f, "[A]");
        dibujar_texto_multilinea_centrado(QUIZ_PREGUNTAS[pregunta_idx].opcion_a, cx, cy - 45.0f, 20.0f);
        
        dibujar_texto_centrado(cx, cy - 85.0f, "[B]");
        dibujar_texto_multilinea_centrado(QUIZ_PREGUNTAS[pregunta_idx].opcion_b, cx, cy - 110.0f, 20.0f);
        
        dibujar_texto_centrado(cx, cy - 150.0f, "[C]");
        dibujar_texto_multilinea_centrado(QUIZ_PREGUNTAS[pregunta_idx].opcion_c, cx, cy - 175.0f, 20.0f);
        
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy - 230.0f, "[S] Terminar la sesion aqui");
    } else {
        /* Fase de feedback */
        if (fue_correcta) {
            glColor3f(0.0f, 1.0f, 0.0f);
            dibujar_texto_centrado(cx, cy - 10.0f, "CORRECTO");
        } else {
            const char* letras[3] = {"A", "B", "C"};
            char msg[128];
            glColor3f(1.0f, 0.3f, 0.0f);
            dibujar_texto_centrado(cx, cy - 10.0f, "INCORRECTO");
            glColor3f(1.0f, 1.0f, 1.0f);
            sprintf_s(msg, sizeof(msg), "La respuesta correcta era: %s", letras[QUIZ_PREGUNTAS[pregunta_idx].correcta]);
            dibujar_texto_centrado(cx, cy - 35.0f, msg);
        }
        
        glColor3f(0.8f, 0.8f, 0.8f);
        dibujar_texto_multilinea_centrado(QUIZ_PREGUNTAS[pregunta_idx].explicacion, cx, cy - 80.0f, 24.0f);
        
        glColor3f(0.5f, 0.5f, 0.5f);
        dibujar_texto_centrado(cx, cy - 180.0f, "[ENTER] Continuar");
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_centrado(cx, cy - 210.0f, "[S] Terminar la sesion aqui");
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
    case ESTADO_ADVERTENCIA:
        dibujar_pantalla_advertencia();
        break;
    case ESTADO_PROPOSITO:
        dibujar_pantalla_proposito();
        break;
    case ESTADO_MENU:
        dibujar_pantalla_menu(info->opcion_menu);
        break;
    case ESTADO_CONFIRMACION_JUEGO:
        dibujar_pantalla_confirmacion_juego();
        break;
    case ESTADO_CHECKPOINT_EDUCATIVO:
        dibujar_pantalla_checkpoint_educativo(info->indice_checkpoint);
        break;
    case ESTADO_QUIZ_EDUCATIVO:
        dibujar_pantalla_quiz(info->quiz_pregunta_idx, info->quiz_fase, info->quiz_respuesta_elegida, info->quiz_fue_correcta);
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

    default:
        break; /* ESTADO_JUGANDO no requiere overlay */
    }
}