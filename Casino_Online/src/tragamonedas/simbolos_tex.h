/*
 * simbolos_tex.h
 * Carga de texturas PNG para los simbolos del tragamonedas.
 * Usa stb_image.h (ya presente en src/) para decodificar PNG a RGBA.
 * Cada simbolo se almacena como una textura OpenGL independiente y se
 * renderiza como un quad texturizado dentro de la celda del rodillo.
 *
 * A CARGO DE: Luis (integracion visual).
 */
#ifndef SIMBOLOS_TEX_H
#define SIMBOLOS_TEX_H

#include <GL/glut.h>
#include "tragamonedas_logica.h"  /* SimboloTragamonedas */

/* Carga todos los PNG desde la carpeta simbolos/ relativa al ejecutable.
   Debe llamarse UNA SOLA VEZ, tras tener un contexto OpenGL valido. */
void simbolos_tex_inicializar(void);

/* Libera todas las texturas de OpenGL (llamar al salir). */
void simbolos_tex_liberar(void);

/* Activa (bind) la textura del simbolo indicado y dibuja un quad
   texturizado centrado en el origen actual, de lado 'tamano'.
   Deja GL_TEXTURE_2D deshabilitado al salir (mismo estado con el que
   entra: se activa y desactiva internamente alrededor del bind). */
void simbolos_tex_dibujar(SimboloTragamonedas simbolo, float tamano);

/* Devuelve 1 si el simbolo tiene una textura PNG cargada, 0 si no (por
   ejemplo SIMBOLO_HERRADURA, que no tiene PNG en simbolos/ y se sigue
   dibujando a mano con primitivas -ver tragamonedas_geometria.c). */
int simbolos_tex_tiene_textura(SimboloTragamonedas simbolo);

#endif /* SIMBOLOS_TEX_H */
