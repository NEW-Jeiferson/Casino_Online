/*
* Implementacion de la geometria de la rueda
*/
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ruleta_geometria.h"
#include "../render/materiales.h"
#include "../utils/bezier.h"
#include "../core/estado_juego.h" 


#define PERFIL_SEGMENTOS     12   
#define REVOLUCION_SEGMENTOS 36   
#define PI_GEOMETRIA         3.14159265358979323846f


/* Datos para la malla de la rueda */
static Punto3D perfil_puntos[PERFIL_SEGMENTOS];
static Punto3D malla_vertices[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];
static Punto3D malla_normales[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];


/* Funciones auxiliares para calculos de vectores 3D */
static Punto3D producto_cruz(Punto3D a, Punto3D b) {
    Punto3D r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}


/* Sirve para normalizar un vector 3D, el cual debe ser no nulo */
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


/* Sirve para obtener la altura de la superficie en un radio dado, se usa para calcular la posición vertical de los vértices de la malla */
float altura_superficie_en_radio(float radio) {
    int i;

    if (radio <= perfil_puntos[0].x) return perfil_puntos[0].y;
    if (radio >= perfil_puntos[PERFIL_SEGMENTOS - 1].x) return perfil_puntos[PERFIL_SEGMENTOS - 1].y;

    for (i = 0; i < PERFIL_SEGMENTOS - 1; i++) {
        float r0 = perfil_puntos[i].x;
        float r1 = perfil_puntos[i + 1].x;

        if (radio >= r0 && radio <= r1) {
            float t = (r1 - r0 < 0.00001f) ? 0.0f : (radio - r0) / (r1 - r0);
            return perfil_puntos[i].y + t * (perfil_puntos[i + 1].y - perfil_puntos[i].y);
        }
    }
    return perfil_puntos[PERFIL_SEGMENTOS - 1].y;
}


/* Genera el perfil de la rueda usando una curva de Bezier cúbica, se usa para calcular la posición horizontal de los vértices de la malla */
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


/* Construye la malla de la rueda a partir del perfil generado, calculando los vértices y normales para cada segmento de revolución */
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

            normal = producto_cruz(tangente_circ, tangente_perfil);
            malla_normales[i][j] = normalizar_vector(normal);
        }
    }
}


/* Dibuja la mesa de la ruleta, incluyendo el fieltro y el borde de madera */
void dibujar_mesa(void) {

    const float BORDE_MESA = 0.5f;

    glPushMatrix();

    aplicar_material(MATERIAL_FIELTRO);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-RADIO_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, -RADIO_MESA);
    glEnd();

    aplicar_material(MATERIAL_MADERA);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

	/* Borde superior (incluye las otras 2 esquinas) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA - BORDE_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA - BORDE_MESA);


    /* Borde inferior (incluye las otras 2 esquinas) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA + BORDE_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA + BORDE_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA);


    /* Borde izquierdo (solo el tramo central, las esquinas ya las
       cubrieron los dos quads de arriba) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, -RADIO_MESA);


    /* Borde derecho */
    glVertex3f(RADIO_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA);

    glEnd();

    glPopMatrix();
}


