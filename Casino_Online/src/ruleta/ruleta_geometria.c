/*
 * ruleta_geometria.c
 * Implementacion de la geometria de la rueda. Ver ruleta_geometria.h.
 *
 * VERSION CON JERARQUIA REAL: dibujar_rueda() ya no gestiona su propia
 * traslacion/rotacion de nodo. Quien la llama (main.c -> display()) es
 * responsable de dejar la matriz ModelView posicionada antes de
 * invocarla, y de mantenerla activa mientras se dibujan sus hijos
 * (la bolita), para lograr una jerarquia real con la pila de matrices
 * (mesa -> rueda -> bolita), en vez de objetos independientes.
 */
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ruleta_geometria.h"
#include "../render/materiales.h"
#include "../utils/bezier.h"
#include "../core/estado_juego.h" /* ORDEN_RUEDA_EUROPEA, color_de_numero: para la pista numerada */

 /* RADIO_MESA ahora se expone en ruleta_geometria.h (Luis la necesita
    para el mapeo de posicion 3D a celda del tablero de apuestas) */
#define PERFIL_SEGMENTOS     12   /* puntos a lo largo del perfil (radio-altura) */
#define REVOLUCION_SEGMENTOS 36   /* divisiones angulares (360 / 36 = 10 grados) */
#define PI_GEOMETRIA         3.14159265358979323846f

    /* ------------------------------------------------------------------- */
    /* Datos de la malla generada.                                          */
    /* perfil_puntos[i].x = radio, perfil_puntos[i].y = altura (el campo z  */
    /* del perfil no se usa, se deja en 0).                                 */
    /* ------------------------------------------------------------------- */
static Punto3D perfil_puntos[PERFIL_SEGMENTOS];
static Punto3D malla_vertices[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];
static Punto3D malla_normales[PERFIL_SEGMENTOS][REVOLUCION_SEGMENTOS];

