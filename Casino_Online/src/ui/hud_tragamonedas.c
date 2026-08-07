#include <GL/glut.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "hud_tragamonedas.h"

/* Barra de control 2D (SALDO / BET / WIN / SPIN / AUTO)                */
/* ------------------------------------------------------------------- */

/* REDISENO (mas atractiva y agrupada): antes las zonas se ubicaban en
   fracciones del ANCHO DE VENTANA (0.30, 0.52, 0.68...), asi que en una
   ventana grande quedaban separadas por cientos de pixeles de fondo
   vacio -de ahi la sensacion de "todo lejos". Ahora el conjunto entero
   es un panel COMPACTO de ancho fijo (no proporcional a la ventana),
   centrado horizontalmente, con 5 columnas parejas bien juntas. */
#define BARRA_CONTROL_ALTO       110.0f
#define BARRA_CONTROL_ANCHO      680.0f
#define BARRA_CONTROL_COL_ANCHO  (BARRA_CONTROL_ANCHO / 5.0f)

/* Texto 2D con fuente de bitmap de GLUT (pensada para overlays 2D, a
   diferencia de la fuente stroke/extruible que usa el resto del
   gabinete 3D). Devuelve el ancho aproximado en pixeles. */
static int dibujar_texto_2d_bitmap(float x, float y, const char* texto, void* fuente) {
    const char* c;
    int ancho = 0;
    glRasterPos2f(x, y);
    for (c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(fuente, *c);
        ancho += glutBitmapWidth(fuente, *c);
    }
    return ancho;
}

/* Version centrada: calcula el ancho del texto primero (sin dibujar)
   para arrancar desplazado y que quede centrado en cx. */
static void dibujar_texto_2d_centrado(float cx, float y, const char* texto, void* fuente) {
    const char* c;
    int ancho = 0;
    for (c = texto; *c != '\0'; c++) ancho += glutBitmapWidth(fuente, *c);
    dibujar_texto_2d_bitmap(cx - (float)ancho / 2.0f, y, texto, fuente);
}

static void dibujar_circulo_2d(float cx, float cy, float radio, int relleno, int segmentos) {
    int i;
    glBegin(relleno ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
    if (relleno) glVertex2f(cx, cy);
    for (i = 0; i <= segmentos; i++) {
        double ang = 2.0 * 3.14159265358979323846 * (double)i / (double)segmentos;
        glVertex2f(cx + radio * (float)cos(ang), cy + radio * (float)sin(ang));
    }
    glEnd();
}

/* Boton circular con aspecto "glossy" (sombra + brillo superior), para
   que los botones se lean como piezas de interfaz de juego pulidas en
   vez de circulos planos lisos. */
static void dibujar_boton_circular_glossy(float cx, float cy, float radio, float r, float g, float b) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    dibujar_circulo_2d(cx, cy - radio * 0.10f, radio, 1, 28);

    glColor3f(r, g, b);
    dibujar_circulo_2d(cx, cy, radio, 1, 28);

    glColor4f(1.0f, 1.0f, 1.0f, 0.30f);
    dibujar_circulo_2d(cx - radio * 0.18f, cy + radio * 0.32f, radio * 0.55f, 1, 20);

    glDisable(GL_BLEND);
}

/* Panel 2D con esquinas cortadas (octogono) y degradado vertical -mismo
   truco que dibujar_caja_biselada() en 3D, pero en pantalla-, para que
   el fondo de la barra se lea como una tarjeta pulida en vez de un
   rectangulo plano de canto vivo. */
