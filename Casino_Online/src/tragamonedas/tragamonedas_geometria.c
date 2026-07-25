/*
 * tragamonedas_geometria.c
 * Implementacion de la geometria del tragamonedas. Ver tragamonedas_geometria.h.
 * A CARGO DE: Luis.
 *
 * PASO 1: gabinete -listo.
 * PASO 2: los 3 rodillos con los simbolos de tragamonedas_logica.c
 * (Dubenny), consultados siempre via obtener_simbolo_en_posicion().
 * PASO 3: palanca.
 *
 * PALETA (revision visual): dorado + rojo, en vez de cromo + negro
 * -inspirado en el estilo clasico de tragamonedas de casino (marco
 * dorado, panel rojo, arco arriba, botones), sin copiar ninguna imagen
 * puntual -todo hecho con primitivas propias.
 *
 * Nota sobre el estado de animacion: dibujar_tragamonedas() no recibe
 * parametros (mismo contrato que el placeholder original), pero para
 * dibujar los rodillos hace falta un EstadoTragamonedas. Como la
 * integracion real con partida (main.c) todavia no se hace, este
 * archivo mantiene su PROPIO estado estatico interno
 * (g_estado_tragamonedas), inicializado una sola vez.
 */
#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "tragamonedas_geometria.h"
#include "tragamonedas_animacion.h"
#include "tragamonedas_logica.h"
#include "../render/materiales.h"

 /* Alto aproximado (en unidades de fuente, antes de escalar) de una
    mayuscula en GLUT_STROKE_ROMAN -se usa para centrar verticalmente el
    texto, ya que glutStrokeCharacter() dibuja desde la linea de base
    hacia arriba, no centrado. */
#define ALTURA_APROX_MAYUSCULA_STROKE 119.05f

    /* Grosor de los paneles laterales que enmarcan la ventana frontal del
       cuerpo -el paso 2 (rodillos) la necesita para saber cuanto espacio
       libre queda adentro. */
#define GROSOR_MARCO_VENTANA 0.30f

       /* Alto de las franjas decorativas doradas entre secciones del gabinete */
#define ALTO_FRANJA 0.08f

/* Dimensiones internas de la ventana donde van los rodillos, derivadas
   de las constantes publicas del gabinete (ver tragamonedas_geometria.h) */
#define VENTANA_ANCHO_INTERNO       (GABINETE_ANCHO - 2.0f * GROSOR_MARCO_VENTANA)
#define VENTANA_ALTO_INTERNO        (GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA)
#define VENTANA_PROFUNDIDAD_INTERNA (GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA)
#define VENTANA_Y_CENTRO            (GABINETE_Y_INICIO_CUERPO + VENTANA_ALTO_INTERNO / 2.0f)

   /* Grosor de los separadores finos entre rodillos */
#define GROSOR_DIVISOR_RODILLO 0.04f

/* Constantes de la palanca -brazo cilindrico inclinado con una perilla
   en la punta, montado en un soporte al costado derecho del gabinete. */
#define PALANCA_EXTENSION_SOPORTE      0.14f
#define PALANCA_RADIO_BRAZO            0.032f
#define PALANCA_RADIO_BRAZO_PUNTA      0.026f
#define PALANCA_LONGITUD_BRAZO         1.05f
#define PALANCA_RADIO_PERILLA          0.095f
#define PALANCA_ANGULO_INCLINACION     55.0f
#define PALANCA_Y_PIVOTE_FACTOR        0.55f

   /* Cuantos simbolos se ven por rodillo (el central + uno arriba y uno
      abajo) y cuanto espacio vertical ocupa cada "unidad de simbolo" de
      EstadoRodillo.posicion_actual -se reparte la altura interna de la
      ventana en 3 franjas iguales. */
#define SIMBOLOS_VISIBLES_POR_RODILLO 3
#define ALTURA_UNIDAD_SIMBOLO (VENTANA_ALTO_INTERNO / (float)SIMBOLOS_VISIBLES_POR_RODILLO)

      /* Estado interno de la animacion, solo para poder dibujar algo
         coherente mientras no existe la integracion real. NO es el estado
         "oficial" del juego. */
static EstadoTragamonedas g_estado_tragamonedas;
static int g_estado_tragamonedas_listo = 0;


/* Dibuja texto con la fuente stroke de GLUT, centrado aproximadamente
   sobre el punto actual. */
static void dibujar_texto_stroke_simple(const char* texto, float escala) {
    int i;
    int len = (int)strlen(texto);

    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-(float)len * 52.38f, 0.0f, 0.0f);
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();
}


/* Dibuja una caja centrada en el origen actual, usando glutSolidCube
   escalado -helper chico para no repetir push/scale/cube/pop. */
static void dibujar_caja(float ancho, float alto, float profundo) {
    glPushMatrix();
    glScalef(ancho, alto, profundo);
    glutSolidCube(1.0);
    glPopMatrix();
}


/* Prisma con esquinas cortadas (octogono extruido en Z), hecho a mano
   con GL_QUADS/GL_POLYGON y normal por cara -da un borde biselado en
   vez del canto vivo de glutSolidCube, que es lo que hace que un panel
   se lea como una pieza fabricada y no como un cubo de CAD. El bisel
   se recorta si es mas grande que la mitad del lado mas chico, para
   nunca generar un octogono invalido (esquinas que se cruzan). */
