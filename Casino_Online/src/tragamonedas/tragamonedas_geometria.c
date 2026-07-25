/*
 * tragamonedas_geometria.c
 * Implementacion de la geometria del tragamonedas. Ver tragamonedas_geometria.h.
 * A CARGO DE: Luis.
 *
 * PASO 1: gabinete (base, cuerpo, marquesina) -listo.
 * PASO 2: los 3 rodillos dentro de la ventana del gabinete, mostrando
 * los simbolos que decide tragamonedas_logica.c (Dubenny) a traves de
 * obtener_simbolo_en_posicion() -esta parte NUNCA decide que simbolo va
 * en cada rodillo, solo pregunta y dibuja lo que le contestan.
 *
 * Nota sobre el estado de animacion: dibujar_tragamonedas() no recibe
 * parametros (mismo contrato que ya tenia el placeholder), pero para
 * dibujar los rodillos hace falta un EstadoTragamonedas (las posiciones
 * de cada rodillo). Como la integracion real con el estado del juego
 * (partida, en main.c) todavia no se hace -ver "INTEGRACION FINAL" en
 * el documento del proyecto-, este archivo mantiene su PROPIO estado
 * estatico interno (g_estado_tragamonedas), inicializado una sola vez.
 * Cuando llegue el paso de integracion, ese estado interno se va a
 * reemplazar por el que main.c/partida termine dueniando -no hace falta
 * cambiar nada de la logica de dibujado de esta parte para eso.
 */
#include <GL/glut.h>
#include <GL/glu.h>
#include <string.h>
#include "tragamonedas_geometria.h"
#include "tragamonedas_animacion.h"
#include "tragamonedas_logica.h"
#include "../render/materiales.h"

 /* Alto aproximado (en unidades de fuente, antes de escalar) de una
    mayuscula en GLUT_STROKE_ROMAN -se usa para centrar verticalmente el
    texto sobre el punto de dibujo, ya que glutStrokeCharacter() dibuja
    desde la linea de base hacia arriba, no centrado. */
#define ALTURA_APROX_MAYUSCULA_STROKE 119.05f

    /* Grosor de los paneles laterales que enmarcan la ventana frontal del
       cuerpo -queda como constante propia porque el paso 2 (rodillos) la
       necesita para saber cuanto espacio libre queda adentro. */
#define GROSOR_MARCO_VENTANA 0.30f

       /* Alto de las franjas decorativas (metal y roja) entre secciones del
          gabinete */
#define ALTO_FRANJA 0.08f

          /* Dimensiones internas de la ventana donde van los rodillos, derivadas
             de las constantes publicas del gabinete (ver tragamonedas_geometria.h) */
#define VENTANA_ANCHO_INTERNO       (GABINETE_ANCHO - 2.0f * GROSOR_MARCO_VENTANA)
#define VENTANA_ALTO_INTERNO        (GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA)
#define VENTANA_PROFUNDIDAD_INTERNA (GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA)
#define VENTANA_Y_CENTRO            (GABINETE_Y_INICIO_CUERPO + VENTANA_ALTO_INTERNO / 2.0f)

             /* Grosor de los separadores finos entre rodillos */
#define GROSOR_DIVISOR_RODILLO 0.04f

/* Constantes de la palanca (PASO 3) -brazo cilindrico inclinado con
   una perilla en la punta, montado en un soporte al costado derecho
   del gabinete. */
#define PALANCA_EXTENSION_SOPORTE      0.18f  /* cuanto sobresale el soporte del costado del gabinete */
#define PALANCA_RADIO_BRAZO            0.045f
#define PALANCA_RADIO_BRAZO_PUNTA      0.038f /* levemente mas fino en la punta, como una varilla real */
#define PALANCA_LONGITUD_BRAZO         1.55f
#define PALANCA_RADIO_PERILLA          0.13f
#define PALANCA_ANGULO_INCLINACION     55.0f  /* grados desde el eje X del soporte, hacia arriba */
#define PALANCA_Y_PIVOTE_FACTOR        0.55f  /* fraccion de GABINETE_ALTURA_CUERPO donde va el pivote */

   /* Cuantos simbolos se ven por rodillo (el central + uno arriba y uno
      abajo, como en una tragamonedas real) y cuanto espacio vertical (en
      unidades de mundo) ocupa cada "unidad de simbolo" de
      EstadoRodillo.posicion_actual -se reparte la altura interna de la
      ventana en 3 franjas iguales. */
