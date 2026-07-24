/*
 * tragamonedas_geometria.c
 * Implementacion de la geometria del tragamonedas. Ver tragamonedas_geometria.h.
 * A CARGO DE: Luis.
 */
#include <GL/glut.h>
#include <string.h>
#include "tragamonedas_geometria.h"

/* PLACEHOLDER: dibuja un gabinete simple (una caja) con un cartel de
   texto arriba, solo para confirmar que el modulo compila y que algo
   aparece en pantalla. Reemplazar por la geometria real (rodillos,
   simbolos, palanca) a medida que se construya -no hace falta tocar
   nada fuera de este archivo mientras eso pasa. */
static void dibujar_texto_stroke_simple(const char* texto, float escala) {
    int i;
    int len = (int)strlen(texto);

    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-(float)len * 52.38f, 0.0f, 0.0f); /* centrado aproximado, mismo criterio que ruleta_geometria.c */
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();
}

void dibujar_tragamonedas(void) {
    GLboolean iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);

    /* Gabinete placeholder: una caja simple centrada en el origen */
    glPushMatrix();
    glColor3f(0.25f, 0.05f, 0.05f); /* rojo oscuro, solo para distinguirlo visualmente de la ruleta mientras es un placeholder */
    glScalef(2.0f, 3.0f, 1.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    /* Cartel de texto arriba del gabinete, mismo estilo (fuente stroke,
       sin iluminacion para que se vea siempre blanco y nitido) que ya
       se usa para los numeros de la rueda. */
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 2.2f, 0.0f);
    glScalef(0.01f, 0.01f, 1.0f);
    dibujar_texto_stroke_simple("TRAGAMONEDAS - EN CONSTRUCCION", 1.0f);
    glPopMatrix();

    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}
