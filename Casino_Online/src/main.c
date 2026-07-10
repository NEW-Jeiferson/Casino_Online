/*
 * main.c
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

 /* Fichas seleccionables con teclas 1-4 (tabla fija, no es estado que
    cambie durante la partida, asi que se queda fuera de EstadoPartida) */
static const float FICHAS[4] = { 10.0f, 25.0f, 50.0f, 100.0f };

/* --- Estado global de la partida (TODO propio resuelto) ---
   Antes estas variables estaban sueltas (jugador, bolita, angulo_rueda,
   el arreglo de apuestas activas, la ficha seleccionada...) directamente
   como statics del archivo. Se agrupan aca en una sola estructura para
   que quede claro que es "el estado de una partida en curso" y se
   pueda pasar/inspeccionar como una unidad si mas adelante hace falta
   (guardar partida, reiniciar todo de una vez, etc.). */
typedef struct {
    Jugador      jugador;
    EstadoBolita bolita;
    float        angulo_rueda;

    Apuesta apuestas_activas[MAX_APUESTAS];
    int     num_apuestas_activas;

    int   ficha_actual_index;
    float monto_ficha_actual;

    /* FIX DE ALEATORIEDAD (ronda 2): el numero ganador ahora se decide
       ANTES de iniciar la animacion del giro (ver tecla ESPACIO), con
       rand() % 37 -uniforme de verdad-. Se guarda aqui para que idle()
       resuelva la apuesta contra ESTE numero exacto cuando la bolita
       se detenga, sin volver a derivarlo del angulo final (evita
       cualquier imprecision numerica de la animacion). */
    int numero_ganador_pendiente;
} EstadoPartida;

static EstadoPartida partida;

/* Agrega una apuesta al arreglo activo si hay espacio y saldo suficiente.
   Devuelve 1 si se agrego, 0 si no (arreglo lleno). */
static int agregar_apuesta(TipoApuesta tipo, int valor, float monto) {
    if (partida.num_apuestas_activas >= MAX_APUESTAS) return 0;
    partida.apuestas_activas[partida.num_apuestas_activas].tipo = tipo;
    partida.apuestas_activas[partida.num_apuestas_activas].valor = valor;
    partida.apuestas_activas[partida.num_apuestas_activas].monto = monto;
    partida.num_apuestas_activas++;

    registrar_apuesta(&partida.jugador, monto); /* suma al HUD ya */
    return 1;
}

/* Quita la apuesta en la posicion 'indice' del arreglo, recorriendo
   el resto un lugar hacia atras para no dejar huecos. Tambien
   descuenta ese monto de total_apostado (ver anular_apuesta). */
static void quitar_apuesta(int indice) {
    int i;
    if (indice < 0 || indice >= partida.num_apuestas_activas) return;

    anular_apuesta(&partida.jugador, partida.apuestas_activas[indice].monto);

    for (i = indice; i < partida.num_apuestas_activas - 1; i++) {
        partida.apuestas_activas[i] = partida.apuestas_activas[i + 1];
    }
    partida.num_apuestas_activas--;
}

/* Busca de atras hacia adelante la ultima apuesta con el tipo/valor
   dados y la quita. Se usa para el clic derecho, tanto sobre un numero
   del grid como sobre una zona especial (docena/mitad/par-impar):
   misma logica en ambos casos, antes duplicada solo para APUESTA_NUMERO. */
