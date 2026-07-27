/*
 * jugador.h
 * -----------------------------------------------------------------------
 * Define la estructura del jugador y las funciones para modificar su
 * saldo, deuda y estadisticas de sesion.
 *
 * Responsable sugerido: Persona C
 * -----------------------------------------------------------------------
 */
#ifndef JUGADOR_H
#define JUGADOR_H

typedef struct {
    float saldo;
    float deuda;
    int   prestamos_activos;

    /* Estadisticas de sesion, para el HUD y el resumen final */
    float total_apostado;
    float total_ganado;
    float total_perdido;
    int   veces_sin_fondos;
    float interes_acumulado;

    /* --- Concientizacion sobre ludopatia: disparadores de mensajes
       reflexivos (ver verificar_mensaje_reflexivo). Estos campos NO
       son estadisticas para mostrar en el HUD -son memoria interna
       para saber "ya avise sobre esto" y no repetir el mismo mensaje
       en cada ronda mientras la condicion se mantenga. --- */
    float saldo_inicial;                    /* copia de saldo al arrancar, para calcular hitos relativos */
    int   racha_perdidas_consecutivas;       /* rondas seguidas con resultado neto negativo */
    int   racha_ya_advertida;                /* 1 si ya se mostro el mensaje para ESTA racha (se resetea a 0 cuando la racha vuelve a 0) */
    float ultimo_hito_apostado_notificado;   /* total_apostado en el momento del ultimo aviso por monto */
    int   ultimo_veces_sin_fondos_notificado;/* veces_sin_fondos en el momento del ultimo aviso por "te quedaste sin saldo" */
    int   mensajes_reflexivos_mostrados;     /* cuantos mensajes reflexivos van en esta sesion (para escalar friccion, ver dibujar_pantalla_mensaje_reflexivo) */
    int   rondas_jugadas;                    /* total de rondas resueltas en la sesion, ganes o pierdas */
    int   ultima_ronda_notificada;           /* rondas_jugadas en el ultimo aviso de "reality check" por cantidad de rondas */

    /* --- Simulacion de consecuencias (tiempo) ---
       Ver tiempo_jugado_minutos(). El reloj real (glutGet) lo consulta
       main.c -este archivo no depende de GLUT a proposito, para que
       la logica de Jugador se pueda seguir leyendo/probando sin
       arrastrar dependencias de la libreria grafica-, pero el valor
       en si vive aca junto con el resto del estado de sesion. */
    int   tiempo_inicio_ms;                  /* glutGet(GLUT_ELAPSED_TIME) al arrancar esta sesion */
} Jugador;

/* Resta el monto del total apostado cuando el jugador deshace una
   ficha antes de girar (no afecta el saldo: la ficha nunca se habia
   descontado, solo se contabilizaba en las estadisticas). */
void anular_apuesta(Jugador* j, float monto);

/* Inicializa el jugador con saldo inicial y estadisticas en cero.
   'tiempo_actual_ms' es glutGet(GLUT_ELAPSED_TIME) tomado por el
   llamador (jugador.c no depende de GLUT a proposito) -se guarda como
   el arranque del reloj de la sesion, ver tiempo_jugado_minutos(). Se
   pide como parametro (en vez de dejar que main.c lo asigne aparte en
   una linea suelta despues de llamar a esta funcion) para que no se
   pueda arrancar una sesion nueva olvidando reiniciar el reloj. */
void inicializar_jugador(Jugador* j, float saldo_inicial, int tiempo_actual_ms);

/* Se llama apenas se coloca una ficha (clic o tecla), no al resolver
   la ronda. Suma el monto al total apostado de la sesion de inmediato,
   para que el HUD refleje la apuesta en el momento en que se hace. */
void registrar_apuesta(Jugador* j, float monto);

/* Se llama cuando la bolita se detiene y se conoce el resultado.
   Solo ajusta el saldo con la ganancia/perdida neta; el monto ya fue
   contabilizado antes por registrar_apuesta(). */
void aplicar_resultado_apuesta(Jugador* j, float ganancia);

/* Otorga un prestamo al jugador y actualiza su deuda con interes */
void pedir_prestamo(Jugador* j, float monto, float tasa_interes);

/* Devuelve 1 si la deuda supera el limite de "game over", 0 si no */
int deuda_es_impagable(const Jugador* j, float limite_deuda);

/* --- Concientizacion sobre ludopatia --- */

/* Revisa las condiciones de juego problematico (racha de perdidas,
   monto total apostado, veces sin fondos, y cantidad de rondas
   jugadas -"reality check" neutral, no depende de que te vaya mal-)
   contra el estado actual del jugador. Devuelve un puntero a un
   mensaje reflexivo con cifras REALES del jugador ya insertadas (ej.
   "llevas apostado 2400.00, mas del doble de tu saldo inicial de
   1000.00" -ver McGivern et al. 2019 y Wohl et al. sobre feedback
   personalizado, docs/analisis-ludopatia.md) si alguna condicion se
   cumplio recien -es decir, se cumple una sola vez por evento, no en
   cada frame mientras la condicion se mantenga-, o NULL si no hay
   nada que mostrar en este momento.

   IMPORTANTE: el puntero devuelto apunta a un buffer ESTATICO interno
   de este archivo (no a memoria propia del llamador, y no es un string
   literal como antes) -valido solo hasta la proxima llamada a esta
   misma funcion. Como el juego nunca muestra dos mensajes reflexivos
   al mismo tiempo, esto es seguro, pero NO guardar el puntero para
   usarlo mas adelante (ej. en otra ronda): pedir uno nuevo cuando haga
   falta. Se debe llamar una vez por ronda resuelta (despues de
   aplicar_resultado_apuesta), no en cada frame. */
const char* verificar_mensaje_reflexivo(Jugador* j);

/* --- Medidor de riesgo ambiental en el HUD ---
   Calcula el nivel de riesgo (0: BAJO, 1: MODERADO, 2: ALTO, 3: RIESGO DE CONDUCTA COMPULSIVA)
   recalculado en cada frame a partir del comportamiento en la sesion. */
int calcular_nivel_riesgo(const Jugador* j, int tiempo_actual_ms);

float tiempo_jugado_minutos(const Jugador* j, int tiempo_actual_ms);

#endif /* JUGADOR_H */