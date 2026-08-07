#ifndef DADOS_LOGICA_H
#define DADOS_LOGICA_H

#include "../core/jugador.h"

typedef enum {
    APUESTA_DADOS_PAR,
    APUESTA_DADOS_IMPAR,
    APUESTA_DADOS_SIETE,
    APUESTA_DADOS_EXTREMOS  /* gana con suma 2 O suma 12 */
} TipoApuestaDados;

/* Decide el resultado al azar (1 a 6 cada dado) ANTES de que arranque la animacion */
void decidir_resultado_dados(int* dado1, int* dado2);

/* Calcula la ganancia neta (positiva si gana, -monto si pierde) */
float calcular_ganancia_dados(TipoApuestaDados tipo, int dado1, int dado2, float monto);

/* Envuelve el contacto con core/jugador.h.
   Llama internamente a aplicar_resultado_apuesta() y verificar_mensaje_reflexivo(). */
const char* resolver_ronda_dados(Jugador* jugador, TipoApuestaDados tipo, int dado1, int dado2, float monto);

#endif /* DADOS_LOGICA_H */
