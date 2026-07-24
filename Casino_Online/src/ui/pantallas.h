/*
 * pantallas.h
 * -----------------------------------------------------------------------
 * Pantallas de transicion superpuestas a la escena 3D: pantalla de
 * prestamo (cuando el saldo llega a 0) y pantalla de Game Over (cuando
 * la deuda se vuelve impagable). Usan blending para el overlay y
 * interpolacion de color para el efecto de fundido (fade).
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef PANTALLAS_H
#define PANTALLAS_H

#include "../core/jugador.h"
#include "../core/estado_juego.h"

 /* Dibuja la pantalla de bienvenida (ESTADO_MENU): titulo, aviso de que
    se juega con saldo virtual, y la lista de controles del juego. */
void dibujar_pantalla_menu(void);

/* Dibuja el overlay y mensaje reflexivo de la pantalla de prestamo.
   Ofrece [P] pedir prestamo y seguir, o [S] terminar la sesion aqui
   en vez de endeudarse -antes solo existia la primera opcion. */
void dibujar_pantalla_prestamo(const Jugador* jugador);

/* Dibuja el resumen de estadisticas al perder por deuda impagable.
   Mismo layout que dibujar_pantalla_sesion_terminada(), pero esta
   pantalla es la que se ve cuando el juego FUERZA el cierre (no hubo
   eleccion posible), asi que mantiene un tono mas serio. */
void dibujar_pantalla_game_over(const Jugador* jugador);

/* Dibuja el resumen de estadisticas cuando el jugador ELIGE terminar
   la sesion (desde el mensaje reflexivo o desde la pantalla de
   prestamo), en vez de que el juego lo obligue. Mismos datos que
   Game Over, pero con tono de cierre respetuoso en vez de derrota. */
void dibujar_pantalla_sesion_terminada(const Jugador* jugador);

/* Dibuja un mensaje de concientizacion sobre ludopatia (ver
   core/jugador.h, verificar_mensaje_reflexivo). 'mensaje' puede tener
   saltos de linea ('\n') para forzar cortes de linea especificos.
   Ofrece [ENTER] seguir jugando o [S] terminar la sesion aqui -antes
   solo se podia descartar con ENTER, sin una eleccion real.
   'jugador' se usa para leer jugador->mensajes_reflexivos_mostrados y
   escalar la fricción visual si no es la primera vez en la sesión
   (ver la implementacion en pantallas.c). */
void dibujar_pantalla_mensaje_reflexivo(const char* mensaje, const Jugador* jugador);

#define EDUCACION_NUM_PAGINAS 6

/* Pilar 4 del "serious game" (ver docs/analisis-ludopatia.md): pantalla
   de informacion paginada sobre ludopatia. 'pagina' va de 0 a
   EDUCACION_NUM_PAGINAS-1 (que es la ludopatia, senales de alerta,
   mitos, consecuencias, recursos de ayuda, prevencion, en ese orden).
   Valores fuera de rango se tratan como 0 (no revienta el programa por
   un indice invalido). [ESPACIO] avanza de pagina (con wrap-around),
   [ENTER] vuelve al menu -ver main.c/teclado(). */
void dibujar_pantalla_educacion(int pagina);

/* Parametros transitorios que algunas pantallas necesitan y otras no
   -se agrupan aca en vez de seguir sumando parametros sueltos a
   dibujar_pantalla_segun_estado() cada vez que se agrega una pantalla
   nueva que necesita "un dato mas". Los campos que no aplican al
   estado actual simplemente no se leen. */
typedef struct {
    const char* mensaje_reflexivo; /* solo se usa si estado == ESTADO_MENSAJE_REFLEXIVO */
    int pagina_educacion;          /* solo se usa si estado == ESTADO_EDUCACION */
} InfoPantalla;

/* Despacha a la funcion de dibujo correspondiente segun el estado
   actual. 'info' puede tener campos sin usar segun el estado (ver
   InfoPantalla arriba); nunca debe ser NULL. */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const InfoPantalla* info);

#endif /* PANTALLAS_H */