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
#include "ruleta_geometria.h"
#include "../render/materiales.h"
#include "../utils/bezier.h"

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