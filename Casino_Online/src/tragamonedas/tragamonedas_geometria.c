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
#include "simbolos_tex.h"          /* texturas PNG para los simbolos */
#include "../render/materiales.h"

 /* Alto aproximado (en unidades de fuente, antes de escalar) de una
    mayuscula en GLUT_STROKE_ROMAN -se usa para centrar verticalmente el
    texto, ya que glutStrokeCharacter() dibuja desde la linea de base
    hacia arriba, no centrado. */
#define ALTURA_APROX_MAYUSCULA_STROKE 119.05f

    /* Grosor de los paneles laterales que enmarcan la ventana frontal del
       cuerpo -el paso 2 (rodillos) la necesita para saber cuanto espacio
       libre queda adentro. */
#define GROSOR_MARCO_VENTANA 0.20f

       /* Alto de las franjas decorativas doradas entre secciones del gabinete */
#define ALTO_FRANJA 0.06f

/* REDISENO: en el mueble antiguo de referencia la ventana de los
   rodillos es una franja angosta pegada arriba del cuerpo -no ocupa
   todo el cuerpo como en la version "casino moderno" anterior-, dejando
   el resto del cuerpo libre para el panel acanalado de abajo. Por eso
   la ventana ahora solo ocupa FRACCION_ALTO_VENTANA del cuerpo (menos
   el marco superior) y queda pegada al techo del cuerpo, no centrada. */
#define FRACCION_ALTO_VENTANA 0.56f

/* Dimensiones internas de la ventana donde van los rodillos, derivadas
   de las constantes publicas del gabinete (ver tragamonedas_geometria.h) */
#define VENTANA_ANCHO_INTERNO       (GABINETE_ANCHO - 2.0f * GROSOR_MARCO_VENTANA)
#define VENTANA_ALTO_INTERNO        ((GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA) * FRACCION_ALTO_VENTANA)
#define VENTANA_PROFUNDIDAD_INTERNA (GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA)
#define VENTANA_Y_CENTRO            (GABINETE_Y_INICIO_CUERPO + (GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA) - VENTANA_ALTO_INTERNO / 2.0f)

   /* Grosor de los separadores finos entre rodillos */
#define GROSOR_DIVISOR_RODILLO 0.04f

/* Constantes de la palanca -brazo cilindrico inclinado con una perilla
   en la punta, montado en un soporte al costado derecho del gabinete.
   REDISENO: en el boceto de referencia la palanca es un varilla simple
   y corta, montada arriba (cerca de la marquesina), no un brazo largo
   que baja hasta la mitad del cuerpo -se acorta y se sube el pivote. */
#define PALANCA_EXTENSION_SOPORTE      0.11f
#define PALANCA_RADIO_BRAZO            0.038f
#define PALANCA_RADIO_BRAZO_PUNTA      0.026f
#define PALANCA_LONGITUD_BRAZO         0.78f
#define PALANCA_RADIO_PERILLA          0.095f
#define PALANCA_ANGULO_INCLINACION     50.0f
#define PALANCA_Y_PIVOTE_FACTOR        0.94f
/* Cuanto se mete el soporte DENTRO de la columna del gabinete (no solo
   tocando el borde) -un contacto al ras entre el metal claro del
   soporte y el negro del cuerpo se lee como un hueco por el contraste,
   aunque geometricamente toquen; superponer de verdad lo resuelve. */
#define PALANCA_SOLAPE_COLUMNA         0.09f

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
    float ancho_total = 0.0f;

    /* BUGFIX (texto no centrado, ej. "JACKPOT!"): esto asumia un ancho
       FIJO de 104.76 por caracter (largo*52.38 para centrar), pero
       GLUT_STROKE_ROMAN es una fuente PROPORCIONAL -cada letra tiene su
       propio ancho real ('!' es mucho mas angosto que 'M', por ejemplo-,
       asi que ese promedio fijo dejaba el texto visiblemente corrido
       para cualquier palabra cuyo ancho real no coincidiera con el
       promedio. Se centra de verdad sumando el ancho REAL de cada
       caracter con glutStrokeWidth(). */
    for (i = 0; i < len; i++) {
        ancho_total += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, texto[i]);
    }

    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-ancho_total / 2.0f, 0.0f, 0.0f);
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


/* Bisel estandar para los paneles grandes del gabinete. Antes era 0.05
   -muy fino a la distancia de camara real-, lo que hacia que el mueble
   se leyera como una caja de canto vivo (aspecto "carton") en vez de
   una pieza moldeada/fabricada. Se sube a 0.09 para que el redondeo se
   note de verdad en el render final. */
#define BISEL_PANEL_GRANDE 0.09f
#define BISEL_FRANJA        0.015f


   /* Material del cuerpo: negro carbon brillante (como las maquinas
      de casino modernas de la referencia -no madera, sino plastico
      laqueado negro de alta gama). */
/* REDISENO (referencia: maquina antigua toda de oro/cromo, sin cuerpo
   negro): el "material de cuerpo" pasa de negro carbon a dorado/laton,
   un poco menos brillante que aplicar_material_dorado() (que se sigue
   usando para molduras/detalles), asi el mueble entero se lee dorado
   pero los bordes/molduras siguen resaltando por encima. */
static void aplicar_material_cuerpo(void) {
    GLfloat ambient[4] = { 0.24f, 0.16f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.62f, 0.43f, 0.07f, 1.0f };
    GLfloat specular[4]= { 0.95f, 0.85f, 0.55f, 1.0f };
    GLfloat shininess  = 118.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Material del acento rojo -deep red para paneles internos. */
static void aplicar_material_acento_rojo(void) {
    GLfloat ambient[4] = { 0.22f, 0.02f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.55f, 0.04f, 0.04f, 1.0f };
    GLfloat specular[4]= { 0.40f, 0.10f, 0.10f, 1.0f };
    GLfloat shininess  = 60.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Material dorado (marquesina, letras, detalles). */
static void aplicar_material_dorado(void) {
    GLfloat ambient[4] = { 0.28f, 0.20f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.75f, 0.55f, 0.05f, 1.0f };
    GLfloat specular[4]= { 1.00f, 0.95f, 0.75f, 1.0f };
    GLfloat shininess  = 128.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT, GL_SHININESS,shininess);
}


/* Fondo de los rodillos: negro muy puro para maximo contraste. */
static void aplicar_material_fondo_rodillo(void) {
    GLfloat ambient[4] = { 0.01f, 0.01f, 0.01f, 1.0f };
    GLfloat diffuse[4] = { 0.03f, 0.03f, 0.03f, 1.0f };
    GLfloat specular[4]= { 0.08f, 0.08f, 0.08f, 1.0f };
    GLfloat shininess  = 15.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT, GL_SHININESS,shininess);
}


/* Material rojo brillante para la perilla. */
static void aplicar_material_rojo_brillante(void) {
    GLfloat ambient[4]  = { 0.35f, 0.02f, 0.02f, 1.0f };
    GLfloat diffuse[4]  = { 0.85f, 0.05f, 0.05f, 1.0f };
    GLfloat specular[4] = { 1.00f, 0.50f, 0.50f, 1.0f };
    GLfloat shininess   = 128.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT, GL_SHININESS,shininess);
}


/* ------------------------------------------------------------------- */
/* Gabinete (PASO 1)                                                    */
/* ------------------------------------------------------------------- */

/* Base simple con un cajon/ranura chico embutido en el frente, cerca
   del piso -como en el boceto de referencia (nada de la doble senal
   "JACKPOT!" ni los botones redondos de la version "casino moderno"
   anterior, que no aparecen en el mueble antiguo). */