static void dibujar_caja_biselada(float ancho, float alto, float profundo, float bisel) {
    float hw, hh, hd;
    float vx[8], vy[8];
    float bisel_max;
    int i;

    bisel_max = 0.45f * (ancho < alto ? ancho : alto);
    if (bisel > bisel_max) bisel = bisel_max;
    if (bisel < 0.0f) bisel = 0.0f;

    hw = ancho / 2.0f;
    hh = alto / 2.0f;
    hd = profundo / 2.0f;

    vx[0] = hw - bisel; vy[0] = hh;
    vx[1] = hw;         vy[1] = hh - bisel;
    vx[2] = hw;         vy[2] = -hh + bisel;
    vx[3] = hw - bisel; vy[3] = -hh;
    vx[4] = -hw + bisel; vy[4] = -hh;
    vx[5] = -hw;         vy[5] = -hh + bisel;
    vx[6] = -hw;         vy[6] = hh - bisel;
    vx[7] = -hw + bisel; vy[7] = hh;

    /* cara frontal (+Z) */
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);
    for (i = 0; i < 8; i++) glVertex3f(vx[i], vy[i], hd);
    glEnd();

    /* cara trasera (-Z), orden invertido para que quede la normal -Z */
    glNormal3f(0.0f, 0.0f, -1.0f);
    glBegin(GL_POLYGON);
    for (i = 7; i >= 0; i--) glVertex3f(vx[i], vy[i], -hd);
    glEnd();

    /* 8 caras laterales, cada una con su propia normal (perpendicular
       al borde que le toca, apuntando hacia afuera) */
    for (i = 0; i < 8; i++) {
        int j = (i + 1) % 8;
        float ex, ey, nx, ny, len;

        ex = vx[j] - vx[i];
        ey = vy[j] - vy[i];
        nx = ey;
        ny = -ex;
        len = (float)sqrt((double)(nx * nx + ny * ny));
        if (len > 0.00001f) { nx /= len; ny /= len; }

        glNormal3f(nx, ny, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(vx[i], vy[i], -hd);
        glVertex3f(vx[j], vy[j], -hd);
        glVertex3f(vx[j], vy[j], hd);
        glVertex3f(vx[i], vy[i], hd);
        glEnd();
    }
}


/* Bisel estandar para los paneles grandes del gabinete -chico en
   proporcion al tamano de cada pieza, para que se note el canto
   trabajado sin perder el volumen general. */
#define BISEL_PANEL_GRANDE 0.05f
#define BISEL_FRANJA        0.015f


   /* Material local del cuerpo del gabinete: marron/caoba oscuro (no
      negro), para la paleta dorado+rojo. */
