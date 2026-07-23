/*
 * estado_juego.h
 * Maquina de estados general del juego, tabla de colores/numeros de la
 * ruleta, y sistema de apuestas multiples (numero, color, docena,
 * par/impar, mitad).
 */

#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H


/* Enum encargado de representar los colores de la ruleta */
typedef enum {
    COLOR_ROJO,
    COLOR_NEGRO,
    COLOR_VERDE
} ColorRuleta;


/* Orden fijo de los numeros en la rueda de la ruleta europea */
extern const int ORDEN_RUEDA_EUROPEA[37];


/* Metodo que determina el color de un numero especifico */
ColorRuleta color_de_numero(int numero);


/* Metodo que determina la docena a la que pertenece un numero especifico */
int docena_de_numero(int numero);


/* Metodo que determina si un numero es par */
int numero_es_par(int numero);

/* Metodo que determina la mitad a la que pertenece un numero especifico */
int mitad_de_numero(int numero);


/* Enum encargado de representar los tipos de apuestas */
typedef enum {
    APUESTA_NUMERO,   
    APUESTA_COLOR,      
    APUESTA_DOCENA,   
    APUESTA_PAR_IMPAR,  
    APUESTA_MITAD       
} TipoApuesta;


/* Estructura que representa una apuesta */
typedef struct {
    TipoApuesta tipo;
    int         valor;  
    float       monto;
} Apuesta;

#define MAX_APUESTAS 20


/* Metodo que calcula la ganancia/perdida de una apuesta específica */
float calcular_ganancia_apuesta(const Apuesta* apuesta, int numero_ganador);


/* Metodo que calcula la ganancia/perdida total de un conjunto de apuestas */
float calcular_ganancia_total(const Apuesta apuestas[], int cantidad, int numero_ganador);

/* Enum encargado de representar los estados posibles del juego (maquina de estados) */
typedef enum {
    ESTADO_MENU,
    ESTADO_JUGANDO,
    ESTADO_PRESTAMO,
    ESTADO_GAME_OVER,
    ESTADO_MENSAJE_REFLEXIVO, 
    ESTADO_SESION_TERMINADA  
} EstadoJuego;


/* Estado global actual (definido en estado_juego.c) */
extern EstadoJuego estado_actual;


/* Metodo encargado de inicializar el estado del juego */
void inicializar_estado_juego(void);


/* Metodo encargado de cambiar el estado del juego */
void cambiar_estado(EstadoJuego nuevo_estado);

#endif