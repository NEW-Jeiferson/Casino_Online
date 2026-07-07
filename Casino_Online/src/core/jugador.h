/*
 * jugador.h
 * -----------------------------------------------------------------------
 * Define la estructura del jugador y las funciones para modificar su
 * saldo, deuda y estadisticas de sesion.
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef JUGADOR_H
#define JUGADOR_H

typedef struct {
    float saldo;
    float deuda;
    int   prestamos_activos;

    /* Estadisticas de sesion, para el HUD y el resumen final */
    float total_apostado;
    int   veces_sin_fondos;
    float interes_acumulado;
} Jugador;

/* Inicializa el jugador con saldo inicial y estadisticas en cero */
void inicializar_jugador(Jugador* j, float saldo_inicial);

/* Aplica el resultado de una apuesta (positivo = gana, negativo = pierde) */
void aplicar_resultado_apuesta(Jugador* j, float monto_apostado, float ganancia);

/* Otorga un prestamo al jugador y actualiza su deuda con interes */
void pedir_prestamo(Jugador* j, float monto, float tasa_interes);

/* Devuelve 1 si la deuda supera el limite de "game over", 0 si no */
int deuda_es_impagable(const Jugador* j, float limite_deuda);

#endif /* JUGADOR_H */
