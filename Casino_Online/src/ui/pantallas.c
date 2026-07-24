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

void dibujar_pantalla_menu(void) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    float cx = (float)ancho / 2.0f;
    float cy = (float)alto / 2.0f;

    /* --- Entrar en modo 2D (igual que en hud.c) --- */
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
    dibujar_texto_2d(cx - 190.0f, cy + 190.0f, "CASINO ONLINE - SIMULADOR DE RULETA");

    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d(cx - 220.0f, cy + 150.0f,
        "Juegas con saldo virtual. Nunca dinero real.");

    /* --- Controles --- */
    dibujar_texto_2d(cx - 220.0f, cy + 105.0f, "Controles:");
    dibujar_texto_2d(cx - 220.0f, cy + 78.0f, "1 - 4            Elegir monto de ficha");
    dibujar_texto_2d(cx - 220.0f, cy + 51.0f, "Clic izquierdo   Apostar en la celda senalada");
    dibujar_texto_2d(cx - 220.0f, cy + 24.0f, "Clic derecho     Quitar una ficha de esa celda");
    dibujar_texto_2d(cx - 220.0f, cy - 3.0f, "BACKSPACE        Deshacer la ultima ficha");
    dibujar_texto_2d(cx - 220.0f, cy - 30.0f, "ESPACIO          Girar la ruleta");
    dibujar_texto_2d(cx - 220.0f, cy - 57.0f, "P                Pedir prestamo (sin saldo)");
    dibujar_texto_2d(cx - 220.0f, cy - 84.0f, "S                Terminar la sesion (si se ofrece la opcion)");
    dibujar_texto_2d(cx - 220.0f, cy - 111.0f, "I                Informacion sobre ludopatia");

    /* --- Llamado a la accion --- */
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d(cx - 150.0f, cy - 140.0f, "[ENTER] Comenzar     [ESC] Salir");

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
    char buffer[128];

    /* --- Entrar en modo 2D (igual que en hud.c) --- */
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

    glColor4f(0.0f, 0.0f, 0.0f, 0.6f); /* negro al 60% de opacidad */
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f, (float)alto);
    glEnd();

    glDisable(GL_BLEND);

    /* --- Mensaje reflexivo --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f + 40.0f,
        "Ya perdiste tu saldo inicial.");
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f + 15.0f,
        "En la vida real, este seria el momento de parar.");

    sprintf_s(buffer, sizeof(buffer), "Deuda actual: %.2f", jugador->deuda);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 15.0f, buffer);

    /* MEJORA (antes la unica salida real de esta pantalla era pedir
       prestamo -la unica alternativa era ESC, que cierra el programa
       entero-. Se agrega [S] como una salida real y sin castigo: cerrar
       la sesion ACA, antes de endeudarse, en vez de forzar a elegir
       entre "pedir plata prestada" o "cerrar todo de golpe". */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 50.0f,
        "[P] Pedir prestamo y seguir");
    glColor3f(1.0f, 0.85f, 0.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 220.0f, (float)alto / 2.0f - 75.0f,
        "[S] Terminar la sesion aqui, sin pedir prestamo");

    /* --- Restaurar estado 3D --- */
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

    /* MEJORA (interaccion de cierre): esta pantalla escala su friccion
       si no es la primera vez en la sesion (jugador->mensajes_reflexivos_mostrados
       ya viene incrementado por verificar_mensaje_reflexivo() antes de
       llegar aca, asi que en la 2da vez ya vale 2). No es que "cueste
       mas" cerrarla -las dos teclas siguen funcionando igual-, pero la
       jerarquia visual se invierte: la primera vez "seguir jugando" es
       la opcion tranquila/default y "terminar" es la alternativa; de
       la 2da vez en adelante, "terminar" pasa a ser la opcion resaltada
       y "seguir jugando" queda en segundo plano. La idea es no
       recompensar visualmente seguir jugando despues de que la misma
       senal de alerta ya se repitio en la sesion. */
    int es_repeticion = (jugador != NULL && jugador->mensajes_reflexivos_mostrados >= 2);

    /* --- Entrar en modo 2D (igual que el resto de las pantallas) --- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* --- Overlay semitransparente. Azul oscuro en vez del negro/rojo
       que usan prestamo/game-over: esta pantalla no es un castigo ni
       una derrota, es una pausa reflexiva -se busca un tono calmo,
       no alarmante. --- */
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

    /* --- Titulo --- */
    glColor3f(0.55f, 0.75f, 1.0f);
    {
        const char* titulo = "UN MOMENTO...";
        dibujar_texto_2d(cx - (float)strlen(titulo) * ANCHO_CHAR_APROX / 2.0f, cy + 130.0f, titulo);
    }

    /* --- Cuerpo del mensaje. El mensaje lo arma verificar_mensaje_reflexivo()
       (core/jugador.c) con los saltos de linea ya puestos a mano en
       largos razonables, asi que no hace falta un word-wrap automatico
       por ancho de pantalla aca. --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_multilinea_centrado(mensaje, cx, cy + 80.0f, 24.0f);

    /* --- Las dos opciones reales: seguir jugando, o terminar aca.
       MEJORA (antes esto era un solo "[ENTER] Continuar jugando": se
       descartaba el mensaje y se seguia jugando exactamente igual, sin
       ninguna alternativa real -que fue precisamente lo que se senalo
       como inconsistente). Ahora las dos rutas llevan a un estado
       distinto de verdad (ver main.c/teclado() y ESTADO_SESION_TERMINADA). --- */
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

    /* --- Restaurar estado 3D --- */
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
    char buffer[128];

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

    /* --- Overlay con interpolacion de color (fade rojo/negro) ---
       Usamos glutGet(GLUT_ELAPSED_TIME) para variar la opacidad en el
       tiempo y dar un efecto de "pulso" dramatico, en vez de un overlay
       estatico. */
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

    /* --- Resumen tipo "recibo" --- */
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

    /* MEJORA (simulacion de consecuencias, pilar 3 del "serious game":
       tiempo, ademas de dinero). No se simula familia/estudios/
       trabajo/salud mental como sistemas independientes -no es
       realista prometer eso en este proyecto-, en cambio se traduce
       narrativamente el tiempo y la deuda YA trackeados en un costo
       reconocible (mismo enfoque que usa el serious game "Spent" sobre
       pobreza: abstrae datos reales en vez de simular cada sistema por
       separado, ver docs/analisis-ludopatia.md). */
    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 10.0f, buffer);

    glColor3f(1.0f, 0.6f, 0.5f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 45.0f,
        "Ese tiempo y esa deuda son el mismo tipo de costo que,");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 70.0f,
        "en la vida real, le resta a la familia, el trabajo o los estudios.");

    /* --- Mensaje final ---
       MEJORA (tono): antes decia "GAME OVER: la casa siempre gana",
       una linea de cierre sin ningun valor reflexivo. Se reformula
       apuntando a lo mismo que ya se le mostro en el "recibo" arriba:
       la deuda es consecuencia directa de haber seguido apostando
       despues de quedarse sin saldo, no un chiste sobre la casa. */
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buffer, sizeof(buffer), "Pediste %d prestamo(s) para poder seguir jugando,", jugador->prestamos_activos);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 105.0f, buffer);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 130.0f,
        "y la deuda crecio hasta ser impagable.");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 165.0f,
        "[ESC] Salir     [ENTER] Reiniciar");

    /* --- Restaurar estado 3D --- */
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
    char buffer[128];

    /* MEJORA (nueva pantalla): se llega aca cuando el jugador ELIGIO
       terminar -desde el mensaje reflexivo o desde la pantalla de
       prestamo- en vez de que el juego lo obligue por quedarse sin
       fondos. Mismo "recibo" de estadisticas que Game Over, pero con
       overlay calmo (mismo azul de la pantalla reflexiva, no el rojo
       pulsante de la derrota) y un cierre que reconoce la decision en
       vez de sancionarla. */
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

    /* --- Titulo --- */
    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 130.0f, (float)alto / 2.0f + 150.0f,
        "SESION TERMINADA POR DECISION PROPIA");

    /* --- Resumen --- */
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f + 100.0f,
        "Se eligio parar antes de que la situacion");
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

    /* MEJORA (simulacion de consecuencias, pilar 3 del "serious game").
       Mismo criterio que en dibujar_pantalla_game_over: se traduce el
       tiempo real jugado en una linea reflexiva, sin pretender simular
       familia/trabajo/estudios/salud mental como sistemas aparte. */
    sprintf_s(buffer, sizeof(buffer), "Tiempo jugado: %.0f minutos", tiempo_jugado_minutos(jugador, glutGet(GLUT_ELAPSED_TIME)));
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 60.0f, buffer);

    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 95.0f,
        "Es un buen momento para pensar en que mas se");
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 120.0f,
        "podria haber hecho con ese tiempo.");

    /* --- Cierre --- */
    glColor3f(0.55f, 0.75f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 155.0f,
        "Parar a tiempo tambien es una forma de jugar bien.");
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d((float)ancho / 2.0f - 180.0f, (float)alto / 2.0f - 185.0f,
        "[ESC] Salir     [ENTER] Jugar una sesion nueva");

    /* --- Restaurar estado 3D --- */
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

    /* --- Cuerpo, alineado a la izquierda (las vinetas se verian mal centradas) --- */
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

void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const InfoPantalla* info) {
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
        if (info->mensaje_reflexivo != NULL) dibujar_pantalla_mensaje_reflexivo(info->mensaje_reflexivo, jugador);
        break;
    case ESTADO_EDUCACION:
        dibujar_pantalla_educacion(info->pagina_educacion);
        break;
    default:
        break; /* ESTADO_JUGANDO no requiere overlay */
    }
}