/*
 * textura.c
 * Implementacion de carga y dibujo de texturas 2D. Ver textura.h.
 *
 * Nota sobre stb_image: el header-only library stb_image.h requiere que
 * exactamente UN archivo .c defina STB_IMAGE_IMPLEMENTATION antes de
 * incluirlo. Aqui se hace ese unico #define; todos los demas archivos
 * que necesiten tipos de stb_image deben incluirlo SIN ese define (o,
 * mejor, usar las funciones de este modulo directamente sin incluir
 * stb_image.h por su cuenta).
 *
 * Compatibilidad: escrito en C89/C90 para ser compatible con el
 * compilador de MSVC del proyecto (declaraciones al inicio del bloque,
 * sin // comments, etc.).
 *
 * Advertencias suprimidas: stb_image.h activa C4996 (funciones
 * "unsafe" de CRT) y C4244 (conversion de int a short) que son
 * normales e inofensivas en ese header. Se suprimen localmente
 * para no contaminar el log de build con ruido que no es nuestro. */
#pragma warning(push)
#pragma warning(disable: 4996 4244 4245 4267)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD           /* evita intrinsecos x86 que a veces dan problemas en MSVC/Win32 */
#define _CRT_SECURE_NO_WARNINGS
/* stb_image.h esta en src/ y este archivo en src/render/ -el path ../
   se resuelve relativo al directorio DEL ARCHIVO FUENTE, igual que en GCC/Clang */
#include "../stb_image.h"
#pragma warning(pop)   /* restaura las advertencias normales para el resto del archivo */

#include <GL/glut.h>
#include <stdio.h>
#include "textura.h"

/* GL_CLAMP_TO_EDGE (OpenGL 1.2) no siempre esta en los headers viejos
   de GLUT/MSVC -se define manualmente si no esta; el valor es el de la
   especificacion oficial y no cambia entre implementaciones. */
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

 /* Handle de la textura OpenGL. 0 = no cargada todavia. */
static GLuint g_textura_fondo = 0;

/* Ruta relativa al ejecutable del archivo de imagen del fondo.
   El ejecutable se genera en Debug/ o x64/Debug/ segun la configuracion.
   Se prueban varias rutas relativas para que funcione tanto desde el
   IDE (Working Directory = $(ProjectDir)) como desde el ejecutable
   directamente (Working Directory = carpeta del .exe). */
static const char* RUTAS_FONDO[] = {
    "src/assets/tragamonedas_fondo.png",        /* desde ProjectDir (IDE) */
    "../src/assets/tragamonedas_fondo.png",     /* desde Debug/           */
    "../../src/assets/tragamonedas_fondo.png",  /* desde x64/Debug/       */
    "assets/tragamonedas_fondo.png",            /* si assets/ esta junto al .exe */
    NULL
};

GLuint cargar_textura_gl(const char* ruta) {
    int ancho, alto, canales;
    unsigned char* data;
    GLuint tex_id;
    char ruta_alt[512];

    tex_id = 0;
    data = stbi_load(ruta, &ancho, &alto, &canales, 0);
    if (!data) {
        sprintf_s(ruta_alt, sizeof(ruta_alt), "../%s", ruta);
        data = stbi_load(ruta_alt, &ancho, &alto, &canales, 0);
    }
    if (!data) {
        fprintf(stderr, "Error al cargar textura: %s\n", ruta);
        return 0;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x2900); /* GL_CLAMP */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x2900); /* GL_CLAMP */

    if (canales == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ancho, alto, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (canales == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    stbi_image_free(data);
    return tex_id;
}

int cargar_textura_fondo_tragamonedas(void) {
    int ancho, alto, canales;
    unsigned char* datos;
    int i;

    /* Si ya estaba cargada (llamada duplicada), no hace nada */
    if (g_textura_fondo != 0) return 1;

    /* Intentar cada ruta hasta encontrar la imagen */
    datos = NULL;
    for (i = 0; RUTAS_FONDO[i] != NULL; i++) {
        /* stb_image carga de arriba hacia abajo por defecto, pero OpenGL
           espera los pixeles de abajo hacia arriba. flip_vertically corrige eso. */
        stbi_set_flip_vertically_on_load(1);
        datos = stbi_load(RUTAS_FONDO[i], &ancho, &alto, &canales, 4); /* fuerza RGBA */
        if (datos != NULL) {
            printf("[textura] Fondo cargado: %s (%dx%d, %d canales)\n",
                RUTAS_FONDO[i], ancho, alto, canales);
            break;
        }
    }

    if (datos == NULL) {
        printf("[textura] ADVERTENCIA: no se pudo cargar el fondo del tragamonedas.\n"
               "  Se dibujo sin imagen de fondo. Rutas probadas:\n");
        for (i = 0; RUTAS_FONDO[i] != NULL; i++) {
            printf("    %s\n", RUTAS_FONDO[i]);
        }
        return 0;
    }

    /* Generar y subir la textura a OpenGL */
    glGenTextures(1, &g_textura_fondo);
    glBindTexture(GL_TEXTURE_2D, g_textura_fondo);

    /* Filtrado bilineal -suficiente para un quad de fondo, sin mipmap */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, datos);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(datos);

    return 1;
}

void dibujar_fondo_tragamonedas(int ancho, int alto) {
    if (g_textura_fondo == 0) {
        /* Textura no cargada: dibujar un gradiente negro/rojo oscuro como
           fallback para que no quede una pantalla blanca/gris. */
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, ancho, 0, alto, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);

        /* Gradiente vertical: negro abajo, rojo muy oscuro arriba */
        glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 0.0f);       glVertex2f(0.0f,    0.0f);
        glColor3f(0.0f, 0.0f, 0.0f);       glVertex2f((float)ancho, 0.0f);
        glColor3f(0.10f, 0.02f, 0.02f);    glVertex2f((float)ancho, (float)alto);
        glColor3f(0.10f, 0.02f, 0.02f);    glVertex2f(0.0f,   (float)alto);
        glEnd();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        return;
    }

    /* ---- Dibujar la textura como quad 2D full-screen ---- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ancho, 0, alto, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_textura_fondo);

    /* Sin blending: el fondo va solido, sin mezcla con lo que habia atras */
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); /* tinte blanco = colores originales de la imagen */

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);  glVertex2f(0.0f,         0.0f);
    glTexCoord2f(1.0f, 0.0f);  glVertex2f((float)ancho, 0.0f);
    glTexCoord2f(1.0f, 1.0f);  glVertex2f((float)ancho, (float)alto);
    glTexCoord2f(0.0f, 1.0f);  glVertex2f(0.0f,         (float)alto);
    glEnd();

    /* Oscurecer ligeramente con un overlay negro semitransparente para
       que la escena 3D del tragamonedas encima se lea bien y no compita
       con el fondo. La opacidad del overlay se calibra para conservar el
       ambiente pero no deslumbrar. */
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f); /* 35% negro encima de la imagen */
    glBegin(GL_QUADS);
    glVertex2f(0.0f,         0.0f);
    glVertex2f((float)ancho, 0.0f);
    glVertex2f((float)ancho, (float)alto);
    glVertex2f(0.0f,         (float)alto);
    glEnd();
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, 0);

    /* Restaurar estado 3D */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void liberar_textura_fondo_tragamonedas(void) {
    if (g_textura_fondo != 0) {
        glDeleteTextures(1, &g_textura_fondo);
        g_textura_fondo = 0;
    }
}
