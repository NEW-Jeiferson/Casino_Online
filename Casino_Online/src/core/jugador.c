/*
* jugador.c
* Implementacion de la logica economica del jugador.
* Ver jugador.h para la documentacion de cada funcion.
*/
#include "jugador.h"
#include <stddef.h> 


/* Inicializacion y actualizacion de estado */
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


/* Logica de apuestas y prestamos */
void registrar_apuesta(Jugador* j, float monto) {
    j->total_apostado += monto;
}


/* Anula una apuesta previamente registrada, para casos donde la apuesta se cancela antes de resolverse */
void anular_apuesta(Jugador* j, float monto) {
    j->total_apostado -= monto;
    if (j->total_apostado < 0.0f) j->total_apostado = 0.0f;
}


/* Aplica el resultado de una apuesta (ganancia o perdida) al saldo del jugador, y actualiza la racha de perdidas consecutivas */
void aplicar_resultado_apuesta(Jugador* j, float ganancia) {

    int fue_perdida = (ganancia < 0.0f);

	/* Si el jugador tiene deuda, primero se aplica la ganancia a pagar la deuda antes de sumarla al saldo */
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


	/* Actualiza la racha de perdidas consecutivas y el flag de aviso */
    if (fue_perdida) {
        j->racha_perdidas_consecutivas++;
    }
    else {
        j->racha_perdidas_consecutivas = 0;
        j->racha_ya_advertida = 0;
    }
}


/* Prestamos y deuda */
void pedir_prestamo(Jugador* j, float monto, float tasa_interes) {
    float interes = monto * tasa_interes;
    j->deuda += monto + interes;
    j->interes_acumulado += interes;
    j->prestamos_activos++;
    j->saldo += monto;
}


/* Devuelve 1 si la deuda del jugador es mayor o igual al limite_deuda, 0 en caso contrario */
int deuda_es_impagable(const Jugador* j, float limite_deuda) {
    return j->deuda >= limite_deuda;
}


#define UMBRAL_RACHA_PERDIDAS 5
#define MULTIPLICADOR_HITO_APOSTADO 2.0f

/* Devuelve un mensaje reflexivo si el jugador cumple alguna condicion de riesgo, o NULL si no hay mensaje que mostrar.
   El mensaje se muestra una sola vez por cada condicion, y se resetea cuando la condicion deja de cumplirse. */
const char* verificar_mensaje_reflexivo(Jugador* j) {


	/* Mensaje por racha de perdidas consecutivas */
    if (j->racha_perdidas_consecutivas >= UMBRAL_RACHA_PERDIDAS && !j->racha_ya_advertida) {
        j->racha_ya_advertida = 1;
        return "Llevas varias rondas seguidas perdiendo.\n"
            "Perseguir las perdidas, es decir, seguir jugando\n"
            "esperando \"recuperar\" lo perdido, es una de los\n"
            "signos mas comunes del juego problematico.\n"
            "\n"
            "Este es un buen momento para hacer una pausa.";
    }


	/* Mensaje por quedarse sin fondos varias veces */
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

		/* Mensaje por haber apostado varias veces el saldo inicial */
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