static void quitar_ultima_apuesta_tipo_valor(TipoApuesta tipo, int valor) {
    int i;
    for (i = partida.num_apuestas_activas - 1; i >= 0; i--) {
        if (partida.apuestas_activas[i].tipo == tipo && partida.apuestas_activas[i].valor == valor) {
            quitar_apuesta(i);
            break;
        }
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Camara mucho mas inclinada hacia abajo que antes (65 grados de
       elevacion, antes 39), para leer mejor los numeros de la rueda y
       el tablero, sin llegar a ser una vista totalmente plana desde
       arriba -se conserva la perspectiva real (gluPerspective, no
       glOrtho), asi que el volumen 3D de la rueda (domo, sombreado de
       Phong) sigue notandose. El punto al que mira se corrio un poco
       hacia el tablero (Z positivo) para que quede mejor encuadrado
       junto con la rueda, en vez de mirar solo al centro de la rueda. */
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
    dibujar_pista_numerada(); /* debe ir aqui: mientras la matriz de la rueda sigue activa, para que gire junto con ella */

    glPushMatrix();
    dibujar_bolita(&partida.bolita);
    glPopMatrix();
    glPopMatrix();

    dibujar_vidrio_protector();
    glPopMatrix();

    dibujar_hud(&partida.jugador, partida.monto_ficha_actual, partida.num_apuestas_activas);
    dibujar_pantalla_segun_estado(estado_actual, &partida.jugador);

    glutSwapBuffers();
}

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

void teclado(unsigned char tecla, int x, int y) {
    (void)x; (void)y;
    switch (tecla) {
    case 27: /* ESC */
        exit(0);
        break;

        /* --- Seleccion de monto de ficha --- */
    case '1': partida.ficha_actual_index = 0; partida.monto_ficha_actual = FICHAS[0]; break;
    case '2': partida.ficha_actual_index = 1; partida.monto_ficha_actual = FICHAS[1]; break;
    case '3': partida.ficha_actual_index = 2; partida.monto_ficha_actual = FICHAS[2]; break;
    case '4': partida.ficha_actual_index = 3; partida.monto_ficha_actual = FICHAS[3]; break;

    case 'r':
    case 'R':
        /* Apuesta a color ROJO: se agrega al arreglo de apuestas
           activas, pero NO dispara el giro (ver tecla ESPACIO). */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.jugador.saldo >= partida.monto_ficha_actual) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_ROJO, partida.monto_ficha_actual);
        }
        break;

    case 'n':
    case 'N':
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.jugador.saldo >= partida.monto_ficha_actual) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_NEGRO, partida.monto_ficha_actual);
        }
        break;

    case ' ':
        /* FIX DE ALEATORIEDAD (ronda 4 - correccion de fondo): las
           rondas anteriores (2 y 3) intentaban involucrar el angulo de
           la RUEDA en el calculo del objetivo de la bolita -primero
           prediciendolo mal con un valor fijo (ronda 2), despues con
           una ecuacion "combinada" rueda+bolita (ronda 3)-, pero
           ambos enfoques partian de una premisa equivocada.

           Se rehizo el algebra completa de la jerarquia de matrices
           (mesa -> rueda -> bolita) y se confirmo que el angulo de la
           rueda SE CANCELA de la ecuacion de correctitud: no importa
           en que angulo este la rueda, el unico numero que le importa
           a iniciar_giro_bolita_hacia_absoluto() es el angulo del
           sector elegido -ni siquiera hace falta calcular la posicion
           actual combinada. Ver el comentario largo en
           ruleta_animacion.c para la derivacion completa. */
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
    case 8: /* BACKSPACE: deshace la ultima ficha colocada (LIFO) */
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && partida.num_apuestas_activas > 0) {
            quitar_apuesta(partida.num_apuestas_activas - 1);
        }
        break;

    case 13: /* ENTER */
        if (estado_actual == ESTADO_MENU) {
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_GAME_OVER) {
            inicializar_jugador(&partida.jugador, 1000.0f);
            partida.num_apuestas_activas = 0;
            cambiar_estado(ESTADO_JUGANDO);
        }
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

/* Clic izquierdo sobre el tablero: agrega una apuesta con la ficha
   actualmente seleccionada, sobre un numero exacto (grid 0-36) o sobre
   una de las zonas especiales (docena / mitad / par-impar). Clic
   derecho: quita la ultima ficha puesta en esa misma celda o zona. */
