/*
 * ruleta_animacion.h
 * -----------------------------------------------------------------------
 * Animacion de la bolita: giro y desaceleracion sobre la rueda, usando
 * una curva de Bezier como funcion de easing para el frenado.
 *
 * Responsable sugerido: Persona A
 * -----------------------------------------------------------------------
 */
#ifndef RULETA_ANIMACION_H
#define RULETA_ANIMACION_H

typedef struct {
    float angulo_actual;    /* posicion angular de la bolita en la rueda */
    float velocidad;        /* velocidad angular actual */
    int   girando;          /* 1 mientras la bolita esta en movimiento */
    float tiempo_transcurrido;
} EstadoBolita;

/* Inicializa el estado de la bolita (detenida, angulo 0) */
void inicializar_bolita(EstadoBolita* bolita);

/* Inicia el giro de la bolita con una velocidad inicial dada */
void iniciar_giro_bolita(EstadoBolita* bolita, float velocidad_inicial);

/* Actualiza la posicion/velocidad de la bolita para el frame actual,
   usando una curva de Bezier cubica para la curva de desaceleracion */
void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo);

/* Dibuja la bolita en su posicion actual sobre la rueda */
void dibujar_bolita(const EstadoBolita* bolita);

#endif /* RULETA_ANIMACION_H */
