/*
 * tablero_apuestas.c
 * Implementacion del tablero de apuestas. Ver tablero_apuestas.h.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tablero_apuestas.h"
#include "../utils/bresenham.h"
#include "../core/estado_juego.h"

#define TABLERO_COLUMNAS 12   /* 12 columnas x 3 filas = 36 numeros */
#define TABLERO_FILAS     3
#define CELDA_PX         40   /* tamano de cada celda en "pixeles" locales */
#define ESCALA_TABLERO   0.012f /* convierte esos pixeles a unidades de mundo */
#define MARGEN_CASILLA    2   /* margen para que se vea el borde de Bresenham */

 /* --- Zonas de apuesta adicionales (docena, mitad, par/impar, color),
    debajo del grid de numeros 1-36. Mismo patron de coordenadas locales
    que el resto del tablero (CELDA_PX y ESCALA_TABLERO). --- */
#define ALTO_ZONA_DOCENA_PX    40  /* alto de la franja de docenas */
#define ALTO_ZONA_INFERIOR_PX  40  /* alto de la franja mitad/par-impar/color */

    /* Celda del 0 (verde): en una ruleta real el 0 vive fuera del grid de
       1-36, no como una "apuesta a color verde" -se apuesta a el igual que
       a cualquier otro numero (APUESTA_NUMERO, valor=0, paga 35 a 1). Se
       agrega como una columna extra a la IZQUIERDA del grid, ocupando el
       alto completo (las 3 filas), como en una mesa real. */
#define ANCHO_CELDA_CERO_PX 30

       /* Calcula que numero (1-36) corresponde a una celda del grid, siguiendo
          el orden real de una mesa de ruleta: columnas verticales de 3 numeros
          consecutivos (columna 0 = 1,2,3 - columna 1 = 4,5,6 - etc). */
static int numero_de_celda(int col, int fila) {
    return col * 3 + fila + 1;
}

/* Dibuja 'texto' centrado en el origen actual, con la fuente vectorial
   de GLUT (stroke), para que escale igual que el resto del tablero.
   'ancho_max' es el espacio disponible (en las mismas unidades locales
   que CELDA_PX) para que el texto siempre quepa dentro de su celda,
   sin importar cuantos caracteres tenga. */
