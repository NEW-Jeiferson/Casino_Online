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
 * ESTADO ACTUAL (PASO 1 de 4): gabinete real (base, cuerpo, marquesina)
 * con primitivas propias. Los rodillos y la palanca todavia no estan
 * (vienen en los proximos pasos) -por ahora la ventana frontal donde van
 * a ir los rodillos queda como un hueco vacio en el cuerpo.
 *
 * Para dibujar los simbolos de cada rodillo durante el giro, llamar a
 * obtener_simbolo_en_posicion() (declarada en tragamonedas_logica.h) -
 * la parte visual NUNCA decide que simbolo va en cada rodillo, solo
 * pregunta y dibuja lo que le contestan.
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_GEOMETRIA_H
#define TRAGAMONEDAS_GEOMETRIA_H

#include "tragamonedas_animacion.h"

/* Dimensiones publicas del gabinete (en unidades de mundo, misma escala
   que RADIO_MESA/RADIO_EXTERIOR_RUEDA en ruleta_geometria.h). Se dejan
   publicas para que los proximos pasos (rodillos, palanca) puedan ubicar
   sus piezas relativas al gabinete sin numeros magicos repetidos. */
#define GABINETE_ANCHO             2.4f
#define GABINETE_PROFUNDIDAD       2.0f
#define GABINETE_ALTURA_BASE       0.5f
#define GABINETE_ALTURA_CUERPO     3.0f
#define GABINETE_ALTURA_MARQUESINA 1.0f
#define GABINETE_ALTURA_TOTAL      (GABINETE_ALTURA_BASE + GABINETE_ALTURA_CUERPO + GABINETE_ALTURA_MARQUESINA)

/* Altura (Y) a la que empieza el cuerpo principal y la marquesina,
   medidas desde el piso (Y = 0). Utiles para el paso 2 (rodillos), que
   va a tener que ubicar la ventana de los rodillos dentro del cuerpo. */
#define GABINETE_Y_INICIO_CUERPO      (GABINETE_ALTURA_BASE)
#define GABINETE_Y_INICIO_MARQUESINA  (GABINETE_ALTURA_BASE + GABINETE_ALTURA_CUERPO)

/* Dibuja la escena 3D completa del tragamonedas (gabinete, rodillos,
   palanca, etc.) en el origen actual de la matriz de modelo -el
   llamador es responsable de la camara/proyeccion, igual que con
   dibujar_mesa()/dibujar_rueda() en la ruleta. */
void dibujar_tragamonedas(void);

/* TEMPORAL, solo para pruebas locales mientras no exista la
   integracion real (ver "INTEGRACION FINAL" en el documento del
   proyecto). dibujar_tragamonedas() mantiene su propio
   EstadoTragamonedas interno (ver tragamonedas_geometria.c) porque su
   firma no recibe parametros. Este getter deja llegar a ese mismo
   estado desde main.c para poder probar iniciar_giro_tragamonedas() +
   actualizar_tragamonedas() de punta a punta, sin esperar a la
   integracion. Cuando llegue ese paso, esto se reemplaza por el
   EstadoTragamonedas que termine viviendo en partida (main.c). */
EstadoTragamonedas* obtener_estado_tragamonedas_para_pruebas(void);

#endif /* TRAGAMONEDAS_GEOMETRIA_H */
