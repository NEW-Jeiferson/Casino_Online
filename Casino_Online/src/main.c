/*
 * main.c
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "core/estado_juego.h"
#include "core/jugador.h"
#include "ruleta/ruleta_geometria.h"
#include "ruleta/ruleta_animacion.h"
#include "ruleta/mouse_picking.h"
#include "render/iluminacion.h"
#include "render/materiales.h"
#include "ui/hud.h"
#include "ui/notificaciones.h"
#include "ui/pantallas.h"
#include "ui/tablero_apuestas.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif


 /* Funcion para obtener el monto de una ficha por su indice */
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

    /* Mensaje de concientizacion pendiente de mostrar (ver
       core/jugador.h, verificar_mensaje_reflexivo). Apunta a un string
       constante devuelto por esa funcion -nunca a memoria propia de
       EstadoPartida-, asi que no hace falta liberarlo ni copiarlo. */
    const char* mensaje_reflexivo_actual;

    /* Pagina actual dentro de ESTADO_EDUCACION (0-based). Es estado de
       NAVEGACION de UI, no del jugador -por eso vive aca y no en
       Jugador-, se resetea a 0 cada vez que se entra a esa pantalla. */
    int pagina_educacion;

    /* Opcion actualmente seleccionada en ESTADO_MENU (0 = Ruleta, 1 = Tragamonedas) */
    int opcion_menu;

    /* Progreso acumulado de la pantalla de carga (0.0f a 1.0f) */
    float progreso_carga;
} EstadoPartida;

static EstadoPartida partida;

static GLuint g_tex_carga = 0;
static GLuint g_tex_casino = 0;

