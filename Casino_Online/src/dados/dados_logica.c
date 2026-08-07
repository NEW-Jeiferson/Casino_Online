#include "dados_logica.h"
#include <stdlib.h>

void decidir_resultado_dados(int* dado1, int* dado2) {
    if (dado1) *dado1 = (rand() % 6) + 1;
    if (dado2) *dado2 = (rand() % 6) + 1;
}

float calcular_ganancia_dados(TipoApuestaDados tipo, int dado1, int dado2, float monto) {
    int suma = dado1 + dado2;
    int ganaste = 0;
    float pago = 0.0f;

    switch (tipo) {
    case APUESTA_DADOS_PAR:
        if (suma % 2 == 0) ganaste = 1;
        pago = 0.9f;
        break;
    case APUESTA_DADOS_IMPAR:
        if (suma % 2 != 0) ganaste = 1;
        pago = 0.9f;
        break;
    case APUESTA_DADOS_SIETE:
        if (suma == 7) ganaste = 1;
        pago = 4.5f;
        break;
    case APUESTA_DADOS_EXTREMOS:
        if (suma == 2 || suma == 12) ganaste = 1;
        pago = 13.0f;
        break;
    }

    if (ganaste) {
        return monto * pago;
    }
    return -monto;
}

const char* resolver_ronda_dados(Jugador* jugador, TipoApuestaDados tipo, int dado1, int dado2, float monto) {
    float ganancia = calcular_ganancia_dados(tipo, dado1, dado2, monto);
    aplicar_resultado_apuesta(jugador, ganancia);
    return verificar_mensaje_reflexivo(jugador, 2);
}