static void dibujar_panel_2d_biselado(float x0, float y0, float x1, float y1, float bisel,
                                       float r_arriba, float g_arriba, float b_arriba, float a_arriba,
                                       float r_abajo, float g_abajo, float b_abajo, float a_abajo) {
    float vx[8], vy[8];
    int i;

    vx[0] = x1 - bisel; vy[0] = y1;
    vx[1] = x1;         vy[1] = y1 - bisel;
    vx[2] = x1;         vy[2] = y0 + bisel;
    vx[3] = x1 - bisel; vy[3] = y0;
    vx[4] = x0 + bisel; vy[4] = y0;
    vx[5] = x0;         vy[5] = y0 + bisel;
    vx[6] = x0;         vy[6] = y1 - bisel;
    vx[7] = x0 + bisel; vy[7] = y1;

    glBegin(GL_POLYGON);
    for (i = 0; i < 8; i++) {
        float t = (vy[i] - y0) / (y1 - y0);
        glColor4f(r_abajo + (r_arriba - r_abajo) * t,
                  g_abajo + (g_arriba - g_abajo) * t,
                  b_abajo + (b_arriba - b_abajo) * t,
                  a_abajo + (a_arriba - a_abajo) * t);
        glVertex2f(vx[i], vy[i]);
    }
    glEnd();

    glColor4f(0.85f, 0.65f, 0.15f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < 8; i++) glVertex2f(vx[i], vy[i]);
    glEnd();
}

/* Centro/radio de cada zona clickeable de la barra, en coordenadas de
   pantalla (origen abajo-izquierda, Y hacia arriba). Centralizado aca
   para que el dibujado y el hit-test usen siempre las mismas zonas.
   Todo el panel es de ancho FIJO (BARRA_CONTROL_ANCHO), centrado en la
   ventana -no proporcional al ancho de ventana- para que quede siempre
   compacto sin importar el tamano de la pantalla. */
static void calcular_zonas_barra_control(int ancho_ventana,
    float* panel_x0, float* panel_x1,
    float* spin_cx, float* spin_cy, float* spin_r,
    float* bet_menos_cx, float* bet_mas_cx, float* bet_y, float* bet_r) {

    float cx_ventana = (float)ancho_ventana / 2.0f;
    float col_x0 = cx_ventana - BARRA_CONTROL_ANCHO / 2.0f;
    float cy = BARRA_CONTROL_ALTO / 2.0f;

    *panel_x0 = col_x0;
    *panel_x1 = col_x0 + BARRA_CONTROL_ANCHO;

    /* columna 1 (indice 1, 0-based): BET */
    *bet_menos_cx = col_x0 + BARRA_CONTROL_COL_ANCHO * 1.0f + 30.0f;
    *bet_mas_cx   = col_x0 + BARRA_CONTROL_COL_ANCHO * 2.0f - 30.0f;
    *bet_y        = cy - 8.0f;
    *bet_r        = 17.0f;

    /* columna 3: SPIN, mas grande, el foco visual del panel */
    *spin_cx = col_x0 + BARRA_CONTROL_COL_ANCHO * 3.5f;
    *spin_cy = cy;
    *spin_r  = 42.0f;
}

