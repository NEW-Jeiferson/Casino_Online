#include <GL/glut.h>
#include <math.h>
#include "dados_geometria.h"
#include "../render/materiales.h"

static void aplicar_material_plastico_rojo(void) {
    GLfloat ambient[] = { 0.3f, 0.05f, 0.05f, 1.0f };
    GLfloat diffuse[] = { 0.8f, 0.1f, 0.1f, 1.0f };
    GLfloat specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat shininess[] = { 70.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

static void aplicar_material_panio_verde(void) {
    aplicar_material(MATERIAL_FIELTRO);
}

/* Dibuja un punto (circulo) en la cara del dado */
static void dibujar_punto(float x, float y, float z, float normal_x, float normal_y, float normal_z) {
    int segmentos = 12;
    float radio = 0.08f;
    int i;
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    /* Orientar el circulo segun la normal de la cara */
    if (normal_z > 0.5f) { } /* Ya esta orientado en XY */
    else if (normal_z < -0.5f) { glRotatef(180.0f, 0.0f, 1.0f, 0.0f); }
    else if (normal_x > 0.5f) { glRotatef(90.0f, 0.0f, 1.0f, 0.0f); }
    else if (normal_x < -0.5f) { glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); }
    else if (normal_y > 0.5f) { glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); }
    else if (normal_y < -0.5f) { glRotatef(90.0f, 1.0f, 0.0f, 0.0f); }

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.001f); /* Ligeramente por delante de la cara */
    for (i = 0; i <= segmentos; i++) {
        float angulo = 2.0f * 3.14159265f * (float)i / (float)segmentos;
        glVertex3f(cosf(angulo) * radio, sinf(angulo) * radio, 0.001f);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

/* Dibuja una cara cuadrada del dado con sus puntos respectivos (1 a 6) */
static void dibujar_cara(int valor, float normal_x, float normal_y, float normal_z) {
    glColor3f(0.8f, 0.1f, 0.1f); /* Rojo normal */
    glBegin(GL_QUADS);
    glNormal3f(normal_x, normal_y, normal_z);
    
    /* Dependiendo de la normal, dibujamos el quad */
    if (normal_z > 0.5f) { /* Frente */
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
    } else if (normal_z < -0.5f) { /* Atras */
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
    } else if (normal_x > 0.5f) { /* Derecha */
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
    } else if (normal_x < -0.5f) { /* Izquierda */
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
    } else if (normal_y > 0.5f) { /* Arriba */
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
    } else if (normal_y < -0.5f) { /* Abajo */
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
    }
    glEnd();

    /* Puntos */
    float off = 0.25f;
    float z_face = 0.5f * normal_z;
    float x_face = 0.5f * normal_x;
    float y_face = 0.5f * normal_y;
    
    if (valor == 1 || valor == 3 || valor == 5) {
        dibujar_punto(x_face, y_face, z_face, normal_x, normal_y, normal_z); /* Centro */
    }
    if (valor > 1) {
        float dx = (normal_x != 0.0f) ? 0.0f : off;
        float dy = (normal_y != 0.0f) ? 0.0f : off;
        float dz = (normal_z != 0.0f) ? 0.0f : ((normal_y != 0.0f) ? off : 0.0f);
        if (normal_x != 0.0f) { dz = off; }
        
        dibujar_punto(x_face - dx, y_face - dy, z_face - dz, normal_x, normal_y, normal_z);
        dibujar_punto(x_face + dx, y_face + dy, z_face + dz, normal_x, normal_y, normal_z);
    }
    if (valor > 3) {
        float dx = (normal_x != 0.0f) ? 0.0f : off;
        float dy = (normal_y != 0.0f) ? 0.0f : -off;
        float dz = (normal_z != 0.0f) ? 0.0f : ((normal_y != 0.0f) ? off : 0.0f);
        if (normal_x != 0.0f) { dz = -off; dy = off; }
        else if (normal_y != 0.0f) { dz = off; dx = -off; }
        else { dy = off; dx = -off; }
        
        dibujar_punto(x_face + dx, y_face + dy, z_face + dz, normal_x, normal_y, normal_z);
        dibujar_punto(x_face - dx, y_face - dy, z_face - dz, normal_x, normal_y, normal_z);
    }
    if (valor == 6) {
        float dx = (normal_x != 0.0f) ? 0.0f : off;
        float dy = 0.0f;
        float dz = (normal_z != 0.0f) ? 0.0f : ((normal_y != 0.0f) ? off : 0.0f);
        if (normal_x != 0.0f) { dz = off; }
        else if (normal_y != 0.0f) { dz = 0.0f; dx = off; }
        
        dibujar_punto(x_face - dx, y_face, z_face - dz, normal_x, normal_y, normal_z);
        dibujar_punto(x_face + dx, y_face, z_face + dz, normal_x, normal_y, normal_z);
    }
}

static void dibujar_un_dado(float rx, float ry, float rz) {
    glPushMatrix();
    
    glRotatef(rx, 1.0f, 0.0f, 0.0f);
    glRotatef(ry, 0.0f, 1.0f, 0.0f);
    glRotatef(rz, 0.0f, 0.0f, 1.0f);
    
    aplicar_material_plastico_rojo();
    
    dibujar_cara(1, 0.0f, 0.0f, 1.0f);   /* Frente */
    dibujar_cara(6, 0.0f, 0.0f, -1.0f);  /* Atras */
    dibujar_cara(2, 1.0f, 0.0f, 0.0f);   /* Derecha */
    dibujar_cara(5, -1.0f, 0.0f, 0.0f);  /* Izquierda */
    dibujar_cara(3, 0.0f, 1.0f, 0.0f);   /* Arriba */
    dibujar_cara(4, 0.0f, -1.0f, 0.0f);  /* Abajo */

    glPopMatrix();
}

/* Base de la mesa (similar a una alfombra de casino con borde) */
static void dibujar_mesa_dados(void) {
    glPushMatrix();
    
    /* Borde de madera (17.4 x 13.4) */
    aplicar_material(MATERIAL_MADERA);
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.0f);
    glScalef(17.4f, 0.2f, 13.4f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    /* Filo dorado (16.1 x 12.1) */
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    aplicar_material(MATERIAL_METAL);
    glColor3f(1.0f, 0.84f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.055f, 0.0f);
    glScalef(16.1f, 0.02f, 12.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glDisable(GL_COLOR_MATERIAL);
    
    /* Fieltro verde principal (16.0 x 12.0) */
    aplicar_material_panio_verde();
    glPushMatrix();
    glTranslatef(0.0f, 0.066f, 0.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-8.0f, 0.0f, -6.0f);
    glVertex3f(-8.0f, 0.0f, 6.0f);
    glVertex3f(8.0f, 0.0f, 6.0f);
    glVertex3f(8.0f, 0.0f, -6.0f);
    glEnd();
    glPopMatrix();
    
    glPopMatrix();
}

void dibujar_dados(const EstadoDados* estado) {
    if (!estado) return;

    dibujar_mesa_dados();

    /* Dado 1 */
    glPushMatrix();
    glTranslatef(-1.5f, estado->altura[0], 0.0f);
    dibujar_un_dado(estado->rotacion_x[0], estado->rotacion_y[0], estado->rotacion_z[0]);
    glPopMatrix();

    /* Dado 2 */
    glPushMatrix();
    glTranslatef(1.5f, estado->altura[1], 0.0f);
    dibujar_un_dado(estado->rotacion_x[1], estado->rotacion_y[1], estado->rotacion_z[1]);
    glPopMatrix();
}