static void aplicar_material_cuerpo(void) {
    GLfloat ambient[4] = { 0.10f, 0.035f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.34f, 0.12f, 0.07f, 1.0f };
    GLfloat specular[4] = { 0.25f, 0.14f, 0.10f, 1.0f };
    GLfloat shininess = 24.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Material local dorado -reemplaza al cromado (MATERIAL_METAL) en todo
   el marco/trim del gabinete para la paleta dorado+rojo. */
static void aplicar_material_dorado(void) {
    GLfloat ambient[4] = { 0.42f, 0.32f, 0.10f, 1.0f };
    GLfloat diffuse[4] = { 0.80f, 0.60f, 0.16f, 1.0f };
    GLfloat specular[4] = { 0.95f, 0.85f, 0.55f, 1.0f };
    GLfloat shininess = 90.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Material local del rojo (paneles y acentos) */
static void aplicar_material_acento_rojo(void) {
    GLfloat ambient[4] = { 0.28f, 0.02f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.70f, 0.05f, 0.05f, 1.0f };
    GLfloat specular[4] = { 0.45f, 0.15f, 0.15f, 1.0f };
    GLfloat shininess = 65.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Fondo de los rodillos: negro terciopelo con un leve toque verde
   oscuro -contraste maximo con los simbolos dorados/rojos, da el look
   "casino de lujo" que no da el gris metalico plano. */
static void aplicar_material_fondo_rodillo(void) {
    GLfloat ambient[4]  = { 0.01f, 0.02f, 0.01f, 1.0f };
    GLfloat diffuse[4]  = { 0.03f, 0.07f, 0.03f, 1.0f };
    GLfloat specular[4] = { 0.12f, 0.20f, 0.12f, 1.0f };
    GLfloat shininess   = 20.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT,   ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
    glMaterialf (GL_FRONT, GL_SHININESS, shininess);
}


/* ------------------------------------------------------------------- */
/* Gabinete (PASO 1)                                                    */
/* ------------------------------------------------------------------- */

static void dibujar_base(void) {
    const float FACTOR_ANCHO_BASE = 1.08f;
    const float FACTOR_PROFUNDIDAD_BASE = 1.08f;

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_ALTURA_BASE / 2.0f, 0.0f);
    dibujar_caja_biselada(GABINETE_ANCHO * FACTOR_ANCHO_BASE, GABINETE_ALTURA_BASE, GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_BASE, BISEL_PANEL_GRANDE);
    glPopMatrix();
}

/* Dibuja un rombo/diamante decorativo de 4 caras, centrado en el
   origen actual -se usa como motivo ornamental en las columnas. */
static void dibujar_rombo_decorativo(float radio) {
    glBegin(GL_QUADS);
    /* cara frontal: rombo en XY */
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f,   radio,  0.002f);
    glVertex3f(radio,  0.0f,   0.002f);
    glVertex3f(0.0f,  -radio,  0.002f);
    glVertex3f(-radio, 0.0f,   0.002f);
    glEnd();
}

static void dibujar_cuerpo_principal(void) {
    float ancho_columna;
    float profundidad_util;
    float x_columna;
    int   k;

    ancho_columna = GROSOR_MARCO_VENTANA;
    profundidad_util = GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA;
    x_columna = (GABINETE_ANCHO - ancho_columna) / 2.0f;

    /* Columnas y panel superior: dorados */
    aplicar_material_dorado();

    glPushMatrix();
    glTranslatef(-x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja_biselada(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD, BISEL_PANEL_GRANDE);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja_biselada(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD, BISEL_PANEL_GRANDE);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA / 2.0f, 0.0f);
    dibujar_caja(GABINETE_ANCHO, GROSOR_MARCO_VENTANA, GABINETE_PROFUNDIDAD);
    glPopMatrix();

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, -profundidad_util / 2.0f);
    dibujar_caja(GABINETE_ANCHO - 2.0f * ancho_columna, GABINETE_ALTURA_CUERPO, GROSOR_MARCO_VENTANA);
    glPopMatrix();

    /* Rombos dorados ornamentales en la cara frontal de cada columna:
       5 rombos equiespaciados verticalmente, color rojo acento para
       contrastar con el fondo caoba de las columnas. */
    glDisable(GL_LIGHTING);
    for (k = 0; k < 5; k++) {
        float y_rombo = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * ((float)(k + 1) / 6.0f);
        float z_front = GABINETE_PROFUNDIDAD / 2.0f + 0.001f;
        float radio   = ancho_columna * 0.28f;

        glColor3f(0.90f, 0.72f, 0.10f); /* dorado brillante */

        /* columna izquierda */
        glPushMatrix();
        glTranslatef(-x_columna, y_rombo, z_front);
        dibujar_rombo_decorativo(radio);
        glPopMatrix();

        /* columna derecha */
        glPushMatrix();
        glTranslatef(x_columna, y_rombo, z_front);
        dibujar_rombo_decorativo(radio);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    /* Ranura de monedas: abertura horizontal en el lado derecho del
       cuerpo, a media altura -detalle de autenticidad. */
    {
        float y_slot  = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * 0.30f;
        float x_slot  = x_columna + ancho_columna / 2.0f;
        /* Marco dorado de la ranura */
        aplicar_material_dorado();
        glPushMatrix();
        glTranslatef(x_slot, y_slot, 0.0f);
        dibujar_caja_biselada(0.10f, 0.04f, GABINETE_PROFUNDIDAD * 0.12f, 0.008f);
        glPopMatrix();
        /* Hendidura oscura dentro de la ranura */
        {
            GLfloat am[4] = {0.01f, 0.01f, 0.01f, 1.0f};
            GLfloat di[4] = {0.02f, 0.02f, 0.02f, 1.0f};
            GLfloat sp[4] = {0.05f, 0.05f, 0.05f, 1.0f};
            glMaterialfv(GL_FRONT, GL_AMBIENT,   am);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,   di);
            glMaterialfv(GL_FRONT, GL_SPECULAR,  sp);
            glMaterialf (GL_FRONT, GL_SHININESS, 8.0f);
        }
        glPushMatrix();
        glTranslatef(x_slot + 0.001f, y_slot, 0.0f);
        dibujar_caja(0.07f, 0.018f, GABINETE_PROFUNDIDAD * 0.08f);
        glPopMatrix();
    }

    /* Bandeja de monedas: canaleta curva en la base del frente,
       donde caen las monedas -elemento esencial de aspecto real. */
    {
        float z_front_base = (GABINETE_PROFUNDIDAD * 1.08f) / 2.0f;
        float y_bandeja    = GABINETE_ALTURA_BASE * 0.72f;
        aplicar_material_cuerpo();
        glPushMatrix();
        glTranslatef(0.0f, y_bandeja, z_front_base);
        glScalef(GABINETE_ANCHO * 0.50f, 0.055f, 0.12f);
        glutSolidCube(1.0);
        glPopMatrix();
        /* borde dorado de la bandeja */
        aplicar_material_dorado();
        glPushMatrix();
        glTranslatef(0.0f, y_bandeja, z_front_base + 0.005f);
        dibujar_caja_biselada(GABINETE_ANCHO * 0.52f, 0.070f, 0.07f, 0.010f);
        glPopMatrix();
    }
}

static void dibujar_franja_dorada(float y, float ancho, float profundo) {
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);
    dibujar_caja_biselada(ancho, ALTO_FRANJA, profundo, BISEL_FRANJA);
    glPopMatrix();
}

/* Panel inferior: placa roja con marco dorado en el frente de la base,
   con una fila de botones blancos arriba -estilo clasico dorado/rojo
   de tragamonedas de casino (paleta y disposicion general, no copia de
   ninguna imagen puntual). */
static void dibujar_panel_inferior(void) {
    float y_centro_panel;
    float z_frente_base;
    float ancho_panel;
    float alto_panel;
    int i;
    const int NUM_BOTONES = 5;

    z_frente_base = (GABINETE_PROFUNDIDAD * 1.08f) / 2.0f;
    ancho_panel = GABINETE_ANCHO * 0.70f;
    alto_panel = GABINETE_ALTURA_BASE * 0.50f;
    y_centro_panel = GABINETE_ALTURA_BASE * 0.40f;

    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_panel, z_frente_base - 0.005f);
    dibujar_caja_biselada(ancho_panel + 0.07f, alto_panel + 0.07f, 0.05f, BISEL_FRANJA);
    glPopMatrix();

    aplicar_material_acento_rojo();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_panel, z_frente_base + 0.015f);
    dibujar_caja_biselada(ancho_panel, alto_panel, 0.04f, BISEL_FRANJA);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.95f, 0.95f, 0.92f);
    for (i = 0; i < NUM_BOTONES; i++) {
        float t = (float)i / (float)(NUM_BOTONES - 1);
        float x_boton = -ancho_panel / 2.0f * 0.75f + t * (ancho_panel * 0.75f);

        glPushMatrix();
        glTranslatef(x_boton, y_centro_panel + alto_panel / 2.0f + 0.08f, z_frente_base + 0.03f);
        glScalef(0.09f, 0.05f, 0.04f);
        glutSolidSphere(1.0, 10, 8);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}

