/*
 * jugador.c
 * Implementacion de la logica economica del jugador.
 * Ver jugador.h para la documentacion de cada funcion.
 */
#include "jugador.h"
#include <stddef.h> /* NULL, usado por verificar_mensaje_reflexivo */
#include <stdio.h>  /* sprintf_s, para insertar cifras reales en los mensajes reflexivos */

void inicializar_jugador(Jugador* j, float saldo_inicial, int tiempo_actual_ms) {
    j->saldo = saldo_inicial;
    j->deuda = 0.0f;
    j->prestamos_activos = 0;
    j->total_apostado = 0.0f;
    j->total_ganado = 0.0f;
    j->total_perdido = 0.0f;
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
    j->tiempo_inicio_ms = tiempo_actual_ms;
}

void registrar_apuesta(Jugador* j, float monto) {
    j->total_apostado += monto;
}

void anular_apuesta(Jugador* j, float monto) {
    j->total_apostado -= monto;
    if (j->total_apostado < 0.0f) j->total_apostado = 0.0f;
}

void aplicar_resultado_apuesta(Jugador* j, float ganancia) {
    /* Se guarda el signo y valor ORIGINAL antes de que el bloque de abajo
       (pago de deuda) pueda modificar 'ganancia'. */
    int fue_perdida = (ganancia < 0.0f);

    /* Acumulacion de estadisticas de la sesion usando el resultado bruto original */
    if (ganancia > 0.0f) {
        j->total_ganado += ganancia;
    }
    else if (ganancia < 0.0f) {
        j->total_perdido += -ganancia;
    }

    /* Si el jugador tiene deuda pendiente y esta ronda dio ganancia neta positiva,
       esa ganancia abona la deuda primero (hasta saldarla) antes de sumarse al saldo disponible. */
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

int calcular_nivel_riesgo(const Jugador* j, int tiempo_actual_ms) {
    int puntos = 0;
    float mins;

    if (j == NULL) return 0;

    mins = tiempo_jugado_minutos(j, tiempo_actual_ms);

    if (j->racha_perdidas_consecutivas >= 3) puntos += 1;
    if (j->racha_perdidas_consecutivas >= 5) puntos += 1;
    
    if (j->veces_sin_fondos >= 1) puntos += 2;
    if (j->veces_sin_fondos >= 2) puntos += 2;

    if (j->prestamos_activos >= 1) puntos += 2;
    if (j->prestamos_activos >= 2) puntos += 2;

    if (j->deuda > 0.0f) puntos += 1;

    if (j->total_apostado >= j->saldo_inicial * 1.5f) puntos += 1;
    if (j->total_apostado >= j->saldo_inicial * 3.0f) puntos += 1;
    if (mins >= 15.0f) puntos += 1;

    if (puntos == 0) return 0;
    if (puntos <= 2) return 1;
    if (puntos <= 4) return 2;
    return 3;
}

/* --- Concientizacion sobre ludopatia --- */
#define UMBRAL_RACHA_PERDIDAS 5
#define MULTIPLICADOR_HITO_APOSTADO 2.0f /* cada 2x el saldo inicial apostado en total */
#define RONDAS_ENTRE_REALITY_CHECK 5 /* aviso neutral cada N rondas, sin importar el resultado */

static const char* DATOS_EDUCATIVOS_GENERALES_RULETA[8] = {
    "Dato: en la ruleta, cada giro es totalmente\n"
    "independiente del anterior.\n"
    "\n"
    "Una racha de perdidas (o de victorias) no hace\n"
    "que el proximo resultado sea mas o menos\n"
    "probable. Es la base matematica del juego.",

    "Dato: la ludopatia esta reconocida como un\n"
    "trastorno clinico real (DSM-5), no como una\n"
    "falta de fuerza de voluntad.\n"
    "\n"
    "Se trata igual que otras adicciones, aunque no\n"
    "involucre ninguna sustancia.",

    "Dato: necesitar apostar montos cada vez\n"
    "mayores para sentir la misma emocion se llama\n"
    "tolerancia, y es uno de los signos de alerta\n"
    "reconocidos del juego problematico.\n"
    "\n"
    "Si notas que te esta pasando, vale la pena\n"
    "prestarle atencion.",

    "Dato: definir un presupuesto y un limite de\n"
    "tiempo ANTES de jugar, y respetarlo pase lo\n"
    "que pase, es una de las formas mas efectivas\n"
    "de jugar de manera responsable.\n"
    "\n"
    "Hay mas info en la pantalla de Informacion\n"
    "([I] desde el menu).",

    "Dato: mentir sobre cuanto dinero se juega\n"
    "o se pierde es un signo de alerta clinico\n"
    "reconocido internacionalmente (DSM-5).\n"
    "\n"
    "La transparencia personal es clave para detectar\n"
    "el juego de riesgo.",

    "Dato: jugar para intentar escapar del estres,\n"
    "la ansiedad o el aburrimiento es un patron\n"
    "de riesgo, no una forma sana de descanso.\n"
    "\n"
    "Buscar alternativas saludables protege tu salud.",

    "Dato: ningun sistema de apuestas ni patron\n"
    "visual altera la probabilidad matematica real\n"
    "de la ruleta europea (2.7 por ciento casa).\n"
    "\n"
    "Los patrones percibidos son solo ilusiones.",

    "Dato: solicitar dinero prestado para intentar\n"
    "recuperar lo perdido es una escalada real de\n"
    "deuda, no una solucion temporal.\n"
    "\n"
    "Detenerte a tiempo previene mayores danos."
};

static const char* DATOS_EDUCATIVOS_GENERALES_TRAGAMONEDAS[8] = {
    "Dato: en las tragamonedas, cada giro es totalmente\n"
    "independiente del anterior por el sistema RNG.\n"
    "\n"
    "Una racha sin premios no hace que un premio\n"
    "este 'por salir'. Es la base del algoritmo.",

    "Dato: la ludopatia esta reconocida como un\n"
    "trastorno clinico real (DSM-5), no como una\n"
    "falta de fuerza de voluntad.\n"
    "\n"
    "Se trata igual que otras adicciones, aunque no\n"
    "involucre ninguna sustancia.",

    "Dato: necesitar jugar mas rapido o apostar\n"
    "mas para sentir la misma emocion se llama\n"
    "tolerancia, y es un signo de alerta clinico.\n"
    "\n"
    "Si notas que te esta pasando, vale la pena\n"
    "prestarle atencion.",

    "Dato: definir un presupuesto y un limite de\n"
    "tiempo ANTES de jugar, y respetarlo pase lo\n"
    "que pase, es vital en tragamonedas rapidas.\n"
    "\n"
    "Hay mas info en la pantalla de Informacion\n"
    "([I] desde el menu).",

    "Dato: mentir sobre cuanto dinero se juega\n"
    "o se pierde es un signo de alerta clinico\n"
    "reconocido internacionalmente (DSM-5).\n"
    "\n"
    "La transparencia personal es clave para detectar\n"
    "el juego de riesgo.",

    "Dato: jugar para intentar escapar del estres,\n"
    "la ansiedad o el aburrimiento es un patron\n"
    "de riesgo, no una forma sana de descanso.\n"
    "\n"
    "Buscar alternativas saludables protege tu salud.",

    "Dato: los 'casi aciertos' (near misses) son\n"
    "ilusiones opticas programadas para enganarte\n"
    "y que sientas que 'casi ganas'.\n"
    "\n"
    "Matematicamente, perdiste igual que siempre.",

    "Dato: las 'falsas victorias' (ganar menos\n"
    "dinero del que apostaste en un giro) enganan\n"
    "al cerebro con luces y sonidos de victoria.\n"
    "\n"
    "Detenerte a tiempo previene mayores danos."
};

static const char* DATOS_EDUCATIVOS_GENERALES_DADOS[8] = {
    "Dato: en los dados, soplar o lanzar los\n"
    "dados de cierta forma no altera las matematicas.\n"
    "\n"
    "Creer que tienes el control sobre el resultado\n"
    "fomenta la conducta de juego problematico.",

    "Dato: la ludopatia esta reconocida como un\n"
    "trastorno clinico real (DSM-5), no como una\n"
    "falta de fuerza de voluntad.\n"
    "\n"
    "Se trata igual que otras adicciones, aunque no\n"
    "involucre ninguna sustancia.",

    "Dato: necesitar apostar montos cada vez\n"
    "mayores para sentir la misma emocion se llama\n"
    "tolerancia, y es uno de los signos de alerta\n"
    "reconocidos del juego problematico.\n"
    "\n"
    "Si notas que te esta pasando, vale la pena\n"
    "prestarle atencion.",

    "Dato: definir un presupuesto y un limite de\n"
    "tiempo ANTES de jugar, y respetarlo pase lo\n"
    "que pase, es una de las formas mas efectivas\n"
    "de jugar de manera responsable.\n"
    "\n"
    "Hay mas info en la pantalla de Informacion\n"
    "([I] desde el menu).",

    "Dato: mentir sobre cuanto dinero se juega\n"
    "o se pierde es un signo de alerta clinico\n"
    "reconocido internacionalmente (DSM-5).\n"
    "\n"
    "La transparencia personal es clave para detectar\n"
    "el juego de riesgo.",

    "Dato: jugar para intentar escapar del estres,\n"
    "la ansiedad o el aburrimiento es un patron\n"
    "de riesgo, no una forma sana de descanso.\n"
    "\n"
    "Buscar alternativas saludables protege tu salud.",

    "Dato: apostar al Par/Impar paga 0.9x en vez\n"
    "de 1.0x para dar ventaja matematica a la casa.\n"
    "\n"
    "Todas las apuestas estan calculadas en tu contra\n"
    "a largo plazo.",

    "Dato: solicitar dinero prestado para intentar\n"
    "recuperar lo perdido es una escalada real de\n"
    "deuda, no una solucion temporal.\n"
    "\n"
    "Detenerte a tiempo previene mayores danos."
};

const char* verificar_mensaje_reflexivo(Jugador* j, int juego_activo) {
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
            "\n"
            "Cuando seguimos jugando solo para recuperar\n"
            "lo que perdimos, se llama \"perseguir las\n"
            "perdidas\": es una de las muestras mas\n"
            "comunes del juego problematico.\n"
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
            "esta sesion. Ya acumulas %.2f de deuda,\n"
            "y %.2f de eso es puro interes.\n"
            "\n"
            "Asi es como una deuda real empieza a\n"
            "escalar. Es este el momento de parar?",
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
                "Llevas apostado %.2f en total: %.1f veces\n"
                "tu saldo inicial de %.2f.\n"
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
       multiplo de 5 (nunca se apilan dos mensajes el mismo round).

       MEJORA (parte 3): en vez de mostrar SIEMPRE el mismo "llevas X
       rondas...", alterna con un dato educativo general (ver
       DATOS_EDUCATIVOS_GENERALES arriba) -ocurrencias impares (1ra,
       3ra, 5ta...) muestran el reality check personal; ocurrencias
       pares muestran un dato, rotando por el pool para no repetir
       siempre el mismo. Asi el disparador se siente menos repetitivo
       en sesiones largas, y refuerza la concientizacion con contenido
       distinto al personalizado. */
    if (j->rondas_jugadas > 0 && j->rondas_jugadas % RONDAS_ENTRE_REALITY_CHECK == 0 && j->rondas_jugadas != j->ultima_ronda_notificada) {
        int ocurrencia = j->rondas_jugadas / RONDAS_ENTRE_REALITY_CHECK;
        j->ultima_ronda_notificada = j->rondas_jugadas;
        j->mensajes_reflexivos_mostrados++;

        if (ocurrencia % 2 == 1) {
            sprintf_s(buffer, sizeof(buffer),
                "Llevas %d rondas jugadas en esta sesion.\n"
                "\n"
                "No es ni bueno ni malo, es solo un\n"
                "recordatorio de cuanto tiempo llevas en\n"
                "la mesa. Es el ritmo que querias tener\n"
                "cuando te sentaste a jugar?",
                j->rondas_jugadas);
        }
        else {
            int indice_dato = (ocurrencia / 2 - 1) % 8;
            if (indice_dato < 0) indice_dato = 0;
            
            if (juego_activo == 1) {
                sprintf_s(buffer, sizeof(buffer), "%s", DATOS_EDUCATIVOS_GENERALES_TRAGAMONEDAS[indice_dato]);
            } else if (juego_activo == 2) {
                sprintf_s(buffer, sizeof(buffer), "%s", DATOS_EDUCATIVOS_GENERALES_DADOS[indice_dato]);
            } else {
                sprintf_s(buffer, sizeof(buffer), "%s", DATOS_EDUCATIVOS_GENERALES_RULETA[indice_dato]);
            }
        }
        return buffer;
    }

    return NULL;
}

float tiempo_jugado_minutos(const Jugador* j, int tiempo_actual_ms) {
    int delta_ms;
    if (j == NULL) return 0.0f;
    delta_ms = tiempo_actual_ms - j->tiempo_inicio_ms;
    if (delta_ms < 0) delta_ms = 0; /* proteccion, no deberia pasar nunca */
    return (float)delta_ms / 60000.0f;
}