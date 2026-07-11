/*
* Maquina de estados general del juego, tabla de colores/numeros de la
* ruleta, y sistema de apuestas multiples (numero, color, docena,
* par/impar, mitad).
*/
#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H

typedef enum {
    COLOR_ROJO,
    COLOR_NEGRO,
    COLOR_VERDE
} ColorRuleta;

extern const int ORDEN_RUEDA_EUROPEA[37];

/* Determina el color de un numero especifico (0-36) */
ColorRuleta color_de_numero(int numero);

/* Determina a que docena pertenece un numero (1-36) */
int docena_de_numero(int numero);


/* Determina si un numero es par (1) o impar (0) */
int numero_es_par(int numero);


/* Determina a que mitad pertenece un numero (1-36) */
int mitad_de_numero(int numero);


/* Tipos de apuestas disponibles */
typedef enum {
    APUESTA_NUMERO,     
    APUESTA_COLOR,      
    APUESTA_DOCENA,     
    APUESTA_PAR_IMPAR,  
    APUESTA_MITAD       
} TipoApuesta;


/* Estructura para representar una apuesta */
typedef struct {
    TipoApuesta tipo;
    int         valor;  
    float       monto;
} Apuesta;

#define MAX_APUESTAS 20


/* Calcula la ganancia de una apuesta específica */
float calcular_ganancia_apuesta(const Apuesta* apuesta, int numero_ganador);


/* Calcula la ganancia total de todas las apuestas activas */
float calcular_ganancia_total(const Apuesta apuestas[], int cantidad, int numero_ganador);


/* Estados posibles del juego */
typedef enum {
    ESTADO_MENU,
    ESTADO_JUGANDO,
    ESTADO_PRESTAMO,
    ESTADO_GAME_OVER,
    ESTADO_MENSAJE_REFLEXIVO

} EstadoJuego;


/* Variable global que mantiene el estado actual del juego */
extern EstadoJuego estado_actual;


/* Inicializa el estado del juego al estado de menú */
void inicializar_estado_juego(void);


/* Cambia el estado del juego a un nuevo estado, si la transición es válida */
void cambiar_estado(EstadoJuego nuevo_estado);

#endif