/* 4 patas doradas en las esquinas de la base */
static void dibujar_patas(void) {
    float x_offset;
    float z_offset;
    int sx, sz;

    x_offset = (GABINETE_ANCHO * 1.08f) / 2.0f - 0.15f;
    z_offset = (GABINETE_PROFUNDIDAD * 1.08f) / 2.0f - 0.15f;

    aplicar_material_dorado();
    for (sx = -1; sx <= 1; sx += 2) {
        for (sz = -1; sz <= 1; sz += 2) {
            glPushMatrix();
            glTranslatef((float)sx * x_offset, -0.04f, (float)sz * z_offset);
            glScalef(0.10f, 0.10f, 0.06f);
            glutSolidSphere(1.0, 10, 10);
            glPopMatrix();
        }
    }
}

/* Marquesina: marco dorado con arco arriba, panel rojo adentro con el
   texto, y una fila de foquitos en el borde -estilo cartel de luces
   clasico. */
static void dibujar_marquesina(void) {
    const float FACTOR_ANCHO_MARQUESINA = 0.92f;
    const float FACTOR_PROFUNDIDAD_MARQUESINA = 0.75f;
    const int NUM_FOQUITOS_POR_BORDE = 9;
    float y_centro_marquesina;
    float ancho_marquesina;
    float profundidad_marquesina;
    float z_frente_marquesina;
    int i;

    y_centro_marquesina = GABINETE_Y_INICIO_MARQUESINA + GABINETE_ALTURA_MARQUESINA / 2.0f;
    ancho_marquesina = GABINETE_ANCHO * FACTOR_ANCHO_MARQUESINA;
    profundidad_marquesina = GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_MARQUESINA;

    /* Marco dorado (caja exterior) */
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, 0.0f);
    dibujar_caja_biselada(ancho_marquesina, GABINETE_ALTURA_MARQUESINA, profundidad_marquesina, BISEL_PANEL_GRANDE);
    glPopMatrix();

    /* Arco dorado arriba (media esfera achatada) */
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA / 2.0f, 0.0f);
    glScalef(ancho_marquesina / 2.0f * 0.97f, 0.24f, profundidad_marquesina / 2.0f * 0.97f);
    glutSolidSphere(1.0, 20, 10);
    glPopMatrix();

    /* Panel rojo recesado adentro del marco, con el texto encima */
    z_frente_marquesina = profundidad_marquesina / 2.0f + 0.015f;
    aplicar_material_acento_rojo();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, z_frente_marquesina - 0.03f);
    dibujar_caja_biselada(ancho_marquesina * 0.88f, GABINETE_ALTURA_MARQUESINA * 0.78f, 0.05f, BISEL_FRANJA);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    /* Texto de la marquesina: muestra "WIN!" parpadeando si hay premio
       (durante 5 segundos), o el titulo normal en otro caso. */
    {
        int hay_win_texto = g_estado_tragamonedas.hay_ganancia &&
                            g_estado_tragamonedas.tiempo_desde_parada < 5.0f;
        if (hay_win_texto) {
            /* Parpadeo: alterna dorado brillante y blanco-amarillo */
            float t_parpadeo = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
            int   fase_p     = (int)(t_parpadeo * 3.0f) % 2;
            if (fase_p == 0)
                glColor3f(1.0f, 0.95f, 0.05f);  /* dorado vivo */
            else
                glColor3f(1.0f, 1.0f, 0.80f);   /* blanco-amarillo */
            glPushMatrix();
            glTranslatef(0.0f, y_centro_marquesina + 0.05f, z_frente_marquesina);
            glScalef(0.0025f, 0.0025f, 1.0f);
            dibujar_texto_stroke_simple("WIN!", 1.0f);
            glPopMatrix();
        } else {
            glColor3f(1.0f, 0.85f, 0.2f);
            glPushMatrix();
            glTranslatef(0.0f, y_centro_marquesina, z_frente_marquesina);
            glScalef(0.0016f, 0.0016f, 1.0f);
            dibujar_texto_stroke_simple("TRAGAMONEDAS", 1.0f);
            glPopMatrix();
        }
    }

    /* Foquitos animados en chase: cada uno se enciende en secuencia
       a 6 ciclos/s -efecto clasico de las luces de marquesina.
       Cuando hay ganancia y el timer de parada es menor a 5s, alterna
       entre rojo y dorado a ritmo rapido para celebrar el premio. */
    {
        float tiempo_ms = (float)glutGet(GLUT_ELAPSED_TIME);
        float t_seg     = tiempo_ms / 1000.0f;
        int   hay_win   = g_estado_tragamonedas.hay_ganancia &&
                          g_estado_tragamonedas.tiempo_desde_parada < 5.0f;
        /* Fase de la ola de luces (0..NUM_FOQUITOS_POR_BORDE-1) */
        int   fase_ola  = (int)(t_seg * 6.0f) % NUM_FOQUITOS_POR_BORDE;
        /* Fase de parpadeo dorado/rojo en victoria (2 Hz) */
        int   fase_win  = (int)(t_seg * 4.0f) % 2;

        for (i = 0; i < NUM_FOQUITOS_POR_BORDE; i++) {
            float x_foquito;
            int encendido;
            float t_pos = (float)i / (float)(NUM_FOQUITOS_POR_BORDE - 1);

            x_foquito  = -ancho_marquesina / 2.0f * 0.88f + t_pos * (ancho_marquesina * 0.88f);

            /* Una "ola" de 2 luces contiguas recorre la fila */
            encendido = (i == fase_ola ||
                         i == (fase_ola + 1) % NUM_FOQUITOS_POR_BORDE);

            if (hay_win) {
                /* Victoria: todos encendidos, alternando rojo/dorado */
                if (fase_win == 0)
                    glColor3f(1.0f, 0.15f, 0.05f);   /* rojo brillante */
                else
                    glColor3f(1.0f, 0.90f, 0.10f);   /* dorado */
            } else if (encendido) {
                glColor3f(1.0f, 0.95f, 0.20f);       /* dorado vivo */
            } else {
                glColor3f(0.45f, 0.38f, 0.10f);      /* apagado */
            }

            glPushMatrix();
            glTranslatef(x_foquito,
                         y_centro_marquesina + GABINETE_ALTURA_MARQUESINA / 2.0f - 0.08f,
                         z_frente_marquesina);
            glutSolidSphere(0.028, 8, 8);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(x_foquito,
                         y_centro_marquesina - GABINETE_ALTURA_MARQUESINA / 2.0f + 0.08f,
                         z_frente_marquesina);
            glutSolidSphere(0.028, 8, 8);
            glPopMatrix();
        }
    }
    glEnable(GL_LIGHTING);
}

