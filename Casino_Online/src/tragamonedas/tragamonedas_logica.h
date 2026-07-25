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
 * A CARGO DE: Dubenny (logica) / Luis (integracion visual).
 *
 * CONTRATO IMPORTANTE con el resto del proyecto: el dinero, la deuda,
 * las estadisticas de sesion y los mensajes reflexivos de
 * concientizacion son UN SOLO sistema compartido entre todos los
 * minijuegos (ver core/jugador.h). El tragamonedas NO debe inventar su
 * propio manejo de saldo/deuda -debe llamar a las mismas funciones que
 * ya usa la ruleta:
 *   - registrar_apuesta(jugador, monto)             al colocar la apuesta
 *   - aplicar_resultado_apuesta(jugador, ganancia)  al resolver la ronda
 *   - verificar_mensaje_reflexivo(jugador)           despues de cada ronda resuelta
 * Asi, jugar al tragamonedas cuenta para la misma racha de perdidas, el
 * mismo total apostado, el mismo sistema de concientizacion que ya
 * existe -no dos sistemas de plata separados y sin relacion.
 *
 * CONTRATO con la parte visual (Luis, tragamonedas_geometria.c y
 * tragamonedas_animacion.c): los simbolos y su orden en cada rodillo se
 * deciden ACA. La parte visual solo pregunta "que simbolo hay en tal
 * posicion" (obtener_simbolo_en_posicion) y dibuja la respuesta -nunca
 * decide el orden ni el resultado por su cuenta.
 *
 * NUM_RODILLOS se define aqui (no en animacion.h) para que este header
 * sea auto-contenido y las firmas de decidir/calcular puedan usarlo sin
 * depender de animacion.h (evita dependencia circular, ya que
 * animacion.h incluye este header). El #ifndef permite que animacion.h
 * lo redefina con el mismo valor si el orden de inclusion fuera distinto.
 * -----------------------------------------------------------------------
 */
#ifndef TRAGAMONEDAS_LOGICA_H
#define TRAGAMONEDAS_LOGICA_H

/* Numero de rodillos. Definido aqui para que las firmas de
   decidir_resultado / calcular_ganancia sean auto-contenidas.
   En tragamonedas_animacion.h se usa este mismo valor via #include. */
#ifndef NUM_RODILLOS
#define NUM_RODILLOS 3
#endif

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

/* Tira de simbolos del rodillo (se repite en ciclo). Los 3 rodillos
   comparten la misma tira; la variacion de probabilidad real se
   controla mediante pesos en decidir_resultado_tragamonedas(), no
   duplicando simbolos en la tira (que solo tiene una entrada por
   simbolo distinto). */
extern const SimboloTragamonedas ORDEN_TIRA_RODILLO[NUM_SIMBOLOS];

/* Dado un rodillo (0 a NUM_RODILLOS-1) y su posicion actual en
   "unidades de simbolo" (0.0 = primer simbolo centrado, 1.0 = el
   siguiente, ciclando con modulo NUM_SIMBOLOS), devuelve que simbolo
   esta centrado en ese momento. La parte visual llama a esto en cada
   frame mientras el rodillo gira. */
SimboloTragamonedas obtener_simbolo_en_posicion(int rodillo, float posicion);

/* Decide al azar el simbolo que cae en cada rodillo al terminar el
   giro, usando probabilidades PONDERADAS (CEREZA: mas frecuente para
   dar pequenos premios que mantienen el interes; SIETE: muy raro, es
   el jackpot). El resultado se decide ANTES de animar, equivalente a
   como main.c decide numero_ganador con rand()%37 antes de girar la
   bolita de la ruleta. La animacion solo ajusta los rodillos para
   caer visualmente ahi. */
void decidir_resultado_tragamonedas(SimboloTragamonedas resultado[NUM_RODILLOS]);

/* Calcula la ganancia neta (positiva si gano, -monto si no hay
   combinacion pagadora) dado el resultado de los 3 rodillos y el
   monto apostado. Tabla de pagos (multiplicador x monto apostado):
     3 x SIETE       -> x100   (jackpot)
     3 x DIAMANTE    -> x50
     3 x BARRA       -> x20
     3 x CAMPANA     -> x15
     3 x HERRADURA   -> x10
     3 x CEREZA      -> x5
     CEREZA en R0+R1 -> x2     (R2 libre)
     CEREZA solo R0  -> x1     (R1 y R2 sin cereza)
     Sin combinacion -> -monto (perdida) */
float calcular_ganancia_tragamonedas(const SimboloTragamonedas resultado[NUM_RODILLOS], float monto);

#endif /* TRAGAMONEDAS_LOGICA_H */
