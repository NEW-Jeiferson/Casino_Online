/** Punto de entrada principal del Simulador de Ruleta 3D.
 * Coordina el bucle principal de OpenGL (GLUT), el estado global de la partida,
 * la interaccion del usuario (teclado/raton) y la integracion de los distintos modulos.
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "core/estado_juego.h"
#include "core/jugador.h"
#include "ruleta/ruleta_geometria.h"
#include "ruleta/ruleta_animacion.h"
#include "ruleta/mouse_picking.h"
#include "render/iluminacion.h"
#include "render/materiales.h"
#include "ui/hud.h"
#include "ui/pantallas.h"
#include "ui/tablero_apuestas.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

 /* Valores fijos de las fichas seleccionables con las teclas 1-4.
    Como es una tabla estatica que no cambia durante la partida,
    se mantiene fuera de la estructura dinamica EstadoPartida. */
static const float FICHAS[4] = { 10.0f, 25.0f, 50.0f, 100.0f };

/** Estructura central que agrupa todo el estado de una partida en curso.
 * Anteriormente estas variables estaban dispersas como variables globales estaticas.
 * Agruparlas facilita la inspeccion de datos, la coordinacion entre sistemas y
 * permite en un futuro reiniciar o guardar la partida de forma centralizada.
 */
typedef struct {
    Jugador      jugador;
    EstadoBolita bolita;
    float        angulo_rueda;

    Apuesta apuestas_activas[MAX_APUESTAS];
    int     num_apuestas_activas;

    int   ficha_actual_index;
    float monto_ficha_actual;

    /* FIX DE ALEATORIEDAD: El numero ganador ahora se decide ANTES de iniciar
       la animacion del giro (usando rand() % 37) y se guarda aqui. La funcion idle()
       resuelve la apuesta contra ESTE numero exacto, desvinculando la logica de
       ganancias de la animacion visual y evitando imprecisiones numericas. */
    int numero_ganador_pendiente;

    /* Puntero al mensaje de concientizacion sobre ludopatia.
       Apunta a una cadena de solo lectura devuelta por el modulo del jugador,
       por lo que no requiere liberacion de memoria. */
    const char* mensaje_reflexivo_actual;
} EstadoPartida;

static EstadoPartida partida;

/* Calcula el total de dinero que el jugador ha comprometido en la mesa
   durante la ronda actual, sumando todas las apuestas activas. */
static float monto_apostado_en_ronda(void) {
    float total = 0.0f;
    int i;
    for (i = 0; i < partida.num_apuestas_activas; i++) {
        total += partida.apuestas_activas[i].monto;
    }
    return total;
}

/* Verifica la integridad economica al momento de colocar una ficha.
   BUGFIX: Antes se validaba cada ficha nueva contra el saldo absoluto. Como el saldo
   solo se descuenta al finalizar la ronda, un jugador podia colocar fichas infinitas.
   Ahora, se exige que el saldo sea suficiente para cubrir lo que YA aposto
   en esta ronda mas el valor de la nueva ficha que intenta colocar. */
static int saldo_alcanza_para_ficha(float monto_ficha) {
    return (partida.jugador.saldo - monto_apostado_en_ronda()) >= monto_ficha;
}

/* Evalua si el saldo actual le permite al jugador seguir apostando.
   Si el jugador no tiene fondos suficientes ni para la ficha minima, cambia
   el estado del juego hacia la pantalla de solicitud de prestamo.
   Centraliza la validacion posterior a la resolucion de rondas y mensajes reflexivos. */
static void verificar_fondos_y_pedir_prestamo_si_hace_falta(void) {
    if (estado_actual == ESTADO_JUGANDO && partida.jugador.saldo < partida.monto_ficha_actual) {
        cambiar_estado(ESTADO_PRESTAMO);
    }
}

/* Registra una nueva apuesta en la mesa si se cumplen los requisitos.
   Retorna 1 si la apuesta fue agregada con exito, o 0 si se alcanzo el
   limite maximo de apuestas permitidas simultaneamente. */