static void dibujar_gabinete(void) {
    dibujar_patas();
    dibujar_base();
    dibujar_panel_inferior();
    dibujar_franja_dorada(GABINETE_Y_INICIO_CUERPO, GABINETE_ANCHO * 1.08f, GABINETE_PROFUNDIDAD * 1.08f);
    dibujar_cuerpo_principal();
    dibujar_franja_dorada(GABINETE_Y_INICIO_MARQUESINA, GABINETE_ANCHO, GABINETE_PROFUNDIDAD);
    dibujar_marquesina();
}


/* ------------------------------------------------------------------- */
/* Rodillos (PASO 2)                                                    */
/* ------------------------------------------------------------------- */

static void dibujar_simbolo(SimboloTragamonedas simbolo) {
    GLboolean iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    switch (simbolo) {

    case SIMBOLO_CEREZA:
        glColor3f(0.75f, 0.05f, 0.08f);
        glPushMatrix();
        glTranslatef(-0.10f, -0.14f, 0.0f);
        glutSolidSphere(0.09, 12, 12);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.10f, -0.14f, 0.0f);
        glutSolidSphere(0.09, 12, 12);
        glPopMatrix();

        glColor3f(0.15f, 0.45f, 0.15f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex3f(-0.10f, -0.06f, 0.01f); glVertex3f(0.0f, 0.20f, 0.01f);
        glVertex3f(0.10f, -0.06f, 0.01f);  glVertex3f(0.0f, 0.20f, 0.01f);
        glEnd();

        glPushMatrix();
        glTranslatef(0.06f, 0.20f, 0.0f);
        glScalef(0.10f, 0.045f, 0.02f);
        glutSolidSphere(1.0, 8, 8);
        glPopMatrix();
        break;

    case SIMBOLO_CAMPANA:
        glColor3f(0.85f, 0.68f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, -0.13f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.16, 0.30, 16, 8);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -0.13f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(0.018, 0.165, 8, 20);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 0.17f, 0.0f);
        glutSolidSphere(0.03, 8, 8);
        glPopMatrix();

        glColor3f(0.55f, 0.45f, 0.10f);
        glPushMatrix();
        glTranslatef(0.0f, -0.22f, 0.0f);
        glutSolidSphere(0.035, 8, 8);
        glPopMatrix();
        break;

    case SIMBOLO_HERRADURA:
        glColor3f(0.75f, 0.75f, 0.78f);
        glutSolidTorus(0.055, 0.20, 10, 20);
        break;

    case SIMBOLO_DIAMANTE:
        glColor3f(0.35f, 0.75f, 0.90f);
        glPushMatrix();
        glScalef(0.17f, 0.24f, 0.17f);
        glutSolidOctahedron();
        glPopMatrix();
        break;

    case SIMBOLO_BARRA:
        glColor3f(0.05f, 0.05f, 0.05f);
        glPushMatrix();
        dibujar_caja(0.46f, 0.20f, 0.04f);
        glPopMatrix();

        glColor3f(0.85f, 0.68f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.03f);
        glScalef(0.0013f, 0.0013f, 1.0f);
        glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
        dibujar_texto_stroke_simple("BAR", 1.0f);
        glPopMatrix();
        break;

    case SIMBOLO_SIETE:
        glColor3f(0.8f, 0.1f, 0.1f);
        glPushMatrix();
        glScalef(0.0035f, 0.0035f, 1.0f);
        glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
        dibujar_texto_stroke_simple("7", 1.0f);
        glPopMatrix();
        break;

    default:
        break;
    }

    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}


/* Separadores finos entre las 3 filas de simbolos de UN rodillo */
static void dibujar_separadores_de_fila(float x_centro, float z_rodillo) {
    aplicar_material_dorado();

    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO + ALTURA_UNIDAD_SIMBOLO / 2.0f, z_rodillo);
    dibujar_caja(VENTANA_ANCHO_INTERNO / 3.0f - GROSOR_DIVISOR_RODILLO, 0.02f, 0.05f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO - ALTURA_UNIDAD_SIMBOLO / 2.0f, z_rodillo);
    dibujar_caja(VENTANA_ANCHO_INTERNO / 3.0f - GROSOR_DIVISOR_RODILLO, 0.02f, 0.05f);
    glPopMatrix();
}


