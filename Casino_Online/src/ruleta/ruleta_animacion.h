/*
 * ruleta_animacion.h
 * -----------------------------------------------------------------------
 * Animacion de la bolita: giro y desaceleracion sobre la rueda, usando
 * una curva de Bezier como funcion de easing para el frenado.
 *
 * Responsable: Jeiferson
 * -----------------------------------------------------------------------
 */
#ifndef RULETA_ANIMACION_H
#define RULETA_ANIMACION_H

 /* Velocidad a la que debe girar la RUEDA (no la bolita) mientras la
    bolita esta en movimiento. Se expone aqui, y main.c debe usar ESTA
    misma constante en su idle() para incrementar angulo_rueda -no un
    numero repetido a mano-, porque iniciar_giro_bolita_hacia_absoluto()
    asume este valor exacto al resolver la duracion del giro. Si alguna
    vez cambia, hay que cambiarla solo aqui.
    DOBLADA de 81 a 162 (a pedido explicito: "el doble de rapido"), junto
    con VELOCIDAD_TIPICA_GIRO y los limites de duracion en
    ruleta_animacion.c, para que la rueda y la bolita se sigan viendo a
    velocidades proporcionadas entre si. */
#define VELOCIDAD_RUEDA_DURANTE_GIRO 162.0f

typedef struct {
    float angulo_actual;      /* posicion angular de la bolita en la rueda */
    float velocidad;          /* velocidad angular actual (resultado del easing) */
    float velocidad_inicial;  /* velocidad angular al iniciar el giro */
    float duracion_total;     /* duracion total de la animacion de frenado (segundos) */
    int   girando;            /* 1 mientras la bolita esta en movimiento */
    float tiempo_transcurrido;
} EstadoBolita;

/* Inicializa el estado de la bolita (detenida, angulo 0) */
void inicializar_bolita(EstadoBolita* bolita);

/* Inicia el giro resolviendo, de una sola vez, cuanto deben avanzar
   JUNTAS la rueda y la bolita para que la posicion ABSOLUTA final de
   la bolita (rueda + bolita combinadas) avance exactamente
   delta_absoluto_deseado grados (mas 'vueltas_extra' vueltas completas
   de regalo visual, recomendado 1).

   IMPORTANTE: no intenta predecir por separado "donde va a quedar la
   rueda" -eso causaba un bug real (la rueda no gira un angulo fijo por
   giro, porque la duracion del giro ya no es fija). En su lugar,
   calcula la duracion contra la tasa COMBINADA de avance (rueda +
   bolita), lo cual es correcto sin importar cuanto dure el giro.

   Este es el metodo que se debe usar para resolver una apuesta: el
   numero ganador se decide ANTES de llamar a esta funcion (con
   rand() % 37, uniforme de verdad, ver main.c), y delta_absoluto_deseado
   se calcula como (angulo_del_sector_elegido - posicion_absoluta_actual
   de la bolita). */
void iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float delta_absoluto_deseado, int vueltas_extra);

/* Actualiza la posicion/velocidad de la bolita para el frame actual,
   usando una curva de Bezier cubica para la curva de desaceleracion */
void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo);

/* Dibuja la bolita en su posicion actual sobre la rueda */
void dibujar_bolita(const EstadoBolita* bolita);

#endif /* RULETA_ANIMACION_H */