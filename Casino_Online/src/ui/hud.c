/** Implementacion del HUD (Heads-Up Display).
 * Contiene las funciones para renderizar la interfaz 2D sobrepuesta al juego 3D,
 * mostrando informacion en tiempo real del jugador como saldo, apuestas y configuracion actual.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include "hud.h"

 /* Funcion auxiliar para dibujar una cadena de texto en pantalla en 2D.
    Utiliza la fuente  de 18 puntos de GLUT y renderiza caracter por caracter
    empezando en las coordenadas especificadas (x, y). */
static void dibujar_texto_2d(float x, float y, const char* texto) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

/* Dibuja en pantalla toda la informacion de la interfaz del usuario (HUD).
   Maneja internamente la transicion temporal a modo ortogonal (2D), renderiza
   los datos del jugador y restaura el estado 3D al finalizar. */

static const char* MENSAJES_PROBABILIDAD[12] = {
    "Ganar antes no cambia la probabilidad de este giro.",
    "Ninguna racha o patron cambia la probabilidad real.",
    "Necesitas ver este giro, o podrias parar aca?",
    "Cuanto llevas apostado hoy? Es lo que pensabas gastar?",
    "Si perdes esta ronda, vas a parar o vas a seguir apostando?",
    "Jugar por diversion no es lo mismo que jugar para recuperar.",
    "Apostar mas para recuperar no cambia la probabilidad.",
    "La ruleta no tiene memoria de los giros anteriores.",
    "Como le explicarias a alguien por que seguis jugando?",
    "Un descanso ahora no cuesta nada. Seguir apostando, si.",
    "Ganar ahora no borra lo que ya perdiste en la sesion.",
    "Si esto fuera dinero real, jugarias igual?"
};