static int agregar_apuesta(TipoApuesta tipo, int valor, float monto) {
    if (partida.num_apuestas_activas >= MAX_APUESTAS) return 0;

    partida.apuestas_activas[partida.num_apuestas_activas].tipo = tipo;
    partida.apuestas_activas[partida.num_apuestas_activas].valor = valor;
    partida.apuestas_activas[partida.num_apuestas_activas].monto = monto;
    partida.num_apuestas_activas++;

    /* Actualiza inmediatamente el monto total mostrado en el HUD */
    registrar_apuesta(&partida.jugador, monto);
    return 1;
}

/* Elimina una apuesta especifica del arreglo, compactando los elementos
   restantes para evitar huecos en la memoria y descontando el monto del HUD. */
static void quitar_apuesta(int indice) {
    int i;
    if (indice < 0 || indice >= partida.num_apuestas_activas) return;

    anular_apuesta(&partida.jugador, partida.apuestas_activas[indice].monto);

    for (i = indice; i < partida.num_apuestas_activas - 1; i++) {
        partida.apuestas_activas[i] = partida.apuestas_activas[i + 1];
    }
    partida.num_apuestas_activas--;
}

/* Busca y elimina la ultima ficha colocada en una zona o numero especifico.
   Se utiliza para procesar el "clic derecho" deshaciendo apuestas sobre la celda
   actualmente apuntada, unificando la logica para numeros y zonas especiales. */
static void quitar_ultima_apuesta_tipo_valor(TipoApuesta tipo, int valor) {
    int i;
    for (i = partida.num_apuestas_activas - 1; i >= 0; i--) {
        if (partida.apuestas_activas[i].tipo == tipo && partida.apuestas_activas[i].valor == valor) {
            quitar_apuesta(i);
            break;
        }
    }
}

/* Funcion principal de renderizado .
   Gestiona la secuencia de dibujado de la escena 3D, iluminacion, camara,
   elementos de la mesa y superposicion de interfaces 2D (HUD y pantallas). */
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    
    gluLookAt(0.0, 15.0, 7.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0);

    glPushMatrix();
    dibujar_mesa();
    dibujar_tablero_apuestas(partida.apuestas_activas, partida.num_apuestas_activas);

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    glRotatef(partida.angulo_rueda, 0.0f, 1.0f, 0.0f);
    dibujar_rueda();
   
    dibujar_pista_numerada();

    glPushMatrix();
    dibujar_bolita(&partida.bolita);
    glPopMatrix();
    glPopMatrix();

    dibujar_vidrio_protector();
    glPopMatrix();

    dibujar_hud(&partida.jugador, partida.monto_ficha_actual, partida.num_apuestas_activas);
    dibujar_pantalla_segun_estado(estado_actual, &partida.jugador, partida.mensaje_reflexivo_actual);

    glutSwapBuffers();
}

