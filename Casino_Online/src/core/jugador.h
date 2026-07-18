/*
* Define la estructura del jugador y las funciones para modificar su
* saldo, deuda y estadisticas de sesion.
*/
#ifndef JUGADOR_H
#define JUGADOR_H


/* Estructura que representa al jugador y sus estadisticas de sesion */
typedef struct {
    float saldo;
    float deuda;
    int   prestamos_activos;

    /* Estadisticas de sesion, para el HUD y el resumen final */
    float total_apostado;
    int   veces_sin_fondos;
    float interes_acumulado;


	/* Estadisticas para concientizacion sobre ludopatia */
    float saldo_inicial;                    
    int   racha_perdidas_consecutivas;       
    int   racha_ya_advertida;                
    float ultimo_hito_apostado_notificado;   
    int   ultimo_veces_sin_fondos_notificado;
} Jugador;


/* Sirve para anular una apuesta previamente registrada */
void anular_apuesta(Jugador* j, float monto);


/* Inicializa el jugador con saldo inicial y estadisticas en cero */
void inicializar_jugador(Jugador* j, float saldo_inicial);


/* Registra una apuesta, sumando el monto al total apostado */
void registrar_apuesta(Jugador* j, float monto);


/* Sirve para aplicar el resultado de una apuesta al saldo del jugador */
void aplicar_resultado_apuesta(Jugador* j, float ganancia);


/* Otorga un prestamo al jugador y actualiza su deuda con interes */
void pedir_prestamo(Jugador* j, float monto, float tasa_interes);


/* Sirve para verificar si la deuda de un jugador es impagable */
int deuda_es_impagable(const Jugador* j, float limite_deuda);


/* Verifica si el jugador necesita recibir un mensaje reflexivo sobre ludopatia */
const char* verificar_mensaje_reflexivo(Jugador* j);

#endif 