#define SIMBOLOS_VISIBLES_POR_RODILLO 3
#define ALTURA_UNIDAD_SIMBOLO (VENTANA_ALTO_INTERNO / (float)SIMBOLOS_VISIBLES_POR_RODILLO)

      /* Estado interno de la animacion, solo para poder dibujar algo
         coherente mientras no existe la integracion real (ver comentario
         arriba). NO es el estado "oficial" del juego. */
static EstadoTragamonedas g_estado_tragamonedas;
static int g_estado_tragamonedas_listo = 0;


/* Dibuja texto con la fuente stroke de GLUT, centrado aproximadamente
   sobre el punto actual -mismo criterio que ya se usaba en el
   placeholder y que se usa para los numeros de la ruleta. */
static void dibujar_texto_stroke_simple(const char* texto, float escala) {
    int i;
    int len = (int)strlen(texto);

    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-(float)len * 52.38f, 0.0f, 0.0f); /* centrado aproximado, mismo criterio que ruleta_geometria.c */
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();
}


/* Dibuja una caja centrada en el origen actual, de las dimensiones
   dadas, usando glutSolidCube escalado -helper chico para no repetir
   el mismo glPushMatrix/glScalef/glutSolidCube/glPopMatrix en cada
   pieza del gabinete. */
static void dibujar_caja(float ancho, float alto, float profundo) {
    glPushMatrix();
    glScalef(ancho, alto, profundo);
    glutSolidCube(1.0);
    glPopMatrix();
}


/* Material local del cuerpo del gabinete: negro/gris muy oscuro, con
   algo de brillo para que no se vea plano bajo la luz. No se agrega a
   materiales.h porque es especifico de este mueble. */
static void aplicar_material_cuerpo(void) {
    GLfloat ambient[4] = { 0.03f, 0.03f, 0.035f, 1.0f };
    GLfloat diffuse[4] = { 0.08f, 0.08f, 0.09f, 1.0f };
    GLfloat specular[4] = { 0.25f, 0.25f, 0.28f, 1.0f };
    GLfloat shininess = 40.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* Material local del acento rojo (franja entre cuerpo y marquesina) */
static void aplicar_material_acento_rojo(void) {
    GLfloat ambient[4] = { 0.25f, 0.02f, 0.02f, 1.0f };
    GLfloat diffuse[4] = { 0.65f, 0.05f, 0.05f, 1.0f };
    GLfloat specular[4] = { 0.4f, 0.15f, 0.15f, 1.0f };
    GLfloat shininess = 60.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


/* ------------------------------------------------------------------- */
/* Gabinete (PASO 1, sin cambios de logica -ver historial)             */
/* ------------------------------------------------------------------- */

static void dibujar_base(void) {
    const float FACTOR_ANCHO_BASE = 1.08f;
    const float FACTOR_PROFUNDIDAD_BASE = 1.08f;

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_ALTURA_BASE / 2.0f, 0.0f);
    dibujar_caja(GABINETE_ANCHO * FACTOR_ANCHO_BASE, GABINETE_ALTURA_BASE, GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_BASE);
    glPopMatrix();
}

static void dibujar_cuerpo_principal(void) {
    float ancho_columna;
    float profundidad_util;
    float x_columna;

    ancho_columna = GROSOR_MARCO_VENTANA;
    profundidad_util = GABINETE_PROFUNDIDAD - GROSOR_MARCO_VENTANA;
    x_columna = (GABINETE_ANCHO - ancho_columna) / 2.0f;

    aplicar_material(MATERIAL_METAL);

    glPushMatrix();
    glTranslatef(-x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_columna, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, 0.0f);
    dibujar_caja(ancho_columna, GABINETE_ALTURA_CUERPO, GABINETE_PROFUNDIDAD);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO - GROSOR_MARCO_VENTANA / 2.0f, 0.0f);
    dibujar_caja(GABINETE_ANCHO, GROSOR_MARCO_VENTANA, GABINETE_PROFUNDIDAD);
    glPopMatrix();

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO / 2.0f, -profundidad_util / 2.0f);
    dibujar_caja(GABINETE_ANCHO - 2.0f * ancho_columna, GABINETE_ALTURA_CUERPO, GROSOR_MARCO_VENTANA);
    glPopMatrix();
}