/* ------------------------------------------------------------------- */
/* Utilidades vectoriales pequenas (solo usadas aqui).                  */
/* ------------------------------------------------------------------- */
static Punto3D producto_cruz(Punto3D a, Punto3D b) {
    Punto3D r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

static Punto3D normalizar_vector(Punto3D v) {
    float longitud;
    Punto3D r;

    longitud = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (longitud < 0.00001f) {
        r.x = 0.0f; r.y = 1.0f; r.z = 0.0f;
        return r;
    }
    r.x = v.x / longitud;
    r.y = v.y / longitud;
    r.z = v.z / longitud;
    return r;
}



/* Ver declaracion en ruleta_geometria.h. perfil_puntos[].x es radio
   creciente (garantizado: los puntos de control de la Bezier tienen
   X monotonamente creciente, 0 -> 1.05 -> 2.25 -> 3.0, asi que la
   curva resultante tambien lo es), asi que basta interpolar linealmente
   entre los dos puntos muestreados que rodean el radio pedido. */
float altura_superficie_en_radio(float radio) {
    int i;

    if (radio <= perfil_puntos[0].x) return perfil_puntos[0].y;
    if (radio >= perfil_puntos[PERFIL_SEGMENTOS - 1].x) return perfil_puntos[PERFIL_SEGMENTOS - 1].y;

    for (i = 0; i < PERFIL_SEGMENTOS - 1; i++) {
        float r0 = perfil_puntos[i].x;
        float r1 = perfil_puntos[i + 1].x;

        if (radio >= r0 && radio <= r1) {
            float t = (r1 - r0 < 0.00001f) ? 0.0f : (radio - r0) / (r1 - r0);
            return perfil_puntos[i].y + t * (perfil_puntos[i + 1].y - perfil_puntos[i].y);
        }
    }
    return perfil_puntos[PERFIL_SEGMENTOS - 1].y; /* no deberia llegar aca */
}

/* ------------------------------------------------------------------- */

void generar_perfil_bezier_rueda(void) {
    Punto3D p0, p1, p2, p3;
    int i;
    float t;

    p0.x = 0.0f;                        p0.y = 0.35f; p0.z = 0.0f;
    p1.x = RADIO_EXTERIOR_RUEDA * 0.35f; p1.y = 0.40f; p1.z = 0.0f;
    p2.x = RADIO_EXTERIOR_RUEDA * 0.75f; p2.y = 0.10f; p2.z = 0.0f;
    p3.x = RADIO_EXTERIOR_RUEDA;         p3.y = 0.22f; p3.z = 0.0f;

    for (i = 0; i < PERFIL_SEGMENTOS; i++) {
        t = (float)i / (float)(PERFIL_SEGMENTOS - 1);
        perfil_puntos[i] = evaluar_bezier_cubica(p0, p1, p2, p3, t);
    }
}

void construir_malla_rueda(void) {
    int i, j;
    float angulo, radio, altura, dradio, daltura;
    Punto3D tangente_perfil, tangente_circ, normal;

    for (i = 0; i < PERFIL_SEGMENTOS; i++) {
        radio = perfil_puntos[i].x;
        altura = perfil_puntos[i].y;

        if (i == 0) {
            dradio = perfil_puntos[i + 1].x - perfil_puntos[i].x;
            daltura = perfil_puntos[i + 1].y - perfil_puntos[i].y;
        }
        else if (i == PERFIL_SEGMENTOS - 1) {
            dradio = perfil_puntos[i].x - perfil_puntos[i - 1].x;
            daltura = perfil_puntos[i].y - perfil_puntos[i - 1].y;
        }
        else {
            dradio = perfil_puntos[i + 1].x - perfil_puntos[i - 1].x;
            daltura = perfil_puntos[i + 1].y - perfil_puntos[i - 1].y;
        }

        for (j = 0; j < REVOLUCION_SEGMENTOS; j++) {
            angulo = (float)j * (2.0f * PI_GEOMETRIA / (float)REVOLUCION_SEGMENTOS);

            malla_vertices[i][j].x = radio * cosf(angulo);
            malla_vertices[i][j].y = altura;
            malla_vertices[i][j].z = radio * sinf(angulo);

            tangente_perfil.x = dradio * cosf(angulo);
            tangente_perfil.y = daltura;
            tangente_perfil.z = dradio * sinf(angulo);

            tangente_circ.x = -sinf(angulo);
            tangente_circ.y = 0.0f;
            tangente_circ.z = cosf(angulo);

            /* Orden confirmado: tangente_circ x tangente_perfil da la
               normal apuntando hacia afuera de la superficie (ver
               bug documentado: el orden opuesto daba normales
               invertidas y la rueda se veia negra/sin luz). */
            normal = producto_cruz(tangente_circ, tangente_perfil);
            malla_normales[i][j] = normalizar_vector(normal);
        }
    }
}

void dibujar_mesa(void) {
    /* Ancho del borde de madera alrededor del fieltro. Se dibuja FUERA
       del cuadrado del fieltro (nunca se superpone con el), asi que no
       hay riesgo de z-fighting entre los dos aunque ambos esten en
       Y=0. */
    const float BORDE_MESA = 0.5f;

    glPushMatrix();

    aplicar_material(MATERIAL_FIELTRO);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-RADIO_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, -RADIO_MESA);
    glEnd();

    /* BUGFIX (codigo muerto): MATERIAL_MADERA estaba completamente
       definido en materiales.c (ambient/diffuse/specular/shininess
       calibrados) pero no se aplicaba a ninguna geometria en toda la
       escena -el propio comentario en materiales.c lo admitia ("aun
       no aplicado... no hay bordes/patas de mesa todavia"). Se le da
       uso real aca: un marco de madera alrededor del fieltro, como en
       una mesa de casino real. Son 4 quads (arriba/abajo cubren las
       esquinas completas; izquierda/derecha solo el tramo central,
       para no dibujar dos veces la misma esquina). */
    aplicar_material(MATERIAL_MADERA);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    /* Borde superior (incluye las 2 esquinas de ese lado) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA - BORDE_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA - BORDE_MESA);

    /* Borde inferior (incluye las otras 2 esquinas) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA + BORDE_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA + BORDE_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA);

    /* Borde izquierdo (solo el tramo central, las esquinas ya las
       cubrieron los dos quads de arriba) */
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(-RADIO_MESA - BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(-RADIO_MESA, 0.0f, -RADIO_MESA);

    /* Borde derecho */
    glVertex3f(RADIO_MESA, 0.0f, -RADIO_MESA);
    glVertex3f(RADIO_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, RADIO_MESA);
    glVertex3f(RADIO_MESA + BORDE_MESA, 0.0f, -RADIO_MESA);

    glEnd();

    glPopMatrix();
}

void dibujar_rueda(void) {
    int i, j, jj;

    /* Ya NO hace glPushMatrix/glTranslatef/glRotatef/glPopMatrix aqui:
       el llamador (main.c) posiciona el nodo "rueda" antes de invocar
       esta funcion, y mantiene esa matriz activa para que la bolita
       (dibujada despues, como hijo) herede la misma transformacion. */

    aplicar_material(MATERIAL_METAL);

    for (i = 0; i < PERFIL_SEGMENTOS - 1; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= REVOLUCION_SEGMENTOS; j++) {
            jj = j % REVOLUCION_SEGMENTOS;

            glNormal3f(malla_normales[i + 1][jj].x, malla_normales[i + 1][jj].y, malla_normales[i + 1][jj].z);
            glVertex3f(malla_vertices[i + 1][jj].x, malla_vertices[i + 1][jj].y, malla_vertices[i + 1][jj].z);

            glNormal3f(malla_normales[i][jj].x, malla_normales[i][jj].y, malla_normales[i][jj].z);
            glVertex3f(malla_vertices[i][jj].x, malla_vertices[i][jj].y, malla_vertices[i][jj].z);
        }
        glEnd();
    }
}