/* Callback de redimensionamiento de ventana */
void reshape(int ancho, int alto) {
    float aspecto;
    if (alto == 0) alto = 1;
    aspecto = (float)ancho / (float)alto;

    glViewport(0, 0, ancho, alto);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspecto, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

/* Manejador de eventos de teclado .
   Procesa comandos principales como eleccion de fichas, giros de ruleta y menu. */
void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: /* ESC - Cierra la aplicacion */
        exit(0);
        break;

        /* Controles de seleccion del monto de ficha --- */
    case '1': partida.ficha_actual_index = 0; partida.monto_ficha_actual = FICHAS[0]; break;
    case '2': partida.ficha_actual_index = 1; partida.monto_ficha_actual = FICHAS[1]; break;
    case '3': partida.ficha_actual_index = 2; partida.monto_ficha_actual = FICHAS[2]; break;
    case '4': partida.ficha_actual_index = 3; partida.monto_ficha_actual = FICHAS[3]; break;

    case 'r':
    case 'R':
        /* Apuesta rapida a color ROJO por teclado. */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_ROJO, partida.monto_ficha_actual);
        }
        break;

    case 'n':
    case 'N':
        /* Apuesta rapida a color NEGRO por teclado. */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_NEGRO, partida.monto_ficha_actual);
        }
        break;

    case ' ':
        /* Inicia el giro de la ruleta (Barra Espaciadora).
           El calculo matematico determina el angulo absoluto de la caida basado en un numero
           aleatorio real.*/

        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.num_apuestas_activas > 0) {

            int sector_ganador = rand() % 37;
            int numero_ganador = ORDEN_RUEDA_EUROPEA[sector_ganador];
            float angulo_sector_centro = ((float)sector_ganador + 0.5f) * (360.0f / 37.0f);

            partida.numero_ganador_pendiente = numero_ganador;

            iniciar_giro_bolita_hacia_absoluto(&partida.bolita, angulo_sector_centro, 1 /* vuelta extra visual */);
        }
        break;

    case 'p':
    case 'P':
        /* Solicitud de prestamo financiero ante falta de fondos. */
        if (estado_actual == ESTADO_PRESTAMO) {
            pedir_prestamo(&partida.jugador, 200.0f, 0.20f);

            if (deuda_es_impagable(&partida.jugador, 1000.0f)) {
                cambiar_estado(ESTADO_GAME_OVER);
            }
            else {
                cambiar_estado(ESTADO_JUGANDO);
            }
        }
        break;

    case 8: /* BACKSPACE Deshace la ultima ficha colocada en la mesa (Sistema LIFO) */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.num_apuestas_activas > 0) {
            quitar_apuesta(partida.num_apuestas_activas - 1);
        }
        break;

    case 13: /* ENTER Transicion generica entre estados */
        if (estado_actual == ESTADO_MENU) {
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_GAME_OVER) {
            /* Reinicio completo de la partida */
            inicializar_jugador(&partida.jugador, 1000.0f);
            partida.num_apuestas_activas = 0;
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_MENSAJE_REFLEXIVO) {
            /* Descarta el mensaje reflexivo y verifica si adicionalmente
               es necesario pedir un prestamo para poder continuar. */
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
            verificar_fondos_y_pedir_prestamo_si_hace_falta();
        }
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

/* Manejador de eventos de los clics del raton .
   Clic Izquierdo = Apostar, Clic Derecho = Deshacer. */
void mouse_click(int boton, int estado_boton, int x, int y) {
    float wx, wz;
    int numero;
    TipoApuesta tipo_zona;
    int valor_zona;

    if (estado_boton != GLUT_DOWN) return;
    if (estado_actual != ESTADO_JUGANDO || partida.bolita.girando) return;

    if (boton == GLUT_LEFT_BUTTON) {
        /* Intento de colocar una nueva ficha */
        if (!saldo_alcanza_para_ficha(partida.monto_ficha_actual)) return;
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;

        if (obtener_numero_en_punto(wx, wz, &numero)) {
            agregar_apuesta(APUESTA_NUMERO, numero, partida.monto_ficha_actual);
            glutPostRedisplay();
        }
        else if (obtener_zona_especial_en_punto(wx, wz, &tipo_zona, &valor_zona)) {
            agregar_apuesta(tipo_zona, valor_zona, partida.monto_ficha_actual);
            glutPostRedisplay();
        }
    }
    else if (boton == GLUT_RIGHT_BUTTON) {
        /* Intento de remover la ultima ficha del numero apuntado */
        if (!obtener_punto_clic_en_mesa(x, y, &wx, &wz)) return;

        if (obtener_numero_en_punto(wx, wz, &numero)) {
            quitar_ultima_apuesta_tipo_valor(APUESTA_NUMERO, numero);
            glutPostRedisplay();
        }
        else if (obtener_zona_especial_en_punto(wx, wz, &tipo_zona, &valor_zona)) {
            quitar_ultima_apuesta_tipo_valor(tipo_zona, valor_zona);
            glutPostRedisplay();
        }
    }
}

/* Funcion de movimiento del raton sin botones presionados
   Actualiza las coordenadas logicas del tapete para aplicar efectos visuales */