static void dibujar_franja_metal(float y, float ancho, float profundo) {
    aplicar_material(MATERIAL_METAL);
    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);
    dibujar_caja(ancho, ALTO_FRANJA, profundo);
    glPopMatrix();
}

static void dibujar_franja_roja(float y, float ancho, float profundo) {
    aplicar_material_acento_rojo();
    glPushMatrix();
    glTranslatef(0.0f, y, 0.0f);
    dibujar_caja(ancho, ALTO_FRANJA, profundo);
    glPopMatrix();
}

static void dibujar_marquesina(void) {
    const float FACTOR_ANCHO_MARQUESINA = 0.92f;
    const float FACTOR_PROFUNDIDAD_MARQUESINA = 0.75f;
    float y_centro_marquesina;

    y_centro_marquesina = GABINETE_Y_INICIO_MARQUESINA + GABINETE_ALTURA_MARQUESINA / 2.0f;

    aplicar_material_cuerpo();
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, 0.0f);
    dibujar_caja(GABINETE_ANCHO * FACTOR_ANCHO_MARQUESINA, GABINETE_ALTURA_MARQUESINA, GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_MARQUESINA);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.85f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, y_centro_marquesina, (GABINETE_PROFUNDIDAD * FACTOR_PROFUNDIDAD_MARQUESINA) / 2.0f + 0.02f);
    glScalef(0.0016f, 0.0016f, 1.0f);
    dibujar_texto_stroke_simple("TRAGAMONEDAS", 1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

static void dibujar_gabinete(void) {
    dibujar_base();
    dibujar_franja_metal(GABINETE_Y_INICIO_CUERPO, GABINETE_ANCHO * 1.08f, GABINETE_PROFUNDIDAD * 1.08f);
    dibujar_cuerpo_principal();
    dibujar_franja_roja(GABINETE_Y_INICIO_MARQUESINA, GABINETE_ANCHO, GABINETE_PROFUNDIDAD);
    dibujar_marquesina();
}


/* ------------------------------------------------------------------- */
/* Rodillos (PASO 2)                                                    */
/* ------------------------------------------------------------------- */

/* Dibuja un simbolo centrado en el origen actual, con primitivas
   propias. Todo se dibuja sin iluminacion (como el texto de la
   marquesina) para que se lea siempre nitido, como un icono pintado,
   sin depender de como le pegue la luz dentro del gabinete. */
static void dibujar_simbolo(SimboloTragamonedas simbolo) {
    GLboolean iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    switch (simbolo) {

    case SIMBOLO_CEREZA:
        /* Dos cerezas rojas con tallo verde, convergiendo a un punto
           arriba -forma clasica del simbolo de cereza. */
        glColor3f(0.75f, 0.05f, 0.08f);
        glPushMatrix();
        glTranslatef(-0.10f, -0.14f, 0.0f);
        glutSolidSphere(0.09, 12, 12);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.10f, -0.14f, 0.0f);
        glutSolidSphere(0.09, 12, 12);
        glPopMatrix();

        glColor3f(0.15f, 0.45f, 0.15f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex3f(-0.10f, -0.06f, 0.01f); glVertex3f(0.0f, 0.20f, 0.01f);
        glVertex3f(0.10f, -0.06f, 0.01f);  glVertex3f(0.0f, 0.20f, 0.01f);
        glEnd();

        glPushMatrix();
        glTranslatef(0.06f, 0.20f, 0.0f);
        glScalef(0.10f, 0.045f, 0.02f);
        glutSolidSphere(1.0, 8, 8); /* hoja, achatada con el escalado */
        glPopMatrix();
        break;

    case SIMBOLO_CAMPANA:
        /* Cuerpo: cono ancho abajo y angosto arriba (silueta real de
           campana, no un domo achatado). glutSolidCone dibuja con la
           base en el plano XY y el apex hacia +Z; se rota -90 en X
           para que el apex quede apuntando +Y (arriba). */
        glColor3f(0.85f, 0.68f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, -0.13f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.16, 0.30, 16, 8);
        glPopMatrix();

        /* Falda/reborde inferior: un anillo fino justo en la base del
           cono, un poco mas ancho que la base -da el borde acampanado
           tipico. Se rota 90 en X para que el "agujero" del torus
           quede alineado con el eje Y (asi encierra el cono, en vez de
           quedar de frente encarando la camara como en la herradura). */
        glPushMatrix();
        glTranslatef(0.0f, -0.13f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(0.018, 0.165, 8, 20);
        glPopMatrix();

        /* Perilla arriba */
        glPushMatrix();
        glTranslatef(0.0f, 0.17f, 0.0f);
        glutSolidSphere(0.03, 8, 8);
        glPopMatrix();

        /* Badajo colgando por debajo de la boca de la campana */
        glColor3f(0.55f, 0.45f, 0.10f);
        glPushMatrix();
        glTranslatef(0.0f, -0.22f, 0.0f);
        glutSolidSphere(0.035, 8, 8);
        glPopMatrix();
        break;

    case SIMBOLO_HERRADURA:
        /* Simplificacion estilizada: un anillo (torus) plateado en vez
           de la forma de "U" completa -da la idea de herradura/suerte
           sin necesitar geometria de arco parcial. */
        glColor3f(0.75f, 0.75f, 0.78f);
        glutSolidTorus(0.055, 0.20, 10, 20);
        break;

    case SIMBOLO_DIAMANTE:
        /* Octaedro estirado en vertical -forma clasica de diamante */
        glColor3f(0.35f, 0.75f, 0.90f);
        glPushMatrix();
        glScalef(0.17f, 0.24f, 0.17f);
        glutSolidOctahedron();
        glPopMatrix();
        break;

    case SIMBOLO_BARRA:
        /* Placa negra con el texto "BAR" -simbolo clasico de tragamonedas */
        glColor3f(0.05f, 0.05f, 0.05f);
        glPushMatrix();
        dibujar_caja(0.46f, 0.20f, 0.04f);
        glPopMatrix();

        glColor3f(0.85f, 0.68f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.03f);
        glScalef(0.0013f, 0.0013f, 1.0f);
        glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f); /* centrado vertical, ver comentario en la constante */
        dibujar_texto_stroke_simple("BAR", 1.0f);
        glPopMatrix();
        break;

    case SIMBOLO_SIETE:
        /* El clasico "7 de la suerte", rojo */
        glColor3f(0.8f, 0.1f, 0.1f);
        glPushMatrix();
        glScalef(0.0035f, 0.0035f, 1.0f);
        glTranslatef(0.0f, -ALTURA_APROX_MAYUSCULA_STROKE / 2.0f, 0.0f); /* centrado vertical -antes quedaba corrido hacia arriba y se recortaba con el marco */
        dibujar_texto_stroke_simple("7", 1.0f);
        glPopMatrix();
        break;

    default:
        break;
    }

    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}


/* Dibuja un rodillo: la placa metalica de fondo (el "tambor") y hasta
   3 simbolos apilados verticalmente (el que esta centrado en
   rodillo->posicion_actual, mas el anterior y el siguiente en la tira
   -mismo efecto visual que una tragamonedas real, donde se alcanza a
   ver un pedazo del simbolo de arriba y de abajo). Consulta siempre a
   obtener_simbolo_en_posicion() -nunca decide el simbolo por su cuenta. */
static void dibujar_rodillo(int indice, const EstadoRodillo* rodillo, float x_centro, float z_rodillo) {
    int offset;

    /* Placa de fondo del rodillo (el "tambor" metalico visible detras
       de los simbolos) */
    aplicar_material(MATERIAL_METAL);
    glPushMatrix();
    glTranslatef(x_centro, VENTANA_Y_CENTRO, z_rodillo);
    dibujar_caja(VENTANA_ANCHO_INTERNO / 3.0f - GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, 0.04f);
    glPopMatrix();

    /* Simbolos: -1 (arriba), 0 (centro), +1 (abajo). Cuando en el PASO
       4 la animacion vaya sumando a posicion_actual cuadro a cuadro,
       este mismo codigo va a mostrar el scroll sin ningun cambio. */
    for (offset = -1; offset <= 1; offset++) {
        float posicion_consultada;
        SimboloTragamonedas simbolo;
        float y_simbolo;

        posicion_consultada = rodillo->posicion_actual + (float)offset;
        simbolo = obtener_simbolo_en_posicion(indice, posicion_consultada);
        y_simbolo = VENTANA_Y_CENTRO - (float)offset * ALTURA_UNIDAD_SIMBOLO;

        glPushMatrix();
        glTranslatef(x_centro, y_simbolo, z_rodillo + 0.03f);
        dibujar_simbolo(simbolo);
        glPopMatrix();
    }
}


/* Separadores finos entre rodillos (cromados, igual que el marco de la
   ventana) para que se vean como carriles distintos. */
static void dibujar_divisores_rodillos(float x_izquierdo, float x_derecho) {
    aplicar_material(MATERIAL_METAL);

    glPushMatrix();
    glTranslatef(x_izquierdo, VENTANA_Y_CENTRO, 0.0f);
    dibujar_caja(GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, VENTANA_PROFUNDIDAD_INTERNA);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x_derecho, VENTANA_Y_CENTRO, 0.0f);
    dibujar_caja(GROSOR_DIVISOR_RODILLO, VENTANA_ALTO_INTERNO, VENTANA_PROFUNDIDAD_INTERNA);
    glPopMatrix();
}


