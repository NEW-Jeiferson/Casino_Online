/*
 * ruleta_animacion.c
 * Implementacion de la animacion de la bolita. Ver ruleta_animacion.h.
 *
 * VERSION FINAL (reemplaza la desaceleracion lineal del placeholder):
 * la velocidad de la bolita en cada instante se obtiene evaluando una
 * curva de Bezier cubica de "easing" (factor de velocidad de 1.0 a 0.0
 * a lo largo del tiempo de giro), en vez de restarle una cantidad fija
 * cada frame. Esto produce un frenado no lineal (rapido al inicio,
 * mas suave hacia el final), que es justo lo que se espera de un
 * frenado real de ruleta.
 */
#include <GL/glut.h>
#include "ruleta_animacion.h"
#include "ruleta_geometria.h" /* RADIO_EXTERIOR_RUEDA */
#include "../render/materiales.h"
#include "../utils/bezier.h"

 /* Duracion total del frenado de la bolita, en segundos. Se deja como
    constante simple por ahora; si mas adelante se quiere variar segun
    la apuesta o dificultad, se puede exponer como parametro. */
#define DURACION_GIRO_BOLA 4.0f

    /* Radio al que orbita la bolita: un poco dentro del borde exterior de
       la rueda (RADIO_EXTERIOR_RUEDA), para que se vea "sobre" la pista y
       no flotando fuera de la rueda. */
#define RADIO_ORBITA_BOLITA (RADIO_EXTERIOR_RUEDA - 0.4f)

void inicializar_bolita(EstadoBolita* bolita) {
    bolita->angulo_actual = 0.0f;
    bolita->velocidad = 0.0f;
    bolita->velocidad_inicial = 0.0f;
    bolita->duracion_total = DURACION_GIRO_BOLA;
    bolita->girando = 0;
    bolita->tiempo_transcurrido = 0.0f;
}

void iniciar_giro_bolita(EstadoBolita* bolita, float velocidad_inicial) {
    bolita->velocidad_inicial = velocidad_inicial;
    bolita->velocidad = velocidad_inicial;
    bolita->duracion_total = DURACION_GIRO_BOLA;
    bolita->girando = 1;
    bolita->tiempo_transcurrido = 0.0f;
}

void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo) {
    /* Puntos de control de la curva de easing (factor de velocidad).
       Solo se usa la componente Y como "factor" en [0,1]; X y Z se
       dejan en 0 porque evaluar_bezier_cubica trabaja en 3D pero aqui
       solo interesa una curva de valor contra tiempo (uso clasico de
       Bezier como funcion de easing). p0 -> factor 1.0 (velocidad
       plena), p3 -> factor 0.0 (detenida). p1/p2 controlan que tan
       "brusco" o "suave" es el frenado. */
    Punto3D p0, p1, p2, p3, punto_easing;
    float t, factor;

    if (!bolita->girando) return;

    bolita->tiempo_transcurrido += delta_tiempo;
    t = bolita->tiempo_transcurrido / bolita->duracion_total;
    if (t >= 1.0f) t = 1.0f;

    p0.x = 0.0f; p0.y = 1.0f; p0.z = 0.0f;
    p1.x = 0.0f; p1.y = 0.85f; p1.z = 0.0f;
    p2.x = 0.0f; p2.y = 0.25f; p2.z = 0.0f;
    p3.x = 0.0f; p3.y = 0.0f; p3.z = 0.0f;

    punto_easing = evaluar_bezier_cubica(p0, p1, p2, p3, t);
    factor = punto_easing.y;
    if (factor < 0.0f) factor = 0.0f; /* seguridad por si la curva overshoot */

    bolita->velocidad = bolita->velocidad_inicial * factor;
    bolita->angulo_actual += bolita->velocidad * delta_tiempo;

    if (t >= 1.0f) {
        bolita->velocidad = 0.0f;
        bolita->girando = 0;
    }
}

void dibujar_bolita(const EstadoBolita* bolita) {
    glPushMatrix();

    glTranslatef(0.0f, 0.2f, 0.0f);       /* altura sobre la superficie de la rueda */
    glRotatef(bolita->angulo_actual, 0.0f, 1.0f, 0.0f);
    glTranslatef(RADIO_ORBITA_BOLITA, 0.0f, 0.0f);

    aplicar_material(MATERIAL_METAL);
    glutSolidSphere(0.2, 16, 16);

    glPopMatrix();
}