/* Dibuja la rueda de la ruleta usando la malla generada, aplicando el material metálico y las normales calculadas para cada vértice */
void dibujar_rueda(void) {
    int i, j, jj;

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


/* Dibuja el vidrio protector de la rueda, aplicando un material transparente y habilitando el blending para lograr el efecto de transparencia */
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


/* Dibuja los números de la pista de la ruleta, centrados en cada sector y escalados para que quepan dentro del arco correspondiente */
#define ESCALA_NUMERO_PISTA_BASE 0.0024f
static float ancho_max_numero_pista(void) {
    const float paso_angular = 360.0f / 37.0f;
    const float radio_medio = (RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f;
    return 2.0f * radio_medio * sinf((paso_angular / 2.0f) * PI_GEOMETRIA / 180.0f) * 0.95f;
}


/* Dibuja un número de la pista de la ruleta, centrado y escalado para que quepa dentro del arco correspondiente */
static void dibujar_numero_pista(int numero) {
    char texto[4];
    int i, len;
    GLboolean iluminacion_estaba_activa;
    float escala, ancho_total;

    snprintf(texto, sizeof(texto), "%d", numero);
    len = (int)strlen(texto);


    iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);


	/* Calcula la escala necesaria para que el número quepa dentro del arco correspondiente, ajustando si es necesario para no exceder el ancho máximo permitido */
    escala = ESCALA_NUMERO_PISTA_BASE;
    ancho_total = (float)len * 104.76f * escala;
    {
        float ancho_max = ancho_max_numero_pista();
        if (ancho_total > ancho_max) {
            escala *= ancho_max / ancho_total;  
            ancho_total = ancho_max;
        }
    }


	/* Ajusta el grosor de la línea según la escala, para que los números se vean consistentes en tamaño y grosor */
    {
        float grosor = 2.5f * (escala / ESCALA_NUMERO_PISTA_BASE);
        if (grosor < 1.1f) grosor = 1.1f;
        glLineWidth(grosor);
    }


	/* Dibuja cada caracter del número usando glutStrokeCharacter, escalando y centrando el texto en el arco correspondiente */
    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-(float)len * 52.38f, -59.5f, 0.0f);
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();

    glLineWidth(1.0f);
    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}

#define OFFSET_PISTA 0.006f


/* Dibuja la pista numerada de la ruleta, con los sectores de color correspondientes y los números centrados en cada sector */
void dibujar_pista_numerada(void) {
    int sector;
    const float paso_angular = 360.0f / 37.0f;
    GLboolean line_smooth_estaba_activo;
    GLboolean blend_estaba_activo;

    float altura_interna = altura_superficie_en_radio(RADIO_INTERNO_PISTA) + OFFSET_PISTA;
    float altura_externa = altura_superficie_en_radio(RADIO_EXTERNO_PISTA) + OFFSET_PISTA;

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glDisable(GL_CULL_FACE);



	/* Habilita suavizado de líneas y blending para que los números se vean más suaves y legibles, guardando el estado previo para restaurarlo después */
    line_smooth_estaba_activo = glIsEnabled(GL_LINE_SMOOTH);
    blend_estaba_activo = glIsEnabled(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

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


		/* Dibuja el sector de color correspondiente al número, usando un GL_TRIANGLE_STRIP para crear la banda de color entre el radio interno y externo de la pista */
        glBegin(GL_TRIANGLE_STRIP);
        for (k = 0; k <= segmentos_arco; k++) {
            float t = (float)k / segmentos_arco;
            float ang = (angulo_inicio + t * (angulo_fin - angulo_inicio)) * PI_GEOMETRIA / 180.0f;
            glNormal3f(0.0f, 1.0f, 0.0f);

            glVertex3f(RADIO_INTERNO_PISTA * cosf(ang), altura_interna, -RADIO_INTERNO_PISTA * sinf(ang));
            glVertex3f(RADIO_EXTERNO_PISTA * cosf(ang), altura_externa, -RADIO_EXTERNO_PISTA * sinf(ang));
        }
        glEnd();

        {
			/* Calcula la posición y orientación del número centrado en el sector correspondiente, usando la altura media de la pista y el radio medio para posicionarlo correctamente */
            float ang_medio = (angulo_inicio + paso_angular / 2.0f) * PI_GEOMETRIA / 180.0f;
            float radio_medio = (RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f;
            float altura_numero = (altura_interna + altura_externa) / 2.0f + OFFSET_PISTA * 2.0f;


			/* Dibuja el número centrado en el sector correspondiente, deshabilitando la iluminación para que se vea correctamente y restaurando el estado después */
            glColor3f(1.0f, 1.0f, 1.0f);
            glPushMatrix();
            glTranslatef(radio_medio * cosf(ang_medio), altura_numero, -radio_medio * sinf(ang_medio));
            glRotatef((angulo_inicio + paso_angular / 2.0f) + 90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glDisable(GL_LIGHTING);
            dibujar_numero_pista(numero);
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }


	/* Restaura el estado previo de OpenGL, reactivando el culling, deshabilitando el color material y restaurando el estado de suavizado de líneas y blending según corresponda */
    glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);
    if (!line_smooth_estaba_activo) glDisable(GL_LINE_SMOOTH);
    if (!blend_estaba_activo) glDisable(GL_BLEND);
}