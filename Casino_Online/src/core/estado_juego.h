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

/* Calcula el color ganador segun el angulo final de la bolita (0-360),
   dividiendo la rueda en 37 sectores como una ruleta europea real */
ColorRuleta calcular_color_ganador(float angulo_final);
extern const int ORDEN_RUEDA_EUROPEA[37];

/* Determina el numero ganador (0-36) segun el angulo final de la bolita */
int calcular_numero_ganador(float angulo_final);

/* Determina el color real de un numero especifico (0-36), segun la
   tabla fija de una ruleta europea real */
ColorRuleta color_de_numero(int numero);

/* --- Derivados del numero ganador (para resolver docena/par-impar/
   mitad). Todos parten del numero que ya devuelve
   calcular_numero_ganador(); no reimplementan nada de la rueda. --- */

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

typedef enum {
    ESTADO_MENU,
    ESTADO_JUGANDO,
    ESTADO_PRESTAMO,
    ESTADO_GAME_OVER
} EstadoJuego;

/* Estado global actual (definido en estado_juego.c) */
extern EstadoJuego estado_actual;

void inicializar_estado_juego(void);
void cambiar_estado(EstadoJuego nuevo_estado);

#endif /* ESTADO_JUEGO_H */