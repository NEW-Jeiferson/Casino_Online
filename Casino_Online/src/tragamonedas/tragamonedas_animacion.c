/*
 * tragamonedas_animacion.c
 * Implementacion de la animacion de los rodillos. Ver
 * tragamonedas_animacion.h.
 * A CARGO DE: Luis.
 *
 * PASO 4: easing de desaceleracion real, mismo enfoque que
 * actualizar_bolita()/iniciar_giro_bolita_hacia_absoluto() en
 * ruleta_animacion.c -misma curva de Bezier para el frenado (para que
 * ambos minijuegos "se sientan" parecidos), pero en unidades de
 * simbolo en vez de grados.
 */
#include <math.h>
#include <stdlib.h>
#include "tragamonedas_animacion.h"
#include "tragamonedas_logica.h"
#include "../utils/bezier.h"

/* Velocidad tipica de giro, en unidades de simbolo por segundo
   (equivalente a VELOCIDAD_TIPICA_GIRO de la ruleta, pero en la unidad
   que usa el tragamonedas) */
#define VELOCIDAD_TIPICA_GIRO_RODILLO 9.0f

/* Factor de integracion del easing -es una propiedad de la curva de
   Bezier usada (los mismos puntos de control que en
   actualizar_bolita()), no del juego en particular, por eso se reusa
   el mismo valor que ruleta_animacion.c */
#define FACTOR_INTEGRAL_EASING 0.525f

/* Duracion minima/maxima de giro por rodillo, en segundos -bastante
   mas corto que la ruleta (los tragamonedas paran rapido). Se le suma
   un pequenio escalonado por indice de rodillo (DURACION_ESCALON) para
   que no paren los 3 exactamente al mismo tiempo -efecto clasico de
   tragamonedas, donde se van deteniendo de a uno. */
#define DURACION_MINIMA_GIRO_RODILLO 1.4f
#define DURACION_MAXIMA_GIRO_RODILLO 2.6f
#define DURACION_ESCALON_POR_RODILLO 0.35f

/* Cuantas vueltas completas de mas da la tira antes de llegar al
   resultado -solo estetico, para que se vea girar y no "salte" directo
   al simbolo final */
#define VUELTAS_EXTRA_RODILLO 5


/* -------------------------------------------------------------------
 * TEMPORAL: decidir_resultado_tragamonedas() (Dubenny) todavia no
 * esta implementada en tragamonedas_logica.c -este archivo es de
 * Luis, asi que en vez de esperar (o meterse a escribir la logica de
 * otra persona), se arma aca un resultado al azar equivalente, para
 * poder probar y validar la animacion mientras tanto.
 *
 * CUANDO DUBENNY TERMINE SU FUNCION: borrar
 * decidir_resultado_tragamonedas_temporal() completa, y en
 * iniciar_giro_tragamonedas() cambiar la linea que la llama por una
 * llamada a decidir_resultado_tragamonedas() (la de ella, declarada en
 * tragamonedas_logica.h) -un solo cambio de una linea, el resto de
 * esta funcion no cambia.
 * ------------------------------------------------------------------- */
static void decidir_resultado_tragamonedas_temporal(SimboloTragamonedas resultado[NUM_RODILLOS]) {
    int i;
    for (i = 0; i < NUM_RODILLOS; i++) {
        resultado[i] = (SimboloTragamonedas)(rand() % NUM_SIMBOLOS);
    }
}


/* Busca en que posicion "base" (entre 0 y NUM_SIMBOLOS-1) de la tira
   del rodillo hay que dejarlo centrado para que
   obtener_simbolo_en_posicion() devuelva el simbolo pedido. No asume
   que ORDEN_TIRA_RODILLO este en el mismo orden que el enum -pregunta,
   como corresponde segun el contrato con tragamonedas_logica.c. */
static float buscar_posicion_base_para_simbolo(int rodillo, SimboloTragamonedas simbolo_deseado) {
    int k;
    for (k = 0; k < NUM_SIMBOLOS; k++) {
        if (obtener_simbolo_en_posicion(rodillo, (float)k) == simbolo_deseado) {
            return (float)k;
        }
    }
    return 0.0f; /* no deberia pasar si el simbolo pedido existe en la tira */
}


void inicializar_tragamonedas_animacion(EstadoTragamonedas* estado) {
    int i;
    for (i = 0; i < NUM_RODILLOS; i++) {
        estado->rodillos[i].posicion_actual = 0.0f;
        estado->rodillos[i].velocidad = 0.0f;
        estado->rodillos[i].girando = 0;
        estado->rodillos[i].velocidad_inicial = 0.0f;
        estado->rodillos[i].duracion_total = 0.0f;
        estado->rodillos[i].tiempo_transcurrido = 0.0f;
        estado->rodillos[i].posicion_objetivo = 0.0f;
    }
}


void actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo) {
    int i;

    for (i = 0; i < NUM_RODILLOS; i++) {
        EstadoRodillo* rodillo = &estado->rodillos[i];
        Punto3D p0, p1, p2, p3, punto_easing;
        float t, factor;

        if (!rodillo->girando) continue;

        rodillo->tiempo_transcurrido += delta_tiempo;
        t = rodillo->tiempo_transcurrido / rodillo->duracion_total;
        if (t >= 1.0f) t = 1.0f;

        /* Misma curva de easing que actualizar_bolita() en
           ruleta_animacion.c -velocidad alta al principio, cae suave
           hacia el final. */
        p0.x = 0.0f; p0.y = 1.0f;  p0.z = 0.0f;
        p1.x = 0.0f; p1.y = 0.85f; p1.z = 0.0f;
        p2.x = 0.0f; p2.y = 0.25f; p2.z = 0.0f;
        p3.x = 0.0f; p3.y = 0.0f;  p3.z = 0.0f;

        punto_easing = evaluar_bezier_cubica(p0, p1, p2, p3, t);
        factor = punto_easing.y;
        if (factor < 0.0f) factor = 0.0f;

        rodillo->velocidad = rodillo->velocidad_inicial * factor;
        rodillo->posicion_actual += rodillo->velocidad * delta_tiempo;

        if (t >= 1.0f) {
            rodillo->velocidad = 0.0f;
            rodillo->girando = 0;
            rodillo->posicion_actual = rodillo->posicion_objetivo; /* clava el valor exacto, sin arrastre de punto flotante */
        }
    }
}


void iniciar_giro_tragamonedas(EstadoTragamonedas* estado) {
    SimboloTragamonedas resultado[NUM_RODILLOS];
    int i;

    /* No reiniciar un giro si alguno ya esta en marcha */
    for (i = 0; i < NUM_RODILLOS; i++) {
        if (estado->rodillos[i].girando) return;
    }

    /* TEMPORAL: ver el comentario grande arriba de
       decidir_resultado_tragamonedas_temporal(). Cuando Dubenny tenga
       lista la suya, esta es la unica linea que cambia:
         decidir_resultado_tragamonedas(resultado); */
    decidir_resultado_tragamonedas_temporal(resultado);

    for (i = 0; i < NUM_RODILLOS; i++) {
        EstadoRodillo* rodillo = &estado->rodillos[i];
        float posicion_base;
        float actual_mod;
        float delta_relativo;
        float duracion_necesaria;
        float duracion_minima_este_rodillo;
        float duracion_maxima_este_rodillo;

        posicion_base = buscar_posicion_base_para_simbolo(i, resultado[i]);

        actual_mod = (float)fmod(rodillo->posicion_actual, (float)NUM_SIMBOLOS);
        if (actual_mod < 0.0f) actual_mod += (float)NUM_SIMBOLOS;

        delta_relativo = posicion_base - actual_mod;
        while (delta_relativo < 0.0f) delta_relativo += (float)NUM_SIMBOLOS;
        while (delta_relativo >= (float)NUM_SIMBOLOS) delta_relativo -= (float)NUM_SIMBOLOS;

        delta_relativo += (float)NUM_SIMBOLOS * (float)VUELTAS_EXTRA_RODILLO;

        /* Escalonado: cada rodillo (de izquierda a derecha) para un
           poco mas tarde que el anterior, como en una tragamonedas real */
        duracion_minima_este_rodillo = DURACION_MINIMA_GIRO_RODILLO + (float)i * DURACION_ESCALON_POR_RODILLO;
        duracion_maxima_este_rodillo = DURACION_MAXIMA_GIRO_RODILLO + (float)i * DURACION_ESCALON_POR_RODILLO;

        duracion_necesaria = delta_relativo / (VELOCIDAD_TIPICA_GIRO_RODILLO * FACTOR_INTEGRAL_EASING);
        rodillo->velocidad_inicial = VELOCIDAD_TIPICA_GIRO_RODILLO;

        if (duracion_necesaria < duracion_minima_este_rodillo) {
            duracion_necesaria = duracion_minima_este_rodillo;
            rodillo->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
        }
        else if (duracion_necesaria > duracion_maxima_este_rodillo) {
            duracion_necesaria = duracion_maxima_este_rodillo;
            rodillo->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
        }

        rodillo->velocidad = rodillo->velocidad_inicial;
        rodillo->duracion_total = duracion_necesaria;
        rodillo->girando = 1;
        rodillo->tiempo_transcurrido = 0.0f;
        rodillo->posicion_objetivo = rodillo->posicion_actual + delta_relativo;
    }
}
