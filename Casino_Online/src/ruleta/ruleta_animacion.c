/*
* Implementacion de la animacion de la bolita. Ver ruleta_animacion.h.
*/
#include <GL/glut.h>
#include <math.h>
#include "ruleta_animacion.h"
#include "ruleta_geometria.h"
#include "../render/materiales.h"
#include "../utils/bezier.h"


/* Sirve para definir la duracion base del giro de la bolita, en segundos. */
#define DURACION_GIRO_BOLA 4.0f


/* Esto es para definir el radio de la orbita de la bolita */
#define RADIO_ORBITA_BOLITA ((RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f)


/* Esto es la velocidad tipica con la que se ve giro la bolita. */
#define VELOCIDAD_TIPICA_GIRO 378.0f


/* Esto es para definir el factor de integracion para el easing, el cual sirve para calcular la duracion del giro */
#define FACTOR_INTEGRAL_EASING 0.525f


/* Esto es para definir la duracion minima y maxima del giro de la bolita, en segundos. */
#define DURACION_MINIMA_GIRO 1.75f
#define DURACION_MAXIMA_GIRO 3.5f


/* Inicializa el estado de la bolita. */
void inicializar_bolita(EstadoBolita* bolita) {
    bolita->angulo_actual = 0.0f;
    bolita->velocidad = 0.0f;
    bolita->velocidad_inicial = 0.0f;
    bolita->duracion_total = DURACION_GIRO_BOLA;
    bolita->girando = 0;
    bolita->tiempo_transcurrido = 0.0f;
}


/* Inicia el giro de la bolita hacia un angulo absoluto (en grados) en la rueda, con un numero de vueltas extra. */
void iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float angulo_sector_centro, int vueltas_extra) {
    float target_mod;
    float actual_mod;
    float delta_relativo;
    float duracion_necesaria;

    
	/* Se calcula el angulo relativo que la bolita debe recorrer para llegar al angulo objetivo, considerando vueltas extra. */
    target_mod = (float)fmod(-angulo_sector_centro, 360.0f);
    if (target_mod < 0.0f) target_mod += 360.0f;


	/* Se calcula el angulo relativo que la bolita debe recorrer para llegar al angulo objetivo, considerando vueltas extra. */
    actual_mod = (float)fmod(bolita->angulo_actual, 360.0f);
    if (actual_mod < 0.0f) actual_mod += 360.0f;


	/* Se calcula el angulo relativo que la bolita debe recorrer para llegar al angulo objetivo, considerando vueltas extra. */
    delta_relativo = target_mod - actual_mod;
    while (delta_relativo < 0.0f) delta_relativo += 360.0f;
    while (delta_relativo >= 360.0f) delta_relativo -= 360.0f;

    if (vueltas_extra > 0) delta_relativo += 360.0f * (float)vueltas_extra;

    duracion_necesaria = delta_relativo / (VELOCIDAD_TIPICA_GIRO * FACTOR_INTEGRAL_EASING);

    bolita->velocidad_inicial = VELOCIDAD_TIPICA_GIRO;


	/* Se ajusta la duracion del giro de la bolita para que no sea demasiado corta ni demasiado larga. */
    if (duracion_necesaria < DURACION_MINIMA_GIRO) {
        duracion_necesaria = DURACION_MINIMA_GIRO;
        bolita->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
    }

    else if (duracion_necesaria > DURACION_MAXIMA_GIRO) {
        duracion_necesaria = DURACION_MAXIMA_GIRO;
        bolita->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
    }


	/* Se inicializa el estado de la bolita para que comience a girar. */
    bolita->velocidad = bolita->velocidad_inicial;
    bolita->duracion_total = duracion_necesaria;
    bolita->girando = 1;
    bolita->tiempo_transcurrido = 0.0f;
}


/* Actualiza el estado de la bolita, aplicando el easing para que desacelere suavemente. */
void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo) {


	/* Se definen los puntos de control de la curva de Bezier para el easing. */
    Punto3D p0, p1, p2, p3, punto_easing;
    float t, factor;

    if (!bolita->girando) return;

    bolita->tiempo_transcurrido += delta_tiempo;
    t = bolita->tiempo_transcurrido / bolita->duracion_total;
    if (t >= 1.0f) t = 1.0f;


	/* Se definen los puntos de control de la curva de Bezier para el easing. */
    p0.x = 0.0f; p0.y = 1.0f; p0.z = 0.0f;
    p1.x = 0.0f; p1.y = 0.85f; p1.z = 0.0f;
    p2.x = 0.0f; p2.y = 0.25f; p2.z = 0.0f;
    p3.x = 0.0f; p3.y = 0.0f; p3.z = 0.0f;

    punto_easing = evaluar_bezier_cubica(p0, p1, p2, p3, t);
    factor = punto_easing.y;
    if (factor < 0.0f) factor = 0.0f;

    bolita->velocidad = bolita->velocidad_inicial * factor;
    bolita->angulo_actual += bolita->velocidad * delta_tiempo;

    if (t >= 1.0f) {
        bolita->velocidad = 0.0f;
        bolita->girando = 0;
    }
}


/* Dibuja la bolita en su posicion actual, considerando su angulo y la orbita. */
void dibujar_bolita(const EstadoBolita* bolita) {
    const float MARGEN_SOBRE_SUPERFICIE = 0.02f;
    const float RADIO_BOLITA = 0.12f;
    float altura_bolita = altura_superficie_en_radio(RADIO_ORBITA_BOLITA) + RADIO_BOLITA + MARGEN_SOBRE_SUPERFICIE;

    glPushMatrix();


	// Se posiciona la bolita en su orbita, considerando su altura y el angulo actual. */
    glTranslatef(0.0f, altura_bolita, 0.0f);


	// Se posiciona la bolita en su orbita, considerando su altura y el angulo actual. */
    glRotatef(-bolita->angulo_actual, 0.0f, 1.0f, 0.0f);
    glTranslatef(RADIO_ORBITA_BOLITA, 0.0f, 0.0f);

    aplicar_material(MATERIAL_METAL);
    glutSolidSphere(RADIO_BOLITA, 16, 16);

    glPopMatrix();
}