static void dibujar_base(void) {
    const float FACTOR_ANCHO_BASE = 1.05f;
    const float FACTOR_PROFUNDIDAD_BASE = 1.05f;
    float z_frente_base;
    float ancho_cajon;
    float alto_cajon;
    float y_cajon;

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_ALTURA_BASE / 2.0f, 0.0f);
    dibujar_caja_biselada(GABINETE_ANCHO * FACTOR_ANCHO_BASE, GABINETE_ALTURA_BASE, GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_BASE, BISEL_PANEL_GRANDE);
    glPopMatrix();

    /* Cajon/ranura chico -inset oscuro con marco dorado fino */
    z_frente_base = (GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_BASE) / 2.0f;
    ancho_cajon = GABINETE_ANCHO * 0.30f;
    alto_cajon  = GABINETE_ALTURA_BASE * 0.30f;
    y_cajon     = GABINETE_ALTURA_BASE * 0.28f;

    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_cajon, z_frente_base + 0.006f);
    dibujar_caja_biselada(ancho_cajon + 0.03f, alto_cajon + 0.025f, 0.025f, 0.006f);
    glPopMatrix();

    {
        GLfloat am[4] = { 0.01f, 0.01f, 0.01f, 1.0f };
        GLfloat di[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
        GLfloat sp[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT,   am);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   di);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  sp);
        glMaterialf (GL_FRONT, GL_SHININESS, 8.0f);
    }
    glPushMatrix();
    glTranslatef(0.0f, y_cajon, z_frente_base + 0.014f);
    dibujar_caja(ancho_cajon, alto_cajon, 0.018f);
    glPopMatrix();

    /* Monedas sueltas adentro del cajon -antes quedaba como un
       rectangulo negro vacio sin proposito visual; unas monedas
       doradas chatas le dan contenido, coherente con la bandeja de
       una maquina real. */
    {
        GLfloat am[4] = { 0.20f, 0.14f, 0.02f, 1.0f };
        GLfloat di[4] = { 0.55f, 0.40f, 0.08f, 1.0f };
        GLfloat sp[4] = { 0.85f, 0.68f, 0.25f, 1.0f };
        float coin_r = alto_cajon * 0.28f;
        int ci;
        const float cx_off[3] = { -0.55f, 0.05f, 0.60f };
        const float cy_off[3] = { -0.15f, 0.25f, -0.05f };

        glMaterialfv(GL_FRONT, GL_AMBIENT,  am);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  di);
        glMaterialfv(GL_FRONT, GL_SPECULAR, sp);
        glMaterialf (GL_FRONT, GL_SHININESS, 90.0f);
        for (ci = 0; ci < 3; ci++) {
            glPushMatrix();
            glTranslatef(cx_off[ci] * ancho_cajon * 0.35f,
                         y_cajon + cy_off[ci] * alto_cajon * 0.5f,
                         z_frente_base + 0.017f);
            glScalef(1.0f, 1.0f, 0.30f);
            glutSolidSphere(coin_r, 14, 14);
            glPopMatrix();
        }
    }

    /* Ranura de monedas: hendidura angosta cerca de arriba de la base,
       corrida hacia el costado derecho -como en la maquina de
       referencia toda dorada. */
    {
        float x_ranura = GABINETE_ANCHO * 0.20f;
        float y_ranura = GABINETE_ALTURA_BASE * 0.78f;

        aplicar_material_dorado();
        glPushMatrix();
        glTranslatef(x_ranura, y_ranura, z_frente_base + 0.006f);
        dibujar_caja_biselada(0.16f, 0.045f, 0.02f, 0.008f);
        glPopMatrix();

        {
            GLfloat am[4] = { 0.01f, 0.01f, 0.01f, 1.0f };
            GLfloat di[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
            GLfloat sp[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
            glMaterialfv(GL_FRONT, GL_AMBIENT,   am);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,   di);
            glMaterialfv(GL_FRONT, GL_SPECULAR,  sp);
            glMaterialf (GL_FRONT, GL_SHININESS, 8.0f);
        }
        glPushMatrix();
        glTranslatef(x_ranura, y_ranura, z_frente_base + 0.012f);
        dibujar_caja(0.11f, 0.018f, 0.015f);
        glPopMatrix();
    }
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

/* Panel acanalado: franjas verticales delgadas que simulan el frente de
   metal fundido con relieve del boceto de referencia (maquina antigua
   tipo Mills/Jennings) -en vez de un panel liso, el relieve lo dan las
   barras protuberantes (mas claras) sobre el fondo negro, con la sombra
   entre barras haciendo el resto. Nada de textura importada: solo
   primitivas propias, mismo criterio que el resto del gabinete. */
static void dibujar_panel_acanalado(float ancho, float alto, float y_centro, float z_frente) {
    const int NUM_BARRAS = 14;
    const float profundo = 0.05f;
    float ancho_barra;
    float x0;
    int i;

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, y_centro, z_frente - profundo / 2.0f);
    dibujar_caja_biselada(ancho, alto, profundo, BISEL_FRANJA);
    glPopMatrix();

    ancho_barra = ancho / (float)NUM_BARRAS;
    x0 = -ancho / 2.0f + ancho_barra / 2.0f;
    /* Barras en tono dorado -antes eran gris frio, lo que desentonaba
       con el resto del mueble (ya todo dorado). */
    {
        GLfloat amb[4] = { 0.16f, 0.11f, 0.02f, 1.0f };
        GLfloat dif[4] = { 0.42f, 0.30f, 0.06f, 1.0f };
        GLfloat spe[4] = { 0.75f, 0.62f, 0.28f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spe);
        glMaterialf (GL_FRONT, GL_SHININESS, 85.0f);
    }
    for (i = 0; i < NUM_BARRAS; i++) {
        float x = x0 + (float)i * ancho_barra;
        glPushMatrix();
        glTranslatef(x, y_centro, z_frente + 0.006f);
        dibujar_caja(ancho_barra * 0.55f, alto * 0.94f, 0.012f);
        glPopMatrix();
    }
}