static void dibujar_texto_stroke_centrado(const char* texto, float ancho_max) {
    int i, len;
    float ancho_total, alto_total;
    float escala;

    len = (int)strlen(texto);

    /* Altura/ancho parten de la misma escala (fuente sin deformar). */
    escala = 0.24f;
    ancho_total = len * 104.76f * escala;

    /* BUGFIX (texto deformado en las zonas especiales del tablero -
       Par/Impar/Rojo/Negro/1-18/19-36-): antes, si el texto no entraba
       en su celda, SOLO se reducia escala_x (dejando escala_y fija en
       0.24) -es decir, un glScalef(escala_x, escala_y, 1) NO uniforme.
       Eso aplastaba las letras horizontalmente sin tocar su altura,
       deformando curvas y trazos de la fuente stroke: mucho mas
       notorio en palabras largas ("NEGRO", "IMPAR", 5 caracteres en
       solo 74px de celda -> hacia falta comprimir a un ~59% del ancho
       "natural") que en los digitos de la grilla principal (1-2
       caracteres, con mucho mas margen de sobra, por eso esos se veian
       bien).
       Fix: si no entra, se reduce el MISMO factor en X y en Y (escala
       uniforme) -el texto sale mas chico, pero nunca deformado, en
       cualquier celda del tablero. */
    if (ancho_total > ancho_max) {
        escala *= ancho_max / ancho_total;
        ancho_total = ancho_max;
    }
    alto_total = 119.05f * escala;

    glPushMatrix();
    glTranslatef(-ancho_total / 2.0f, -alto_total / 2.0f, 0.0f);
    glScalef(escala, escala, 1.0f);

    /* BUGFIX (texto "borroso"): la fuente stroke solo dibuja lineas
       delgadas (grosor 1.0 por defecto); a la escala final que queda
       tras el ajuste de arriba, esas lineas de 1px se ven casi
       invisibles/borrosas en varias resoluciones. Se engruesa un poco,
       igual que ya se hacia para los numeros de la rueda. Ademas se
       activa GL_LINE_SMOOTH (con blending, que es lo que necesita para
       funcionar) mientras dura el trazo -en todo el proyecto no habia
       ningun antialiasing de lineas activo (el MSAA global esta
       documentado como poco confiable en GLUT clasico), asi que los
       trazos diagonales de la fuente se veian dentados/escalonados a
       tamanos chicos. Se restaura el estado previo despues, para no
       afectar el resto de la escena (Bresenham, etc., que no lo
       necesitan). */
    {
        GLboolean line_smooth_estaba_activo = glIsEnabled(GL_LINE_SMOOTH);
        GLboolean blend_estaba_activo = glIsEnabled(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glLineWidth(1.6f);

        for (i = 0; i < len; i++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
        }

        glLineWidth(1.0f); /* restaurar, para no afectar otras lineas (Bresenham, etc.) */
        if (!line_smooth_estaba_activo) glDisable(GL_LINE_SMOOTH);
        if (!blend_estaba_activo) glDisable(GL_BLEND);
    }
    glPopMatrix();
}

/* Dibuja un numero centrado en el origen actual (celda del grid 0-36). */
static void dibujar_numero_centrado(int numero) {
    char texto[4];
    const float ANCHO_MAX = 34.0f; /* ancho disponible dentro de la celda (40px - margen) */

    snprintf(texto, sizeof(texto), "%d", numero);
    dibujar_texto_stroke_centrado(texto, ANCHO_MAX);
}

/* Capa 1: casillas coloreadas (rojo/negro segun color_de_numero), mas
   la celda verde del 0 a la izquierda del grid. */
static void dibujar_casillas(void) {
    int col, fila, numero;
    ColorRuleta color;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    /* Celda del 0: a la izquierda del grid (u negativo), ocupa el alto
       completo de las 3 filas. */
    glColor3f(0.0f, 0.42f, 0.15f); /* verde, mismo tono que la pista de la rueda */
    glBegin(GL_QUADS);
    glVertex2i(-ANCHO_CELDA_CERO_PX + MARGEN_CASILLA, MARGEN_CASILLA);
    glVertex2i(0 - MARGEN_CASILLA, MARGEN_CASILLA);
    glVertex2i(0 - MARGEN_CASILLA, TABLERO_FILAS * CELDA_PX - MARGEN_CASILLA);
    glVertex2i(-ANCHO_CELDA_CERO_PX + MARGEN_CASILLA, TABLERO_FILAS * CELDA_PX - MARGEN_CASILLA);
    glEnd();

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);
            color = color_de_numero(numero);

            if (color == COLOR_ROJO)
                glColor3f(0.75f, 0.08f, 0.08f);
            else
                glColor3f(0.08f, 0.08f, 0.08f); /* negro */

            glBegin(GL_QUADS);
            glVertex2i(col * CELDA_PX + MARGEN_CASILLA, fila * CELDA_PX + MARGEN_CASILLA);
            glVertex2i((col + 1) * CELDA_PX - MARGEN_CASILLA, fila * CELDA_PX + MARGEN_CASILLA);
            glVertex2i((col + 1) * CELDA_PX - MARGEN_CASILLA, (fila + 1) * CELDA_PX - MARGEN_CASILLA);
            glVertex2i(col * CELDA_PX + MARGEN_CASILLA, (fila + 1) * CELDA_PX - MARGEN_CASILLA);
            glEnd();
        }
    }

    glPopMatrix();
}

