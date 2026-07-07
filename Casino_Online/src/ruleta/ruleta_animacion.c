/*
 * ruleta_animacion.c
 * Implementacion de la animacion de la bolita. Ver ruleta_animacion.h.
 */
#include <GL/glut.h>
#include "ruleta_animacion.h"
#include "../utils/bezier.h"

void inicializar_bolita(EstadoBolita* bolita) {
    bolita->angulo_actual = 0.0f;
    bolita->velocidad = 0.0f;
    bolita->girando = 0;
    bolita->tiempo_transcurrido = 0.0f;
}

void iniciar_giro_bolita(EstadoBolita* bolita, float velocidad_inicial) {
    bolita->velocidad = velocidad_inicial;
    bolita->girando = 1;
    bolita->tiempo_transcurrido = 0.0f;
}

void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo) {
    if (!bolita->girando) return;

    /* TODO: usar evaluar_bezier_cubica() con puntos de control que
       representen la curva de desaceleracion (easing), en vez de una
       resta lineal simple, y detener bolita->girando cuando el tiempo
       total de la animacion se cumpla. */
    bolita->tiempo_transcurrido += delta_tiempo;
    bolita->angulo_actual += bolita->velocidad * delta_tiempo;
}

void dibujar_bolita(const EstadoBolita* bolita) {
    glPushMatrix();
    glRotatef(bolita->angulo_actual, 0.0f, 1.0f, 0.0f);
    /* TODO: trasladar al radio de la rueda y dibujar una esfera pequena
       (glutSolidSphere) con su material correspondiente */
    glPopMatrix();
}