static void dibujar_cuerpo_principal(void) {
    float ancho_columna;
    float profundidad_util;
    float x_columna;

    ancho_columna    = GROSOR_MARCO_VENTANA;
    profundidad_util = GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA;
    x_columna        = (GABINETE_ANCHO - ancho_columna) / 2.0f;

    /* Columnas negras a los lados */
    aplicar_material_cuerpo();

    glPushMatrix();
    glTranslatef(-x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja_biselada(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD, BISEL_PANEL_GRANDE);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja_biselada(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD, BISEL_PANEL_GRANDE);
    glPopMatrix();

    /* Motivo decorativo (rombos grabados) en ambas columnas -mas
       detalle de superficie, como el relieve tallado de la maquina de
       referencia dorada, en vez de columnas completamente lisas. */
    {
        GLfloat amb[4] = { 0.14f, 0.09f, 0.01f, 1.0f };
        GLfloat dif[4] = { 0.35f, 0.23f, 0.03f, 1.0f };
        GLfloat spe[4] = { 0.55f, 0.42f, 0.15f, 1.0f };
        float y0 = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * 0.30f;
        float y1 = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * 0.70f;
        float z_rombo = GABINETE_PROFUNDIDAD / 2.0f + 0.003f;
        int sx;

        glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spe);
        glMaterialf (GL_FRONT, GL_SHININESS, 60.0f);
        for (sx = -1; sx <= 1; sx += 2) {
            glPushMatrix();
            glTranslatef((float)sx * x_columna, y0, z_rombo);
            dibujar_rombo_decorativo(ancho_columna * 0.32f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef((float)sx * x_columna, y1, z_rombo);
            dibujar_rombo_decorativo(ancho_columna * 0.32f);
            glPopMatrix();
        }
    }

    /* Panel superior del cuerpo (negro, biselado) */
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA / 2.0f, 0.0f);
    dibujar_caja_biselada(GABINETE_ANCHO, GROSOR_MARCO_VENTANA, GABINETE_PROFUNDIDAD, BISEL_FRANJA * 2.0f);
    glPopMatrix();

    /* Panel trasero */
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, -profundidad_util / 2.0f);
    dibujar_caja(GABINETE_ANCHO - 2.0f * ancho_columna, GABINETE_ALTURA_CUERPO, GROSOR_MARCO_VENTANA);
    glPopMatrix();

    /* (Se saco la tira de neon rojo continua que antes recorria las
       columnas de punta a punta -el mueble ya tiene suficiente acento
       con el dorado del marco/franjas/monedero; una linea de color
       recorriendo todo el costado se leia mas como un contorno dibujado
       encima que como una luz montada en el gabinete real.) */

    /* Repisa angosta debajo de la ventana + boton SPIN grande (con dos
       botones redondos chicos a los costados) + panel acanalado que
       llena el resto del cuerpo -estilo control de tragamonedas
       moderno (boton central grande rojo con aro dorado, acompanado
       de dos redondos chicos), no los botones cuadrados chatos de
       antes. */
    {
        float y_ventana_inferior = VENTANA_Y_CENTRO - VENTANA_ALTO_INTERNO / 2.0f;
        float y_repisa   = y_ventana_inferior - 0.05f;
        float z_frente   = GABINETE_PROFUNDIDAD / 2.0f;
        float ancho_util = GABINETE_ANCHO - 2.0f * ancho_columna;
        float y_botones;
        float alto_panel;
        float y_centro_panel;

        aplicar_material_dorado();
        glPushMatrix();
        glTranslatef(0.0f, y_repisa, z_frente + 0.01f);
        dibujar_caja_biselada(ancho_util, 0.045f, 0.05f, 0.010f);
        glPopMatrix();

        y_botones = y_repisa - 0.16f;

        /* BUGFIX/PEDIDO: el boton SPIN fisico de aca no hacia nada -el
           control real es la barra 2D de la interfaz, abajo de todo-,
           asi que quedaba como un boton fantasma que invitaba a hacerle
           clic sin efecto. Se saca y se reemplaza por un emblema
           decorativo (placa ovalada + gema + remaches), que se lee como
           una insignia del gabinete en vez de un control roto. */
        /* REDISENO (la placa ovalada + gema tampoco gusto): rosetita
           circular con mini-sunburst y foquitos en el borde, mismo
           lenguaje visual que la cupula de arriba -consistente en vez
           de otro motivo distinto suelto. */
        {
            const int N_RAYOS_CHICO = 10;
            const double PI2 = 2.0 * 3.14159265358979323846;
            float radio_rosa = 0.115f;
            int i;

            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 0.85f, 0.35f, 0.25f);
            glPushMatrix();
            glTranslatef(0.0f, y_botones, z_frente + 0.010f);
            glutSolidSphere(radio_rosa * 1.7, 16, 12);
            glPopMatrix();
            glDisable(GL_BLEND);

            for (i = 0; i < N_RAYOS_CHICO; i++) {
                double a1 = PI2 * (double)i / (double)N_RAYOS_CHICO;
                double a2 = PI2 * (double)(i + 1) / (double)N_RAYOS_CHICO;
                if (i % 2 == 0) glColor3f(0.98f, 0.85f, 0.40f);
                else            glColor3f(0.45f, 0.07f, 0.08f);
                glBegin(GL_TRIANGLES);
                glVertex3f(0.0f, y_botones, z_frente + 0.014f);
                glVertex3f(radio_rosa * (float)cos(a1), y_botones + radio_rosa * (float)sin(a1), z_frente + 0.014f);
                glVertex3f(radio_rosa * (float)cos(a2), y_botones + radio_rosa * (float)sin(a2), z_frente + 0.014f);
                glEnd();
            }

            for (i = 0; i < N_RAYOS_CHICO; i++) {
                double a = PI2 * (double)i / (double)N_RAYOS_CHICO;
                glColor3f(1.0f, 0.92f, 0.55f);
                glPushMatrix();
                glTranslatef(radio_rosa * (float)cos(a), y_botones + radio_rosa * (float)sin(a), z_frente + 0.018f);
                glutSolidSphere(0.016, 8, 8);
                glPopMatrix();
            }
            glEnable(GL_LIGHTING);
        }

        alto_panel     = (y_botones - 0.17f) - GABINETE_Y_INICIO_CUERPO;
        y_centro_panel = GABINETE_Y_INICIO_CUERPO + alto_panel / 2.0f;
        dibujar_panel_acanalado(ancho_util, alto_panel, y_centro_panel, z_frente);
    }

    glEnable(GL_LIGHTING);
}

static void dibujar_franja_dorada(float y, float ancho, float profundo) {
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);
    dibujar_caja_biselada(ancho, ALTO_FRANJA, profundo, BISEL_FRANJA);
    glPopMatrix();
}

/* Marquesina estilo casino moderno: pantalla oscura con "JACKPOT!"
   en letras doradas arriba, tabla de pagos en cuadricula roja/blanca,
   y borde de neon rojo alrededor. Inspirado en el panel superior de
   la imagen de referencia (pantalla de paytable sobre el reel). */
/* Marquesina compacta: titulo + tabla de pagos chica, del mismo ancho
   que el cuerpo (sin el offset de FACTOR_ANCHO_MARQUESINA de antes) para
   que el contorno quede continuo con las columnas de abajo y la cupula
   de arriba -como el mueble de una sola pieza del boceto de referencia,
   en vez de la pantalla ancha estilo "casino moderno" anterior. Sin
   foquitos: el boceto no tiene luces, solo el marco solido. */
static void dibujar_marquesina(void) {
    const float FACTOR_PROFUNDIDAD_MARQUESINA = 0.80f;
    const float ancho_marq  = GABINETE_ANCHO;
    const float profund_marq = GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_MARQUESINA;
    float y_centro_marquesina;
    float z_frente_marq;
    float z_panel;
    int i;

    y_centro_marquesina = GABINETE_Y_INICIO_MARQUESINA + GABINETE_ALTURA_MARQUESINA / 2.0f;
    z_frente_marq = profund_marq / 2.0f;
    z_panel       = z_frente_marq + 0.03f;

    /* Cuerpo negro de la marquesina (biselado) */
    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, 0.0f);
    dibujar_caja_biselada(ancho_marq, GABINETE_ALTURA_MARQUESINA, profund_marq, BISEL_PANEL_GRANDE);
    glPopMatrix();

    /* Panel de pantalla oscuro (ligeramente diferente al cuerpo) */
    {
        GLfloat amb[4] = {0.03f, 0.03f, 0.035f, 1.0f};
        GLfloat dif[4] = {0.07f, 0.07f, 0.08f,  1.0f};
        GLfloat spe[4] = {0.30f, 0.28f, 0.35f,  1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT,   amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  spe);
        glMaterialf (GL_FRONT, GL_SHININESS, 50.0f);
    }
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, z_frente_marq + 0.012f);
    dibujar_caja(ancho_marq * 0.92f, GABINETE_ALTURA_MARQUESINA * 0.90f, 0.02f);
    glPopMatrix();

    glDisable(GL_LIGHTING);

    /* Resplandor detras del titulo -franja calida semitransparente,
       como un letrero retroiluminado de verdad en vez de texto pintado
       sobre la pantalla oscura. */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.75f, 0.10f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(-ancho_marq * 0.42f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.19f, z_panel - 0.004f);
    glVertex3f( ancho_marq * 0.42f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.19f, z_panel - 0.004f);
    glVertex3f( ancho_marq * 0.42f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.42f, z_panel - 0.004f);
    glVertex3f(-ancho_marq * 0.42f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.42f, z_panel - 0.004f);
    glEnd();
    glDisable(GL_BLEND);

    /* Titulo "JACKPOT!" en dorado, chico para que entre en la banda
       angosta -ya centrado de verdad (ver bugfix en
       dibujar_texto_stroke_simple). */
    glColor3f(1.0f, 0.85f, 0.05f);
    glLineWidth(2.5f);
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.30f, z_panel);
    glScalef(0.00105f, 0.00105f, 1.0f);
    dibujar_texto_stroke_simple("JACKPOT!", 1.0f);
    glPopMatrix();

    /* Tabla de pagos compacta: 3 filas, texto chico */
    {
        const char* filas[3][2] = {
            { "7 7 7",  "500" },
            { "* * *",  "100" },
            { "X X X",  " 50" }
        };
        float y_tabla = y_centro_marquesina + GABINETE_ALTURA_MARQUESINA * 0.06f;
        float alto_fila = GABINETE_ALTURA_MARQUESINA * 0.155f;

        for (i = 0; i < 3; i++) {
            float y_fila = y_tabla - (float)i * alto_fila;

            /* Fondo alternado de fila */
            if (i % 2 == 0) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.55f, 0.04f, 0.04f, 0.60f); /* rojo oscuro semitransparente */
                glBegin(GL_QUADS);
                glVertex3f(-ancho_marq * 0.42f, y_fila - alto_fila * 0.46f, z_panel - 0.005f);
                glVertex3f( ancho_marq * 0.42f, y_fila - alto_fila * 0.46f, z_panel - 0.005f);
                glVertex3f( ancho_marq * 0.42f, y_fila + alto_fila * 0.46f, z_panel - 0.005f);
                glVertex3f(-ancho_marq * 0.42f, y_fila + alto_fila * 0.46f, z_panel - 0.005f);
                glEnd();
                glDisable(GL_BLEND);
            }

            /* Texto del simbolo (blanco) */
            glColor3f(0.95f, 0.95f, 0.92f);
            glLineWidth(1.4f);
            glPushMatrix();
            glTranslatef(-ancho_marq * 0.20f, y_fila, z_panel);
            glScalef(0.00055f, 0.00055f, 1.0f);
            glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
            {
                int ci;
                for (ci = 0; filas[i][0][ci] != '\0'; ci++)
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, filas[i][0][ci]);
            }
            glPopMatrix();

            /* Monto (dorado / amarillo) */
            glColor3f(1.0f, 0.85f, 0.10f);
            glPushMatrix();
            glTranslatef(ancho_marq * 0.24f, y_fila, z_panel);
            glScalef(0.00055f, 0.00055f, 1.0f);
            glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
            {
                int ci;
                for (ci = 0; filas[i][1][ci] != '\0'; ci++)
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, filas[i][1][ci]);
            }
            glPopMatrix();
        }
    }

    glEnable(GL_LIGHTING);
}

