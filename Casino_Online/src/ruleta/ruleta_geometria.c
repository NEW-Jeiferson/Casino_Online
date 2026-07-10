/*
 * ruleta_geometria.c
 * Implementacion de la geometria de la rueda. Ver ruleta_geometria.h.
 *
 * VERSION CON JERARQUIA REAL: dibujar_rueda() ya no gestiona su propia
 * traslacion/rotacion de nodo. Quien la llama (main.c -> display()) es
 * responsable de dejar la matriz ModelView posicionada antes de
 * invocarla, y de mantenerla activa mientras se dibujan sus hijos
 * (la bolita), para lograr una jerarquia real con la pila de matrices
 * (mesa -> rueda -> bolita), en vez de objetos independientes.
 */
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ruleta_geometria.h"
#include "../render/materiales.h"
#include "../utils/bezier.h"
#include "../core/estado_juego.h" /* ORDEN_RUEDA_EUROPEA, color_de_numero: para la pista numerada */

 /* RADIO_MESA ahora se expone en ruleta_geometria.h (Luis la necesita
    para el mapeo de posicion 3D a celda del tablero de apuestas) */
#define PERFIL_SEGMENTOS     12   /* puntos a lo largo del perfil (radio-altura) */
#define REVOLUCION_SEGMENTOS 36   /* divisiones angulares (360 / 36 = 10 grados) */
#define PI_GEOMETRIA         3.14159265358979323846f

    /* ------------------------------------------------------------------- */
    /* Datos de la malla generada.                                          */
    /* perfil_puntos[i].x = radio, perfil_puntos[i].y = altura (el campo z  */
    /* del perfil no se usa, se deja en 0).                                 */
    /* ------------------------------------------------------------------- */
static Punto3D perfil_puntos[PERFIL_SEGMENTOS];
static Punto3D malla_vertices[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];
static Punto3D malla_normales[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];

/* ------------------------------------------------------------------- */
/* Utilidades vectoriales pequenas (solo usadas aqui).                  */
/* ------------------------------------------------------------------- */
static Punto3D producto_cruz(Punto3D a, Punto3D b) {
    Punto3D r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

static Punto3D normalizar_vector(Punto3D v) {
    float longitud;
    Punto3D r;

    longitud = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (longitud < 0.00001f) {
        r.x = 0.0f; r.y = 1.0f; r.z = 0.0f;
        return r;
    }
    r.x = v.x / longitud;
    r.y = v.y / longitud;
    r.z = v.z / longitud;
    return r;
}

/* ------------------------------------------------------------------- */

void generar_perfil_bezier_rueda(void) {
    Punto3D p0, p1, p2, p3;
    int i;
    float t;

    p0.x = 0.0f;                        p0.y = 0.35f; p0.z = 0.0f;
    p1.x = RADIO_EXTERIOR_RUEDA * 0.35f; p1.y = 0.40f; p1.z = 0.0f;
    p2.x = RADIO_EXTERIOR_RUEDA * 0.75f; p2.y = 0.10f; p2.z = 0.0f;
    p3.x = RADIO_EXTERIOR_RUEDA;         p3.y = 0.22f; p3.z = 0.0f;

    for (i = 0; i < PERFIL_SEGMENTOS; i++) {
        t = (float)i / (float)(PERFIL_SEGMENTOS - 1);
        perfil_puntos[i] = evaluar_bezier_cubica(p0, p1, p2, p3, t);
    }
}

void construir_malla_rueda(void) {
    int i, j;
    float angulo, radio, altura, dradio, daltura;
    Punto3D tangente_perfil, tangente_circ, normal;

    for (i = 0; i < PERFIL_SEGMENTOS; i++) {
        radio = perfil_puntos[i].x;
        altura = perfil_puntos[i].y;

        if (i == 0) {
            dradio = perfil_puntos[i + 1].x - perfil_puntos[i].x;
            daltura = perfil_puntos[i + 1].y - perfil_puntos[i].y;
        }
        else if (i == PERFIL_SEGMENTOS - 1) {
            dradio = perfil_puntos[i].x - perfil_puntos[i - 1].x;
            daltura = perfil_puntos[i].y - perfil_puntos[i - 1].y;
        }
        else {
            dradio = perfil_puntos[i + 1].x - perfil_puntos[i - 1].x;
            daltura = perfil_puntos[i + 1].y - perfil_puntos[i - 1].y;
        }

        for (j = 0; j < REVOLUCION_SEGMENTOS; j++) {
            angulo = (float)j * (2.0f * PI_GEOMETRIA / (float)REVOLUCION_SEGMENTOS);

            malla_vertices[i][j].x = radio * cosf(angulo);
            malla_vertices[i][j].y = altura;
            malla_vertices[i][j].z = radio * sinf(angulo);

            tangente_perfil.x = dradio * cosf(angulo);
            tangente_perfil.y = daltura;
            tangente_perfil.z = dradio * sinf(angulo);

            tangente_circ.x = -sinf(angulo);
            tangente_circ.y = 0.0f;
            tangente_circ.z = cosf(angulo);

            /* Orden confirmado: tangente_circ x tangente_perfil da la
               normal apuntando hacia afuera de la superficie (ver
               bug documentado: el orden opuesto daba normales
               invertidas y la rueda se veia negra/sin luz). */
            normal = producto_cruz(tangente_circ, tangente_perfil);
            malla_normales[i][j] = normalizar_vector(normal);
        }
    }
}

void dibujar_mesa(void) {
    glPushMatrix();

    aplicar_material(MATERIAL_FIELTRO);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-RADIO_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, -RADIO_MESA);
    glEnd();

    glPopMatrix();
}