static void dibujar_rodillo(int indice, const EstadoRodillo* rodillo, float x_centro, float z_rodillo) {
    int offset;

    /* Fondo negro-terciopelo del rodillo: maximo contraste con los
       simbolos dorados/rojos, aspecto de casino premium. */
    aplicar_material_fondo_rodillo();
    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO, z_rodillo);
    dibujar_caja(VENTANA_ANCHO_INTERNO / 3.0f - GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, 0.04f);
    glPopMatrix();

    dibujar_separadores_de_fila(x_centro, z_rodillo);

    for (offset = -1; offset <= 1; offset++) {
        float posicion_consultada;
        SimboloTragamonedas simbolo;
        float y_simbolo;

        posicion_consultada = rodillo->posicion_actual + (float)offset;
        simbolo = obtener_simbolo_en_posicion(indice, posicion_consultada);
        y_simbolo = VENTANA_Y_CENTRO - (float)offset * ALTURA_UNIDAD_SIMBOLO;

        glPushMatrix();
        glTranslatef(x_centro, y_simbolo, z_rodillo + 0.03f);
        glScalef(1.2f, 1.2f, 1.2f);
        dibujar_simbolo(simbolo);
        glPopMatrix();
    }
}


static void dibujar_divisores_rodillos(float x_izquierdo, float x_derecho) {
    aplicar_material_dorado();

    glPushMatrix();
    glTranslatef(x_izquierdo, VENTANA_Y_CENTRO, 0.0f);
    dibujar_caja(GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, VENTANA_PROFUNDIDAD_INTERNA);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_derecho, VENTANA_Y_CENTRO, 0.0f);
    dibujar_caja(GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, VENTANA_PROFUNDIDAD_INTERNA);
    glPopMatrix();
}


/* Anillo de foquitos alrededor del marco de la ventana de rodillos.
   Efecto chase igual al de la marquesina pero mas rapido; en victoria
   todos parpadean alternando rojo/dorado. Es el detalle mas
   caracteristico de una tragamonedas fisica real. */