static GLuint cargar_textura(const char* ruta) {
    int ancho, alto, canales;
    unsigned char* data;
    GLuint tex_id;
    char ruta_alt[512];

    tex_id = 0;
    data = stbi_load(ruta, &ancho, &alto, &canales, 0);
    if (!data) {
        sprintf_s(ruta_alt, sizeof(ruta_alt), "../%s", ruta);
        data = stbi_load(ruta_alt, &ancho, &alto, &canales, 0);
    }
    if (!data) {
        fprintf(stderr, "Error al cargar textura: %s\n", ruta);
        return 0;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    if (canales == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ancho, alto, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (canales == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    stbi_image_free(data);
    return tex_id;
}

static void limpiar_audio(void) {
    mciSendStringA("close musica", NULL, 0, NULL);
}

static void iniciar_musica_fondo(void) {
    DWORD attr;
    const char* ruta_usada;
    char cmd_open[512];

    ruta_usada = NULL;
    attr = GetFileAttributesA("musica\\fondo.wav");
    if (attr != INVALID_FILE_ATTRIBUTES) {
        ruta_usada = "musica\\fondo.wav";
    } else {
        attr = GetFileAttributesA("..\\musica\\fondo.wav");
        if (attr != INVALID_FILE_ATTRIBUTES) {
            ruta_usada = "..\\musica\\fondo.wav";
        }
    }

    if (ruta_usada != NULL) {
        sprintf_s(cmd_open, sizeof(cmd_open), "open \"%s\" alias musica", ruta_usada);
        if (mciSendStringA(cmd_open, NULL, 0, NULL) == 0) {
            mciSendStringA("play musica", NULL, 0, NULL);
        }
    }
}

/* BUGFIX (integridad economica): antes se validaba cada ficha nueva
   contra partida.jugador.saldo "a secas", pero registrar_apuesta() NO
   descuenta el saldo al colocar una ficha (el saldo solo se ajusta una
   vez, al resolver la ronda, con la ganancia/perdida neta -ver
   jugador.c). Eso significaba que el saldo nunca "bajaba" mientras se
   colocaban fichas dentro de la misma ronda, y el jugador podia
   colocar muchas mas fichas de las que realmente podia pagar (hasta
   MAX_APUESTAS), porque cada clic se validaba contra el mismo saldo
   sin descontar.

   Fix: se calcula cuanto se lleva apostado YA en esta ronda (sumando
   partida.apuestas_activas), y se exige que el saldo alcance para eso
   MAS la ficha nueva. No se toca jugador.c/aplicar_resultado_apuesta
   (la liquidacion neta al final de la ronda sigue igual), solo se
   corrige la validacion de "me alcanza para esta ficha". */
static float monto_apostado_en_ronda(void) {
    float total = 0.0f;
    int i;
    for (i = 0; i < partida.num_apuestas_activas; i++) {
        total += partida.apuestas_activas[i].monto;
    }
    return total;
}

static int saldo_alcanza_para_ficha(float monto_ficha) {
    return (partida.jugador.saldo - monto_apostado_en_ronda()) >= monto_ficha;
}

/* Si el saldo no alcanza ni para la ficha minima seleccionada, manda
   al jugador a la pantalla de prestamo. Se llama desde dos lugares:
   1) al resolver una ronda en idle() (cuando NO hubo mensaje reflexivo
      ese mismo round), y 2) justo despues de descartar un mensaje
      reflexivo con ENTER (por si la condicion de fondos tambien se
      cumplia esa misma ronda -se prioriza mostrar primero el mensaje
      reflexivo, y recien al continuar se revisa si ademas hace falta
      pedir prestamo). Antes esta logica estaba duplicada a mano en los
      dos lugares; ahora hay una sola version. */
static void verificar_fondos_y_pedir_prestamo_si_hace_falta(void) {
    if (estado_actual == ESTADO_JUGANDO && partida.jugador.saldo < partida.monto_ficha_actual) {
        cambiar_estado(ESTADO_PRESTAMO);
    }
}

/* Agrega una apuesta al arreglo activo si hay espacio y saldo suficiente.
   Devuelve 1 si se agrego, 0 si no (arreglo lleno). */
static int agregar_apuesta(TipoApuesta tipo, int valor, float monto) {
    char notifbuf[128];

    if (partida.num_apuestas_activas >= MAX_APUESTAS) return 0;
    partida.apuestas_activas[partida.num_apuestas_activas].tipo = tipo;
    partida.apuestas_activas[partida.num_apuestas_activas].valor = valor;
    partida.apuestas_activas[partida.num_apuestas_activas].monto = monto;
    partida.num_apuestas_activas++;

    registrar_apuesta(&partida.jugador, monto);

    if (tipo == APUESTA_NUMERO) {
        sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f al Numero %d", monto, valor);
    } else if (tipo == APUESTA_COLOR) {
        if (valor == (int)COLOR_ROJO) {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a ROJO", monto);
        } else {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a NEGRO", monto);
        }
    } else if (tipo == APUESTA_PAR_IMPAR) {
        if (valor == 0) {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a PAR", monto);
        } else {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a IMPAR", monto);
        }
    } else if (tipo == APUESTA_MITAD) {
        if (valor == 1) {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a 1-18", monto);
        } else {
            sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a 19-36", monto);
        }
    } else if (tipo == APUESTA_DOCENA) {
        sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta: $%.0f a Docena %d", monto, valor);
    } else {
        sprintf_s(notifbuf, sizeof(notifbuf), "Apuesta colocada: $%.0f", monto);
    }
    agregar_notificacion(notifbuf, 1.0f, 0.84f, 0.0f);

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

    /* Fondo con textura de casino (si existe y no estamos en pantalla de carga) */
    if (g_tex_casino != 0 && estado_actual != ESTADO_CARGA) {
        int ancho = glutGet(GLUT_WINDOW_WIDTH);
        int alto = glutGet(GLUT_WINDOW_HEIGHT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, ancho, 0, alto, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_tex_casino);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f((float)ancho, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f((float)ancho, (float)alto);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, (float)alto);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glClear(GL_DEPTH_BUFFER_BIT);
    }

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
    dibujar_notificaciones();
    {
        InfoPantalla info;
        info.mensaje_reflexivo = partida.mensaje_reflexivo_actual;
        info.pagina_educacion = partida.pagina_educacion;
        info.opcion_menu = partida.opcion_menu;
        info.progreso_carga = partida.progreso_carga;
        dibujar_pantalla_segun_estado(estado_actual, &partida.jugador, &info);
    }

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
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
            agregar_apuesta(APUESTA_COLOR, (int)COLOR_ROJO, partida.monto_ficha_actual);
        }
        break;

    case 'n':
    case 'N':
        if (estado_actual == ESTADO_JUGANDO && !partida.bolita.girando
            && saldo_alcanza_para_ficha(partida.monto_ficha_actual)) {
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

            iniciar_giro_bolita_hacia_absoluto(&partida.bolita, angulo_sector_centro, 6 /* vueltas extra visuales: subido de 3 a 6 para llegar a los 10.7s pedidos, ver ruleta_animacion.c */);
        }
        else if (estado_actual == ESTADO_EDUCACION) {
            /* Avanza de pagina con wrap-around (de la ultima vuelve a
               la primera), para poder recorrer las 6 en loop sin tener
               que ir "para atras" -no hay tanto contenido como para
               necesitar retroceder. */
            partida.pagina_educacion = (partida.pagina_educacion + 1) % EDUCACION_NUM_PAGINAS;
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

    case 's':
    case 'S':
        /* MEJORA (interaccion de cierre real, no decorativa): terminar
           la sesion aca es una eleccion tan valida como seguir jugando,
           disponible desde los dos puntos donde antes la unica salida
           real era continuar (el mensaje reflexivo se descartaba con
           ENTER sin alternativa, y la pantalla de prestamo solo
           ofrecia pedir mas plata prestada). No se pasa por
           ESTADO_GAME_OVER -esa pantalla es para cuando el juego FUERZA
           el cierre por deuda impagable, no para cuando el jugador elige
           parar por su cuenta. */
        if (estado_actual == ESTADO_MENSAJE_REFLEXIVO) {
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_SESION_TERMINADA);
        }
        else if (estado_actual == ESTADO_PRESTAMO) {
            cambiar_estado(ESTADO_SESION_TERMINADA);
        }
        break;

    case 'i':
    case 'I':
        /* Pilar 4 del "serious game" (ver docs/analisis-ludopatia.md):
           pantalla de informacion, accesible desde el menu y desde las
           2 pantallas de cierre de sesion -asi se puede consultar
           tanto antes de jugar como despues de terminar. */
        if (estado_actual == ESTADO_MENU || estado_actual == ESTADO_GAME_OVER || estado_actual == ESTADO_SESION_TERMINADA) {
            partida.pagina_educacion = 0;
            cambiar_estado(ESTADO_EDUCACION);
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
            if (partida.opcion_menu == 0) {
                cambiar_estado(ESTADO_JUGANDO);
            } else {
                cambiar_estado(ESTADO_TRAGAMONEDAS_PLACEHOLDER);
            }
        }
        else if (estado_actual == ESTADO_TRAGAMONEDAS_PLACEHOLDER) {
            cambiar_estado(ESTADO_MENU);
        }
        else if (estado_actual == ESTADO_GAME_OVER) {
            inicializar_jugador(&partida.jugador, 1000.0f, glutGet(GLUT_ELAPSED_TIME));
            partida.num_apuestas_activas = 0;
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_SESION_TERMINADA) {
            inicializar_jugador(&partida.jugador, 1000.0f, glutGet(GLUT_ELAPSED_TIME));
            partida.num_apuestas_activas = 0;
            partida.mensaje_reflexivo_actual = NULL;
            cambiar_estado(ESTADO_JUGANDO);
        }
        else if (estado_actual == ESTADO_EDUCACION) {
            cambiar_estado(ESTADO_MENU);
        }
        else if (estado_actual == ESTADO_MENSAJE_REFLEXIVO) {
            /* Se descarta el mensaje y se vuelve a jugar. Si la MISMA
               ronda que disparo el mensaje reflexivo tambien dejo al
               jugador sin fondos para la ficha actual, recien ahora
               (al continuar) se manda a la pantalla de prestamo -ver
               comentario de verificar_fondos_y_pedir_prestamo_si_hace_falta(). */
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
    /* BUGFIX (velocidad de giro): antes delta_tiempo era una constante
       fija (0.016f, "como si" el juego corriera siempre a 60 FPS). Pero
       GLUT clasico no limita cuantas veces por segundo se llama a
       idle() -sin vsync, esto puede correr cientos o miles de veces
       por segundo segun el equipo-, y cada llamada sumaba esos 16ms
       "de mentira" sin importar cuanto tiempo real hubiera pasado.
       Resultado: una animacion pensada para durar 3.5-7 segundos
       terminaba en menos de 1 segundo de reloj real.

       Fix: medir el tiempo real transcurrido con glutGet(GLUT_ELAPSED_TIME)
       (milisegundos desde glutInit) y usar la diferencia real entre
       frames. Se limita (clamp) a un maximo de 0.1s por frame para
       evitar saltos enormes si la ventana se arrastra, se minimiza, o
       el sistema se congela un instante -sin el clamp, un solo frame
       "lento" podria saltar la bolita varios grados de golpe. */
    static int tiempo_anterior_ms = -1;
    int   tiempo_actual_ms = glutGet(GLUT_ELAPSED_TIME);
    float delta_tiempo;
    int estaba_girando = partida.bolita.girando;
    char estado_musica[128];

    if (tiempo_anterior_ms < 0) tiempo_anterior_ms = tiempo_actual_ms;
    delta_tiempo = (float)(tiempo_actual_ms - tiempo_anterior_ms) / 1000.0f;
    tiempo_anterior_ms = tiempo_actual_ms;
    if (delta_tiempo < 0.0f) delta_tiempo = 0.0f;   /* por si el contador diera un valor raro */
    if (delta_tiempo > 0.1f) delta_tiempo = 0.1f;   /* clamp anti-salto */

    /* Bucle manual de audio MCI (waveaudio no soporta la bandera repeat) */
    mciSendStringA("status musica mode", estado_musica, sizeof(estado_musica), NULL);
    if (strcmp(estado_musica, "stopped") == 0) {
        mciSendStringA("seek musica to start", NULL, 0, NULL);
        mciSendStringA("play musica", NULL, 0, NULL);
    }

    if (estado_actual == ESTADO_CARGA) {
        partida.progreso_carga += delta_tiempo / 3.0f;
        if (partida.progreso_carga >= 1.0f) {
            partida.progreso_carga = 1.0f;
            cambiar_estado(ESTADO_MENU);
        }
    }

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
        int   numero_ganador = partida.numero_ganador_pendiente;
        float ganancia_total = calcular_ganancia_total(partida.apuestas_activas, partida.num_apuestas_activas, numero_ganador);
        float apostado = monto_apostado_en_ronda();

        {
            char notifbuf[128];
            const char* color_str = (color_de_numero(numero_ganador) == COLOR_ROJO) ? "Rojo"
                                  : (color_de_numero(numero_ganador) == COLOR_NEGRO) ? "Negro" : "Verde";

            if (ganancia_total > 0.0f) {
                sprintf_s(notifbuf, sizeof(notifbuf), "Numero %d (%s) - Ganaste $%.0f!", numero_ganador, color_str, ganancia_total);
                agregar_notificacion(notifbuf, 0.2f, 1.0f, 0.3f);
            } else {
                sprintf_s(notifbuf, sizeof(notifbuf), "Numero %d (%s) - Perdiste $%.0f", numero_ganador, color_str, apostado);
                agregar_notificacion(notifbuf, 1.0f, 0.35f, 0.35f);
            }
        }

#ifdef _DEBUG
        {
            const char* color_texto = (color_de_numero(numero_ganador) == COLOR_ROJO) ? "ROJO"
                : (color_de_numero(numero_ganador) == COLOR_NEGRO) ? "NEGRO" : "VERDE";
            printf("[RESULTADO REAL] numero=%d color=%s ganancia=%.2f\n", numero_ganador, color_texto, ganancia_total);
        }
#endif

        aplicar_resultado_apuesta(&partida.jugador, ganancia_total);
        partida.num_apuestas_activas = 0;

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

static void teclas_especiales(int tecla, int x, int y) {
    (void)x; (void)y;
    if (estado_actual == ESTADO_MENU) {
        if (tecla == GLUT_KEY_UP || tecla == GLUT_KEY_DOWN) {
            partida.opcion_menu = (partida.opcion_menu == 0) ? 1 : 0;
            glutPostRedisplay();
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    /* Semilla del generador de numeros aleatorios */
    srand((unsigned int)time(NULL));

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Casino Online - Ruleta");
    glutFullScreen();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    inicializar_iluminacion();
    inicializar_notificaciones();
    inicializar_jugador(&partida.jugador, 1000.0f, glutGet(GLUT_ELAPSED_TIME));
    inicializar_estado_juego();
    inicializar_bolita(&partida.bolita);
    partida.angulo_rueda = 0.0f;
    partida.num_apuestas_activas = 0;
    partida.ficha_actual_index = 0;
    partida.monto_ficha_actual = FICHAS[0];
    partida.numero_ganador_pendiente = -1;
    partida.mensaje_reflexivo_actual = NULL;
    partida.pagina_educacion = 0;
    partida.opcion_menu = 0;
    partida.progreso_carga = 0.0f;

    generar_perfil_bezier_rueda();
    construir_malla_rueda();

    atexit(limpiar_audio);
    iniciar_musica_fondo();

    g_tex_carga = cargar_textura("texturas/loading_bg.png");
    g_tex_casino = cargar_textura("texturas/casino_bg.png");
    inicializar_texturas_pantallas(g_tex_carga, g_tex_casino);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutSpecialFunc(teclas_especiales);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutPassiveMotionFunc(mouse_mover);

    glutMainLoop();
    return 0;
}