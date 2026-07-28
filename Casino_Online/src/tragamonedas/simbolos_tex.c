/*
 * simbolos_tex.c
 * Implementacion del sistema de texturas para simbolos del tragamonedas.
 *
 * Tecnica: cada simbolo es una textura OpenGL 2D (GL_RGBA) cargada desde
 * un PNG con stb_image. El quad se dibuja con iluminacion desactivada para
 * que los colores del PNG se vean exactamente como en la imagen, sin que
 * la luz de la escena los desvirtue.
 *
 * Rutas de los PNG: se prueban varios prefijos de carpeta en orden (misma
 * tecnica que RUTAS_FONDO en render/textura.c) para que funcione tanto
 * desde el IDE (Working Directory = ProjectDir) como desde el ejecutable
 * generado en Debug/ (con la carpeta simbolos/ copiada por el post-build).
 * Si un archivo no se encuentra, la textura queda en 0 y el simbolo se
 * dibuja de color magenta solido (fallback visible para depuracion).
 */

 /* stb_image ya se compila en textura.c (STB_IMAGE_IMPLEMENTATION definido ahi).
   Aqui solo incluimos el header para usar stbi_load. */
#pragma warning(push)
#pragma warning(disable: 4996 4244 4100 4702 4267)
#define _CRT_SECURE_NO_WARNINGS
#include "../stb_image.h"
#pragma warning(pop)

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include "simbolos_tex.h"
#include "tragamonedas_logica.h"

/* Una textura por cada valor real del enum SimboloTragamonedas (ver
   tragamonedas_logica.h) -se usa ese mismo NUM_SIMBOLOS en vez de un
   numero propio, para que esta tabla nunca quede desalineada si el
   enum cambia (ver el bug que se corrige mas abajo: el mapeo viejo
   asumia un orden de enum distinto al actual). */
#define TOTAL_TEXTURAS_SIMBOLO NUM_SIMBOLOS

/* Nombre de archivo (sin carpeta) para cada simbolo, EN EL MISMO ORDEN
   que el enum SimboloTragamonedas: CEREZA, CAMPANA, HERRADURA,
   DIAMANTE, BARRA, SIETE.
   SIMBOLO_HERRADURA no tiene PNG en simbolos/ -se deja NULL y ese
   simbolo se sigue dibujando a mano (ver tragamonedas_geometria.c). */
static const char* NOMBRES_PNG[TOTAL_TEXTURAS_SIMBOLO] = {
    "sym_cherry.png",   /* SIMBOLO_CEREZA */
    "sym_bell.png",     /* SIMBOLO_CAMPANA */
    NULL,               /* SIMBOLO_HERRADURA -- sin PNG, dibujo a mano */
    "sym_diamond.png",  /* SIMBOLO_DIAMANTE */
    "sym_bar.png",      /* SIMBOLO_BARRA */
    "sym_7.png"         /* SIMBOLO_SIETE */
};

/* Prefijos de carpeta a probar, en orden, hasta encontrar el archivo. */
static const char* PREFIJOS_CARPETA[] = {
    "src/tragamonedas/simbolos/",       /* desde ProjectDir (IDE) */
    "../src/tragamonedas/simbolos/",    /* desde Debug/           */
    "../../src/tragamonedas/simbolos/", /* desde x64/Debug/       */
    "simbolos/",                        /* si la carpeta esta junto al .exe */
    NULL
};

static GLuint g_texturas[TOTAL_TEXTURAS_SIMBOLO];
static int    g_inicializado = 0;

/* Umbral de brillo (0-255) para sintetizar el canal alpha a partir del
   fondo negro -ver comentario en cargar_recorte_por_brillo() de por que
   hace falta esto (los archivos sym_*.png son en realidad JPEG, sin
   canal alpha real). Por debajo de BAJO: transparente. Por encima de
   ALTO: opaco. En el medio: rampa lineal, para que el recorte no quede
   con un borde duro/dentado sobre el brillo/resplandor de cada icono. */
#define RECORTE_BRILLO_BAJO 16
#define RECORTE_BRILLO_ALTO 55

/* Los archivos en simbolos/ tienen extension .png pero son en realidad
   JPEG (ver cabecera FF D8 FF E0) -formato sin canal alpha, asi que el
   fondo negro de cada icono viene horneado como color solido opaco, no
   como transparencia real. stbi_load con forceChannels=4 les agrega un
   alpha=255 parejo en toda la imagen (no hay dato de transparencia que
   extraer). Esta funcion sintetiza esa transparencia despues de cargar:
   como el fondo es negro/casi negro y el arte del icono siempre es mas
   brillante, usar el brillo de cada pixel como clave de recorte separa
   limpiamente el simbolo de su fondo, y de paso conserva el resplandor
   tipo neon de los bordes (que ya es un degrade de brillo hacia negro)
   como un fundido a transparente en vez de cortarlo de golpe. */
