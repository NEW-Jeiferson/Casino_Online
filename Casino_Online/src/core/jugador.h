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

    /* --- Concientizacion sobre ludopatia: disparadores de mensajes
       reflexivos (ver verificar_mensaje_reflexivo). Estos campos NO
       son estadisticas para mostrar en el HUD -son memoria interna
       para saber "ya avise sobre esto" y no repetir el mismo mensaje
       en cada ronda mientras la condicion se mantenga. --- */
    float saldo_inicial;                    /* copia de saldo al arrancar, para calcular hitos relativos */
    int   racha_perdidas_consecutivas;       /* rondas seguidas con resultado neto negativo */
    int   racha_ya_advertida;                /* 1 si ya se mostro el mensaje para ESTA racha (se resetea a 0 cuando la racha vuelve a 0) */
    float ultimo_hito_apostado_notificado;   /* total_apostado en el momento del ultimo aviso por monto */
    int   ultimo_veces_sin_fondos_notificado;/* veces_sin_fondos en el momento del ultimo aviso por "te quedaste sin saldo" */
} Jugador;

/* Resta el monto del total apostado cuando el jugador deshace una
   ficha antes de girar (no afecta el saldo: la ficha nunca se habia
   descontado, solo se contabilizaba en las estadisticas). */
void anular_apuesta(Jugador* j, float monto);

/* Inicializa el jugador con saldo inicial y estadisticas en cero */
void inicializar_jugador(Jugador* j, float saldo_inicial);

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

/* --- Concientizacion sobre ludopatia --- */

/* Revisa las condiciones de juego problematico (racha de perdidas,
   monto total apostado, veces sin fondos) contra el estado actual del
   jugador. Devuelve un puntero a un mensaje reflexivo (string
   constante, NO liberar) si alguna condicion se cumplio recien -es
   decir, se cumple una sola vez por evento, no en cada frame mientras
   la condicion se mantenga-, o NULL si no hay nada que mostrar en este
   momento. Se debe llamar una vez por ronda resuelta (despues de
   aplicar_resultado_apuesta), no en cada frame. */
const char* verificar_mensaje_reflexivo(Jugador* j);

#endif /* JUGADOR_H */