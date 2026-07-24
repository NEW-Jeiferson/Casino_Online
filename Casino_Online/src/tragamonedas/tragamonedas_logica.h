/*
 * tragamonedas_logica.h
 * -----------------------------------------------------------------------
 * Reglas propias del tragamonedas: simbolos, lineas de pago, calculo de
 * ganancia. Es el equivalente, para este minijuego, de lo que
 * calcular_ganancia_apuesta()/calcular_ganancia_total() son para la
 * ruleta en core/estado_juego.c -pero se queda ACA, en su propia
 * carpeta, en vez de meterse en core/, porque son reglas especificas
 * de este juego (core/ es el estado compartido entre minijuegos, no el
 * lugar para reglas de un juego en particular).
 *
 * A CARGO DE: Dubenny.
 *
 * CONTRATO IMPORTANTE con el resto del proyecto: el dinero, la deuda,
 * las estadisticas de sesion y los mensajes reflexivos de
 * concientizacion son UN SOLO sistema compartido entre todos los
 * minijuegos (ver core/jugador.h). El tragamonedas NO debe inventar su
 * propio manejo de saldo/deuda -debe llamar a las mismas funciones que
 * ya usa la ruleta:
 *   - registrar_apuesta(jugador, monto)             al colocar la apuesta
 *   - aplicar_resultado_apuesta(jugador, ganancia)  al resolver la ronda
 *   - verificar_mensaje_reflexivo(jugador)          despues de cada ronda resuelta
 * Asi, jugar al tragamonedas cuenta para la misma racha de perdidas, el
 * mismo total apostado, el mismo sistema de concientizacion que ya
 * existe -no dos sistemas de plata separados y sin relacion.
 *
 * CONTRATO con la parte visual (Luis, tragamonedas_geometria.c y
 * tragamonedas_animacion.c): los simbolos y su orden en cada rodillo se
 * deciden ACA. La parte visual solo pregunta "que simbolo hay en tal
 * posicion" (obtener_simbolo_en_posicion) y dibuja la respuesta -nunca
 * decide el orden ni el resultado por su cuenta.
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_LOGICA_H
#define TRAGAMONEDAS_LOGICA_H

typedef enum {
    SIMBOLO_CEREZA,
    SIMBOLO_CAMPANA,
    SIMBOLO_HERRADURA,
    SIMBOLO_DIAMANTE,
    SIMBOLO_BARRA,
    SIMBOLO_SIETE,
    NUM_SIMBOLOS /* cantidad total de simbolos distintos -usar esta constante
                    para dimensionar arrays en vez de un numero a mano, asi
                    si se agrega/saca un simbolo no hay que buscar todos los
                    lugares donde aparecia el numero viejo */
} SimboloTragamonedas;

/* Tira de simbolos de un rodillo (se repite en ciclo, como en una
   maquina real). Por ahora los 3 rodillos usan la MISMA tira -se puede
   diferenciar por rodillo mas adelante si hace falta variar la
   probabilidad de cada uno, pero no hace falta para arrancar. */
extern const SimboloTragamonedas ORDEN_TIRA_RODILLO[NUM_SIMBOLOS];

/* Dado un rodillo (0 a NUM_RODILLOS-1, ver tragamonedas_animacion.h) y
   su posicion actual (EstadoRodillo.posicion_actual, en "unidades de
   simbolo": 0.0 = el primer simbolo de la tira centrado, 1.0 = el
   siguiente, 2.5 = a mitad de camino entre el tercero y el cuarto,
   etc., ciclando con modulo cuando pasa de NUM_SIMBOLOS), devuelve que
   simbolo esta centrado en ese momento. La parte visual llama a esto
   en cada frame mientras el rodillo gira, para saber que dibujar. Ya
   esta implementada (ver tragamonedas_logica.c) -es simple aritmetica,
   no hace falta tocarla salvo que cambie como se representa la
   posicion. */
SimboloTragamonedas obtener_simbolo_en_posicion(int rodillo, float posicion);

/* TODO (Dubenny): funcion que decida el resultado al azar (que simbolo
   cae en cada rodillo al detenerse) - equivalente a como main.c decide
   numero_ganador con rand() % 37 ANTES de que gire la bolita en la
   ruleta. Sugerencia de firma:
     void decidir_resultado_tragamonedas(SimboloTragamonedas resultado[NUM_RODILLOS]);
*/

/* TODO (Dubenny): funcion equivalente a calcular_ganancia_total() de la
   ruleta (core/estado_juego.c), que reciba el resultado (los simbolos
   que cayeron, ver arriba) y el monto apostado, y devuelva la ganancia
   neta (positiva si gana, negativa el monto perdido si no hay
   combinacion que pague). Sugerencia de firma:
     float calcular_ganancia_tragamonedas(const SimboloTragamonedas resultado[NUM_RODILLOS], float monto);
*/

#endif /* TRAGAMONEDAS_LOGICA_H */