/* Cupula: medio cilindro horizontal (eje en Z) que remata el gabinete en
   un arco redondeado, en vez de un canto recto -el tope curvo tipico de
   las tragamonedas antiguas de la referencia (Mills/Jennings). Hecha a
   mano con GL_QUAD_STRIP (superficie de revolucion parcial) + 2 tapas
   en abanico para cerrar el volumen por los costados; misma tecnica que
   ya se usa para la rueda de la ruleta (ver utils/bezier.h), sin ningun
   modelo 3D externo. */
static void dibujar_cupula(float radio, float profundo, int segmentos) {
    const double PI_LOCAL = 3.14159265358979323846;
    float hz = profundo / 2.0f;
    int i;

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= segmentos; i++) {
        double t = (double)i / (double)segmentos;
        double ang = PI_LOCAL * (1.0 - t); /* pi -> 0: de izquierda a derecha */
        float x = radio * (float)cos(ang);
        float y = radio * (float)sin(ang);

        glNormal3f((float)cos(ang), (float)sin(ang), 0.0f);
        glVertex3f(x, y, -hz);
        glVertex3f(x, y,  hz);
    }
    glEnd();

    /* Tapa frontal (+Z). BUGFIX: el orden de los vertices del abanico
       determina el sentido de giro (winding) visto DESDE la camara, y
       ese sentido -no el glNormal- es lo que glCullFace(GL_BACK) usa
       para decidir si la cara se dibuja o se descarta. Con el barrido
       de angulo original (de PI a 0, igual que la tapa trasera de mas
       abajo) esta tapa quedaba con sentido horario visto desde +Z -es
       decir, "de espaldas" a la camara- y GL_CULL_FACE la descartaba
       por completo. Lo que se veia en su lugar era la tapa trasera
       (normal (0,0,-1), mirando en contra de la camara y de la luz),
       de ahi que la cupula saliera negra en vez de dorada. Se invierte
       el barrido (0 -> PI) para que el sentido quede antihorario desde
       +Z y la tapa sobreviva el culling. */
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, hz);
    for (i = segmentos; i >= 0; i--) {
        double t = (double)i / (double)segmentos;
        double ang = PI_LOCAL * (1.0 - t);
        glVertex3f(radio * (float)cos(ang), radio * (float)sin(ang), hz);
    }
    glEnd();

    /* Tapa trasera (-Z), barrido invertido respecto a la frontal para
       que quede con el sentido correcto (descartada por el culling
       desde la camara, como corresponde al lado de "adentro"). */
    glNormal3f(0.0f, 0.0f, -1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, -hz);
    for (i = 0; i <= segmentos; i++) {
        double t = (double)i / (double)segmentos;
        double ang = PI_LOCAL * (1.0 - t);
        glVertex3f(radio * (float)cos(ang), radio * (float)sin(ang), -hz);
    }
    glEnd();
}

/* Perilla/finial dorado en la punta de la cupula -remate final tipico
   del tope curvo de las maquinas antiguas de la referencia. */
/* Finial de dos pisos (collar + perilla) en vez de una sola esfera
   suelta -mas presencia en la punta de la cupula. */
static void dibujar_finial_cupula(void) {
    float y_base = GABINETE_Y_INICIO_CUPULA + GABINETE_RADIO_CUPULA;

    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_base, 0.0f);
    glScalef(1.0f, 0.55f, 1.0f);
    glutSolidSphere(0.10, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, y_base + 0.07f, 0.0f);
    glutSolidSphere(0.065, 14, 14);
    glPopMatrix();
}

/* Sunburst decorativo en el frente de la cupula: franjas triangulares
   alternando dorado claro y rojo profundo, irradiando desde la base
   hacia el borde curvo -asi la cupula deja de leerse como un solo
   color plano parejo (por como esta construida, la cara visible de la
   cupula termina siendo una sola superficie chata de frente a la
   camara, sin variacion real de sombreado propia). Se dibuja sin
   iluminacion (colores fijos, como una pieza pintada/con incrustacion)
   ligeramente delante de la cara de la cupula para evitar z-fighting. */
static void dibujar_sunburst_cupula(void) {
    const int N_RAYOS = 11;
    const double PI_LOCAL = 3.14159265358979323846;
    float radio = GABINETE_RADIO_CUPULA * 0.92f;
    float z = GABINETE_PROFUNDIDAD * 0.80f / 2.0f + 0.010f;
    int i;

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUPULA, 0.0f);

    /* Resplandor calido detras del sunburst -varios anillos concentricos
       semitransparentes, cada vez mas chicos y opacos hacia el centro,
       para simular luz de fondo (pedido explicito: "que funcione con
       luz"). */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (i = 3; i >= 1; i--) {
        float r = radio * (0.55f + 0.20f * (float)i);
        float a = 0.10f - (float)i * 0.02f;
        int s;
        glColor4f(1.0f, 0.85f, 0.35f, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, z - 0.002f);
        for (s = 0; s <= 24; s++) {
            double ang = PI_LOCAL * (double)s / 24.0;
            glVertex3f(r * (float)cos(ang), r * (float)sin(ang), z - 0.002f);
        }
        glEnd();
    }
    glDisable(GL_BLEND);

    /* Rayos: dorado claro / rojo profundo alternados */
    for (i = 0; i < N_RAYOS; i++) {
        double ang1 = PI_LOCAL * (double)i / (double)N_RAYOS;
        double ang2 = PI_LOCAL * (double)(i + 1) / (double)N_RAYOS;
        if (i % 2 == 0) glColor3f(0.98f, 0.85f, 0.40f);
        else            glColor3f(0.45f, 0.07f, 0.08f);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, 0.0f, z);
        glVertex3f(radio * (float)cos(ang1), radio * (float)sin(ang1), z);
        glVertex3f(radio * (float)cos(ang2), radio * (float)sin(ang2), z);
        glEnd();
    }

    /* Foquitos en la punta de cada rayo, sobre el borde curvo -como una
       marquesina real con luces, no solo colores pintados. */
    for (i = 0; i <= N_RAYOS; i++) {
        double ang = PI_LOCAL * (double)i / (double)N_RAYOS;
        float bx = radio * (float)cos(ang);
        float by = radio * (float)sin(ang);

        glColor3f(1.0f, 0.95f, 0.70f);
        glPushMatrix();
        glTranslatef(bx, by, z + 0.012f);
        glutSolidSphere(0.026, 8, 8);
        glPopMatrix();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.90f, 0.45f, 0.35f);
        glPushMatrix();
        glTranslatef(bx, by, z + 0.012f);
        glutSolidSphere(0.052, 8, 8);
        glPopMatrix();
        glDisable(GL_BLEND);
    }

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

