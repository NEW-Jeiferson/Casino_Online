/*
 * pantallas.c
 * Implementacion de las pantallas de transicion. Ver pantallas.h.
 */
#include <GL/glut.h>
#include "pantallas.h"

void dibujar_pantalla_prestamo(const Jugador* jugador) {
    /* TODO: activar GL_BLEND, dibujar un rectangulo semitransparente
       cubriendo la pantalla, y el mensaje reflexivo:
       "Ya perdiste tu saldo inicial. En la vida real, este seria el
       momento de parar." seguido de las opciones (seguir/salir). */
    (void)jugador; /* referencia al parametro hasta implementar */
}

void dibujar_pantalla_game_over(const Jugador* jugador) {
    /* TODO: primero dibujar el resumen tipo "recibo" (tiempo jugado,
       total perdido, prestamos solicitados), y despues el overlay
       rojo/negro con interpolacion de color (fade) y el mensaje
       final en tono comico-dramatico. */
    (void)jugador;
}

void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador) {
    switch (estado) {
        case ESTADO_PRESTAMO:
            dibujar_pantalla_prestamo(jugador);
            break;
        case ESTADO_GAME_OVER:
            dibujar_pantalla_game_over(jugador);
            break;
        default:
            break; /* ESTADO_MENU, ESTADO_JUGANDO, ESTADO_SIN_FONDOS
                       no requieren overlay o se manejan aparte */
    }
}
