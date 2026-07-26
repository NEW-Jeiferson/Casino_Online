/*
 * textura.h
 * -----------------------------------------------------------------------
 * Carga y dibujo de texturas 2D para fondos de pantalla.
 * Usa stb_image.h (ya incluido en el proyecto bajo src/) para decodificar
 * PNG/JPG sin dependencias externas adicionales.
 *
 * Uso tipico:
 *   1) Al iniciar: cargar_textura_fondo_tragamonedas()
 *   2) Antes de dibujar la escena del tragamonedas:
 *        dibujar_fondo_tragamonedas(ancho_ventana, alto_ventana)
 * -----------------------------------------------------------------------
 */
#ifndef TEXTURA_H
#define TEXTURA_H

/* Carga el archivo PNG del fondo del tragamonedas y lo sube como
   textura OpenGL. Devuelve 1 si tuvo exito, 0 si hubo un error (archivo
   no encontrado, formato no soportado, etc.). Se llama UNA sola vez
   al iniciar el programa (en main, antes de glutMainLoop). */
int cargar_textura_fondo_tragamonedas(void);

/* Dibuja el fondo del tragamonedas como un quad 2D que ocupa toda la
   pantalla, en modo ortografico (sin perspectiva), SIN iluminacion.
   Debe llamarse ANTES de dibujar la escena 3D del tragamonedas, para
   que quede detras de todo. ancho y alto son las dimensiones actuales
   de la ventana (glutGet(GLUT_WINDOW_WIDTH/HEIGHT)). */
void dibujar_fondo_tragamonedas(int ancho, int alto);

/* Libera la textura OpenGL (llamar al cerrar el programa si hace falta).
   No es estrictamente necesario -el SO libera los recursos al salir-,
   pero se provee para completitud. */
void liberar_textura_fondo_tragamonedas(void);

#endif /* TEXTURA_H */