static void aplicar_recorte_por_brillo(unsigned char* datos, int ancho, int alto) {
    int total = ancho * alto;
    int i;
    for (i = 0; i < total; i++) {
        unsigned char* px = datos + (size_t)i * 4;
        int brillo = px[0];
        if (px[1] > brillo) brillo = px[1];
        if (px[2] > brillo) brillo = px[2];

        if (brillo <= RECORTE_BRILLO_BAJO) {
            px[3] = 0;
        } else if (brillo >= RECORTE_BRILLO_ALTO) {
            px[3] = 255;
        } else {
            px[3] = (unsigned char)(255 * (brillo - RECORTE_BRILLO_BAJO)
                                     / (RECORTE_BRILLO_ALTO - RECORTE_BRILLO_BAJO));
        }
    }
}

/* ---------- ayuda: cargar un PNG como textura OpenGL -------------------- */
static GLuint cargar_textura_png(const char* nombre_archivo) {
    int ancho, alto, canales;
    unsigned char* datos = NULL;
    GLuint id = 0;
    char ruta[256];
    int i;

    if (nombre_archivo == NULL) return 0;

    /* Este loader asume que stb_image NO invierte verticalmente (ver los
       texcoords en simbolos_tex_dibujar). render/textura.c activa el flip
       global para el fondo -se lo desactiva aca de forma explicita para
       no heredar ese estado sin importar el orden de inicializacion. */
    stbi_set_flip_vertically_on_load(0);

    for (i = 0; PREFIJOS_CARPETA[i] != NULL; i++) {
        sprintf_s(ruta, sizeof(ruta), "%s%s", PREFIJOS_CARPETA[i], nombre_archivo);
        datos = stbi_load(ruta, &ancho, &alto, &canales, 4); /* fuerza RGBA */
        if (datos != NULL) {
            printf("[simbolos_tex] Cargado: %s (%dx%d)\n", ruta, ancho, alto);
            break;
        }
    }

    if (datos == NULL) {
        fprintf(stderr, "[simbolos_tex] No se pudo cargar '%s' (probadas %d rutas)\n",
                nombre_archivo, i);
        return 0;
    }

    aplicar_recorte_por_brillo(datos, ancho, alto);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 ancho, alto, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, datos);

    stbi_image_free(datos);
    return id;
}

/* ---------- API publica -------------------------------------------------- */

void simbolos_tex_inicializar(void) {
    int i;
    if (g_inicializado) return;

    memset(g_texturas, 0, sizeof(g_texturas));
    for (i = 0; i < TOTAL_TEXTURAS_SIMBOLO; i++) {
        g_texturas[i] = cargar_textura_png(NOMBRES_PNG[i]);
    }
    g_inicializado = 1;
}

void simbolos_tex_liberar(void) {
    if (!g_inicializado) return;
    glDeleteTextures(TOTAL_TEXTURAS_SIMBOLO, g_texturas);
    memset(g_texturas, 0, sizeof(g_texturas));
    g_inicializado = 0;
}

int simbolos_tex_tiene_textura(SimboloTragamonedas simbolo) {
    int idx = (int)simbolo;
    if (idx < 0 || idx >= TOTAL_TEXTURAS_SIMBOLO) return 0;
    return g_texturas[idx] != 0;
}

void simbolos_tex_dibujar(SimboloTragamonedas simbolo, float tamano) {
    int idx = (int)simbolo;
    float h  = tamano * 0.5f;
    GLuint tex;

    if (idx < 0 || idx >= TOTAL_TEXTURAS_SIMBOLO) idx = 0;
    tex = g_texturas[idx];

    glDisable(GL_LIGHTING);

    if (tex == 0) {
        /* Fallback magenta si la textura no cargo */
        glColor3f(1.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-h, -h, 0.0f);
        glVertex3f( h, -h, 0.0f);
        glVertex3f( h,  h, 0.0f);
        glVertex3f(-h,  h, 0.0f);
        glEnd();
    } else {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); /* sin tinte -color exacto del PNG */
        glBindTexture(GL_TEXTURE_2D, tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, -h, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( h, -h, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( h,  h, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-h,  h, 0.0f);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    glEnable(GL_LIGHTING);
}
