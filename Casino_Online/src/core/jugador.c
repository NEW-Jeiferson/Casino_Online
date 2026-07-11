/*
 * jugador.c
 * Implementacion de la logica economica del jugador.
 * Ver jugador.h para la documentacion de cada funcion.
 */
#include "jugador.h"
#include <stddef.h> /* NULL, usado por verificar_mensaje_reflexivo */

void inicializar_jugador(Jugador* j, float saldo_inicial) {
    j->saldo = saldo_inicial;
    j->deuda = 0.0f;
    j->prestamos_activos = 0;
    j->total_apostado = 0.0f;
    j->veces_sin_fondos = 0;
    j->interes_acumulado = 0.0f;

    j->saldo_inicial = saldo_inicial;
    j->racha_perdidas_consecutivas = 0;
    j->racha_ya_advertida = 0;
    j->ultimo_hito_apostado_notificado = 0.0f;
    j->ultimo_veces_sin_fondos_notificado = 0;
}

void registrar_apuesta(Jugador* j, float monto) {
    j->total_apostado += monto;
}

void anular_apuesta(Jugador* j, float monto) {
    j->total_apostado -= monto;
    if (j->total_apostado < 0.0f) j->total_apostado = 0.0f;
}

void aplicar_resultado_apuesta(Jugador* j, float ganancia) {
    /* Se guarda el signo ANTES de que el bloque de abajo (pago de
       deuda) pueda achicar 'ganancia' -pagar deuda solo pasa cuando
       ganancia > 0, asi que nunca le cambia el signo, pero se deja
       explicito para que este calculo no dependa de ese detalle. */
    int fue_perdida = (ganancia < 0.0f);

    /* MEJORA OPCIONAL: si el jugador tiene deuda pendiente y esta
       ronda dio ganancia neta positiva, esa ganancia abona la deuda
       primero (hasta saldarla) antes de sumarse al saldo disponible.
       Antes la deuda solo podia crecer (via pedir_prestamo) y nunca
       bajaba con nada que pasara en la mesa -lo cual no reflejaba
       "pagar la deuda apostando", una escalada real del jugador
       problematico que el proyecto busca mostrar. Si se prefiere el
       comportamiento original (deuda solo baja pidiendo mas
       prestamos, nunca jugando), basta con borrar este bloque. */
    if (ganancia > 0.0f && j->deuda > 0.0f) {
        float abono = (ganancia < j->deuda) ? ganancia : j->deuda;
        j->deuda -= abono;
        ganancia -= abono;
    }

    j->saldo += ganancia;

    if (j->saldo <= 0.0f) {
        j->saldo = 0.0f;
        j->veces_sin_fondos++;
    }

    /* Racha de perdidas consecutivas, para el sistema de mensajes
       reflexivos (ver verificar_mensaje_reflexivo). Un resultado neto
       de exactamente 0.0 no cuenta como perdida (no bajo el saldo). */
    if (fue_perdida) {
        j->racha_perdidas_consecutivas++;
    }
    else {
        j->racha_perdidas_consecutivas = 0;
        j->racha_ya_advertida = 0; /* nueva racha futura puede volver a avisar */
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

/* --- Concientizacion sobre ludopatia ---
   Umbrales elegidos para un MVP de demo (una sesion de juego dura
   minutos, no dias reales), pensados para que un jugador que juega
   "normal" rara vez los vea, pero alguien que efectivamente esta
   mostrando el patron de juego problematico que el proyecto busca
   ilustrar los vea con claridad. Recalibrar si en la demo se disparan
   demasiado seguido o casi nunca. */
#define UMBRAL_RACHA_PERDIDAS 5
#define MULTIPLICADOR_HITO_APOSTADO 2.0f /* cada 2x el saldo inicial apostado en total */

const char* verificar_mensaje_reflexivo(Jugador* j) {
    /* Prioridad fija: si mas de una condicion se cumple en la misma
       ronda, se muestra solo UNA (nunca se apilan dos pantallas
       reflexivas seguidas). La que no se muestra esta vez sigue
       "pendiente" -su propio contador no se actualiza- asi que va a
       volver a evaluarse en la proxima ronda resuelta. */

    if (j->racha_perdidas_consecutivas >= UMBRAL_RACHA_PERDIDAS && !j->racha_ya_advertida) {
        j->racha_ya_advertida = 1;
        return "Llevas varias rondas seguidas perdiendo.\n"
            "Perseguir las perdidas -seguir jugando esperando\n"
            "\"recuperar\" lo perdido- es una de las senales mas\n"
            "comunes del juego problematico.\n"
            "\n"
            "Este es un buen momento para hacer una pausa.";
    }

    if (j->veces_sin_fondos >= 2 && j->veces_sin_fondos != j->ultimo_veces_sin_fondos_notificado) {
        j->ultimo_veces_sin_fondos_notificado = j->veces_sin_fondos;
        return "Te quedaste sin saldo otra vez en esta sesion.\n"
            "En la vida real, este es exactamente el momento en\n"
            "el que la deuda empieza a escalar de verdad.\n"
            "\n"
            "Si esto te esta pasando fuera de este simulador,\n"
            "hablarlo con alguien de confianza o un profesional\n"
            "puede ayudar.";
    }

    {
        float siguiente_hito = j->ultimo_hito_apostado_notificado + j->saldo_inicial * MULTIPLICADOR_HITO_APOSTADO;
        if (j->saldo_inicial > 0.0f && j->total_apostado >= siguiente_hito) {
            j->ultimo_hito_apostado_notificado = siguiente_hito;
            return "Ya llevas apostado, en total, varias veces tu\n"
                "saldo inicial en esta sesion.\n"
                "\n"
                "Fuera de un simulador, esto equivale a haber puesto\n"
                "en juego mucho mas dinero del que tenias pensado al\n"
                "sentarte a jugar. Vale la pena preguntarse si este\n"
                "seria el momento de parar.";
        }
    }

    return NULL;
}