void dibujar_rueda(void) {
    int i, j, jj;

    /* Ya NO hace glPushMatrix/glTranslatef/glRotatef/glPopMatrix aqui:
       el llamador (main.c) posiciona el nodo "rueda" antes de invocar
       esta funcion, y mantiene esa matriz activa para que la bolita
       (dibujada despues, como hijo) herede la misma transformacion. */

    aplicar_material(MATERIAL_METAL);

    for (i = 0; i < PERFIL_SEGMENTOS - 1; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= REVOLUCION_SEGMENTOS; j++) {
            jj = j % REVOLUCION_SEGMENTOS;

            glNormal3f(malla_normales[i + 1][jj].x, malla_normales[i + 1][jj].y, malla_normales[i + 1][jj].z);
            glVertex3f(malla_vertices[i + 1][jj].x, malla_vertices[i + 1][jj].y, malla_vertices[i + 1][jj].z);

            glNormal3f(malla_normales[i][jj].x, malla_normales[i][jj].y, malla_normales[i][jj].z);
            glVertex3f(malla_vertices[i][jj].x, malla_vertices[i][jj].y, malla_vertices[i][jj].z);
        }
        glEnd();
    }
}

void dibujar_vidrio_protector(void) {
    glPushMatrix();

    glTranslatef(0.0f, 0.05f, 0.0f);

    aplicar_material(MATERIAL_VIDRIO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glScalef(RADIO_EXTERIOR_RUEDA + 0.3f, 0.35f, RADIO_EXTERIOR_RUEDA + 0.3f);
    glutSolidSphere(1.0, 24, 24);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glPopMatrix();
}

/* ------------------------------------------------------------------- */
/* Pista numerada (Opcion A: colores geometricos por sector + numeros   */
/* rectos, sin textura). Se llama justo despues de dibujar_rueda(),     */
/* mientras la matriz del nodo "rueda" sigue activa, para que la pista  */
/* gire junto con la rueda en vez de quedarse fija.                     */
/*                                                                       */
/* Usa ORDEN_RUEDA_EUROPEA y color_de_numero() de estado_juego.h        */
/* (Luis) -misma fuente de verdad que usa main.c para decidir el       */
/* numero ganador, asi que lo que se VE en la rueda siempre coincide    */
/* con lo que CUENTA como resultado. No hay copia local de estos datos. */
/* ------------------------------------------------------------------- */

/* Radios de la banda donde va la pista (entre el domo central y el
   labio del borde). Ajustados a ojo contra el perfil de Bezier actual
   (p1 en radio ~1.05, p2 en radio ~2.25, p3 -borde- en radio 3.0);
   recalibrar viendo la escena real si hace falta. */
#define RADIO_INTERNO_PISTA 1.6f
#define RADIO_EXTERNO_PISTA 2.6f

   /* Altura a la que se dibuja la pista: justo por encima del punto mas
      alto del perfil en esa banda (p1.y = 0.40), para que no quede
      "enterrada" dentro de la superficie curva en ningun punto. */
#define ALTURA_PISTA 0.42f

static void dibujar_numero_pista(int numero) {
    char texto[4];
    int i, len;

    snprintf(texto, sizeof(texto), "%d", numero);
    len = (int)strlen(texto);

    glPushMatrix();
    glScalef(0.0022f, 0.0022f, 1.0f); /* la fuente stroke es "grande" por defecto */
    glTranslatef(-(float)len * 52.0f, 0.0f, 0.0f); /* centrar aproximadamente */
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();
}

void dibujar_pista_numerada(void) {
    int sector;
    const float paso_angular = 360.0f / 37.0f;

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    /* FIX: el winding del triangle strip de la banda queda invertido
       (normal calculada hacia -Y en vez de +Y), por lo que GL_CULL_FACE
       (GL_BACK, activo globalmente en main.c) lo descartaba entero -de
       ahi que solo se vieran los numeros (son GL_LINE, no los afecta
       el culling). Se desactiva culling solo para esta banda en vez
       de invertir el orden de vertices, para no arriesgar romper el
       calculo de normal que ya usa glNormal3f(0,1,0). */
    glDisable(GL_CULL_FACE);

    for (sector = 0; sector < 37; sector++) {
        int numero = ORDEN_RUEDA_EUROPEA[sector];
        ColorRuleta color = color_de_numero(numero);
        float angulo_inicio = sector * paso_angular;
        float angulo_fin = angulo_inicio + paso_angular;
        int segmentos_arco = 4;
        int k;

        if (color == COLOR_VERDE)      glColor3f(0.0f, 0.5f, 0.15f);
        else if (color == COLOR_ROJO)  glColor3f(0.75f, 0.08f, 0.08f);
        else                            glColor3f(0.05f, 0.05f, 0.05f);

        glBegin(GL_TRIANGLE_STRIP);
        for (k = 0; k <= segmentos_arco; k++) {
            float t = (float)k / segmentos_arco;
            float ang = (angulo_inicio + t * (angulo_fin - angulo_inicio)) * PI_GEOMETRIA / 180.0f;
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(RADIO_INTERNO_PISTA * cosf(ang), ALTURA_PISTA, RADIO_INTERNO_PISTA * sinf(ang));
            glVertex3f(RADIO_EXTERNO_PISTA * cosf(ang), ALTURA_PISTA, RADIO_EXTERNO_PISTA * sinf(ang));
        }
        glEnd();

        {
            float ang_medio = (angulo_inicio + paso_angular / 2.0f) * PI_GEOMETRIA / 180.0f;
            float radio_medio = (RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f;

            glColor3f(1.0f, 1.0f, 1.0f);
            glPushMatrix();
            glTranslatef(radio_medio * cosf(ang_medio), ALTURA_PISTA + 0.001f, radio_medio * sinf(ang_medio));
            glRotatef(-(angulo_inicio + paso_angular / 2.0f) + 90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            dibujar_numero_pista(numero);
            glPopMatrix();
        }
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);
}