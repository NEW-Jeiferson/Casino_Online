/*
 * tragamonedas_geometria.h
 * -----------------------------------------------------------------------
 * Geometria 3D del tragamonedas: el mueble/gabinete de la maquina, los
 * rodillos, la palanca, y cualquier decoracion (misma idea que
 * ruleta_geometria.h para la ruleta -mesa, rueda, bolita).
 *
 * A CARGO DE: Luis (parte visual, junto con tragamonedas_animacion.c/h).
 *
 * IMPORTANTE (regla del proyecto): toda la geometria se construye a mano
 * con primitivas de OpenGL (GL_QUADS, GL_TRIANGLE_STRIP, superficies de
 * revolucion, etc.) o curvas de Bezier propias (ver utils/bezier.h) -no
 * se importan modelos 3D externos (nada de OBJ/FBX/glTF ni assets de
 * sitios como Sketchfab). Ver contexto-prompt-ruleta-visual.md para el
 * detalle completo de esta regla.
 *
 * ESTADO ACTUAL: esqueleto minimo. dibujar_tragamonedas() por ahora solo
 * dibuja un placeholder (un cubo simple + texto) para confirmar que el
 * modulo compila y se puede ver algo en pantalla. Reemplazar el
 * contenido de la funcion a medida que se construya la geometria real
 * -no hace falta cambiar la firma ni como se llama desde afuera.
 *
 * Para dibujar los simbolos de cada rodillo durante el giro, llamar a
 * obtener_simbolo_en_posicion() (declarada en tragamonedas_logica.h) -
 * la parte visual NUNCA decide que simbolo va en cada rodillo, solo
 * pregunta y dibuja lo que le contestan.
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_GEOMETRIA_H
#define TRAGAMONEDAS_GEOMETRIA_H

/* Dibuja la escena 3D completa del tragamonedas (gabinete, rodillos,
   palanca, etc.) en el origen actual de la matriz de modelo -el
   llamador es responsable de la camara/proyeccion, igual que con
   dibujar_mesa()/dibujar_rueda() en la ruleta. */
void dibujar_tragamonedas(void);

#endif /* TRAGAMONEDAS_GEOMETRIA_H */
