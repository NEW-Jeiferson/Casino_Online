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
    como duracion fija de cada giro (ver iniciar_giro_bolita_hacia_absoluto,
    que calcula una duracion distinta por giro) -se deja solo como valor
    inicial de inicializar_bolita() (bolita detenida, antes de cualquier
    giro real). */
#define DURACION_GIRO_BOLA 4.0f

    /* BUGFIX: antes era (RADIO_EXTERIOR_RUEDA - 0.4f) = 2.6, que
       coincidia EXACTAMENTE con RADIO_EXTERNO_PISTA (tambien 2.6) -la
       bolita orbitaba justo en el borde exterior de la pista, no
       comodamente dentro de una casilla, lo cual hacia ambiguo
       visualmente sobre que casilla estaba realmente parada. Ahora usa
       el centro real de la banda (mismo radio_medio que usa
       dibujar_pista_numerada() para colocar los numeros). */
#define RADIO_ORBITA_BOLITA ((RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f)

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

/* NOTA: aqui existia iniciar_giro_bolita(bolita, velocidad_inicial),
   la version vieja que giraba con una velocidad fija en vez de
   resolver hacia un angulo objetivo. No la usa nada del codigo actual
   (el flujo real es iniciar_giro_bolita_hacia_absoluto(), que es el
   unico que garantiza que el numero visual coincide con el numero
   decidido). Se elimino para no dejarla como alternativa "valida" -si
   se llamara por error, la animacion terminaria en un angulo que no
   corresponde a ningun numero ganador real. */

   /* VELOCIDAD_RUEDA_DURANTE_GIRO ahora se expone en ruleta_animacion.h
      (main.c la necesita para su idle(), ver comentario ahi) */
void iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float angulo_sector_centro, int vueltas_extra) {
    float target_mod;
    float actual_mod;
    float delta_relativo;
    float duracion_necesaria;

    /* --------------------------------------------------------------
     * CORRECCION DE FONDO (reemplaza el enfoque de "tasa combinada"
     * rueda+bolita usado antes): se hizo el algebra completa de la
     * jerarquia de matrices y se confirmo que el angulo de la RUEDA
     * SE CANCELA de la ecuacion de correctitud -no influye para nada
     * en donde debe terminar la bolita relativa a un sector.
     *
     * Derivacion (ver conversacion para el detalle completo):
     *   posicion_mundial_pista(sector)  = angulo_rueda + angulo_sector
     *   posicion_mundial_bolita         = angulo_rueda - bolita.angulo_actual
     *   (coinciden)  =>  bolita.angulo_actual_final = -angulo_sector
     *
     * Por eso esta funcion ya NO necesita saber nada de
     * VELOCIDAD_RUEDA_DURANTE_GIRO ni de angulo_rueda: solo le importa
     * a donde debe llegar el angulo PROPIO de la bolita. El parametro
     * se sigue llamando angulo_sector_centro (no "absoluto") porque
     * ya no representa una posicion absoluta -era ahi donde estaba el
     * error conceptual de la version anterior.
     * -------------------------------------------------------------- */

    target_mod = (float)fmod(-angulo_sector_centro, 360.0f);
    if (target_mod < 0.0f) target_mod += 360.0f;

    actual_mod = (float)fmod(bolita->angulo_actual, 360.0f);
    if (actual_mod < 0.0f) actual_mod += 360.0f;

    delta_relativo = target_mod - actual_mod;
    while (delta_relativo < 0.0f) delta_relativo += 360.0f;
    while (delta_relativo >= 360.0f) delta_relativo -= 360.0f;

    if (vueltas_extra > 0) delta_relativo += 360.0f * (float)vueltas_extra;

    duracion_necesaria = delta_relativo / (VELOCIDAD_TIPICA_GIRO * FACTOR_INTEGRAL_EASING);

    bolita->velocidad_inicial = VELOCIDAD_TIPICA_GIRO;

    if (duracion_necesaria < DURACION_MINIMA_GIRO) {
        duracion_necesaria = DURACION_MINIMA_GIRO;
        bolita->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
    }
    else if (duracion_necesaria > DURACION_MAXIMA_GIRO) {
        duracion_necesaria = DURACION_MAXIMA_GIRO;
        bolita->velocidad_inicial = delta_relativo / (duracion_necesaria * FACTOR_INTEGRAL_EASING);
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
    const float RADIO_BOLITA = 0.12f; /* antes 0.2f: ocupaba ~91% del ancho de un sector (0.442 unidades a este radio de orbita), haciendo que pareciera "entre dos colores" incluso bien centrada. Con 0.12, el diametro (0.24) queda en ~54% del ancho del sector. */
    float altura_bolita = altura_superficie_en_radio(RADIO_ORBITA_BOLITA) + RADIO_BOLITA + MARGEN_SOBRE_SUPERFICIE;

    glPushMatrix();

    glTranslatef(0.0f, altura_bolita, 0.0f);
    /* Sentido CONTRARIO a la rueda (signo negativo), como en una ruleta
       real: el crupier lanza la bolita en sentido opuesto al giro de
       la rueda. Esto tambien es lo que hace que la desaceleracion se
       vea claramente (ver comentario largo en
       iniciar_giro_bolita_hacia_absoluto sobre por que esto importa). */
    glRotatef(-bolita->angulo_actual, 0.0f, 1.0f, 0.0f);
    glTranslatef(RADIO_ORBITA_BOLITA, 0.0f, 0.0f);

    aplicar_material(MATERIAL_METAL);
    glutSolidSphere(RADIO_BOLITA, 16, 16);

    glPopMatrix();
}