/* Capa 2: lineas divisorias (grid de numeros + borde de la celda del 0). */
static void dibujar_lineas(void) {
    int col, fila;

    glPushMatrix();
    glTranslatef(-2.9f, 0.1f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);

    for (col = 0; col <= TABLERO_COLUMNAS; col++) {
        int x = col * CELDA_PX;
        dibujar_linea_bresenham(x, 0, x, TABLERO_FILAS * CELDA_PX);
    }
    for (fila = 0; fila <= TABLERO_FILAS; fila++) {
        int y = fila * CELDA_PX;
        dibujar_linea_bresenham(0, y, TABLERO_COLUMNAS * CELDA_PX, y);
    }

    /* Borde de la celda del 0 (izquierda del grid) */
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, 0, -ANCHO_CELDA_CERO_PX, TABLERO_FILAS * CELDA_PX);
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, 0, 0, 0);
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, TABLERO_FILAS * CELDA_PX, 0, TABLERO_FILAS * CELDA_PX);

    glPopMatrix();
}

/* Capa 1b: franja de docenas (1ra 12 / 2da 12 / 3ra 12), justo debajo
   del grid de numeros. Cada franja cubre 4 columnas (4*3 = 12 numeros),
   alineada exactamente con docena_de_numero() de estado_juego.c, ya
   que numero_de_celda(col, fila) = col*3 + fila + 1: columnas 0-3 dan
   numeros 1-12, columnas 4-7 dan 13-24, columnas 8-11 dan 25-36. */
static void dibujar_zona_docenas(void) {
    static const char* ETIQUETAS_DOCENA[3] = { "1ra 12", "2da 12", "3ra 12" };
    const int ANCHO_ZONA = 4 * CELDA_PX; /* 160 px = 4 columnas por docena */
    const int Y0 = -ALTO_ZONA_DOCENA_PX;
    int i;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (i = 0; i < 3; i++) {
        glColor3f(0.05f, 0.32f, 0.10f); /* verde fieltro: distinto de rojo/negro del grid */
        glBegin(GL_QUADS);
        glVertex2i(i * ANCHO_ZONA + MARGEN_CASILLA, Y0 + MARGEN_CASILLA);
        glVertex2i((i + 1) * ANCHO_ZONA - MARGEN_CASILLA, Y0 + MARGEN_CASILLA);
        glVertex2i((i + 1) * ANCHO_ZONA - MARGEN_CASILLA, Y0 + ALTO_ZONA_DOCENA_PX - MARGEN_CASILLA);
        glVertex2i(i * ANCHO_ZONA + MARGEN_CASILLA, Y0 + ALTO_ZONA_DOCENA_PX - MARGEN_CASILLA);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(i * ANCHO_ZONA + ANCHO_ZONA / 2.0f, Y0 + ALTO_ZONA_DOCENA_PX / 2.0f, 0.0f);
        dibujar_texto_stroke_centrado(ETIQUETAS_DOCENA[i], (float)ANCHO_ZONA - 6.0f);
        glPopMatrix();
    }

    glPopMatrix();
}

/* Capa 1c: franja inferior con mitad (1-18 / 19-36), par/impar, y ahora
   tambien color (ROJO/NEGRO) -6 celdas en total, debajo de la franja de
   docenas: [1-18][PAR][ROJO][NEGRO][IMPAR][19-36]. Las celdas de color
   se pintan con su propio color real (rojo/negro), a diferencia de las
   demas que usan el azul de fondo, para que se reconozcan de inmediato
   como "apostar a ese color" -igual que en una mesa real. */
