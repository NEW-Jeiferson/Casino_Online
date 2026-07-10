/*
 * ruleta_animacion.c
 * Implementacion de la animacion de la bolita. Ver ruleta_animacion.h.
 *
 * La velocidad de la bolita en cada instante se obtiene evaluando una
 * curva de Bezier cubica de "easing" (factor de velocidad de 1.0 a 0.0
 * a lo largo del tiempo de giro), en vez de restarle una cantidad fija
 * cada frame. Esto produce un frenado no lineal (rapido al inicio,
 * mas suave hacia el final).
 *
 * FIX DE ALEATORIEDAD (ronda 2): la primera correccion (velocidad
 * inicial aleatoria entre 180-270, con duracion fija de 4s) rompio el
 * ciclo corto de 3 giros detectado al inicio, pero no garantizaba
 * uniformidad real: el angulo total recorrido con ese rango no cubre
 * el circulo completo de 360 grados, lo cual puede sesgar el resultado.
 *
 * La solucion correcta: decidir el resultado PRIMERO de forma
 * garantizadamente uniforme (rand() % 37, en main.c), y usar
 * iniciar_giro_bolita_hacia_absoluto() para que la animacion termine
 * visualmente ahi. La fisica del giro ya no tiene ninguna influencia
 * sobre el resultado -solo es estetica.
 */
#include <GL/glut.h>
#include <math.h>
#include "ruleta_animacion.h"
#include "ruleta_geometria.h" /* RADIO_EXTERIOR_RUEDA */
#include "../render/materiales.h"
#include "../utils/bezier.h"

 /* Duracion "base" del frenado de la bolita, en segundos. Ya no se usa
    como duracion fija de cada giro (ver iniciar_giro_bolita_hacia,
    que calcula una duracion distinta por giro) -se deja como valor por
    defecto de iniciar_giro_bolita() para quien la siga llamando
    directamente con una velocidad fija en vez de un angulo destino. */
#define DURACION_GIRO_BOLA 4.0f

#define RADIO_ORBITA_BOLITA (RADIO_EXTERIOR_RUEDA - 0.4f)

    /* Velocidad "tipica" con la que siempre se ve girar la bolita, sin
       importar cuanto tenga que recorrer en total -lo que cambia entre
       giros es la DURACION, no que tan rapido se ve. */
#define VELOCIDAD_TIPICA_GIRO 140.0f

       /* --------------------------------------------------------------------
        * FACTOR_INTEGRAL_EASING: relacion entre angulo total recorrido,
        * velocidad y duracion del giro.
        *
        * actualizar_bolita() integra velocidad_inicial * factor(t) a lo largo
        * del tiempo real, donde factor(t) es la componente Y de la curva de
        * Bezier evaluada en t = tiempo_transcurrido/duracion, con puntos de
        * control (Y): p0=1.0, p1=0.85, p2=0.25, p3=0.0.
        *
        * angulo_total = velocidad_inicial * duracion * integral en [0,1] de factor(u) du
        *
        * Cada polinomio de Bernstein de grado 3 integra a 1/4 sobre [0,1], asi
        * que integral de factor(u) du = (p0+p1+p2+p3)/4 = (1+0.85+0.25+0)/4 = 0.525
        *
        * IMPORTANTE: si se modifican los puntos de control p0-p3 de
        * actualizar_bolita(), este factor debe recalcularse (suma de p_i.y,
        * entre 4).
        * ------------------------------------------------------------------ */
#define FACTOR_INTEGRAL_EASING 0.525f

        /* Limites de seguridad para que un giro nunca se sienta instantaneo
           ni exageradamente largo, sin importar el azar del angulo objetivo */
#define DURACION_MINIMA_GIRO 3.5f
#define DURACION_MAXIMA_GIRO 7.0f

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

/* VELOCIDAD_RUEDA_DURANTE_GIRO ahora se expone en ruleta_animacion.h
   (main.c la necesita para su idle(), ver comentario ahi) */
void iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float delta_absoluto_deseado, int vueltas_extra) {
    float delta_total;
    float tasa_combinada;
    float duracion_necesaria;

    /* delta_absoluto_deseado es cuanto debe avanzar la POSICION ABSOLUTA
       de la bolita en pantalla (rueda + bolita combinadas) para llegar
       al sector elegido. Normalizar a [0, 360) y agregar vueltas extra
       de regalo visual. */
    delta_total = delta_absoluto_deseado;
    while (delta_total < 0.0f) delta_total += 360.0f;
    while (delta_total >= 360.0f) delta_total -= 360.0f;
    if (vueltas_extra > 0) delta_total += 360.0f * (float)vueltas_extra;

    /* Tasa combinada: cuantos grados de posicion ABSOLUTA se recorren
       por segundo, sumando el aporte de la rueda (que gira a
       VELOCIDAD_RUEDA_DURANTE_GIRO mientras dura el giro) y el aporte
       relativo de la bolita (VELOCIDAD_TIPICA_GIRO * FACTOR_INTEGRAL_EASING,
       la misma relacion usada antes). Al resolver la duracion contra
       esta tasa COMBINADA de una sola vez, ya no hace falta predecir
       por separado "donde va a quedar la rueda" -eso era la
       dependencia circular que causaba el bug (la rueda ya no gira
       siempre los mismos 240 grados por giro, porque la duracion ya
       no es fija; predecirla con un 240 fijo daba un resultado
       incorrecto casi siempre). */
    tasa_combinada = VELOCIDAD_RUEDA_DURANTE_GIRO + (VELOCIDAD_TIPICA_GIRO * FACTOR_INTEGRAL_EASING);

    duracion_necesaria = delta_total / tasa_combinada;

    bolita->velocidad_inicial = VELOCIDAD_TIPICA_GIRO;

    if (duracion_necesaria < DURACION_MINIMA_GIRO) {
        duracion_necesaria = DURACION_MINIMA_GIRO;
        /* Si se recorta la duracion, la rueda YA NO es controlable
           desde aqui (gira a su propia velocidad fija en idle()), asi
           que hay que recalcular SOLO la velocidad de la bolita para
           que compense: bolita_delta_necesario = lo que falta despues
           de descontar lo que la rueda ya aporta en ese tiempo
           (recortado). */
        float aporte_rueda = VELOCIDAD_RUEDA_DURANTE_GIRO * duracion_necesaria;
        float bolita_delta_necesario = delta_total - aporte_rueda;
        if (bolita_delta_necesario < 0.0f) bolita_delta_necesario = 0.0f; /* seguridad, no deberia pasar con los rangos usados */
        bolita->velocidad_inicial = bolita_delta_necesario / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
    }
    else if (duracion_necesaria > DURACION_MAXIMA_GIRO) {
        float aporte_rueda2, bolita_delta_necesario2;
        duracion_necesaria = DURACION_MAXIMA_GIRO;
        aporte_rueda2 = VELOCIDAD_RUEDA_DURANTE_GIRO * duracion_necesaria;
        bolita_delta_necesario2 = delta_total - aporte_rueda2;
        if (bolita_delta_necesario2 < 0.0f) bolita_delta_necesario2 = 0.0f;
        bolita->velocidad_inicial = bolita_delta_necesario2 / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
    }

    bolita->velocidad = bolita->velocidad_inicial;
    bolita->duracion_total = duracion_necesaria;
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
    /* FIX: antes se usaba una altura fija (0.2f) que quedaba por
       debajo de la superficie real de la rueda en el radio de orbita
       (ver altura_superficie_en_radio) -la bolita se hundia dentro de
       la malla metalica en vez de apoyarse sobre ella. Ahora se calcula
       contra la altura real de la superficie en RADIO_ORBITA_BOLITA,
       mas el propio radio de la bolita (para que quede apoyada por
       encima, no centrada en la superficie) mas un margen chico
       para que no la toque. */
    const float MARGEN_SOBRE_SUPERFICIE = 0.02f;
    const float RADIO_BOLITA = 0.2f;
    float altura_bolita = altura_superficie_en_radio(RADIO_ORBITA_BOLITA) + RADIO_BOLITA + MARGEN_SOBRE_SUPERFICIE;

    glPushMatrix();

    glTranslatef(0.0f, altura_bolita, 0.0f);
    glRotatef(bolita->angulo_actual, 0.0f, 1.0f, 0.0f);
    glTranslatef(RADIO_ORBITA_BOLITA, 0.0f, 0.0f);

    aplicar_material(MATERIAL_METAL);
    glutSolidSphere(RADIO_BOLITA, 16, 16);

    glPopMatrix();
}