static void dibujar_luces_marco_rodillos(void) {
    float t_seg    = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    int   hay_win  = g_estado_tragamonedas.hay_ganancia &&
                     g_estado_tragamonedas.tiempo_desde_parada < 5.0f;
    int   fase_win = (int)(t_seg * 5.0f) % 2;

    /* Borde superior e inferior (horizontal) */
    const int N_H = 8;
    /* Bordes izquierdo y derecho (vertical) */
    const int N_V = 12;
    const int TOTAL = 2 * N_H + 2 * N_V;
    int fase = (int)(t_seg * 10.0f) % TOTAL;

    float x_izq  = -VENTANA_ANCHO_INTERNO / 2.0f - GROSOR_MARCO_VENTANA * 0.6f;
    float x_der  =  VENTANA_ANCHO_INTERNO / 2.0f + GROSOR_MARCO_VENTANA * 0.6f;
    float y_top  = GABINETE_Y_INICIO_CUERPO + VENTANA_ALTO_INTERNO + GROSOR_MARCO_VENTANA * 0.75f;
    float y_bot  = GABINETE_Y_INICIO_CUERPO + GROSOR_MARCO_VENTANA * 0.25f;
    float z_luz  = GABINETE_PROFUNDIDAD / 2.0f + 0.04f;
    float r_luz  = 0.022f;
    int   k, idx;

    glDisable(GL_LIGHTING);

    /* Franja superior */
    for (k = 0; k < N_H; k++) {
        float t   = (float)k / (float)(N_H - 1);
        float x   = x_izq + t * (x_der - x_izq);
        idx = k;
        if (hay_win) {
            glColor3f(fase_win == 0 ? 1.0f : 1.0f,
                      fase_win == 0 ? 0.10f : 0.85f,
                      0.05f);
        } else {
            int on = (idx % TOTAL == fase || (idx+1) % TOTAL == fase);
            glColor3f(on ? 1.0f : 0.35f, on ? 0.88f : 0.28f, on ? 0.15f : 0.06f);
        }
        glPushMatrix();
        glTranslatef(x, y_top, z_luz);
        glutSolidSphere(r_luz, 6, 6);
        glPopMatrix();
    }

    /* Franja inferior */
    for (k = 0; k < N_H; k++) {
        float t   = (float)k / (float)(N_H - 1);
        float x   = x_izq + t * (x_der - x_izq);
        idx = N_H + k;
        if (hay_win) {
            glColor3f(1.0f, fase_win == 0 ? 0.10f : 0.85f, 0.05f);
        } else {
            int on = (idx % TOTAL == fase || (idx+1) % TOTAL == fase);
            glColor3f(on ? 1.0f : 0.35f, on ? 0.88f : 0.28f, on ? 0.15f : 0.06f);
        }
        glPushMatrix();
        glTranslatef(x, y_bot, z_luz);
        glutSolidSphere(r_luz, 6, 6);
        glPopMatrix();
    }

    /* Borde izquierdo */
    for (k = 0; k < N_V; k++) {
        float t = (float)k / (float)(N_V - 1);
        float y = y_bot + t * (y_top - y_bot);
        idx = 2 * N_H + k;
        if (hay_win) {
            glColor3f(1.0f, fase_win == 0 ? 0.10f : 0.85f, 0.05f);
        } else {
            int on = (idx % TOTAL == fase || (idx+1) % TOTAL == fase);
            glColor3f(on ? 1.0f : 0.35f, on ? 0.88f : 0.28f, on ? 0.15f : 0.06f);
        }
        glPushMatrix();
        glTranslatef(x_izq, y, z_luz);
        glutSolidSphere(r_luz, 6, 6);
        glPopMatrix();
    }

    /* Borde derecho */
    for (k = 0; k < N_V; k++) {
        float t = (float)k / (float)(N_V - 1);
        float y = y_bot + t * (y_top - y_bot);
        idx = 2 * N_H + N_V + k;
        if (hay_win) {
            glColor3f(1.0f, fase_win == 0 ? 0.10f : 0.85f, 0.05f);
        } else {
            int on = (idx % TOTAL == fase || (idx+1) % TOTAL == fase);
            glColor3f(on ? 1.0f : 0.35f, on ? 0.88f : 0.28f, on ? 0.15f : 0.06f);
        }
        glPushMatrix();
        glTranslatef(x_der, y, z_luz);
        glutSolidSphere(r_luz, 6, 6);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}


static void dibujar_rodillos(const EstadoTragamonedas* estado) {
    const float ancho_rodillo = VENTANA_ANCHO_INTERNO / 3.0f;
    const float z_rodillo = 0.15f;
    int i;

    for (i = 0; i < NUM_RODILLOS; i++) {
        float x_centro = -VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo / 2.0f + (float)i * ancho_rodillo;
        dibujar_rodillo(i, &estado->rodillos[i], x_centro, z_rodillo);
    }

    dibujar_divisores_rodillos(-VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo,
        -VENTANA_ANCHO_INTERNO / 2.0f + 2.0f * ancho_rodillo);

    dibujar_luces_marco_rodillos();
}


/* ------------------------------------------------------------------- */
/* Palanca (PASO 3)                                                     */
/* ------------------------------------------------------------------- */

static void dibujar_palanca(float angulo_extra_grados) {
    float x_lateral;
    float x_pivote;
    float y_pivote;
    GLUquadric* quad;

    x_lateral = GABINETE_ANCHO / 2.0f;
    x_pivote = x_lateral + PALANCA_EXTENSION_SOPORTE;
    y_pivote = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * PALANCA_Y_PIVOTE_FACTOR;

    /* Soporte y brazo: dorados (antes cromados), perilla roja -misma
       paleta que el resto del gabinete */
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(x_lateral + PALANCA_EXTENSION_SOPORTE / 2.0f, y_pivote, 0.0f);
    dibujar_caja(PALANCA_EXTENSION_SOPORTE, 0.17f, 0.24f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_pivote, y_pivote, 0.0f);
    glScalef(0.075f, 0.075f, 0.045f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_pivote, y_pivote, 0.0f);
    glRotatef(PALANCA_ANGULO_INCLINACION + angulo_extra_grados, 0.0f, 0.0f, 1.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    aplicar_material_dorado();
    quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, PALANCA_RADIO_BRAZO, PALANCA_RADIO_BRAZO_PUNTA, PALANCA_LONGITUD_BRAZO, 12, 4);
    gluDeleteQuadric(quad);

    glTranslatef(0.0f, 0.0f, PALANCA_LONGITUD_BRAZO);
    aplicar_material_acento_rojo();
    glutSolidSphere(PALANCA_RADIO_PERILLA, 16, 16);
    glPopMatrix();
}


/* Inicializacion perezosa (una sola vez) del estado interno */
static void asegurar_estado_inicializado(void) {
    if (!g_estado_tragamonedas_listo) {
        inicializar_tragamonedas_animacion(&g_estado_tragamonedas);
        g_estado_tragamonedas_listo = 1;
    }
}


EstadoTragamonedas* obtener_estado_tragamonedas_para_pruebas(void) {
    asegurar_estado_inicializado();
    return &g_estado_tragamonedas;
}


/* Duracion y amplitud de la animacion visual de "tirar la palanca"
   -no es una animacion de fisica real, es un gesto corto (bajar y
   volver) que arranca apenas el rodillo 0 empieza a girar. Se calcula
   en base a tiempo_transcurrido de ese rodillo, que ya vive en
   g_estado_tragamonedas -no hace falta ningun campo ni cambio en
   tragamonedas_animacion.c para esto. */
#define DURACION_TIRON_PALANCA 0.4f
#define AMPLITUD_TIRON_PALANCA 45.0f

   /* Devuelve el angulo extra (en grados) que hay que sumarle a la
      inclinacion de reposo de la palanca en este instante: 0 si no hay
      ningun giro en marcha (o el tiron ya termino), y una curva tipo seno
      -baja rapido y vuelve- durante los primeros DURACION_TIRON_PALANCA
      segundos del giro. */
static float calcular_angulo_tiron_palanca(void) {
    const double PI_LOCAL = 3.14159265358979323846;
    float t;

    if (!g_estado_tragamonedas_listo) return 0.0f;

    t = g_estado_tragamonedas.rodillos[0].tiempo_transcurrido;
    if (t <= 0.0f || t >= DURACION_TIRON_PALANCA) return 0.0f;

    return -AMPLITUD_TIRON_PALANCA * (float)sin(PI_LOCAL * (double)(t / DURACION_TIRON_PALANCA));
}


/* ------------------------------------------------------------------- */
/* PASO 5: Indicador de linea de pago, display de creditos              */
/* ------------------------------------------------------------------- */

/* Linea de pago central: una raya dorada y dos flechas que apuntan
   al rodillo central (la fila ganadora). Parpadea durante 5s si hay
   premio, para guiar la vista del jugador al resultado. */
static void dibujar_linea_pago(void) {
    float z_frente   = GABINETE_PROFUNDIDAD / 2.0f + 0.09f;
    float x_izq      = -VENTANA_ANCHO_INTERNO / 2.0f - 0.14f;
    float x_der      =  VENTANA_ANCHO_INTERNO / 2.0f + 0.14f;
    float y          = VENTANA_Y_CENTRO;
    float r, g, b;

    /* Calcular color: parpadeo rapido si hay ganancia reciente */
    if (g_estado_tragamonedas.hay_ganancia &&
        g_estado_tragamonedas.tiempo_desde_parada < 5.0f) {
        float t_s    = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float pulso  = 0.5f + 0.5f * (float)sinf(t_s * 10.0f);
        r = 1.0f;
        g = 0.7f + pulso * 0.3f;
        b = 0.0f;
    } else {
        /* Dorado tenue en reposo */
        r = 0.85f; g = 0.70f; b = 0.10f;
    }

    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glColor3f(r, g, b);

    /* Linea horizontal a lo ancho de la ventana */
    glBegin(GL_LINES);
    glVertex3f(x_izq, y, z_frente);
    glVertex3f(x_der, y, z_frente);
    glEnd();

    /* Flecha izquierda: triangulo apuntando a la derecha */
    glBegin(GL_TRIANGLES);
    glVertex3f(x_izq + 0.11f, y,          z_frente);
    glVertex3f(x_izq,         y + 0.055f, z_frente);
    glVertex3f(x_izq,         y - 0.055f, z_frente);
    glEnd();

    /* Flecha derecha: triangulo apuntando a la izquierda */
    glBegin(GL_TRIANGLES);
    glVertex3f(x_der - 0.11f, y,          z_frente);
    glVertex3f(x_der,         y + 0.055f, z_frente);
    glVertex3f(x_der,         y - 0.055f, z_frente);
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}


/* Display de creditos: panel digital en el frente de la base que
   muestra el monto apostado y el ultimo resultado (PREMIO o PERDIDA).
   Usa texto stroke escalado, sobre un fondo oscuro con marco dorado. */
static void dibujar_display_creditos(void) {
    float z_frente    = (GABINETE_PROFUNDIDAD * 1.08f) / 2.0f + 0.015f;
    float ancho_disp  = GABINETE_ANCHO * 0.55f;
    float alto_disp   = 0.15f;
    float y_disp      = GABINETE_ALTURA_BASE * 0.80f; /* justo encima del panel de botones */
    char  buf_apuesta[32];
    char  buf_premio[32];
    float ganancia    = g_estado_tragamonedas.ganancia_ultima;
    float apuesta     = g_estado_tragamonedas.monto_apuesta;

    /* Formato de textos */
    sprintf_s(buf_apuesta, sizeof(buf_apuesta), "BET:%.0f", apuesta);
    if (g_estado_tragamonedas.todos_detenidos && g_estado_tragamonedas.hay_ganancia)
        sprintf_s(buf_premio, sizeof(buf_premio), "WIN:%.0f", ganancia);
    else if (g_estado_tragamonedas.todos_detenidos && !g_estado_tragamonedas.hay_ganancia &&
             g_estado_tragamonedas.tiempo_desde_parada < 4.0f && ganancia < 0.0f)
        sprintf_s(buf_premio, sizeof(buf_premio), "LOSE");
    else
        sprintf_s(buf_premio, sizeof(buf_premio), "---");

    /* Marco dorado */
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_disp, z_frente - 0.01f);
    dibujar_caja_biselada(ancho_disp + 0.05f, alto_disp + 0.04f, 0.04f, BISEL_FRANJA);
    glPopMatrix();

    /* Panel oscuro (casi negro) */
    {
        GLfloat amb[4]  = { 0.02f, 0.02f, 0.02f, 1.0f };
        GLfloat dif[4]  = { 0.05f, 0.05f, 0.05f, 1.0f };
        GLfloat spe[4]  = { 0.10f, 0.10f, 0.10f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT,   amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  spe);
        glMaterialf (GL_FRONT, GL_SHININESS, 10.0f);
    }
    glPushMatrix();
    glTranslatef(0.0f, y_disp, z_frente + 0.01f);
    dibujar_caja(ancho_disp, alto_disp, 0.03f);
    glPopMatrix();

    /* Texto: BET a la izquierda, WIN/LOSE a la derecha */
    glDisable(GL_LIGHTING);

    /* BET: verde digital */
    glColor3f(0.10f, 0.95f, 0.25f);
    glPushMatrix();
    glTranslatef(-ancho_disp * 0.22f, y_disp, z_frente + 0.03f);
    glScalef(0.00100f, 0.00100f, 1.0f);
    glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
    dibujar_texto_stroke_simple(buf_apuesta, 1.0f);
    glPopMatrix();

    /* WIN/LOSE: dorado si gana, rojo si pierde */
    if (g_estado_tragamonedas.hay_ganancia)
        glColor3f(1.0f, 0.90f, 0.05f);   /* dorado */
    else
        glColor3f(0.95f, 0.15f, 0.10f);  /* rojo */

    glPushMatrix();
    glTranslatef(ancho_disp * 0.22f, y_disp, z_frente + 0.03f);
    glScalef(0.00100f, 0.00100f, 1.0f);
    glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
    dibujar_texto_stroke_simple(buf_premio, 1.0f);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}


void dibujar_tragamonedas(void) {
    asegurar_estado_inicializado();

    dibujar_gabinete();
    dibujar_rodillos(&g_estado_tragamonedas);
    dibujar_palanca(calcular_angulo_tiron_palanca());
    dibujar_linea_pago();
    dibujar_display_creditos();
}