static void dibujar_zona_inferior(void) {
    static const char* ETIQUETAS_INFERIOR[6] = { "1-18", "PAR", "ROJO", "NEGRO", "IMPAR", "19-36" };
    const int ANCHO_CELDA_INF = (TABLERO_COLUMNAS * CELDA_PX) / 6; /* 80 px */
    const int Y0 = -(ALTO_ZONA_DOCENA_PX + ALTO_ZONA_INFERIOR_PX);
    int i;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (i = 0; i < 6; i++) {
        if (i == 2) glColor3f(0.75f, 0.08f, 0.08f);      /* ROJO: mismo tono que las casillas rojas */
        else if (i == 3) glColor3f(0.08f, 0.08f, 0.08f); /* NEGRO: mismo tono que las casillas negras */
        else glColor3f(0.10f, 0.10f, 0.34f);              /* resto: azul oscuro, como antes */

        glBegin(GL_QUADS);
        glVertex2i(i * ANCHO_CELDA_INF + MARGEN_CASILLA, Y0 + MARGEN_CASILLA);
        glVertex2i((i + 1) * ANCHO_CELDA_INF - MARGEN_CASILLA, Y0 + MARGEN_CASILLA);
        glVertex2i((i + 1) * ANCHO_CELDA_INF - MARGEN_CASILLA, Y0 + ALTO_ZONA_INFERIOR_PX - MARGEN_CASILLA);
        glVertex2i(i * ANCHO_CELDA_INF + MARGEN_CASILLA, Y0 + ALTO_ZONA_INFERIOR_PX - MARGEN_CASILLA);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(i * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f, Y0 + ALTO_ZONA_INFERIOR_PX / 2.0f, 0.0f);
        dibujar_texto_stroke_centrado(ETIQUETAS_INFERIOR[i], (float)ANCHO_CELDA_INF - 6.0f);
        glPopMatrix();
    }

    glPopMatrix();
}

/* Divisores (Bresenham) de las dos franjas nuevas, mismo estilo que
   dibujar_lineas() para el grid de numeros. */
static void dibujar_lineas_zonas_especiales(void) {
    const int ANCHO_ZONA_DOCENA = 4 * CELDA_PX;
    const int ANCHO_CELDA_INF = (TABLERO_COLUMNAS * CELDA_PX) / 6;
    const int Y_TOP = 0;
    const int Y_MID = -ALTO_ZONA_DOCENA_PX;
    const int Y_BOTTOM = -(ALTO_ZONA_DOCENA_PX + ALTO_ZONA_INFERIOR_PX);
    int i;

    glPushMatrix();
    glTranslatef(-2.9f, 0.1f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);

    /* separadores horizontales: grid/docenas y docenas/franja inferior */
    dibujar_linea_bresenham(0, Y_TOP, TABLERO_COLUMNAS * CELDA_PX, Y_TOP);
    dibujar_linea_bresenham(0, Y_MID, TABLERO_COLUMNAS * CELDA_PX, Y_MID);
    dibujar_linea_bresenham(0, Y_BOTTOM, TABLERO_COLUMNAS * CELDA_PX, Y_BOTTOM);

    /* divisores verticales de las 3 docenas */
    for (i = 0; i <= 3; i++) {
        int x = i * ANCHO_ZONA_DOCENA;
        dibujar_linea_bresenham(x, Y_TOP, x, Y_MID);
    }
    /* divisores verticales de las 6 celdas de la franja inferior */
    for (i = 0; i <= 6; i++) {
        int x = i * ANCHO_CELDA_INF;
        dibujar_linea_bresenham(x, Y_MID, x, Y_BOTTOM);
    }

    glPopMatrix();
}

/* Capa 5: resaltado de la celda bajo el mouse (hover), usando
   celda_hover_col/celda_hover_fila (actualizadas desde mouse_mover()
   en main.c). Ademas del borde original, se agrega un relleno dorado
   semitransparente (mismo dorado que las fichas apostadas) para dar
   sensacion de "casilla activa" sin tapar el rojo/negro de fondo. */