void dibujar_hud(const Jugador* jugador, float monto_ficha_actual, int num_apuestas_activas, int bolita_girando, int indice_probabilidad) {
    char buffer[128];
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    int nivel_riesgo = 0;
    int tiempo_actual_ms = glutGet(GLUT_ELAPSED_TIME);
    int len_label_riesgo;
    float val_x_riesgo;

    if (jugador != NULL) {
        nivel_riesgo = calcular_nivel_riesgo(jugador, tiempo_actual_ms);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* Fondo semitransparente (Glassmorphism) expandido para incluir el medidor de riesgo y la ayuda de tecla S */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.70f);
    glBegin(GL_QUADS);
    glVertex2f(5.0f, (float)alto - 346.0f);
    glVertex2f(420.0f, (float)alto - 346.0f);
    glVertex2f(420.0f, (float)alto - 130.0f);
    glVertex2f(5.0f, (float)alto - 130.0f);
    glEnd();
    glDisable(GL_BLEND);

    /* Borde dorado */
    glColor3f(1.0f, 0.85f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(5.0f, (float)alto - 346.0f);
    glVertex2f(420.0f, (float)alto - 346.0f);
    glVertex2f(420.0f, (float)alto - 130.0f);
    glVertex2f(5.0f, (float)alto - 130.0f);
    glEnd();
    glLineWidth(1.0f);

    /* 1. Saldo */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 154.0f, "Saldo: ");
    {
        const float UMBRAL_SALDO_BAJO = 200.0f;
        float t = (jugador != NULL) ? (jugador->saldo / UMBRAL_SALDO_BAJO) : 1.0f;
        float r, g, b;

        if (t > 1.0f) t = 1.0f;
        if (t < 0.0f) t = 0.0f;

        r = 1.0f;
        g = t;
        b = t;

        glColor3f(r, g, b);
    }
    sprintf_s(buffer, sizeof(buffer), "%.2f", (jugador != NULL) ? jugador->saldo : 0.0f);
    dibujar_texto_2d(15.0f + 65.0f, (float)alto - 154.0f, buffer);

    /* 2. Total apostado */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 178.0f, "Total apostado: ");
    if (jugador != NULL && jugador->total_apostado > 0.0f) {
        glColor3f(1.0f, 0.6f, 0.2f);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    sprintf_s(buffer, sizeof(buffer), "%.2f", (jugador != NULL) ? jugador->total_apostado : 0.0f);
    dibujar_texto_2d(15.0f + 140.0f, (float)alto - 178.0f, buffer);

    /* 3. Prestamos */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 202.0f, "Prestamos: ");
    if (jugador != NULL && jugador->prestamos_activos > 0) {
        glColor3f(1.0f, 0.3f, 0.3f);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    sprintf_s(buffer, sizeof(buffer), "%d", (jugador != NULL) ? jugador->prestamos_activos : 0);
    dibujar_texto_2d(15.0f + 100.0f, (float)alto - 202.0f, buffer);

    /* 4. Ficha seleccionada y circulo de color */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 226.0f, "Ficha: ");
    if (monto_ficha_actual <= 10.0f) {
        glColor3f(1.0f, 0.9f, 0.0f); /* Amarillo */
    } else if (monto_ficha_actual <= 25.0f) {
        glColor3f(0.0f, 0.7f, 0.1f); /* Verde */
    } else if (monto_ficha_actual <= 50.0f) {
        glColor3f(0.1f, 0.4f, 0.9f); /* Azul */
    } else {
        glColor3f(0.6f, 0.1f, 0.7f); /* Morado */
    }
    sprintf_s(buffer, sizeof(buffer), "%.2f", monto_ficha_actual);
    dibujar_texto_2d(15.0f + 60.0f, (float)alto - 226.0f, buffer);

    /* Circulo indicador al lado */
    {
        const int SEGMENTOS = 16;
        int k;
        float cx = 155.0f;
        float cy = (float)alto - 221.0f;
        float rad = 7.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (k = 0; k <= SEGMENTOS; k++) {
            float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
            glVertex2f(cx + cosf(ang) * rad, cy + sinf(ang) * rad);
        }
        glEnd();
        
        /* Borde interior blanco del circulo */
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        for (k = 0; k <= SEGMENTOS; k++) {
            float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
            glVertex2f(cx + cosf(ang) * (rad - 2.0f), cy + sinf(ang) * (rad - 2.0f));
        }
        glEnd();
    }

    /* 5. Cantidad apuestas */
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf_s(buffer, sizeof(buffer), "Apuestas: %d", num_apuestas_activas);
    dibujar_texto_2d(15.0f, (float)alto - 250.0f, buffer);

    /* --- MEDIDOR DE RIESGO AMBIENTAL EN EL HUD --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 276.0f, "Riesgo de sesion: ");
    len_label_riesgo = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Riesgo de sesion: ");
    val_x_riesgo = 15.0f + (float)len_label_riesgo + 6.0f;

    if (nivel_riesgo == 0) {
        glColor3f(0.2f, 0.8f, 0.2f);
        dibujar_texto_2d(val_x_riesgo, (float)alto - 276.0f, "BAJO");
    } else if (nivel_riesgo == 1) {
        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_2d(val_x_riesgo, (float)alto - 276.0f, "MODERADO");
    } else if (nivel_riesgo == 2) {
        glColor3f(1.0f, 0.55f, 0.0f);
        dibujar_texto_2d(val_x_riesgo, (float)alto - 276.0f, "ALTO");
    } else {
        glColor3f(1.0f, 0.3f, 0.3f);
        dibujar_texto_2d(val_x_riesgo, (float)alto - 276.0f, "CONDUCTA COMPULSIVA");
    }

    /* Aclaracion en ASCII */
    glColor3f(0.6f, 0.6f, 0.6f);
    dibujar_texto_2d(15.0f, (float)alto - 298.0f, "(simulacion educativa, no un diagnostico real)");

    /* Indicador explicito de tecla S */
    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d(15.0f, (float)alto - 324.0f, "[S] Terminar la sesion en cualquier momento");

    /* --- PROBABILIDADES REALES DURANTE EL GIRO --- */
    if (bolita_girando) {
        int idx = indice_probabilidad % 12;
        if (idx < 0) idx = 0;
        const char* msg_prob = MENSAJES_PROBABILIDAD[idx];
        int msg_len = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)msg_prob);
        float banner_cx = (float)ancho / 2.0f;
        float banner_y = (float)alto - 45.0f;
        float half_w = (float)msg_len / 2.0f + 12.0f;

        /* Margen de seguridad: evitar que el banner central pise el HUD (345px) o las notificaciones (ancho - 315px) */
        if (banner_cx - half_w < 360.0f) {
            banner_cx = 360.0f + half_w;
        }
        if (banner_cx + half_w > (float)ancho - 330.0f) {
            banner_cx = (float)ancho - 330.0f - half_w;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.02f, 0.08f, 0.18f, 0.88f);
        glBegin(GL_QUADS);
        glVertex2f(banner_cx - half_w, banner_y - 8.0f);
        glVertex2f(banner_cx + half_w, banner_y - 8.0f);
        glVertex2f(banner_cx + half_w, banner_y + 24.0f);
        glVertex2f(banner_cx - half_w, banner_y + 24.0f);
        glEnd();

        glColor3f(0.55f, 0.75f, 1.0f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(banner_cx - half_w, banner_y - 8.0f);
        glVertex2f(banner_cx + half_w, banner_y - 8.0f);
        glVertex2f(banner_cx + half_w, banner_y + 24.0f);
        glVertex2f(banner_cx - half_w, banner_y + 24.0f);
        glEnd();
        glLineWidth(1.0f);
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.85f, 0.0f);
        dibujar_texto_2d(banner_cx - (float)msg_len / 2.0f, banner_y, msg_prob);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}