/* Franja roja en la costura cupula/marquesina -antes esa union era
   dorado sobre dorado (sin ningun quiebre de color), ahora corta el
   monocromatismo justo donde el ojo detecta el cambio de forma. */
static void dibujar_franja_roja(float y, float ancho, float profundo) {
    aplicar_material_acento_rojo();
    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);
    dibujar_caja_biselada(ancho, ALTO_FRANJA * 0.7f, profundo, BISEL_FRANJA);
    glPopMatrix();
}

/* Medallon decorativo: disco embutido con un relieve central chico,
   encarado hacia adelante -aproximacion simple (con primitivas propias,
   sin esculpir una cara) de los medallones ornamentales que la maquina
   de referencia toda dorada tiene a los costados de la cupula. */
static void dibujar_medallon(float x, float y, float z, float radio) {
    GLfloat amb[4] = { 0.16f, 0.11f, 0.015f, 1.0f };
    GLfloat dif[4] = { 0.40f, 0.27f, 0.045f, 1.0f };
    GLfloat spe[4] = { 0.60f, 0.48f, 0.18f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spe);
    glMaterialf (GL_FRONT, GL_SHININESS, 70.0f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(1.0f, 1.0f, 0.35f);
    glutSolidSphere(radio, 18, 18);
    glPopMatrix();

    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(x, y, z + radio * 0.30f);
    glScalef(1.0f, 1.0f, 0.5f);
    glutSolidSphere(radio * 0.42f, 14, 14);
    glPopMatrix();
}

/* Par de medallones a los costados de la base de la marquesina. */
static void dibujar_medallones_marquesina(void) {
    float y = GABINETE_Y_INICIO_MARQUESINA + GABINETE_ALTURA_MARQUESINA * 0.85f;
    float z = (GABINETE_PROFUNDIDAD * 0.80f) / 2.0f + 0.015f;
    float x = GABINETE_ANCHO * 0.34f;

    dibujar_medallon(-x, y, z, 0.10f);
    dibujar_medallon( x, y, z, 0.10f);
}

static void dibujar_gabinete(void) {
    dibujar_base();
    dibujar_franja_dorada(GABINETE_Y_INICIO_CUERPO, GABINETE_ANCHO * 1.05f, GABINETE_PROFUNDIDAD * 1.05f);
    dibujar_cuerpo_principal();
    dibujar_franja_dorada(GABINETE_Y_INICIO_MARQUESINA, GABINETE_ANCHO, GABINETE_PROFUNDIDAD);
    dibujar_marquesina();

    dibujar_franja_roja(GABINETE_Y_INICIO_CUPULA, GABINETE_ANCHO, GABINETE_PROFUNDIDAD * 0.80f);

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUPULA, 0.0f);
    dibujar_cupula(GABINETE_RADIO_CUPULA, GABINETE_PROFUNDIDAD * 0.80f, 24);
    glPopMatrix();
    dibujar_sunburst_cupula();
    dibujar_finial_cupula();
    dibujar_medallones_marquesina();
}


/* ------------------------------------------------------------------- */
/* Rodillos (PASO 2)                                                    */
/* ------------------------------------------------------------------- */

/* Dibuja el simbolo indicado, centrado en el origen actual, dentro de
   una celda de lado aproximado 'tamano'. La mayoria de los simbolos
   usan las texturas PNG reales de simbolos/ (ver simbolos_tex.h) -mucho
   mas fieles a una tragamonedas real que primitivas de GLUT. La unica
   excepcion es SIMBOLO_HERRADURA, que no tiene PNG disponible y se
   sigue dibujando a mano con primitivas propias (permitido por la regla
   del proyecto: nada de modelos 3D externos, solo primitivas propias o
   texturas 2D como esta). */