static void dibujar_hover(void) {
    if (celda_hover_col < 0 || celda_hover_fila < 0) return; /* sin hover activo */

    glPushMatrix();
    glTranslatef(-2.9f, 0.13f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Relleno: dorado al 25% de opacidad, mismo tono que las fichas
       apostadas (1.0, 0.85, 0.0) para mantener consistencia visual. */
    glColor4f(1.0f, 0.85f, 0.0f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glEnd();

    glDisable(GL_BLEND);

    /* Borde original: blanco brillante, bien visible sobre rojo/negro */
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        celda_hover_fila * CELDA_PX + MARGEN_CASILLA);
    glVertex2i((celda_hover_col + 1) * CELDA_PX - MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glVertex2i(celda_hover_col * CELDA_PX + MARGEN_CASILLA,
        (celda_hover_fila + 1) * CELDA_PX - MARGEN_CASILLA);
    glEnd();

    glLineWidth(1.0f); /* restaurar grosor por defecto, para no afectar otras lineas */

    glPopMatrix();
}

/* Capa 3: numeros en blanco, centrados sobre cada casilla (mas el 0). */
static void dibujar_numeros(void) {
    int col, fila, numero;

    glPushMatrix();
    glTranslatef(-2.9f, 0.12f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(-ANCHO_CELDA_CERO_PX / 2.0f, (TABLERO_FILAS * CELDA_PX) / 2.0f, 0.0f);
    dibujar_numero_centrado(0);
    glPopMatrix();

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);

            glPushMatrix();
            glTranslatef(col * CELDA_PX + CELDA_PX / 2.0f,
                fila * CELDA_PX + CELDA_PX / 2.0f,
                0.0f);
            dibujar_numero_centrado(numero);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

/* Capa 4: fichas doradas sobre los numeros donde hay apuesta activa
   (incluyendo el 0). */
static void dibujar_fichas_apostadas(const Apuesta apuestas[], int cantidad) {
    int col, fila, numero, i, hay_ficha;

    glPushMatrix();
    glTranslatef(-2.9f, 0.14f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 0.85f, 0.0f); /* dorado, bien visible sobre rojo/negro/verde */

    /* Ficha sobre el 0, si hay apuesta ahi */
    hay_ficha = 0;
    for (i = 0; i < cantidad; i++) {
        if (apuestas[i].tipo == APUESTA_NUMERO && apuestas[i].valor == 0) {
            hay_ficha = 1;
            break;
        }
    }
    if (hay_ficha) {
        glPushMatrix();
        glTranslatef(-ANCHO_CELDA_CERO_PX / 2.0f, (TABLERO_FILAS * CELDA_PX) / 2.0f, 0.0f);
        {
            const int SEGMENTOS = 16;
            const float RADIO = 12.0f;
            int k;
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0.0f, 0.0f);
            for (k = 0; k <= SEGMENTOS; k++) {
                float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
                glVertex2f(cosf(ang) * RADIO, sinf(ang) * RADIO);
            }
            glEnd();
        }
        glPopMatrix();
    }

    for (col = 0; col < TABLERO_COLUMNAS; col++) {
        for (fila = 0; fila < TABLERO_FILAS; fila++) {
            numero = numero_de_celda(col, fila);
            hay_ficha = 0;
            for (i = 0; i < cantidad; i++) {
                if (apuestas[i].tipo == APUESTA_NUMERO && apuestas[i].valor == numero) {
                    hay_ficha = 1;
                    break;
                }
            }
            if (!hay_ficha) continue;

            glPushMatrix();
            glTranslatef(col * CELDA_PX + CELDA_PX / 2.0f,
                fila * CELDA_PX + CELDA_PX / 2.0f, 0.0f);
            {
                const int SEGMENTOS = 16;
                const float RADIO = 12.0f;
                int k;
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(0.0f, 0.0f);
                for (k = 0; k <= SEGMENTOS; k++) {
                    float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
                    glVertex2f(cosf(ang) * RADIO, sinf(ang) * RADIO);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }

    glPopMatrix();
}

/* Capa 4b: fichas doradas sobre las zonas especiales (docena, mitad,
   par/impar) donde hay una apuesta activa de ese tipo. Mismo estilo
   visual que dibujar_fichas_apostadas(), pero centrado en la celda de
   zona correspondiente en vez de en un numero del grid. */
static void dibujar_fichas_zonas_especiales(const Apuesta apuestas[], int cantidad) {
    const int ANCHO_ZONA_DOCENA = 4 * CELDA_PX;
    const int ANCHO_CELDA_INF = (TABLERO_COLUMNAS * CELDA_PX) / 6;
    const int Y_TOP = 0;
    const int Y_MID = -ALTO_ZONA_DOCENA_PX;
    int i;

    glPushMatrix();
    glTranslatef(-2.9f, 0.14f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glColor3f(1.0f, 0.85f, 0.0f); /* dorado, igual que las fichas del grid */

    for (i = 0; i < cantidad; i++) {
        float cx, cy;

        if (apuestas[i].tipo == APUESTA_DOCENA) {
            cx = (apuestas[i].valor - 1) * ANCHO_ZONA_DOCENA + ANCHO_ZONA_DOCENA / 2.0f;
            cy = Y_TOP + ALTO_ZONA_DOCENA_PX / 2.0f;
        }
        else if (apuestas[i].tipo == APUESTA_MITAD) {
            int indice = (apuestas[i].valor == 1) ? 0 : 5;
            cx = indice * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f;
            cy = Y_MID + ALTO_ZONA_INFERIOR_PX / 2.0f;
        }
        else if (apuestas[i].tipo == APUESTA_PAR_IMPAR) {
            int indice = (apuestas[i].valor == 0) ? 1 : 4; /* 0=PAR, 1=IMPAR */
            cx = indice * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f;
            cy = Y_MID + ALTO_ZONA_INFERIOR_PX / 2.0f;
        }
        else if (apuestas[i].tipo == APUESTA_COLOR) {
            int indice = (apuestas[i].valor == (int)COLOR_ROJO) ? 2 : 3; /* 2=ROJO, 3=NEGRO */
            cx = indice * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f;
            cy = Y_MID + ALTO_ZONA_INFERIOR_PX / 2.0f;
        }
        else {
            continue; /* APUESTA_NUMERO no se dibuja aca (ver dibujar_fichas_apostadas) */
        }

        glPushMatrix();
        glTranslatef(cx, cy, 0.0f);
        {
            const int SEGMENTOS = 16;
            const float RADIO = 10.0f;
            int k;
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0.0f, 0.0f);
            for (k = 0; k <= SEGMENTOS; k++) {
                float ang = (float)k / SEGMENTOS * 2.0f * 3.14159265f;
                glVertex2f(cosf(ang) * RADIO, sinf(ang) * RADIO);
            }
            glEnd();
        }
        glPopMatrix();
    }

    glPopMatrix();
}

void dibujar_tablero_apuestas(const Apuesta apuestas_activas[], int num_apuestas) {
    glDisable(GL_LIGHTING);

    dibujar_casillas();
    dibujar_zona_docenas();
    dibujar_zona_inferior();
    dibujar_lineas();
    dibujar_lineas_zonas_especiales();
    dibujar_numeros();
    dibujar_fichas_apostadas(apuestas_activas, num_apuestas);
    dibujar_fichas_zonas_especiales(apuestas_activas, num_apuestas);
    dibujar_hover();

    glEnable(GL_LIGHTING);
}

/* Celda actualmente resaltada (hover del mouse), compartida con el
   modulo de render (Dubenny) para que pueda dibujar el highlight
   visual correspondiente. -1 significa "ninguna celda". */
int celda_hover_col = -1;
int celda_hover_fila = -1;

void fijar_celda_hover(int col, int fila) {
    celda_hover_col = col;
    celda_hover_fila = fila;
}

/* Inversa de numero_de_celda(): dado un punto (x, z) del mundo (el que
   entrega obtener_punto_clic_en_mesa de Jeiferson), calcula a que
   columna/fila del grid corresponde.
   Es la transformacion inversa exacta de la que usa dibujar_casillas():
   ese glTranslatef(-2.9, 0.08, 4.9) + glRotatef(-90, X) + glScalef
   mapea un punto local (u, v) del grid a world = (u*ESCALA - 2.9, 0.08, -v*ESCALA + 4.9).
   Aqui despejamos u y v a partir de world (x, z). */
int obtener_celda_en_punto(float x, float z, int* col_out, int* fila_out) {
    float u = (x - (-2.9f)) / ESCALA_TABLERO;
    float v = (4.9f - z) / ESCALA_TABLERO;
    int col, fila;

    if (u < 0.0f || u >= (float)(TABLERO_COLUMNAS * CELDA_PX)) return 0;
    if (v < 0.0f || v >= (float)(TABLERO_FILAS * CELDA_PX)) return 0;

    col = (int)(u / CELDA_PX);
    fila = (int)(v / CELDA_PX);
    if (col >= TABLERO_COLUMNAS) col = TABLERO_COLUMNAS - 1;
    if (fila >= TABLERO_FILAS)   fila = TABLERO_FILAS - 1;

    *col_out = col;
    *fila_out = fila;
    return 1;
}

int obtener_numero_en_punto(float x, float z, int* numero_out) {
    float u = (x - (-2.9f)) / ESCALA_TABLERO;
    float v = (4.9f - z) / ESCALA_TABLERO;
    int col, fila;

    /* Celda del 0: u negativo, dentro del ancho de esa celda, mismo
       rango de v que el grid completo (las 3 filas). */
    if (u >= -(float)ANCHO_CELDA_CERO_PX && u < 0.0f
        && v >= 0.0f && v < (float)(TABLERO_FILAS * CELDA_PX)) {
        *numero_out = 0;
        return 1;
    }

    if (!obtener_celda_en_punto(x, z, &col, &fila)) return 0;
    *numero_out = numero_de_celda(col, fila);
    return 1;
}

/* Misma transformacion inversa que obtener_celda_en_punto(), pero
   extendida a las dos franjas de zonas especiales que estan debajo del
   grid de numeros (ver dibujar_zona_docenas/dibujar_zona_inferior). */
int obtener_zona_especial_en_punto(float x, float z, TipoApuesta* tipo_out, int* valor_out) {
    float u = (x - (-2.9f)) / ESCALA_TABLERO;
    float v = (4.9f - z) / ESCALA_TABLERO;
    const int ANCHO_ZONA_DOCENA = 4 * CELDA_PX;
    const int ANCHO_CELDA_INF = (TABLERO_COLUMNAS * CELDA_PX) / 6;
    const int Y_TOP = 0;
    const int Y_MID = -ALTO_ZONA_DOCENA_PX;
    const int Y_BOTTOM = -(ALTO_ZONA_DOCENA_PX + ALTO_ZONA_INFERIOR_PX);

    if (u < 0.0f || u >= (float)(TABLERO_COLUMNAS * CELDA_PX)) return 0;

    if (v >= (float)Y_MID && v < (float)Y_TOP) {
        int indice = (int)(u / ANCHO_ZONA_DOCENA);
        if (indice > 2) indice = 2;
        *tipo_out = APUESTA_DOCENA;
        *valor_out = indice + 1; /* 1, 2 o 3 */
        return 1;
    }

    if (v >= (float)Y_BOTTOM && v < (float)Y_MID) {
        int indice = (int)(u / ANCHO_CELDA_INF);
        if (indice > 5) indice = 5;
        switch (indice) {
        case 0: *tipo_out = APUESTA_MITAD;     *valor_out = 1; break;              /* 1-18 */
        case 1: *tipo_out = APUESTA_PAR_IMPAR; *valor_out = 0; break;              /* PAR */
        case 2: *tipo_out = APUESTA_COLOR;     *valor_out = (int)COLOR_ROJO; break; /* ROJO */
        case 3: *tipo_out = APUESTA_COLOR;     *valor_out = (int)COLOR_NEGRO; break;/* NEGRO */
        case 4: *tipo_out = APUESTA_PAR_IMPAR; *valor_out = 1; break;              /* IMPAR */
        default:*tipo_out = APUESTA_MITAD;     *valor_out = 2; break;              /* 19-36 */
        }
        return 1;
    }

    return 0; /* v < Y_TOP (dentro del grid) o v >= Y_BOTTOM (fuera de todo) */
}