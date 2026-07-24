/*
 * estado_juego.h
 * -----------------------------------------------------------------------
 * Maquina de estados general del juego, tabla de colores/numeros de la
 * ruleta, y sistema de apuestas multiples (numero, color, docena,
 * par/impar, mitad).
 * -----------------------------------------------------------------------
 */
#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H

typedef enum {
    COLOR_ROJO,
    COLOR_NEGRO,
    COLOR_VERDE
} ColorRuleta;

extern const int ORDEN_RUEDA_EUROPEA[37];

/* NOTA: antes existian aqui calcular_color_ganador(angulo_final) y
   calcular_numero_ganador(angulo_final) -derivaban el resultado del
   ANGULO FINAL de la bolita. Ese fue el enfoque viejo y con bugs (ver
   contexto-proyecto.md, seccion 9, items 1 y 3): el numero ganador
   ahora se decide ANTES de girar con rand() % 37 (ver ' ' en
   main.c/teclado()), y ninguna parte del codigo actual llama a esas
   dos funciones. Se eliminaron para que no queden como "fuente de
   verdad" enganosa -si alguna vez hace falta re-derivar un numero a
   partir de un angulo, hay que reconstruirla teniendo en cuenta el fix
   de signo en Z de dibujar_pista_numerada() (ruleta_geometria.c),
   que estas funciones viejas no incorporaban. */

   /* Determina el color real de un numero especifico (0-36), segun la
      tabla fija de una ruleta europea real */
ColorRuleta color_de_numero(int numero);

/* --- Derivados del numero ganador (para resolver docena/par-impar/
   mitad). Todos parten de un numero ya conocido (el que decide main.c
   con rand() % 37 al presionar ESPACIO, ver ORDEN_RUEDA_EUROPEA); no
   reimplementan nada de la rueda. --- */

   /* Docena del numero: 1 (1-12), 2 (13-24), 3 (25-36); 0 si numero es 0 */
int docena_de_numero(int numero);

/* 1 si el numero es par, 0 si es impar (el 0 nunca cuenta como par
   para efectos de apuesta) */
int numero_es_par(int numero);

/* Mitad del numero: 1 = primera mitad (1-18), 2 = segunda (19-36),
   0 si el numero es 0 */
int mitad_de_numero(int numero);

/* --- Sistema de apuestas multiples --- */

typedef enum {
    APUESTA_NUMERO,     /* valor = numero exacto (0-36). Paga 35 a 1 */
    APUESTA_COLOR,      /* valor = ColorRuleta (ROJO o NEGRO). Paga 1 a 1 */
    APUESTA_DOCENA,     /* valor = 1, 2 o 3. Paga 2 a 1 */
    APUESTA_PAR_IMPAR,  /* valor = 0 (par) o 1 (impar). Paga 1 a 1 */
    APUESTA_MITAD       /* valor = 1 (1-18) o 2 (19-36). Paga 1 a 1 */
} TipoApuesta;

typedef struct {
    TipoApuesta tipo;
    int         valor;  /* interpretacion depende de tipo, ver arriba */
    float       monto;
} Apuesta;

#define MAX_APUESTAS 20

/* Ganancia/perdida neta de UNA apuesta contra el numero ganador.
   Devuelve monto*payout si gano, o -monto si perdio (mismo convenio
   que usaba el codigo original: "ganancia" es la variacion neta del
   saldo, no el monto total devuelto). */
float calcular_ganancia_apuesta(const Apuesta* apuesta, int numero_ganador);

/* Suma la ganancia/perdida de todas las apuestas activas. Se usa para
   llamar aplicar_resultado_apuesta() una sola vez con el total de la
   ronda, sin importar cuantas fichas se pusieron. */
float calcular_ganancia_total(const Apuesta apuestas[], int cantidad, int numero_ganador);

/* Estados posibles del juego (maquina de estados) */
typedef enum {
    ESTADO_MENU,
    ESTADO_JUGANDO,
    ESTADO_PRESTAMO,
    ESTADO_GAME_OVER,
    ESTADO_MENSAJE_REFLEXIVO, /* pantalla de concientizacion (ver jugador.h,
                                 verificar_mensaje_reflexivo); interrumpe el
                                 juego brevemente, ofrece seguir jugando O
                                 terminar la sesion aqui mismo */
    ESTADO_SESION_TERMINADA, /* el jugador ELIGIO terminar (desde el mensaje
                                 reflexivo o desde la pantalla de prestamo),
                                 en vez de que el juego se lo imponga por
                                 quedarse sin fondos. Mismo resumen de
                                 estadisticas que Game Over, pero con tono
                                 de cierre respetuoso, no de derrota. */
    ESTADO_EDUCACION        /* pilar 4 del "serious game": pantalla de
                                informacion paginada (que es la ludopatia,
                                senales de alerta, mitos, consecuencias,
                                recursos de ayuda, prevencion). Accesible
                                desde el menu y desde las 2 pantallas de
                                cierre de sesion, ver main.c/teclado(). */
} EstadoJuego;

/* Estado global actual (definido en estado_juego.c) */
extern EstadoJuego estado_actual;

void inicializar_estado_juego(void);
void cambiar_estado(EstadoJuego nuevo_estado);

#endif /* ESTADO_JUEGO_H */