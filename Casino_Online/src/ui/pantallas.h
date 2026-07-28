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

/* Inicializa las IDs de texturas OpenGL cargadas */
void inicializar_texturas_pantallas(unsigned int tex_carga, unsigned int tex_casino);

/* Dibuja la pantalla de carga inicial con barra de progreso */
void dibujar_pantalla_carga(float progreso);

/* Dibuja la pantalla de bienvenida y selector de juego (ESTADO_MENU) */
void dibujar_pantalla_menu(int opcion_seleccionada);

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

/* Dibuja la pantalla placeholder para el modulo de tragamonedas */
void dibujar_pantalla_tragamonedas_placeholder(void);

/* Nuevas pantallas de concientizacion y serious game */
void dibujar_pantalla_advertencia(void);
void dibujar_pantalla_proposito(void);
void dibujar_pantalla_confirmacion_juego(void);
void dibujar_pantalla_checkpoint_educativo(int indice_mensaje);

/* Pilar 5 del "serious game": Quiz Educativo Interactivo */
#define NUM_PREGUNTAS_QUIZ 12

/* Dibuja la pantalla del quiz educativo en base a la fase actual y la respuesta elegida */
void dibujar_pantalla_quiz(int pregunta_idx, int fase, int respuesta_elegida, int fue_correcta);

/* Auxiliar para que main.c pueda verificar si la opcion elegida fue la correcta (0=A, 1=B, 2=C) */
int quiz_evaluar_respuesta(int pregunta_idx, int respuesta);

/* Parametros transitorios que algunas pantallas necesitan y otras no */
typedef struct {
    const char* mensaje_reflexivo; /* solo se usa si estado == ESTADO_MENSAJE_REFLEXIVO */
    int pagina_educacion;          /* solo se usa si estado == ESTADO_EDUCACION */
    int opcion_menu;               /* solo se usa si estado == ESTADO_MENU */
    float progreso_carga;          /* solo se usa si estado == ESTADO_CARGA */
    int indice_checkpoint;         /* solo se usa si estado == ESTADO_CHECKPOINT_EDUCATIVO */
    
    /* Parametros exclusivos del ESTADO_QUIZ_EDUCATIVO */
    int quiz_pregunta_idx;
    int quiz_fase;
    int quiz_respuesta_elegida;
    int quiz_fue_correcta;
} InfoPantalla;

/* Despacha a la funcion de dibujo correspondiente segun el estado
   actual. 'info' puede tener campos sin usar segun el estado (ver
   InfoPantalla arriba); nunca debe ser NULL. */
void dibujar_pantalla_segun_estado(EstadoJuego estado, const Jugador* jugador, const InfoPantalla* info);

#endif /* PANTALLAS_H */