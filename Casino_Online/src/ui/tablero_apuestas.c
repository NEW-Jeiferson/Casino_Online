// Implementacion del tablero de apuestas de la ruleta. //

#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tablero_apuestas.h"
#include "../utils/bresenham.h"
#include "../core/estado_juego.h"

 /* Configuracion de las dimensiones del grid principal de numeros (1-36). */
#define TABLERO_COLUMNAS 12   
#define TABLERO_FILAS     3
#define CELDA_PX         40   
#define ESCALA_TABLERO   0.012f 
#define MARGEN_CASILLA    2   

/* Dimensiones de las zonas de apuestas especiales (docenas, colores, pares, etc.). */
#define ALTO_ZONA_DOCENA_PX    40 
#define ALTO_ZONA_INFERIOR_PX  40  

/* Dimension de la celda del numero 0, ubicada a la izquierda del grid principal. */
#define ANCHO_CELDA_CERO_PX 30


static int numero_de_celda(int col, int fila) {
    return col * 3 + fila + 1;
}

/* Dibuja el texto centrado en el origen actual.
   Calcula la escala dinamicamente para asegurar que el texto nunca
   sobrepase el ancho maximo permitido */
static void dibujar_texto_stroke_centrado(const char* texto, float ancho_max) {
    int i, len;
    float ancho_total, alto_total;
    float escala;

    len = (int)strlen(texto);

    escala = 0.24f;
    ancho_total = len * 104.76f * escala;

   
    if (ancho_total > ancho_max) {
        escala *= ancho_max / ancho_total;
        ancho_total = ancho_max;
    }
    alto_total = 119.05f * escala;

    glPushMatrix();
    glTranslatef(-ancho_total / 2.0f, -alto_total / 2.0f, 0.0f);
    glScalef(escala, escala, 1.0f);

    
       
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

        glLineWidth(1.0f);
        if (!line_smooth_estaba_activo) glDisable(GL_LINE_SMOOTH);
        if (!blend_estaba_activo) glDisable(GL_BLEND);
    }
    glPopMatrix();
}

/* Dibuja un numero centrado dentro de las dimensiones de una celda estandar,
   dejando un margen de seguridad para que el texto no toque los bordes. */
static void dibujar_numero_centrado(int numero) {
    char texto[4];
    const float ANCHO_MAX = 34.0f; /* ancho disponible dentro de la celda (40px - margen) */

    snprintf(texto, sizeof(texto), "%d", numero);
    dibujar_texto_stroke_centrado(texto, ANCHO_MAX);
}

/* Renderiza el fondo solido de todas las celdas de apuestas en el grid 1-36 y el 0.
   Posiciona el plano en el mundo 3D y asigna el color correspondiente rojo/negro/verde. */
static void dibujar_casillas(void) {
    int col, fila, numero;
    ColorRuleta color;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

  
    glColor3f(0.0f, 0.42f, 0.15f);
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
                glColor3f(0.08f, 0.08f, 0.08f);

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

/* Dibuja la cuadricula estructural (las lineas blancas divisorias) del tapete.
   Utiliza el algoritmo de Bresenham para el trazado. */

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

 
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, 0, -ANCHO_CELDA_CERO_PX, TABLERO_FILAS * CELDA_PX);
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, 0, 0, 0);
    dibujar_linea_bresenham(-ANCHO_CELDA_CERO_PX, TABLERO_FILAS * CELDA_PX, 0, TABLERO_FILAS * CELDA_PX);

    glPopMatrix();
}

/* Renderiza la franja de apuestas de las docenas (1ra, 2da y 3ra docena).
   Esta zona se ubica geometricamente debajo del grid principal de numeros. */