/* Dibuja los 3 rodillos dentro de la ventana del gabinete */
static void dibujar_rodillos(const EstadoTragamonedas* estado) {
    const float ancho_rodillo = VENTANA_ANCHO_INTERNO / 3.0f;
    /* Los rodillos se ubican un poco adentro del gabinete, no pegados
       al frente, para que se sientan "dentro" de la ventana */
    const float z_rodillo = 0.15f;
    int i;

    for (i = 0; i < NUM_RODILLOS; i++) {
        float x_centro = -VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo / 2.0f + (float)i * ancho_rodillo;
        dibujar_rodillo(i, &estado->rodillos[i], x_centro, z_rodillo);
    }

    /* Divisores en los 2 bordes internos (entre rodillo 0-1 y 1-2) */
    dibujar_divisores_rodillos(-VENTANA_ANCHO_INTERNO / 2.0f + ancho_rodillo,
        -VENTANA_ANCHO_INTERNO / 2.0f + 2.0f * ancho_rodillo);
}


/* ------------------------------------------------------------------- */
/* Palanca (PASO 3)                                                     */
/* ------------------------------------------------------------------- */

/* Dibuja la palanca: soporte cromado en el costado derecho del
   gabinete, brazo cilindrico inclinado (gluCylinder -parte de GLU,
   misma libreria que ya usa main.c para gluLookAt/gluPerspective, no
   es un modelo importado) y perilla roja en la punta.

   angulo_extra_grados: se suma a la inclinacion de reposo del brazo.
   Por ahora se llama siempre con 0.0f (palanca quieta) -queda como
   parametro para cuando se decida como se dispara el giro (PASO 4 /
   integracion): tirar de la palanca va a ser, en ese momento, animar
   este angulo de 0 a un valor negativo y de vuelta a 0, sin tener que
   tocar esta funcion. */
