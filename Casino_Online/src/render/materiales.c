/*
 * materiales.c
 * Implementacion de los materiales de la escena. Ver materiales.h.
 *
 * Nota: escrito en estilo compatible con C89/MSVC (declaraciones de
 * variables al inicio del bloque), ya que el compilador de C de Visual
 * Studio por defecto no admite todas las extensiones de C99.
 */
#include <GL/glut.h>
#include "materiales.h"

void aplicar_material(TipoMaterial tipo) {
    GLfloat ambient[4]  = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat diffuse[4]  = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat shininess   = 0.0f;

    switch (tipo) {
        case MATERIAL_MADERA:
            ambient[0]=0.3f;  ambient[1]=0.2f;  ambient[2]=0.1f;
            diffuse[0]=0.5f;  diffuse[1]=0.35f; diffuse[2]=0.2f;
            specular[0]=0.1f; specular[1]=0.1f; specular[2]=0.1f;
            shininess = 8.0f;
            break;

        case MATERIAL_METAL:
            ambient[0]=ambient[1]=ambient[2]=0.25f;
            diffuse[0]=diffuse[1]=diffuse[2]=0.6f;
            specular[0]=specular[1]=specular[2]=0.9f;
            shininess = 90.0f;
            break;

        case MATERIAL_FIELTRO:
            ambient[0]=0.05f; ambient[1]=0.2f; ambient[2]=0.05f;
            diffuse[0]=0.1f;  diffuse[1]=0.4f; diffuse[2]=0.1f;
            specular[0]=specular[1]=specular[2]=0.0f;
            shininess = 1.0f;
            break;

        case MATERIAL_VIDRIO:
            ambient[0]=ambient[1]=ambient[2]=0.2f;  ambient[3]=0.3f;
            diffuse[0]=diffuse[1]=diffuse[2]=0.3f;   diffuse[3]=0.3f;
            specular[0]=specular[1]=specular[2]=0.9f; specular[3]=0.3f;
            shininess = 96.0f;
            /* Nota: el componente alpha < 1.0 requiere GL_BLEND activado
               antes de dibujar el objeto con este material */
            break;

        default:
            shininess = 0.0f;
            break;
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}