static void dibujar_zona_docenas(void) {
    static const char* ETIQUETAS_DOCENA[3] = { "1ra 12", "2da 12", "3ra 12" };
    const int ANCHO_ZONA = 4 * CELDA_PX;
    const int Y0 = -ALTO_ZONA_DOCENA_PX;
    int i;

    glPushMatrix();
    glTranslatef(-2.9f, 0.08f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (i = 0; i < 3; i++) {
        glColor3f(0.05f, 0.32f, 0.10f);
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

/* Renderiza la franja mas baja del tablero, correspondiente a las apuestas
   de probabilidad simple (Mitades, Par/Impar, Rojo/Negro). */

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
        if (i == 2) glColor3f(0.75f, 0.08f, 0.08f);       /* ROJO: mismo tono que las casillas rojas */
        else if (i == 3) glColor3f(0.08f, 0.08f, 0.08f); /* NEGRO: mismo tono que las casillas negras */
        else glColor3f(0.10f, 0.10f, 0.34f);             /* resto: azul oscuro */

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

/* Traza las lineas separadoras (Bresenham) exclusivas para las franjas de
   apuestas especiales  */

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


    dibujar_linea_bresenham(0, Y_TOP, TABLERO_COLUMNAS * CELDA_PX, Y_TOP);
    dibujar_linea_bresenham(0, Y_MID, TABLERO_COLUMNAS * CELDA_PX, Y_MID);
    dibujar_linea_bresenham(0, Y_BOTTOM, TABLERO_COLUMNAS * CELDA_PX, Y_BOTTOM);

 
    for (i = 0; i <= 3; i++) {
        int x = i * ANCHO_ZONA_DOCENA;
        dibujar_linea_bresenham(x, Y_TOP, x, Y_MID);
    }
    

    for (i = 0; i <= 6; i++) {
        int x = i * ANCHO_CELDA_INF;
        dibujar_linea_bresenham(x, Y_MID, x, Y_BOTTOM);
    }

    glPopMatrix();
}

/* Dibuja un efecto de resaltado  semitransparente sobre la casilla
   que actualmente esta apuntando el cursor del raton. Da feedback visual al usuario. */

static void dibujar_hover(void) {
    if (celda_hover_col < 0 || celda_hover_fila < 0) return; 

    glPushMatrix();
    glTranslatef(-2.9f, 0.13f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   
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

    glLineWidth(1.0f); 

    glPopMatrix();
}

// Representa la capa textual de numeros. Recorre todo el grid principal 
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

static void aplicar_color_ficha(float monto) {
    if (monto <= 10.0f) {
        glColor3f(1.0f, 0.9f, 0.0f);   /* Amarillo */
    } else if (monto <= 25.0f) {
        glColor3f(0.0f, 0.7f, 0.1f);   /* Verde */
    } else if (monto <= 50.0f) {
        glColor3f(0.1f, 0.4f, 0.9f);   /* Azul */
    } else {
        glColor3f(0.6f, 0.1f, 0.7f);   /* Morado (para >50) */
    }
}

/* Representa la capa superior de las fichas. Dibuja discos con colores segun su valor
   y apilados tridimensionalmente sobre las casillas numericas 0-36. */
static void dibujar_fichas_apostadas(const Apuesta apuestas[], int cantidad) {
    int i, j, count;
    float cx, cy;
    float offset_x, offset_y, offset_z;

    glPushMatrix();
    glTranslatef(-2.9f, 0.14f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (i = 0; i < cantidad; i++) {
        if (apuestas[i].tipo != APUESTA_NUMERO) continue;

        /* Contar apuestas previas en la misma casilla para el apilamiento */
        count = 0;
        for (j = 0; j < i; j++) {
            if (apuestas[j].tipo == APUESTA_NUMERO && apuestas[j].valor == apuestas[i].valor) {
                count++;
            }
        }
        if (count > 5) count = 5; /* Limite visual de apilado */

        offset_x = (float)count * 1.5f;
        offset_y = (float)count * 1.5f;
        offset_z = (float)count * 0.02f; /* Unidades de mundo ya que Z no esta escalado */

        if (apuestas[i].valor == 0) {
            cx = -ANCHO_CELDA_CERO_PX / 2.0f;
            cy = (TABLERO_FILAS * CELDA_PX) / 2.0f;
        } else {
            int col = (apuestas[i].valor - 1) / 3;
            int fila = (apuestas[i].valor - 1) % 3;
            cx = col * CELDA_PX + CELDA_PX / 2.0f;
            cy = fila * CELDA_PX + CELDA_PX / 2.0f;
        }

        glPushMatrix();
        glTranslatef(cx + offset_x, cy + offset_y, offset_z);
        aplicar_color_ficha(apuestas[i].monto);
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

    glPopMatrix();
}

/* Dibuja discos con colores segun su valor y apilados tridimensionalmente 
   sobre las franjas de apuestas especiales. */
static void dibujar_fichas_zonas_especiales(const Apuesta apuestas[], int cantidad) {
    const int ANCHO_ZONA_DOCENA = 4 * CELDA_PX;
    const int ANCHO_CELDA_INF = (TABLERO_COLUMNAS * CELDA_PX) / 6;
    const int Y_TOP = 0;
    const int Y_MID = -ALTO_ZONA_DOCENA_PX;
    int i, j, count;
    float cx, cy;
    float offset_x, offset_y, offset_z;

    glPushMatrix();
    glTranslatef(-2.9f, 0.14f, 4.9f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(ESCALA_TABLERO, ESCALA_TABLERO, 1.0f);

    for (i = 0; i < cantidad; i++) {
        if (apuestas[i].tipo == APUESTA_NUMERO) continue;

        /* Contar apuestas previas en la misma zona especial */
        count = 0;
        for (j = 0; j < i; j++) {
            if (apuestas[j].tipo == apuestas[i].tipo && apuestas[j].valor == apuestas[i].valor) {
                count++;
            }
        }
        if (count > 5) count = 5;

        offset_x = (float)count * 1.5f;
        offset_y = (float)count * 1.5f;
        offset_z = (float)count * 0.02f;

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
            int indice = (apuestas[i].valor == 0) ? 1 : 4;
            cx = indice * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f;
            cy = Y_MID + ALTO_ZONA_INFERIOR_PX / 2.0f;
        }
        else if (apuestas[i].tipo == APUESTA_COLOR) {
            int indice = (apuestas[i].valor == (int)COLOR_ROJO) ? 2 : 3;
            cx = indice * ANCHO_CELDA_INF + ANCHO_CELDA_INF / 2.0f;
            cy = Y_MID + ALTO_ZONA_INFERIOR_PX / 2.0f;
        }
        else {
            continue;
        }

        glPushMatrix();
        glTranslatef(cx + offset_x, cy + offset_y, offset_z);
        aplicar_color_ficha(apuestas[i].monto);
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

/* Orquestador visual del tapete. Apaga la iluminacion para mantener los
   colores exactos 2D sobre el plano 3D y ejecuta secuencialmente todas
   las capas de renderizado del tablero de apuestas. */

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

// Variables globales internas para llevar registro de que celda (col, fila)
   
int celda_hover_col = -1;
int celda_hover_fila = -1;

/* Metodo para actualizar el estado del hover en el tapete numérico.
   Es invocado por el manejador de eventos del ratón. */
void fijar_celda_hover(int col, int fila) {
    celda_hover_col = col;
    celda_hover_fila = fila;
}

// Mapeo inverso de las coordenadas del mundo 3D (x, z) a coordenadas de tablero 2D.
//  Deshace la traslacion y el escalado aplicados en las funciones de dibujado.
   
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

/* Identifica si el jugador ha hecho clic en algun numero especifico del tapete.
   Evalua primero la condicion asimetrica del 0, y si no acierta, delega el
   proceso a obtener_celda_en_punto() para calcular los numeros del 1 al 36. */

int obtener_numero_en_punto(float x, float z, int* numero_out) {
    float u = (x - (-2.9f)) / ESCALA_TABLERO;
    float v = (4.9f - z) / ESCALA_TABLERO;
    int col, fila;

    if (u >= -(float)ANCHO_CELDA_CERO_PX && u < 0.0f
        && v >= 0.0f && v < (float)(TABLERO_FILAS * CELDA_PX)) {
        *numero_out = 0;
        return 1;
    }

    if (!obtener_celda_en_punto(x, z, &col, &fila)) return 0;
    *numero_out = numero_de_celda(col, fila);
    return 1;
}

// Similar a obtener_numero_en_punto, pero enfocado a detectar clics en las
// zonas inferiores que no representan numeros unicos 

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
        *valor_out = indice + 1; 
        return 1;
    }

    // Deteccion de click en la banda Inferior 
    if (v >= (float)Y_BOTTOM && v < (float)Y_MID) {
        int indice = (int)(u / ANCHO_CELDA_INF);
        if (indice > 5) indice = 5;
        switch (indice) {
        case 0: *tipo_out = APUESTA_MITAD;     *valor_out = 1; break;              
        case 1: *tipo_out = APUESTA_PAR_IMPAR; *valor_out = 0; break;              
        case 2: *tipo_out = APUESTA_COLOR;     *valor_out = (int)COLOR_ROJO; break; 
        case 3: *tipo_out = APUESTA_COLOR;     *valor_out = (int)COLOR_NEGRO; break;
        case 4: *tipo_out = APUESTA_PAR_IMPAR; *valor_out = 1; break;              
        default:*tipo_out = APUESTA_MITAD;     *valor_out = 2; break;              
        }
        return 1;
    }

    return 0;
}