static void dibujar_simbolo(SimboloTragamonedas simbolo, float tamano) {
    GLboolean iluminacion_estaba_activa;

    if (simbolo != SIMBOLO_HERRADURA) {
        simbolos_tex_dibujar(simbolo, tamano);
        return;
    }

    iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    /* REDISENO (la herradura no gusto): estrella dorada de 5 puntas con
       halo, en el mismo lenguaje visual "encendido" del resto del
       gabinete (sunburst de la cupula, foquitos) en vez de un objeto
       gris suelto sin relacion con la paleta dorada/roja de la maquina.
       Sigue siendo SIMBOLO_HERRADURA a nivel logico (Dubenny), esto
       solo cambia como se DIBUJA -mismo contrato de siempre: la parte
       visual no decide que simbolo es, solo como se ve.
       Triangulada en abanico desde el CENTRO (no desde un vertice del
       borde): una estrella es "estrella-convexa" respecto al centro
       -cualquier punto del borde es visible en linea recta desde el
       centro sin salir de la figura-, que es justo la condicion que
       GL_TRIANGLE_FAN necesita para triangular bien una forma no
       convexa; un GL_POLYGON comun no esta garantizado para formas
       concavas como esta. */
    {
        const int PUNTAS = 5;
        const double PI2 = 2.0 * 3.14159265358979323846;
        float r_ext = tamano * 0.42f;
        float r_int = r_ext * 0.42f;
        int i;

        /* BUGFIX (orientacion): el offset de angulo original (-PI2/4)
           dejaba una PUNTA apuntando hacia ABAJO y ningun vertice recto
           hacia arriba -una estrella "de cabeza" en vez de la posicion
           clasica con una punta arriba. Se cambia a +PI2/4 para que la
           primera punta (idx=0) quede exactamente en el tope. */

        /* Estrella solida dorada, triangulada en abanico desde el centro */
        glColor3f(1.0f, 0.82f, 0.15f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for (i = 0; i <= PUNTAS * 2; i++) {
            int idx = i % (PUNTAS * 2);
            double ang = PI2 * (double)idx / (double)(PUNTAS * 2) + PI2 / 4.0;
            float r = (idx % 2 == 0) ? r_ext : r_int;
            glVertex3f(r * (float)cos(ang), r * (float)sin(ang), 0.0f);
        }
        glEnd();

        /* Contorno -mas fino y dorado en vez de marron oscuro: el
           marron se leia como una sombra pegada al borde, no un relieve. */
        glColor3f(0.85f, 0.65f, 0.20f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < PUNTAS * 2; i++) {
            double ang = PI2 * (double)i / (double)(PUNTAS * 2) + PI2 / 4.0;
            float r = (i % 2 == 0) ? r_ext : r_int;
            glVertex3f(r * (float)cos(ang), r * (float)sin(ang), 0.001f);
        }
        glEnd();

        /* Brillo chico -mismo truco de "reflejo" que el resto de los
           botones/gemas glossy del gabinete. */
        glColor3f(1.0f, 1.0f, 0.92f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(-r_ext * 0.15f, r_ext * 0.22f, 0.002f);
        for (i = 0; i <= 10; i++) {
            double ang = PI2 * (double)i / 10.0;
            glVertex3f(-r_ext * 0.15f + r_ext * 0.14f * (float)cos(ang),
                        r_ext * 0.22f + r_ext * 0.14f * (float)sin(ang), 0.002f);
        }
        glEnd();
    }

    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}


/* Radio del "cilindro" de cada rodillo: prisma de NUM_SIMBOLOS caras
   (hexagono, uno por cada simbolo distinto de la tira -ver
   tragamonedas_logica.h). La cuerda de una cara a este radio es
   exactamente R (2*R*sen(180/6) = R para un hexagono regular), por eso
   se usa ALTURA_UNIDAD_SIMBOLO como radio: la cara frontal termina
   ocupando mas o menos "un simbolo" de alto, y las caras vecinas
   asoman como lascas curvas arriba y abajo -el mismo efecto de "ver
   el simbolo siguiente" que antes se lograba con 3 quads planos
   sueltos, pero ahora es consecuencia real de la geometria del
   cilindro girando, no una fila fija. */
#define RADIO_RODILLO_CILINDRO ALTURA_UNIDAD_SIMBOLO

/* Rodillo = cilindro real que gira sobre su propio eje (glRotatef
   alrededor de X, el mismo eje a lo largo del cual se extiende el
   cilindro), con 6 caras pianas -una por simbolo- montadas sobre un
   nucleo metalico dibujado con gluCylinder. La rotacion total se deriva
   directamente de EstadoRodillo.posicion_actual (que ya trae toda la
   logica de velocidad/aceleracion/parada de tragamonedas_animacion.c,
   sin necesidad de un angulo ni un timer separados): a
   posicion_actual == k (entero) le corresponde angulo_total tal que la
   cara k queda exactamente al frente, mostrando
   obtener_simbolo_en_posicion(indice, (float)k) -mismo simbolo que ya
   mostraba el diseno anterior en esa misma posicion. */
static void dibujar_rodillo(int indice, const EstadoRodillo* rodillo, float x_centro, float z_rodillo) {
    float ancho_rodillo_local = VENTANA_ANCHO_INTERNO / 3.0f - GROSOR_DIVISOR_RODILLO;
    float radio = RADIO_RODILLO_CILINDRO;
    float angulo_total_grados = -rodillo->posicion_actual * (360.0f / (float)NUM_SIMBOLOS);
    int k;

    /* Placa de fondo del rodillo: se sigue viendo alrededor del
       cilindro (arriba/abajo, donde el tambor no llega), para que no
       quede un hueco vacio dentro de la ventana. */
    aplicar_material_fondo_rodillo();
    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO, z_rodillo);
    dibujar_caja(ancho_rodillo_local, VENTANA_ALTO_INTERNO, 0.04f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO, z_rodillo);
    glRotatef(angulo_total_grados, 1.0f, 0.0f, 0.0f);

    /* Nucleo mecanico: cilindro solido (gluCylinder), eje a lo largo de
       X -mismo eje de giro que las caras de simbolos, montado DENTRO de
       esta misma matriz rotada para que gire exactamente junto con
       ellas. Tono dorado calido en vez del MATERIAL_METAL compartido
       (gris/blanco cromado) -ese material lo usa tambien la rueda de
       la ruleta, asi que tocarlo de aca lo afectaria alla; ademas el
       blanco fuerte contra el dorado del mueble hacia que el tambor se
       viera "aparte" en vez de una sola pieza.
       BUGFIX (iconos "flotando"): el radio del nucleo estaba en 0.92
       del radio de las caras -mucho mas angosto que necesario-, dejando
       un hueco/sombra visible entre el borde del icono plano y el
       tambor curvo debajo. Se sube a 0.985 para que casi no quede hueco,
       y las caras se acercan de radio a radio*0.99 por lo mismo. */
    {
        GLfloat amb[4] = { 0.20f, 0.15f, 0.05f, 1.0f };
        GLfloat dif[4] = { 0.55f, 0.42f, 0.18f, 1.0f };
        GLfloat spe[4] = { 0.85f, 0.72f, 0.40f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spe);
        glMaterialf (GL_FRONT, GL_SHININESS, 100.0f);
    }
    glPushMatrix();
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); /* gluCylinder dibuja a lo largo de +Z: reorientar a +X */
    glTranslatef(0.0f, 0.0f, -ancho_rodillo_local / 2.0f);
    {
        GLUquadric* quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluCylinder(quad, radio * 0.985f, radio * 0.985f, ancho_rodillo_local, 24, 1);
        gluDeleteQuadric(quad);
    }
    glPopMatrix();

    /* 6 caras pianas, una por simbolo de la tira, formando el prisma
       hexagonal alrededor del nucleo -pegadas casi al ras del tambor
       (ver bugfix arriba). */
    for (k = 0; k < NUM_SIMBOLOS; k++) {
        SimboloTragamonedas simbolo = obtener_simbolo_en_posicion(indice, (float)k);
        glPushMatrix();
        glRotatef((float)k * (360.0f / (float)NUM_SIMBOLOS), 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, radio * 0.99f);
        dibujar_simbolo(simbolo, radio * 0.98f);
        glPopMatrix();
    }

    glPopMatrix(); /* cierra el glRotatef(angulo_total_grados, ...) */
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


/* Dibuja los 3 rodillos dentro de la ventana del gabinete, mas la linea
   de pago dorada que los atraviesa en el centro (la linea horizontal que
   indica que combinacion tiene que alinearse para ganar). */
/* Columna de foquitos dorados a cada lado de la ventana de rodillos,
   montados sobre las columnas del cuerpo -detalle de la maquina de
   referencia toda dorada (dos columnas verticales de foquitos, no un
   anillo completo). Estaticos y bien saturados/calidos desde el
   arranque -la vez anterior el problema no era tener foquitos, era que
   el tono "apagado" los hacia leer como remaches. */
static void dibujar_foquitos_costados_ventana(void) {
    const int N = 9;
    float x_izq = -VENTANA_ANCHO_INTERNO / 2.0f - GROSOR_MARCO_VENTANA * 0.55f;
    float x_der =  VENTANA_ANCHO_INTERNO / 2.0f + GROSOR_MARCO_VENTANA * 0.55f;
    float y_top = VENTANA_Y_CENTRO + VENTANA_ALTO_INTERNO / 2.0f - 0.05f;
    float y_bot = VENTANA_Y_CENTRO - VENTANA_ALTO_INTERNO / 2.0f + 0.05f;
    float z_luz = GABINETE_PROFUNDIDAD / 2.0f + 0.04f;
    int k;

    /* BUGFIX (contraste): los foquitos dorados quedaban montados
       directamente sobre el dorado del cuerpo -practicamente
       invisibles, sin contraste. Se agrega una franja oscura fina
       detras de cada columna de foquitos (como el marco negro que
       enmarca los foquitos en la maquina de referencia). */
    {
        GLfloat am[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
        GLfloat di[4] = { 0.04f, 0.04f, 0.045f, 1.0f };
        GLfloat sp[4] = { 0.20f, 0.20f, 0.22f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT,   am);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   di);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  sp);
        glMaterialf (GL_FRONT, GL_SHININESS, 40.0f);
    }
    glPushMatrix();
    glTranslatef(x_izq, VENTANA_Y_CENTRO, z_luz - 0.012f);
    dibujar_caja(0.075f, y_top - y_bot + 0.10f, 0.02f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(x_der, VENTANA_Y_CENTRO, z_luz - 0.012f);
    dibujar_caja(0.075f, y_top - y_bot + 0.10f, 0.02f);
    glPopMatrix();

    /* PEDIDO: que los foquitos "enciendan" de verdad -antes eran solo
       esferas de color solido. Cada foquito tiene un halo translucido
       mas grande alrededor, simulando el resplandor de una luz real
       montada en el gabinete.
       BUGFIX (por que no se veian encendidos): el halo (mas grande) se
       dibujaba ANTES que el foquito solido, y como el depth buffer no
       distingue "transparente" de "opaco" al escribir profundidad, el
       halo dejaba su propia superficie mas cerca de la camara escrita
       en el depth buffer -y el foquito solido, dibujado despues pero
       geometricamente MAS CHICO Y MAS ATRAS que el frente del halo,
       fallaba el test de profundidad y quedaba tapado por su propio
       halo. Se invierte el orden (solido primero, halo despues, mismo
       centro) y se apaga la escritura de profundidad del halo con
       glDepthMask(GL_FALSE) mientras se dibuja, para que nunca vuelva a
       tapar nada dibujado despues. */
    glDisable(GL_LIGHTING);
    for (k = 0; k < N; k++) {
        float t = (float)k / (float)(N - 1);
        float y = y_bot + t * (y_top - y_bot);
        int sx;

        for (sx = -1; sx <= 1; sx += 2) {
            float x = (sx < 0) ? x_izq : x_der;

            glColor3f(1.0f, 0.92f, 0.55f);
            glPushMatrix();
            glTranslatef(x, y, z_luz);
            glutSolidSphere(0.024, 8, 8);
            glPopMatrix();

            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 0.85f, 0.35f, 0.40f);
            glPushMatrix();
            glTranslatef(x, y, z_luz);
            glutSolidSphere(0.050, 10, 10);
            glPopMatrix();
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
    }
    glEnable(GL_LIGHTING);
}


static void dibujar_rodillos(const EstadoTragamonedas* estado) {
    const float ancho_rodillo = VENTANA_ANCHO_INTERNO / 3.0f;
    /* Los rodillos se ubican un poco adentro del gabinete, no pegados
       al frente, para que se sientan "dentro" de la ventana */
    const float z_rodillo = 0.15f;
    int i;

    for (i = 0; i < NUM_RODILLOS; i++) {
        float x_centro = -VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo / 2.0f + (float)i * ancho_rodillo;
        dibujar_rodillo(i, &estado->rodillos[i], x_centro, z_rodillo);
    }

    /* Divisores en los 2 bordes internos (entre rodillo 0-1 y 1-2) */
    dibujar_divisores_rodillos(-VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo,
        -VENTANA_ANCHO_INTERNO / 2.0f + 2.0f * ancho_rodillo);

    /* Marco dorado saliente alrededor de toda la ventana -borde bien
       definido en vez de que la ventana se pierda contra las columnas
       del mismo color. */
    {
        float hw = VENTANA_ANCHO_INTERNO / 2.0f + 0.05f;
        float hh = VENTANA_ALTO_INTERNO / 2.0f + 0.05f;
        float grosor = 0.045f;
        float zm = GABINETE_PROFUNDIDAD / 2.0f + 0.02f;

        aplicar_material_dorado();
        glPushMatrix();
        glTranslatef(0.0f, VENTANA_Y_CENTRO + hh - grosor / 2.0f, zm);
        dibujar_caja_biselada(hw * 2.0f, grosor, 0.05f, 0.010f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f, VENTANA_Y_CENTRO - hh + grosor / 2.0f, zm);
        dibujar_caja_biselada(hw * 2.0f, grosor, 0.05f, 0.010f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-hw + grosor / 2.0f, VENTANA_Y_CENTRO, zm);
        dibujar_caja_biselada(grosor, hh * 2.0f, 0.05f, 0.010f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(hw - grosor / 2.0f, VENTANA_Y_CENTRO, zm);
        dibujar_caja_biselada(grosor, hh * 2.0f, 0.05f, 0.010f);
        glPopMatrix();
    }

    dibujar_foquitos_costados_ventana();

    /* Reflejo de vidrio: franja diagonal semitransparente cruzando la
       ventana, para que se lea como el cristal de una tragamonedas
       real y no como un hueco plano. Se dibuja bien adelante (mas cerca
       de la camara que los rodillos) y sin iluminacion, como un brillo
       fijo de vidrio, no una luz de la escena. */
    {
        float hw = VENTANA_ANCHO_INTERNO / 2.0f;
        float hh = VENTANA_ALTO_INTERNO / 2.0f;
        float zv = GABINETE_PROFUNDIDAD / 2.0f + 0.09f;
        float cxv = VENTANA_Y_CENTRO;

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.14f);
        glBegin(GL_QUADS);
        glVertex3f(-hw,               cxv - hh,          zv);
        glVertex3f(-hw + hw * 0.55f,  cxv - hh,          zv);
        glVertex3f(-hw + hw * 1.35f,  cxv + hh,          zv);
        glVertex3f(-hw + hw * 0.80f,  cxv + hh,          zv);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 0.07f);
        glBegin(GL_QUADS);
        glVertex3f(-hw + hw * 1.10f,  cxv - hh,          zv);
        glVertex3f(-hw + hw * 1.30f,  cxv - hh,          zv);
        glVertex3f(-hw + hw * 1.85f,  cxv + hh,          zv);
        glVertex3f(-hw + hw * 1.65f,  cxv + hh,          zv);
        glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    /* ---- Linea de pago central ----
       Barra dorada horizontal que atraviesa los 3 rodillos a la altura
       de VENTANA_Y_CENTRO. Indica la fila ganadora, como en una maquina
       de casino real. Se dibuja encima de los rodillos (z mas adelante)
       para que sea visible sobre los simbolos. */
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, VENTANA_Y_CENTRO, z_rodillo + 0.10f);
    dibujar_caja(VENTANA_ANCHO_INTERNO, 0.022f, 0.008f); /* barra muy fina, ancho total de la ventana */
    glPopMatrix();

    /* Dos indicadores laterales triangulares (flechas cromadas) que senalan
       la linea de pago desde los bordes del marco de la ventana -detalle
       clasico de las tragamonedas fisicas. Se dibuja con iluminacion
       apagada para que el dorado siempre sea brillante. */
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.88f, 0.15f); /* dorado brillante */
    glLineWidth(3.0f);
    /* Flecha izquierda: triangulo apuntando a la derecha */
    glBegin(GL_TRIANGLES);
    glVertex3f(-VENTANA_ANCHO_INTERNO / 2.0f - 0.22f, VENTANA_Y_CENTRO,              z_rodillo + 0.12f);
    glVertex3f(-VENTANA_ANCHO_INTERNO / 2.0f - 0.05f, VENTANA_Y_CENTRO + 0.07f,      z_rodillo + 0.12f);
    glVertex3f(-VENTANA_ANCHO_INTERNO / 2.0f - 0.05f, VENTANA_Y_CENTRO - 0.07f,      z_rodillo + 0.12f);
    glEnd();
    /* Flecha derecha: triangulo apuntando a la izquierda */
    glBegin(GL_TRIANGLES);
    glVertex3f( VENTANA_ANCHO_INTERNO / 2.0f + 0.22f, VENTANA_Y_CENTRO,              z_rodillo + 0.12f);
    glVertex3f( VENTANA_ANCHO_INTERNO / 2.0f + 0.05f, VENTANA_Y_CENTRO + 0.07f,      z_rodillo + 0.12f);
    glVertex3f( VENTANA_ANCHO_INTERNO / 2.0f + 0.05f, VENTANA_Y_CENTRO - 0.07f,      z_rodillo + 0.12f);
    glEnd();
    glEnable(GL_LIGHTING);
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


