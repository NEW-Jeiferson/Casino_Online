/*
 * tragamonedas_animacion.c
 * Implementacion (esqueleto) de la animacion de los rodillos. Ver
 * tragamonedas_animacion.h.
 * A CARGO DE: Luis.
 */
#include <stddef.h>
#include "tragamonedas_animacion.h"

void inicializar_tragamonedas_animacion(EstadoTragamonedas* estado) {
    int i;
    for (i = 0; i < NUM_RODILLOS; i++) {
        estado->rodillos[i].posicion_actual = 0.0f;
        estado->rodillos[i].velocidad = 0.0f;
        estado->rodillos[i].girando = 0;
    }
}

void actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo) {
    /* TODO: implementar el easing de desaceleracion real. Punto de
       partida sugerido (ver ruleta_animacion.c para el patron completo
       ya resuelto en la ruleta):
       1. Cada rodillo tiene su propia duracion total de giro.
       2. La posicion avanza segun una curva de easing (por ejemplo,
          usando utils/bezier.c) para que la desaceleracion se sienta
          natural, no lineal.
       3. Cuando el tiempo transcurrido llega a la duracion total,
          girando pasa a 0 y posicion_actual queda exactamente en el
          simbolo que decidio decidir_resultado_tragamonedas(). */
    (void)estado;       /* evita warning de parametro sin usar mientras esto es un stub */
    (void)delta_tiempo;
}

void iniciar_giro_tragamonedas(EstadoTragamonedas* estado) {
    /* TODO: llamar a decidir_resultado_tragamonedas() (Dubenny) para
       saber a que posicion tiene que llegar cada rodillo, y calcular
       cuanto tiene que girar cada uno para terminar ahi -mismo patron
       que iniciar_giro_bolita_hacia_absoluto() en ruleta_animacion.c. */
    int i;
    for (i = 0; i < NUM_RODILLOS; i++) {
        estado->rodillos[i].girando = 1;
    }
}
