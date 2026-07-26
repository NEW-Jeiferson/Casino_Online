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
    GLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat diffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat shininess = 0.0f;

    switch (tipo) {
    case MATERIAL_MADERA:
        /* Usado en el borde de la mesa (dibujar_mesa(), alrededor del
           fieltro) -ver ruleta_geometria.c. */
        ambient[0] = 0.3f;  ambient[1] = 0.2f;  ambient[2] = 0.1f;
        diffuse[0] = 0.5f;  diffuse[1] = 0.35f; diffuse[2] = 0.2f;
        specular[0] = 0.15f; specular[1] = 0.12f; specular[2] = 0.08f;
        shininess = 10.0f;
        break;

    case MATERIAL_METAL:
        /* Usado en rueda (superficie curva grande, Bezier) y
           bolita. Diffuse subido de 0.6 a 0.65 para que las caras
           de la rueda que no reciben luz directa no se vean muy
           apagadas. Shininess subido de 90 a 115 para un brillo
           de cromo mas concentrado y realista. */
        ambient[0] = ambient[1] = ambient[2] = 0.25f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.65f;
        specular[0] = specular[1] = specular[2] = 0.95f;
        shininess = 115.0f;
        break;

    case MATERIAL_FIELTRO:
        /* Ambient subido de 0.05/0.2/0.05 a 0.10/0.26/0.10: la
           mesa es un solo quad grande (radio 6) con la luz
           cenital en (0,5,0). En las esquinas el angulo entre
           normal y direccion a la luz llega a ~60 grados
           (cos ~0.5 de luz difusa), asi que con el ambient
           original las esquinas se veian casi negras. Sigue
           siendo notablemente mas oscuro que el centro (fieltro
           real tiene esa caida), pero ya no se pierde a negro. */
        ambient[0] = 0.10f; ambient[1] = 0.26f; ambient[2] = 0.10f;
        diffuse[0] = 0.12f; diffuse[1] = 0.42f; diffuse[2] = 0.12f;
        specular[0] = specular[1] = specular[2] = 0.0f;
        shininess = 1.0f;
        break;

    case MATERIAL_VIDRIO:
        /* Specular ligeramente tintado hacia azul-gris frio en
           vez de blanco puro, para diferenciar el brillo del
           vidrio del brillo del metal ahora que el metal tiene
           specular mas fuerte (0.95). */
        ambient[0] = ambient[1] = 0.2f; ambient[2] = 0.22f; ambient[3] = 0.3f;
        diffuse[0] = diffuse[1] = 0.3f; diffuse[2] = 0.32f; diffuse[3] = 0.3f;
        specular[0] = specular[1] = 0.85f; specular[2] = 0.9f; specular[3] = 0.3f;
        shininess = 96.0f;
        /* Nota: el componente alpha < 1.0 requiere GL_BLEND activado
           antes de dibujar el objeto con este material */
        break;

    case MATERIAL_MADERA_OSCURA:
        /* Madera de roble oscuro, mate y sobria para unificar rueda y mesa */
        ambient[0] = 0.15f;  ambient[1] = 0.09f;  ambient[2] = 0.05f;  ambient[3] = 1.0f;
        diffuse[0] = 0.35f;  diffuse[1] = 0.20f;  diffuse[2] = 0.10f;  diffuse[3] = 1.0f;
        specular[0] = 0.10f; specular[1] = 0.07f; specular[2] = 0.04f; specular[3] = 1.0f;
        shininess = 8.0f;
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