void dibujar_barra_control_2d(int ancho_ventana, int alto_ventana,
                               float saldo, float apuesta, float ganancia,
                               const EstadoTragamonedas* estado) {
    char buf[64];
    int hay_giro, i;
    float panel_x0, panel_x1;
    float spin_cx, spin_cy, spin_r;
    float bet_menos_cx, bet_mas_cx, bet_y, bet_r;
    float saldo_cx, win_cx, bet_label_cx;
    float y_base = BARRA_CONTROL_ALTO / 2.0f;

    calcular_zonas_barra_control(ancho_ventana, &panel_x0, &panel_x1,
        &spin_cx, &spin_cy, &spin_r,
        &bet_menos_cx, &bet_mas_cx, &bet_y, &bet_r);

    saldo_cx     = panel_x0 + BARRA_CONTROL_COL_ANCHO * 0.5f;
    bet_label_cx = (bet_menos_cx + bet_mas_cx) / 2.0f;
    win_cx       = panel_x0 + BARRA_CONTROL_COL_ANCHO * 2.5f;

    hay_giro = 0;
    if (estado != NULL) {
        for (i = 0; i < NUM_RODILLOS; i++) {
            if (estado->rodillos[i].girando) hay_giro = 1;
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho_ventana, 0, alto_ventana, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Panel compacto, centrado, con degradado violeta y borde dorado
       -una sola tarjeta agrupada, no una franja de punta a punta con
       piezas sueltas adentro. */
    dibujar_panel_2d_biselado(panel_x0, 8.0f, panel_x1, BARRA_CONTROL_ALTO,
        14.0f,
        0.30f, 0.08f, 0.42f, 0.94f, /* arriba: violeta mas claro */
        0.08f, 0.02f, 0.14f, 0.94f  /* abajo: violeta oscuro */
    );
    glDisable(GL_BLEND);

    /* SALDO */
    glColor3f(0.75f, 0.90f, 1.0f);
    dibujar_texto_2d_centrado(saldo_cx, y_base + 16.0f, "SALDO", GLUT_BITMAP_HELVETICA_10);
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buf, sizeof(buf), "%.2f", saldo);
    dibujar_texto_2d_centrado(saldo_cx, y_base - 10.0f, buf, GLUT_BITMAP_HELVETICA_18);

    /* BET con -/+ */
    glColor3f(0.85f, 0.75f, 1.0f);
    dibujar_texto_2d_centrado(bet_label_cx, y_base + 26.0f, "APUESTA", GLUT_BITMAP_HELVETICA_10);
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf_s(buf, sizeof(buf), "%.0f", apuesta);
    dibujar_texto_2d_centrado(bet_label_cx, bet_y - 6.0f, buf, GLUT_BITMAP_HELVETICA_18);

    dibujar_boton_circular_glossy(bet_menos_cx, bet_y, bet_r, 0.45f, 0.15f, 0.62f);
    dibujar_boton_circular_glossy(bet_mas_cx, bet_y, bet_r, 0.45f, 0.15f, 0.62f);
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d_centrado(bet_menos_cx, bet_y - 6.0f, "-", GLUT_BITMAP_HELVETICA_18);
    dibujar_texto_2d_centrado(bet_mas_cx, bet_y - 6.0f, "+", GLUT_BITMAP_HELVETICA_18);

    /* WIN */
    if (ganancia > 0.0f) glColor3f(0.35f, 1.0f, 0.40f);
    else                 glColor3f(1.0f, 0.55f, 0.55f);
    dibujar_texto_2d_centrado(win_cx, y_base + 16.0f, "GANANCIA", GLUT_BITMAP_HELVETICA_10);
    sprintf_s(buf, sizeof(buf), "%.2f", ganancia);
    dibujar_texto_2d_centrado(win_cx, y_base - 10.0f, buf, GLUT_BITMAP_HELVETICA_18);

    /* SPIN: boton glossy rojo con aro dorado, gris mientras gira */
    glColor3f(0.85f, 0.65f, 0.15f);
    dibujar_circulo_2d(spin_cx, spin_cy, spin_r + 6.0f, 1, 30);
    if (hay_giro) dibujar_boton_circular_glossy(spin_cx, spin_cy, spin_r, 0.42f, 0.42f, 0.44f);
    else          dibujar_boton_circular_glossy(spin_cx, spin_cy, spin_r, 0.85f, 0.12f, 0.14f);
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d_centrado(spin_cx, spin_cy - 6.0f, "SPIN", GLUT_BITMAP_HELVETICA_18);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

ZonaControlTragamonedas obtener_zona_control_2d(int x_mouse, int y_mouse,
                                                 int ancho_ventana, int alto_ventana) {
    float panel_x0, panel_x1;
    float spin_cx, spin_cy, spin_r;
    float bet_menos_cx, bet_mas_cx, bet_y, bet_r;
    float fx = (float)x_mouse;
    /* GLUT entrega la Y del mouse creciendo hacia ABAJO (origen arriba-
       izquierda); las zonas estan en coordenadas Y-hacia-arriba
       (glOrtho), asi que hay que invertir antes de comparar. */
    float fy = (float)(alto_ventana - y_mouse);
    float dx, dy;

    calcular_zonas_barra_control(ancho_ventana, &panel_x0, &panel_x1,
        &spin_cx, &spin_cy, &spin_r,
        &bet_menos_cx, &bet_mas_cx, &bet_y, &bet_r);

    dx = fx - spin_cx; dy = fy - spin_cy;
    if (dx * dx + dy * dy <= spin_r * spin_r) return ZONA_CONTROL_SPIN;

    dx = fx - bet_menos_cx; dy = fy - bet_y;
    if (dx * dx + dy * dy <= bet_r * bet_r) return ZONA_CONTROL_BET_MENOS;

    dx = fx - bet_mas_cx; dy = fy - bet_y;
    if (dx * dx + dy * dy <= bet_r * bet_r) return ZONA_CONTROL_BET_MAS;

    return ZONA_CONTROL_NINGUNA;
}

static const char* MENSAJES_GIRO_TRAGAMONEDAS[8] = {
    "Un 'casi acierto' no es casi ganar.\nEs una perdida igual que cualquier otra.",
    "Ganar menos de lo apostado es perder,\naunque la maquina festeje.",
    "La maquina no esta fria ni caliente.\nCada giro es totalmente independiente.",
    "El resultado es totalmente al azar,\nsin importar cuanto jugaste antes.",
    "Cuanto apostaste en esta maquina hoy?\nEs lo que tenias pensado?",
    "Si esto fuera dinero real,\nseguirias jugando de la misma forma?",
    "Luces y sonidos buscan retenerte,\nno solo informarte del resultado.",
    "De verdad necesitas ver este giro,\no podrias parar aca?"
};

void dibujar_mensaje_giro_tragamonedas(int indice_mensaje) {
    int ancho = glutGet(GLUT_WINDOW_WIDTH);
    int alto = glutGet(GLUT_WINDOW_HEIGHT);
    int idx = indice_mensaje % 8;
    const char* msg_prob;
    char linea1[256] = {0};
    char linea2[256] = {0};
    char* salto;
    int len1, len2, max_len;
    float banner_cx, banner_y, half_w;

    if (idx < 0) idx = 0;
    msg_prob = MENSAJES_GIRO_TRAGAMONEDAS[idx];

    /* Separar en dos lineas por el \n */
    strncpy_s(linea1, sizeof(linea1), msg_prob, _TRUNCATE);
    salto = strchr(linea1, '\n');
    if (salto) {
        *salto = '\0';
        strncpy_s(linea2, sizeof(linea2), salto + 1, _TRUNCATE);
    }

    len1 = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)linea1);
    len2 = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)linea2);
    max_len = (len1 > len2) ? len1 : len2;

    banner_cx = (float)ancho / 2.0f;
    banner_y = (float)alto - 60.0f; /* Un poco mas bajo porque son 2 lineas */
    half_w = (float)max_len / 2.0f + 16.0f;

    /* Margen de seguridad: evitar pisar UI lateral si la hubiera */
    {
        float margen_hud = 15.0f + (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"[S] Terminar la sesion en cualquier momento") + 20.0f;
        if (banner_cx - half_w < margen_hud) {
            banner_cx = margen_hud + half_w;
        }
    }
    if (banner_cx + half_w > (float)ancho - 10.0f) {
        banner_cx = (float)ancho - 10.0f - half_w;
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.02f, 0.08f, 0.18f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(banner_cx - half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y + 24.0f);
    glVertex2f(banner_cx - half_w, banner_y + 24.0f);
    glEnd();

    glColor3f(0.55f, 0.75f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(banner_cx - half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y - 28.0f);
    glVertex2f(banner_cx + half_w, banner_y + 24.0f);
    glVertex2f(banner_cx - half_w, banner_y + 24.0f);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.85f, 0.0f);
    if (salto) {
        dibujar_texto_2d_centrado(banner_cx, banner_y + 4.0f, linea1, GLUT_BITMAP_HELVETICA_18);
        dibujar_texto_2d_centrado(banner_cx, banner_y - 20.0f, linea2, GLUT_BITMAP_HELVETICA_18);
    } else {
        dibujar_texto_2d_centrado(banner_cx, banner_y, linea1, GLUT_BITMAP_HELVETICA_18);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
