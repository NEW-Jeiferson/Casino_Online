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
   sus piezas relativas al gabinete sin numeros magicos repetidos.

   REDISENO (referencia: boceto de tragamonedas antigua tipo Mills/
   Jennings -mueble angosto y alto, tope curvo/domo, no la caja ancha y
   plana de la version "casino moderno" anterior): el mueble se angosta
   y se agrega una cupula redondeada arriba de la marquesina en vez de
   un canto recto. */
#define GABINETE_ANCHO             1.7f
#define GABINETE_PROFUNDIDAD       1.5f
#define GABINETE_ALTURA_BASE       0.30f
#define GABINETE_ALTURA_CUERPO     2.55f
#define GABINETE_ALTURA_MARQUESINA 0.75f
#define GABINETE_RADIO_CUPULA      (GABINETE_ANCHO / 2.0f)
#define GABINETE_ALTURA_TOTAL      (GABINETE_ALTURA_BASE + GABINETE_ALTURA_CUERPO + GABINETE_ALTURA_MARQUESINA + GABINETE_RADIO_CUPULA)

/* Altura (Y) a la que empieza el cuerpo principal, la marquesina y la
   cupula, medidas desde el piso (Y = 0). Utiles para el paso 2
   (rodillos), que va a tener que ubicar la ventana de los rodillos
   dentro del cuerpo. */
#define GABINETE_Y_INICIO_CUERPO      (GABINETE_ALTURA_BASE)
#define GABINETE_Y_INICIO_MARQUESINA  (GABINETE_ALTURA_BASE + GABINETE_ALTURA_CUERPO)
#define GABINETE_Y_INICIO_CUPULA      (GABINETE_Y_INICIO_MARQUESINA + GABINETE_ALTURA_MARQUESINA)

/* Dibuja la escena 3D completa del tragamonedas (gabinete, rodillos,
   palanca, etc.) en el origen actual de la matriz de modelo -el
   llamador es responsable de la camara/proyeccion, igual que con
   dibujar_mesa()/dibujar_rueda() en la ruleta. */
void dibujar_tragamonedas(void);

/* Barra de control 2D (overlay de pantalla completa, sin perspectiva):
   SALDO / BET (con -/+) / WIN / boton SPIN / boton AUTO-MANUAL, todo en
   una sola franja al pie de la ventana -en vez de un boton suelto- para
   que se lea como la interfaz del juego, no un control flotando aparte.
   Debe llamarse DESPUES de la escena 3D (como dibujar_hud/dibujar_
   pantalla_segun_estado), con ancho/alto = glutGet(GLUT_WINDOW_WIDTH/
   HEIGHT). No se puede pintar literalmente sobre la textura del
   gabinete 3D sin renderizado a textura (fuera de alcance de este
   proyecto) -esta es la aproximacion 2D mas integrada posible. */
void dibujar_barra_control_2d(int ancho_ventana, int alto_ventana,
                               float saldo, float apuesta, float ganancia,
                               int auto_activo);

/* Que zona de la barra de control cae en (x_mouse, y_mouse) -coordenadas
   de mouse de GLUT (origen arriba-izquierda, Y hacia abajo). Se llama
   desde mouse_click() en main.c para decidir que accion disparar. */
typedef enum {
    ZONA_CONTROL_NINGUNA = 0,
    ZONA_CONTROL_SPIN,
    ZONA_CONTROL_BET_MENOS,
    ZONA_CONTROL_BET_MAS,
    ZONA_CONTROL_AUTO
} ZonaControlTragamonedas;

ZonaControlTragamonedas obtener_zona_control_2d(int x_mouse, int y_mouse,
                                                 int ancho_ventana, int alto_ventana);

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