static void dibujar_palanca(float angulo_extra_grados) {
    float x_lateral;
    float x_pivote;
    float y_pivote;
    GLUquadric* quad;

    x_lateral = GABINETE_ANCHO / 2.0f; /* borde derecho del cuerpo del gabinete */
    x_pivote = x_lateral + PALANCA_EXTENSION_SOPORTE;
    y_pivote = GABINETE_Y_INICIO_CUERPO + GABINETE_ALTURA_CUERPO * PALANCA_Y_PIVOTE_FACTOR;

    /* Soporte: caja cromada que conecta el costado del gabinete con el
       pivote de la palanca */
    aplicar_material(MATERIAL_METAL);
    glPushMatrix();
    glTranslatef(x_lateral + PALANCA_EXTENSION_SOPORTE / 2.0f, y_pivote, 0.0f);
    dibujar_caja(PALANCA_EXTENSION_SOPORTE, 0.22f, 0.32f);
    glPopMatrix();

    /* Bisagra/pivote: disco achatado en la base del brazo */
    glPushMatrix();
    glTranslatef(x_pivote, y_pivote, 0.0f);
    glScalef(0.10f, 0.10f, 0.06f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    /* Brazo + perilla: todo dentro de la misma matriz, para que la
       perilla quede exactamente en la punta del brazo sin calcular
       trigonometria a mano (se avanza LONGITUD_BRAZO en el eje local
       ya inclinado). */
    glPushMatrix();
    glTranslatef(x_pivote, y_pivote, 0.0f);
    /* IMPORTANTE: el orden de estas dos rotaciones importa. glRotatef
       post-multiplica la matriz actual, y OpenGL aplica al vertice la
       transformacion mas cercana a el primero -es decir, la ULTIMA
       llamada a glRotatef es la PRIMERA en afectar al cilindro. Por
       eso la inclinacion (Z) va primero en el codigo: para que el
       cilindro (dibujado a lo largo de su propio eje Z) se incline
       DENTRO del plano XY antes de que el rotate en Y lo reoriente
       hacia afuera del gabinete. Al reves (como estaba antes), la
       rotacion en Z no tiene ningun efecto -un punto sobre el propio
       eje Z no se mueve al rotar alrededor de Z- y el brazo queda
       siempre horizontal, sin importar el angulo. */
    glRotatef(PALANCA_ANGULO_INCLINACION + angulo_extra_grados, 0.0f, 0.0f, 1.0f); /* inclina, todavia sobre el eje Z local */
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); /* reorienta ese eje ya inclinado hacia afuera del gabinete (+X) */

    aplicar_material(MATERIAL_METAL);
    quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, PALANCA_RADIO_BRAZO, PALANCA_RADIO_BRAZO_PUNTA, PALANCA_LONGITUD_BRAZO, 12, 4);
    gluDeleteQuadric(quad);

    glTranslatef(0.0f, 0.0f, PALANCA_LONGITUD_BRAZO);
    aplicar_material_acento_rojo();
    glutSolidSphere(PALANCA_RADIO_PERILLA, 16, 16);
    glPopMatrix();
}


void dibujar_tragamonedas(void) {
    if (!g_estado_tragamonedas_listo) {
        inicializar_tragamonedas_animacion(&g_estado_tragamonedas);
        g_estado_tragamonedas_listo = 1;
    }

    dibujar_gabinete();
    dibujar_rodillos(&g_estado_tragamonedas);
    dibujar_palanca(0.0f);
}