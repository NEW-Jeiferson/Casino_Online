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

/* Resta el monto del total apostado cuando el jugador deshace una
   ficha antes de girar (no afecta el saldo: la ficha nunca se habia
   descontado, solo se contabilizaba en las estadisticas). */
void anular_apuesta(Jugador* j, float monto);

/* Inicializa el jugador con saldo inicial y estadisticas en cero */
void inicializar_jugador(Jugador* j, float saldo_inicial);

/* Aplica el resultado de una apuesta (positivo = gana, negativo = pierde) */
/* Se llama apenas se coloca una ficha (clic o tecla), no al resolver
   la ronda. Suma el monto al total apostado de la sesion de inmediato,
   para que el HUD refleje la apuesta en el momento en que se hace. */
void registrar_apuesta(Jugador* j, float monto);

/* Se llama cuando la bolita se detiene y se conoce el resultado.
   Solo ajusta el saldo con la ganancia/perdida neta; el monto ya fue
   contabilizado antes por registrar_apuesta(). */
void aplicar_resultado_apuesta(Jugador* j, float ganancia);

/* Otorga un prestamo al jugador y actualiza su deuda con interes */
void pedir_prestamo(Jugador* j, float monto, float tasa_interes);

/* Devuelve 1 si la deuda supera el limite de "game over", 0 si no */
int deuda_es_impagable(const Jugador* j, float limite_deuda);

#endif /* JUGADOR_H */
