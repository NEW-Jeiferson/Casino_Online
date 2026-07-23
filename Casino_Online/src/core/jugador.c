/*
 * jugador.c
 * Implementacion de la logica economica del jugador.
 * Ver jugador.h para la documentacion de cada funcion.
 */
#include "jugador.h"
#include <stddef.h> /* NULL, usado por verificar_mensaje_reflexivo */
#include <stdio.h>  /* sprintf_s, para insertar cifras reales en los mensajes reflexivos */

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
    j->mensajes_reflexivos_mostrados = 0;
    j->rondas_jugadas = 0;
    j->ultima_ronda_notificada = 0;
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

    /* Cuenta esta ronda para el "reality check" neutral por cantidad
       de rondas jugadas (ver verificar_mensaje_reflexivo) -a diferencia
       de los otros 3 disparadores, este no depende de que la ronda
       haya sido buena o mala, asi que se incrementa siempre. */
    j->rondas_jugadas++;
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
   demasiado seguido o casi nunca.

   Ver docs/analisis-ludopatia.md para la fundamentacion completa: cada
   disparador de aca abajo mapea a un criterio clinico (DSM-5) y a
   literatura real sobre mensajes de responsible gambling, no son
   umbrales inventados sin respaldo. */
#define UMBRAL_RACHA_PERDIDAS 5
#define MULTIPLICADOR_HITO_APOSTADO 2.0f /* cada 2x el saldo inicial apostado en total */
#define RONDAS_ENTRE_REALITY_CHECK 5 /* aviso neutral cada N rondas, sin importar el resultado */

const char* verificar_mensaje_reflexivo(Jugador* j) {
    /* Buffer estatico: ver la advertencia en jugador.h sobre su
       tiempo de vida (valido solo hasta la proxima llamada). */
    static char buffer[700];

    /* Prioridad fija: si mas de una condicion se cumple en la misma
       ronda, se muestra solo UNA (nunca se apilan dos pantallas
       reflexivas seguidas). La que no se muestra esta vez sigue
       "pendiente" -su propio contador no se actualiza- asi que va a
       volver a evaluarse en la proxima ronda resuelta.

       BUGFIX/MEJORA (cifras reales en vez de texto generico, a partir
       de McGivern et al. 2019 -mensajes de perdida en un simulador de
       ruleta online, el antecedente mas directo que existe para este
       proyecto- y Wohl et al. sobre feedback personalizado): los tres
       mensajes ahora insertan valores reales del jugador con
       sprintf_s en vez de frases genericas como "varias veces tu
       saldo inicial". Tambien se redactaron en formato de
       autoevaluacion (le hacen una pregunta al jugador sobre SU
       situacion) en vez de puramente informativo, porque la misma
       literatura encontro que ese formato se recuerda y funciona
       mejor. */

    if (j->racha_perdidas_consecutivas >= UMBRAL_RACHA_PERDIDAS && !j->racha_ya_advertida) {
        j->racha_ya_advertida = 1;
        j->mensajes_reflexivos_mostrados++;
        sprintf_s(buffer, sizeof(buffer),
            "Llevas %d rondas seguidas perdiendo.\n"
            "Perseguir las perdidas -seguir apostando\n"
            "para \"recuperar\" lo perdido- es una de las\n"
            "senales mas comunes del juego problematico.\n"
            "\n"
            "De verdad necesitas jugar la proxima ronda,\n"
            "o podrias parar aca?",
            j->racha_perdidas_consecutivas);
        return buffer;
    }

    if (j->veces_sin_fondos >= 2 && j->veces_sin_fondos != j->ultimo_veces_sin_fondos_notificado) {
        j->ultimo_veces_sin_fondos_notificado = j->veces_sin_fondos;
        j->mensajes_reflexivos_mostrados++;
        sprintf_s(buffer, sizeof(buffer),
            "Van %d veces que te quedas sin saldo en\n"
            "esta sesion. Ya acumulas %.2f de deuda\n"
            "(%.2f de eso es puro interes).\n"
            "\n"
            "Asi es como una deuda real empieza a\n"
            "escalar de verdad. Es este el momento\n"
            "de parar?",
            j->veces_sin_fondos, j->deuda, j->interes_acumulado);
        return buffer;
    }

    {
        float siguiente_hito = j->ultimo_hito_apostado_notificado + j->saldo_inicial * MULTIPLICADOR_HITO_APOSTADO;
        if (j->saldo_inicial > 0.0f && j->total_apostado >= siguiente_hito) {
            float multiplo = j->total_apostado / j->saldo_inicial;
            j->ultimo_hito_apostado_notificado = siguiente_hito;
            j->mensajes_reflexivos_mostrados++;
            sprintf_s(buffer, sizeof(buffer),
                "Llevas apostado %.2f en total: %.1fx tu\n"
                "saldo inicial de %.2f.\n"
                "\n"
                "Fuera de un simulador, eso es haber puesto\n"
                "en juego mucho mas dinero del que tenias\n"
                "pensado al sentarte a jugar. Es este el\n"
                "momento de parar?",
                j->total_apostado, multiplo, j->saldo_inicial);
            return buffer;
        }
    }

    /* MEJORA (disparador 4, "reality check" neutral): los 3 anteriores
       solo se disparan si algo te esta yendo MAL (perdes seguido,
       apostas mucho, te quedas sin fondos repetidas veces). Una sesion
       "normal" -ganas algo, perdes algo, nunca una racha catastrofica-
       podia jugar indefinidamente sin ver un solo mensaje, por mas
       rondas que llevara. Las plataformas reguladas reales resuelven
       esto con un aviso obligatorio por tiempo/cantidad de jugadas,
       sin importar si vas ganando o perdiendo (ver
       docs/analisis-ludopatia.md). Este es ese disparador: uno cada 5
       rondas jugadas, siempre, independiente del resultado.
       Prioridad mas baja a proposito -si esta ronda YA disparo alguno
       de los 3 anteriores, este se salta y espera a la siguiente ronda
       multiplo de 5 (nunca se apilan dos mensajes el mismo round). */
    if (j->rondas_jugadas > 0 && j->rondas_jugadas % RONDAS_ENTRE_REALITY_CHECK == 0 && j->rondas_jugadas != j->ultima_ronda_notificada) {
        j->ultima_ronda_notificada = j->rondas_jugadas;
        j->mensajes_reflexivos_mostrados++;
        sprintf_s(buffer, sizeof(buffer),
            "Llevas %d rondas jugadas en esta sesion.\n"
            "\n"
            "No es ni bueno ni malo -es solo un\n"
            "recordatorio de cuanto tiempo llevas en\n"
            "la mesa. Es el ritmo que querias tener\n"
            "cuando te sentaste a jugar?",
            j->rondas_jugadas);
        return buffer;
    }

    return NULL;
}