void dibujar_vidrio_protector(void) {
    glPushMatrix();

    glTranslatef(0.0f, 0.05f, 0.0f);

    aplicar_material(MATERIAL_VIDRIO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glScalef(RADIO_EXTERIOR_RUEDA + 0.3f, 0.35f, RADIO_EXTERIOR_RUEDA + 0.3f);
    glutSolidSphere(1.0, 24, 24);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glPopMatrix();
}

/* ------------------------------------------------------------------- */
/* Pista numerada (Opcion A: colores geometricos por sector + numeros   */
/* rectos, sin textura). Se llama justo despues de dibujar_rueda(),     */
/* mientras la matriz del nodo "rueda" sigue activa, para que la pista  */
/* gire junto con la rueda en vez de quedarse fija.                     */
/*                                                                       */
/* Usa ORDEN_RUEDA_EUROPEA y color_de_numero() de estado_juego.h        */
/* (Luis) -misma fuente de verdad que usa main.c para decidir el       */
/* numero ganador, asi que lo que se VE en la rueda siempre coincide    */
/* con lo que CUENTA como resultado. No hay copia local de estos datos. */
/* ------------------------------------------------------------------- */

/* Radios de la banda donde va la pista (entre el domo central y el
   labio del borde). Ajustados a ojo contra el perfil de Bezier actual
   (p1 en radio ~1.05, p2 en radio ~2.25, p3 -borde- en radio 3.0);
   recalibrar viendo la escena real si hace falta. */
   /* RADIO_INTERNO_PISTA / RADIO_EXTERNO_PISTA ahora se exponen en
      ruleta_geometria.h (ruleta_animacion.c los necesita para orbitar la
      bolita en el centro de la banda, no adivinar un radio a mano) */

      /* Altura a la que se dibujaba la pista en la version original
         (constante, 0.42): reemplazada por el calculo dinamico
         altura_superficie_en_radio() de abajo, asi que ya no existe
         como #define -queda solo esta nota para no dejar una
         constante sin usar dando vueltas en el archivo. */

         /* BUGFIX (numeros deformados/desbordados en la rueda): antes se usaba
            una escala fija (0.0022) sin importar cuantos digitos tuviera el
            numero. El ancho de arco disponible por sector es fijo (paso_angular
            grados a radio_medio), pero un numero de 2 digitos (27 de los 37
            numeros de la rueda) media con esa escala fija cerca de un 30% MAS
            ancho que su propio sector -se metia visualmente en los sectores
            vecinos. ANCHO_MAX_NUMERO_PISTA es el ancho de arco real disponible
            (cuerda del sector a radio_medio, con un margen de seguridad), y la
            escala se reduce (de forma UNIFORME, x e y por igual, para no
            deformar los digitos) solo si hace falta.
            El margen se subio de 0.85 a 0.95 (menos "colchon" contra el sector
            vecino, mas tamano real para el numero) porque con 0.85 los numeros
            de 2 digitos quedaban innecesariamente chicos -ver BUGFIX de grosor
            de linea proporcional, mas abajo, que es la otra mitad de por que
            se veian "borroneados". */
#define ESCALA_NUMERO_PISTA_BASE 0.0024f
static float ancho_max_numero_pista(void) {
    const float paso_angular = 360.0f / 37.0f;
    const float radio_medio = (RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f;
    return 2.0f * radio_medio * sinf((paso_angular / 2.0f) * PI_GEOMETRIA / 180.0f) * 0.95f;
}

static void dibujar_numero_pista(int numero) {
    char texto[4];
    int i, len;
    GLboolean iluminacion_estaba_activa;
    float escala, ancho_total;

    snprintf(texto, sizeof(texto), "%d", numero);
    len = (int)strlen(texto);

    /* BUGFIX (legibilidad): la fuente stroke dibuja solo el contorno
       (lineas delgadas, no un glifo relleno), y al estar sujeta a la
       iluminacion de la escena se veia opaca/apagada en las zonas de
       la rueda que reciben menos luz directa. Se desactiva la
       iluminacion SOLO para el numero (no afecta las cuñas de color,
       que ya se dibujaron antes de este punto), para que siempre se
       vea blanco puro sin importar donde caiga en la rueda. */
    iluminacion_estaba_activa = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    /* BUGFIX ("los numeros se ven borroneados/deformados", causa real):
       NO es un filtro -es la combinacion de dos cosas:
       1) En todo el proyecto no se activa GL_LINE_SMOOTH en ningun
          lado (el MSAA global ya esta documentado como poco confiable
          en GLUT clasico -ver contexto-proyecto.md-, asi que los
          trazos finos y diagonales de la fuente stroke quedaban
          dentados/escalonados a este tamano tan chico en pantalla).
       2) El grosor de linea era FIJO (2.5px) sin importar cuanto se
          hubiera achicado el numero (ver ancho_max_numero_pista arriba):
          para un numero de 2 digitos, ya reducido a ~2/3 de su tamano
          base para caber en su sector, una linea de 2.5px es
          PROPORCIONALMENTE mucho mas gruesa que en un numero de 1
          digito sin reducir -las curvas de los digitos (3, 6, 8, 9...)
          se embarran entre si y dejan de leerse como numero.
       Fix: GL_LINE_SMOOTH/GL_BLEND ahora los activa UNA sola vez el
       llamador (dibujar_pista_numerada, antes de dibujar los 37
       numeros) en vez de aca -esta funcion se llama una vez por cada
       numero de la rueda, asi que antes se hacian 37 activaciones y
       37 restauraciones de estado por frame para nada, ya que el
       valor no cambia entre un numero y otro. El grosor de linea si
       sigue calculandose por numero, porque CADA numero puede tener
       una 'escala' distinta (numeros de 1 vs 2 digitos no se achican
       igual). */

    escala = ESCALA_NUMERO_PISTA_BASE;
    ancho_total = (float)len * 104.76f * escala;
    {
        float ancho_max = ancho_max_numero_pista();
        if (ancho_total > ancho_max) {
            escala *= ancho_max / ancho_total;      /* escala uniforme (misma en x e y) */
            ancho_total = ancho_max;
        }
    }

    {
        float grosor = 2.5f * (escala / ESCALA_NUMERO_PISTA_BASE);
        if (grosor < 1.1f) grosor = 1.1f;
        glLineWidth(grosor);
    }

    glPushMatrix();
    glScalef(escala, escala, 1.0f);
    glTranslatef(-(float)len * 52.38f, -59.5f, 0.0f); /* centrar horizontal y verticalmente (52.38 = mitad del ancho de avance por caracter, 59.5 = mitad de la altura de mayuscula/digito de GLUT_STROKE_ROMAN) */
    for (i = 0; i < len; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, texto[i]);
    }
    glPopMatrix();

    glLineWidth(1.0f);
    if (iluminacion_estaba_activa) glEnable(GL_LIGHTING);
}

#define OFFSET_PISTA 0.006f /* separacion minima sobre la malla metalica, para evitar z-fighting */

void dibujar_pista_numerada(void) {
    int sector;
    const float paso_angular = 360.0f / 37.0f;
    GLboolean line_smooth_estaba_activo;
    GLboolean blend_estaba_activo;

    /* FIX: alturas leidas de la superficie real (Bezier) en vez del
       ALTURA_PISTA fijo original. El perfil se hunde entre el radio
       interno y externo de la pista (ver comentario de altura_superficie_en_radio),
       asi que un valor constante hacia que la pista entera flotara por
       encima de la rueda -mas notorio hacia el borde externo, que es
       justo donde esta el hundimiento mas fuerte del perfil. */
    float altura_interna = altura_superficie_en_radio(RADIO_INTERNO_PISTA) + OFFSET_PISTA;
    float altura_externa = altura_superficie_en_radio(RADIO_EXTERNO_PISTA) + OFFSET_PISTA;

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glDisable(GL_CULL_FACE);

    /* BUENA PRACTICA (antes activado/restaurado 37 veces por frame,
       una por numero, dentro de dibujar_numero_pista): GL_LINE_SMOOTH
       y GL_BLEND no cambian de valor entre un numero y el siguiente,
       asi que alcanza con activarlos una sola vez aca afuera del loop,
       y restaurarlos una sola vez al final. No afecta a las cuñas de
       color (GL_TRIANGLE_STRIP opacos, alpha=1) dibujadas dentro del
       mismo loop -con blending activado pero alpha=1 se ven identicas
       a sin blending. */
    line_smooth_estaba_activo = glIsEnabled(GL_LINE_SMOOTH);
    blend_estaba_activo = glIsEnabled(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    for (sector = 0; sector < 37; sector++) {
        int numero = ORDEN_RUEDA_EUROPEA[sector];
        ColorRuleta color = color_de_numero(numero);
        float angulo_inicio = sector * paso_angular;
        float angulo_fin = angulo_inicio + paso_angular;
        int segmentos_arco = 4;
        int k;

        if (color == COLOR_VERDE)      glColor3f(0.0f, 0.5f, 0.15f);
        else if (color == COLOR_ROJO)  glColor3f(0.75f, 0.08f, 0.08f);
        else                            glColor3f(0.05f, 0.05f, 0.05f);

        glBegin(GL_TRIANGLE_STRIP);
        for (k = 0; k <= segmentos_arco; k++) {
            float t = (float)k / segmentos_arco;
            float ang = (angulo_inicio + t * (angulo_fin - angulo_inicio)) * PI_GEOMETRIA / 180.0f;
            glNormal3f(0.0f, 1.0f, 0.0f);
            /* BUGFIX: signo negativo en Z, para que coincida con la
               convencion de glRotatef(angulo, 0,1,0) que usa
               dibujar_bolita() (glRotatef aplica z' = -x*sin(angulo),
               no +x*sin(angulo)). Sin este signo, la pista quedaba en
               espejo respecto a donde realmente cae la bolita: se veia
               caer sobre un color/numero que no era el que en verdad
               se habia decidido como ganador. */
            glVertex3f(RADIO_INTERNO_PISTA * cosf(ang), altura_interna, -RADIO_INTERNO_PISTA * sinf(ang));
            glVertex3f(RADIO_EXTERNO_PISTA * cosf(ang), altura_externa, -RADIO_EXTERNO_PISTA * sinf(ang));
        }
        glEnd();

        {
            float ang_medio = (angulo_inicio + paso_angular / 2.0f) * PI_GEOMETRIA / 180.0f;
            float radio_medio = (RADIO_INTERNO_PISTA + RADIO_EXTERNO_PISTA) / 2.0f;
            /* BUGFIX (numeros "deformados"/con partes faltantes, z-fighting):
               antes esta altura se calculaba con altura_superficie_en_radio(radio_medio),
               es decir, la altura REAL de la curva de Bezier en ese radio
               exacto. Pero la banda de color de abajo (el GL_TRIANGLE_STRIP
               de este mismo sector) NO sigue esa curva real entre
               RADIO_INTERNO_PISTA y RADIO_EXTERNO_PISTA -sus vertices
               interpolan LINEALMENTE entre altura_interna y altura_externa.
               Como el perfil de la rueda es concavo (forma de valle) en esa
               zona, la altura real de la curva en el punto medio queda POR
               DEBAJO de esa interpolacion lineal -el numero se dibujaba mas
               abajo que la superficie de la banda que en realidad se ve en
               pantalla, y quedaba peleando en Z contra ella (partes de los
               trazos del numero se recortaban/desaparecian al azar segun el
               angulo de camara).
               Fix: usar la MISMA interpolacion lineal que usa la banda
               (altura_interna y altura_externa ya estan calculadas arriba),
               evaluada en radio_medio -que, por ser el punto medio, es
               simplemente el promedio de ambas-, para que el numero quede
               siempre apoyado sobre la superficie que realmente se dibuja. */
            float altura_numero = (altura_interna + altura_externa) / 2.0f + OFFSET_PISTA * 2.0f;

            glColor3f(1.0f, 1.0f, 1.0f);
            glPushMatrix();
            /* Mismo BUGFIX de signo que en los vertices de arriba */
            glTranslatef(radio_medio * cosf(ang_medio), altura_numero, -radio_medio * sinf(ang_medio));
            /* BUGFIX: la posicion ya se corrigio con el signo negativo
               (arriba), pero esta rotacion se habia quedado con la
               formula vieja (de antes del fix del espejo), por lo que
               el texto quedaba orientado incorrectamente aunque su
               posicion ya fuera correcta -de ahi que se vieran
               "torcidos". Con el signo Z invertido en la posicion, la
               orientacion que corresponde tambien invierte signo: en
               vez de -(angulo)+90, es +(angulo)+90. */
            glRotatef((angulo_inicio + paso_angular / 2.0f) + 90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            /* BUGFIX: los numeros se veian "opacos"/apagados comparado
               con los del tablero, porque tablero_apuestas.c desactiva
               GL_LIGHTING antes de dibujar sus numeros (quedan en
               color plano y brillante), pero aqui nunca se desactivaba
               -el blanco del numero quedaba sombreado por el modelo de
               Phong de la escena (ambient/diffuse/specular segun el
               angulo a la luz), en vez de verse blanco puro. Las
               cuñas de color si se dejan CON iluminacion (para que se
               vean parte de la rueda, con su sombreado natural); solo
               el numero en si se saca de la ecuacion de luces. */
            glDisable(GL_LIGHTING);
            dibujar_numero_pista(numero);
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);
    if (!line_smooth_estaba_activo) glDisable(GL_LINE_SMOOTH);
    if (!blend_estaba_activo) glDisable(GL_BLEND);
}