void mouse_click(int boton, int estado_boton, int x, int y) {
    float wx, wz;
    int numero;
    TipoApuesta tipo_zona;
    int valor_zona;

    if (estado_boton != GLUT_DOWN) return;
    if (estado_actual != ESTADO_JUGANDO || partida.bolita.girando) return;

    if (boton == GLUT_LEFT_BUTTON) {
        /* Clic izquierdo: agregar ficha */
        if (partida.jugador.saldo < partida.monto_ficha_actual) return;
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
        /* Clic derecho: quitar la ultima ficha puesta en ese numero o zona */
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
/* Movimiento pasivo del mouse: actualiza que celda esta en hover, para
   el resaltado visual (ver celda_hover_col/fila en tablero_apuestas.h). */
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

void idle(void) {
    const float delta_tiempo = 0.016f;
    int estaba_girando = partida.bolita.girando;

    if (partida.bolita.girando) {
        /* Usa la MISMA constante que iniciar_giro_bolita_hacia_absoluto()
           asume al resolver la duracion del giro (ver ruleta_animacion.h) -
           antes este 60.0f estaba repetido a mano en 2 lugares distintos,
           lo cual fue justo la causa raiz del bug de "la bolita siempre
           cae en el mismo lugar" cuando se desincronizaron. */
        partida.angulo_rueda += VELOCIDAD_RUEDA_DURANTE_GIRO * delta_tiempo;
        if (partida.angulo_rueda >= 360.0f) partida.angulo_rueda -= 360.0f;
    }

    actualizar_bolita(&partida.bolita, delta_tiempo);

    /* --- Resolver resultado cuando la bolita se acaba de detener --- */
    if (estaba_girando && !partida.bolita.girando && partida.num_apuestas_activas > 0) {
        /* FIX DE ALEATORIEDAD (ronda 2): el numero ganador YA se decidio
           al presionar ESPACIO (ver partida.numero_ganador_pendiente),
           asi que aqui simplemente se usa ese valor -ya no se vuelve a
           derivar del angulo final de la bolita. Esto tambien elimina
           cualquier posible desfase entre "donde cae visualmente la
           bolita" y "que numero cuenta como ganador". */
        int   numero_ganador = partida.numero_ganador_pendiente;
        float ganancia_total = calcular_ganancia_total(partida.apuestas_activas, partida.num_apuestas_activas, numero_ganador);

        /* DIAGNOSTICO TEMPORAL: imprime en la consola el numero/color
           que realmente se decidio, para comparar directamente contra
           lo que se ve en pantalla y confirmar si ya coinciden.
           Se puede quitar una vez confirmado. */
        {
            const char* color_texto = (color_de_numero(numero_ganador) == COLOR_ROJO) ? "ROJO"
                : (color_de_numero(numero_ganador) == COLOR_NEGRO) ? "NEGRO" : "VERDE";
            printf("[RESULTADO REAL] numero=%d color=%s ganancia=%.2f\n", numero_ganador, color_texto, ganancia_total);
        }

        aplicar_resultado_apuesta(&partida.jugador, ganancia_total);
        partida.num_apuestas_activas = 0;

        if (partida.jugador.saldo < partida.monto_ficha_actual) {
            cambiar_estado(ESTADO_PRESTAMO);
        }
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    /* Semilla del generador de numeros aleatorios: una sola vez, al
       arrancar el programa (ver BUGFIX de la tecla ESPACIO en teclado()
       para el porque). Si se llamara mas de una vez -por ejemplo dentro
       del callback de teclado- con time(NULL) como semilla, dos giros
       ocurridos dentro del mismo segundo real quedarian con la misma
       semilla y por lo tanto la misma "aleatoriedad", asi que va aca. */
    srand((unsigned int)time(NULL));

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta (MVP)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_jugador(&partida.jugador, 1000.0f); /* saldo inicial real, ya no 50.0f de prueba */
    inicializar_estado_juego();                     /* arranca en ESTADO_MENU */
    inicializar_bolita(&partida.bolita);
    partida.angulo_rueda = 0.0f;
    partida.num_apuestas_activas = 0;
    partida.ficha_actual_index = 0;
    partida.monto_ficha_actual = FICHAS[0];
    partida.numero_ganador_pendiente = -1;

    generar_perfil_bezier_rueda();
    construir_malla_rueda();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutPassiveMotionFunc(mouse_mover);

    glutMainLoop();
    return 0;
}