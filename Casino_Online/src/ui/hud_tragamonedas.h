#ifndef HUD_TRAGAMONEDAS_H
#define HUD_TRAGAMONEDAS_H

#include "../core/estado_juego.h"
#include "../tragamonedas/tragamonedas_animacion.h"

/* Barra de control 2D (overlay de pantalla completa, sin perspectiva):
   SALDO / BET (con -/+) / WIN / boton SPIN, todo en
   una sola franja al pie de la ventana -en vez de un boton suelto- para
   que se lea como la interfaz del juego, no un control flotando aparte.
   Debe llamarse DESPUES de la escena 3D (como dibujar_hud/dibujar_
   pantalla_segun_estado), con ancho/alto = glutGet(GLUT_WINDOW_WIDTH/
   HEIGHT). No se puede pintar literalmente sobre la textura del
   gabinete 3D sin renderizado a textura (fuera de alcance de este
   proyecto) -esta es la aproximacion 2D mas integrada posible. */
void dibujar_barra_control_2d(int ancho_ventana, int alto_ventana,
                               float saldo, float apuesta, float ganancia,
                               const EstadoTragamonedas* estado);

/* Que zona de la barra de control cae en (x_mouse, y_mouse) -coordenadas
   de mouse de GLUT (origen arriba-izquierda, Y hacia abajo). Se llama
   desde mouse_click() en main.c para decidir que accion disparar. */
typedef enum {
    ZONA_CONTROL_NINGUNA = 0,
    ZONA_CONTROL_SPIN,
    ZONA_CONTROL_BET_MENOS,
    ZONA_CONTROL_BET_MAS
} ZonaControlTragamonedas;

ZonaControlTragamonedas obtener_zona_control_2d(int x_mouse, int y_mouse,
                                                 int ancho_ventana, int alto_ventana);

void dibujar_mensaje_giro_tragamonedas(int indice_mensaje);

#endif /* HUD_TRAGAMONEDAS_H */
