/*
 * jugador.c
 * Implementacion de la logica economica del jugador.
 * Ver jugador.h para la documentacion de cada funcion.
 */
#include "jugador.h"

void inicializar_jugador(Jugador* j, float saldo_inicial) {
    j->saldo = saldo_inicial;
    j->deuda = 0.0f;
    j->prestamos_activos = 0;
    j->total_apostado = 0.0f;
    j->veces_sin_fondos = 0;
    j->interes_acumulado = 0.0f;
}

void registrar_apuesta(Jugador* j, float monto) {
    j->total_apostado += monto;
}

void anular_apuesta(Jugador* j, float monto) {
    j->total_apostado -= monto;
    if (j->total_apostado < 0.0f) j->total_apostado = 0.0f;
}

void aplicar_resultado_apuesta(Jugador* j, float ganancia) {
    j->saldo += ganancia;

    if (j->saldo <= 0.0f) {
        j->saldo = 0.0f;
        j->veces_sin_fondos++;
    }
}

void pedir_prestamo(Jugador* j, float monto, float tasa_interes) {
    float interes = monto * tasa_interes;
    j->deuda += monto + interes;
    j->interes_acumulado += interes;
    j->prestamos_activos++;
    j->saldo += monto;
}

int deuda_es_impagable(const Jugador* j, float limite_deuda) {
    return j->deuda >= limite_deuda;
}