/* ------------------------------------------------------------------- */
/* PASO 5: Display de creditos                                          */
/* ------------------------------------------------------------------- */

/* Display de creditos: panel digital en el frente de la base que
   muestra el monto apostado y el ultimo resultado (PREMIO o PERDIDA).
   Usa texto stroke escalado, sobre un fondo oscuro con marco dorado. */
static void dibujar_display_creditos(void) {
    /* REDISENO: base mucho mas baja que antes (0.30 vs 0.5) y ahora con
       el cajon/ranura de dibujar_base() ocupando la parte de abajo -el
       display se achica y se sube para no solaparse con ese cajon,
       manteniendo margen contra el techo de la base. */
    float z_frente    = (GABINETE_PROFUNDIDAD * 1.05f) / 2.0f + 0.015f;
    float ancho_disp  = GABINETE_ANCHO * 0.55f;
    float alto_disp   = 0.075f;
    float y_disp      = GABINETE_ALTURA_BASE * 0.70f;
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

    /* Marco dorado, biselado -antes era una caja de canto vivo */
    aplicar_material_dorado();
    glPushMatrix();
    glTranslatef(0.0f, y_disp, z_frente - 0.01f);
    dibujar_caja_biselada(ancho_disp + 0.04f, alto_disp + 0.03f, 0.04f, 0.014f);
    glPopMatrix();

    /* Panel oscuro (casi negro), biselado para que se lea como pantalla
       empotrada y no como una placa pegada encima. */
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
    dibujar_caja_biselada(ancho_disp, alto_disp, 0.03f, 0.008f);
    glPopMatrix();

    glDisable(GL_LIGHTING);

    /* Resplandor sutil detras de cada numero (efecto pantalla digital
       retroiluminada, no solo texto flotando sobre negro liso). */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.10f, 0.85f, 0.30f, 0.16f);
    glBegin(GL_QUADS);
    glVertex3f(-ancho_disp * 0.48f, y_disp - alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f(-ancho_disp * 0.03f, y_disp - alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f(-ancho_disp * 0.03f, y_disp + alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f(-ancho_disp * 0.48f, y_disp + alto_disp * 0.42f, z_frente + 0.018f);
    glEnd();
    glColor4f(1.0f, 0.85f, 0.10f, 0.14f);
    glBegin(GL_QUADS);
    glVertex3f( ancho_disp * 0.03f, y_disp - alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f( ancho_disp * 0.48f, y_disp - alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f( ancho_disp * 0.48f, y_disp + alto_disp * 0.42f, z_frente + 0.018f);
    glVertex3f( ancho_disp * 0.03f, y_disp + alto_disp * 0.42f, z_frente + 0.018f);
    glEnd();
    glDisable(GL_BLEND);

    /* Divisor fino dorado entre BET y WIN */
    glColor3f(0.75f, 0.60f, 0.15f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, y_disp - alto_disp * 0.38f, z_frente + 0.019f);
    glVertex3f(0.0f, y_disp + alto_disp * 0.38f, z_frente + 0.019f);
    glEnd();

    /* BET: verde digital */
    glColor3f(0.10f, 0.95f, 0.25f);
    glPushMatrix();
    glTranslatef(-ancho_disp * 0.24f, y_disp, z_frente + 0.03f);
    glScalef(0.00062f, 0.00062f, 1.0f);
    glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
    dibujar_texto_stroke_simple(buf_apuesta, 1.0f);
    glPopMatrix();

    /* WIN/LOSE: dorado si gana, rojo si pierde */
    if (g_estado_tragamonedas.hay_ganancia)
        glColor3f(1.0f, 0.90f, 0.05f);   /* dorado */
    else
        glColor3f(0.95f, 0.15f, 0.10f);  /* rojo */

    glPushMatrix();
    glTranslatef(ancho_disp * 0.24f, y_disp, z_frente + 0.03f);
    glScalef(0.00062f, 0.00062f, 1.0f);
    glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f);
    dibujar_texto_stroke_simple(buf_premio, 1.0f);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}


/* ------------------------------------------------------------------- */
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
    float* bet_menos_cx, float* bet_mas_cx, float* bet_y, float* bet_r,
    float* auto_cx, float* auto_cy, float* auto_r) {

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

    /* columna 4: AUTO/MANUAL, tambien circular para que combine con el
       resto de botones en vez de ser el unico rectangulo de la barra */
    *auto_cx = col_x0 + BARRA_CONTROL_COL_ANCHO * 4.5f;
    *auto_cy = cy;
    *auto_r  = 32.0f;
}

void dibujar_barra_control_2d(int ancho_ventana, int alto_ventana,
                               float saldo, float apuesta, float ganancia,
                               int auto_activo) {
    char buf[64];
    int hay_giro, i;
    float panel_x0, panel_x1;
    float spin_cx, spin_cy, spin_r;
    float bet_menos_cx, bet_mas_cx, bet_y, bet_r;
    float auto_cx, auto_cy, auto_r;
    float saldo_cx, win_cx, bet_label_cx;
    float y_base = BARRA_CONTROL_ALTO / 2.0f;

    calcular_zonas_barra_control(ancho_ventana, &panel_x0, &panel_x1,
        &spin_cx, &spin_cy, &spin_r,
        &bet_menos_cx, &bet_mas_cx, &bet_y, &bet_r,
        &auto_cx, &auto_cy, &auto_r);

    saldo_cx     = panel_x0 + BARRA_CONTROL_COL_ANCHO * 0.5f;
    bet_label_cx = (bet_menos_cx + bet_mas_cx) / 2.0f;
    win_cx       = panel_x0 + BARRA_CONTROL_COL_ANCHO * 2.5f;

    hay_giro = 0;
    if (g_estado_tragamonedas_listo) {
        for (i = 0; i < NUM_RODILLOS; i++) {
            if (g_estado_tragamonedas.rodillos[i].girando) hay_giro = 1;
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

    /* AUTO / MANUAL: tambien circular, para combinar con el resto */
    if (auto_activo) dibujar_boton_circular_glossy(auto_cx, auto_cy, auto_r, 0.16f, 0.62f, 0.24f);
    else             dibujar_boton_circular_glossy(auto_cx, auto_cy, auto_r, 0.32f, 0.32f, 0.35f);
    glColor3f(1.0f, 1.0f, 1.0f);
    dibujar_texto_2d_centrado(auto_cx, auto_cy + 3.0f, auto_activo ? "AUTO" : "MANUAL", GLUT_BITMAP_HELVETICA_10);
    dibujar_texto_2d_centrado(auto_cx, auto_cy - 10.0f, auto_activo ? "ON" : "OFF", GLUT_BITMAP_HELVETICA_10);

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
    float auto_cx, auto_cy, auto_r;
    float fx = (float)x_mouse;
    /* GLUT entrega la Y del mouse creciendo hacia ABAJO (origen arriba-
       izquierda); las zonas estan en coordenadas Y-hacia-arriba
       (glOrtho), asi que hay que invertir antes de comparar. */
    float fy = (float)(alto_ventana - y_mouse);
    float dx, dy;

    calcular_zonas_barra_control(ancho_ventana, &panel_x0, &panel_x1,
        &spin_cx, &spin_cy, &spin_r,
        &bet_menos_cx, &bet_mas_cx, &bet_y, &bet_r,
        &auto_cx, &auto_cy, &auto_r);

    dx = fx - spin_cx; dy = fy - spin_cy;
    if (dx * dx + dy * dy <= spin_r * spin_r) return ZONA_CONTROL_SPIN;

    dx = fx - bet_menos_cx; dy = fy - bet_y;
    if (dx * dx + dy * dy <= bet_r * bet_r) return ZONA_CONTROL_BET_MENOS;

    dx = fx - bet_mas_cx; dy = fy - bet_y;
    if (dx * dx + dy * dy <= bet_r * bet_r) return ZONA_CONTROL_BET_MAS;

    dx = fx - auto_cx; dy = fy - auto_cy;
    if (dx * dx + dy * dy <= auto_r * auto_r) return ZONA_CONTROL_AUTO;

    return ZONA_CONTROL_NINGUNA;
}


/* Sombra de contacto: elipse oscura y semitransparente en el piso,
   debajo de la base -sin esto la maquina se ve "flotando" sobre la
   alfombra del fondo en vez de apoyada de verdad. Se dibuja con
   iluminacion apagada (sombra fija, no depende del angulo de luz) y
   con GL_DEPTH_TEST activo para que quede pegada al piso, no encima de
   la maquina si la camara se mueve. */
static void dibujar_sombra_contacto(void) {
    const int SEGMENTOS = 24;
    float rx = GABINETE_ANCHO * 0.62f;
    float rz = GABINETE_PROFUNDIDAD * 0.62f;
    int i;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.006f, 0.0f);
    for (i = 0; i <= SEGMENTOS; i++) {
        double ang = 2.0 * 3.14159265358979323846 * (double)i / (double)SEGMENTOS;
        glVertex3f(rx * (float)cos(ang), 0.006f, rz * (float)sin(ang));
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void dibujar_tragamonedas(void) {
    asegurar_estado_inicializado();

    dibujar_sombra_contacto();
    dibujar_gabinete();
    dibujar_rodillos(&g_estado_tragamonedas);
    dibujar_display_creditos();
}