void mouse_mover(int x, int y) {
    float wx, wz;
    int col, fila;

    if (obtener_punto_clic_en_mesa(x, y, &wx, &wz)
        && obtener_celda_en_punto(wx, wz, &col, &fila)) {
        fijar_celda_hover(col, fila);
    }
    else {
        fijar_celda_hover(-1, -1);
    }
    glutPostRedisplay();
}

/* Bucle de actualizacion logica (callback Idle de GLUT).
   Calcula la interpolacion temporal e implementa la
   resolucion y cobro de apuestas en el instante que la bolita se detiene. */
void idle(void) {
    
    static int tiempo_anterior_ms = -1;
    int   tiempo_actual_ms = glutGet(GLUT_ELAPSED_TIME);
    float delta_tiempo;
    int estaba_girando = partida.bolita.girando;

    if (tiempo_anterior_ms < 0) tiempo_anterior_ms = tiempo_actual_ms;
    delta_tiempo = (float)(tiempo_actual_ms - tiempo_anterior_ms) / 1000.0f;
    tiempo_anterior_ms = tiempo_actual_ms;

    if (delta_tiempo < 0.0f) delta_tiempo = 0.0f;   
    if (delta_tiempo > 0.1f) delta_tiempo = 0.1f;   

    if (partida.bolita.girando) {
        /* Garantiza que la rueda siga girando a la misma velocidad constante */
        partida.angulo_rueda += VELOCIDAD_RUEDA_DURANTE_GIRO * delta_tiempo;
        if (partida.angulo_rueda >= 360.0f) partida.angulo_rueda -= 360.0f;
    }

    actualizar_bolita(&partida.bolita, delta_tiempo);

    /* Resolucion del resultado */
    if (estaba_girando && !partida.bolita.girando && partida.num_apuestas_activas > 0) {

        /* El numero ganador fue pre-calculado aleatoriamente de forma estricta
           al momento de iniciar el giro. Utilizarlo directo elimina bugs visuales de borde. */
        int   numero_ganador = partida.numero_ganador_pendiente;
        float ganancia_total = calcular_ganancia_total(partida.apuestas_activas, partida.num_apuestas_activas, numero_ganador);

        /* Diagnostico exclusivo en entorno de desarrollo.
           Desaparece automaticamente al compilar en NDEBUG (Release). */
#ifdef _DEBUG
        {
            const char* color_texto = (color_de_numero(numero_ganador) == COLOR_ROJO) ? "ROJO"
                : (color_de_numero(numero_ganador) == COLOR_NEGRO) ? "NEGRO" : "VERDE";
            printf("[RESULTADO REAL] numero=%d color=%s ganancia=%.2f\n", numero_ganador, color_texto, ganancia_total);
        }
#endif

        aplicar_resultado_apuesta(&partida.jugador, ganancia_total);
        partida.num_apuestas_activas = 0;

        /* Evaluacion de riesgos de ludopatia mostrar mensajes reflexivos
           antes de alertar por posible quiebra economica. */
        partida.mensaje_reflexivo_actual = verificar_mensaje_reflexivo(&partida.jugador);
        if (partida.mensaje_reflexivo_actual != NULL) {
            cambiar_estado(ESTADO_MENSAJE_REFLEXIVO);
        }
        else {
            verificar_fondos_y_pedir_prestamo_si_hace_falta();
        }
    }

    glutPostRedisplay();
}

/* Punto de entrada principal de la aplicacion de C.
   Inicializa los buffers graficos, configura el entorno de la partida */

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // ciclo de vida 
    srand((unsigned int)time(NULL));

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta (MVP)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_jugador(&partida.jugador, 1000.0f); 
    inicializar_estado_juego();                     
    inicializar_bolita(&partida.bolita);
    partida.angulo_rueda = 0.0f;
    partida.num_apuestas_activas = 0;
    partida.ficha_actual_index = 0;
    partida.monto_ficha_actual = FICHAS[0];
    partida.numero_ganador_pendiente = -1;
    partida.mensaje_reflexivo_actual = NULL;

    generar_perfil_bezier_rueda();
    construir_malla_rueda();

    /* Asignacion de rutinas OpenGL */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutPassiveMotionFunc(mouse_mover);

    glutMainLoop();
    return 0;
}