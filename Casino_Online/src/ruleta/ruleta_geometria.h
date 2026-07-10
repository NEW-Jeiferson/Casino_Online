/*
 * ruleta_geometria.h
 * -----------------------------------------------------------------------
 * Generacion de la geometria de la mesa y la rueda de ruleta a partir de
 * un perfil de Bezier revolucionado (superficie de revolucion).
 *
 * Jerarquia real con pila de matrices (aplicada por el llamador, ver
 * main.c -> display()):
 *
 *   mesa (raiz)
 *    |- rueda (hijo: hereda traslacion Y + rotacion angulo_rueda)
 *    |   |- bolita (nieto: hereda ademas la traslacion/rotacion propia)
 *    |- vidrio protector (hijo de mesa, hermano de rueda, sin rotacion)
 *
 * dibujar_rueda() y dibujar_bolita() YA NO gestionan su propia
 * traslacion/rotacion de nodo -- asumen que el llamador dejo la matriz
 * de ModelView correctamente posicionada antes de invocarlas (con
 * glPushMatrix/glTranslatef/glRotatef), y que la mantiene activa
 * mientras se dibuja a los hijos, antes de hacer el glPopMatrix
 * correspondiente. Esto es lo que logra la jerarquia real (mover/rotar
 * el nodo rueda arrastraria tambien a la bolita, si en el futuro se
 * anima la posicion de la rueda en el espacio).
 *
 * Responsable: Jeiferson
 * -----------------------------------------------------------------------
 */
#ifndef RULETA_GEOMETRIA_H
#define RULETA_GEOMETRIA_H

 /* Radio exterior de la rueda (borde). Se expone aqui porque
	ruleta_animacion.c lo necesita para saber a que distancia del centro
	debe orbitar la bolita sobre el borde de la rueda. */
#define RADIO_EXTERIOR_RUEDA 3.0f

	/* Medio lado de la mesa (mesa cuadrada de -RADIO_MESA a +RADIO_MESA en
	   X y Z, en Y=0). Se expone aqui porque el sistema de mouse picking
	   (mouse_picking.c) y el mapeo de posicion 3D a celda del tablero
	   (tablero_apuestas.c, responsabilidad de Luis) lo necesitan para sus
	   calculos, en vez de repetir el numero "magico" 6.0f en otro archivo. */
#define RADIO_MESA 6.0f

	   /* Genera los puntos del perfil de la rueda usando una curva de Bezier
		  cubica, y los guarda para usarse en la superficie de revolucion */
void generar_perfil_bezier_rueda(void);

/* Construye la malla 3D de la rueda (superficie de revolucion) a partir
   del perfil generado, calculando normales de vertice */
void construir_malla_rueda(void);

/* Dibuja la mesa (plano/paño con material fieltro). Nodo raiz de la
   jerarquia: se dibuja en el origen del mundo. */
void dibujar_mesa(void);

/* Dibuja la malla de la rueda ya construida. NO aplica traslacion ni
   rotacion propia: el llamador debe posicionar la matriz ModelView
   (traslacion Y + rotacion segun el angulo de giro) antes de llamar a
   esta funcion, y mantenerla activa si se van a dibujar hijos (como la
   bolita) que deban heredar esa transformacion. */
void dibujar_rueda(void);

/* Dibuja el vidrio protector semitransparente sobre la rueda (blending).
   Es hijo de la mesa pero hermano de la rueda: no hereda su rotacion,
   ya que es una tapa fija que no gira. */
void dibujar_vidrio_protector(void);

/* Dibuja la pista de 37 casillas coloreadas (rojo/negro/verde) con sus
   numeros, sobre la superficie de la rueda. Se llama DESPUES de
   dibujar_rueda() y mientras la misma matriz de la rueda sigue activa
   (para que la pista gire junto con la rueda). Usa ORDEN_RUEDA_EUROPEA
   y color_de_numero() de core/estado_juego.h, la misma fuente de
   verdad que usa main.c para decidir el numero ganador. */
void dibujar_pista_numerada(void);
float altura_superficie_en_radio(float radio);

/* Radios de la banda de la pista, expuestos aqui (antes solo internos
   a ruleta_geometria.c) para que ruleta_animacion.c pueda orbitar la
   bolita en el CENTRO de la banda en vez de adivinar un radio a mano
   -antes RADIO_ORBITA_BOLITA coincidia exactamente con el borde
   exterior de la pista (2.6 = 2.6), haciendo ambiguo visualmente en
   que casilla estaba realmente parada la bolita. */
#define RADIO_INTERNO_PISTA 1.6f
#define RADIO_EXTERNO_PISTA 2.6f

#endif /* RULETA_GEOMETRIA_H */