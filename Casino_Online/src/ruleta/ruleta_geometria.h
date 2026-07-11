/*
* ruleta_geometria.h
* Geometria de la mesa y la rueda (superficie de revolucion sobre un
* perfil de Bezier). Responsable: Jeiferson.
*
* dibujar_rueda() y dibujar_bolita() no manejan su propia
* traslacion/rotacion: main.c posiciona la matriz antes de llamarlas
* y la mantiene activa mientras se dibujan los hijos (jerarquia
* mesa -> rueda -> bolita).
*/
#ifndef RULETA_GEOMETRIA_H
#define RULETA_GEOMETRIA_H

/* Radio exterior de la rueda; lo usa tambien ruleta_animacion.c para
   la orbita de la bolita. */
#define RADIO_EXTERIOR_RUEDA 3.0f

   /* Medio lado de la mesa cuadrada; lo usan mouse_picking.c y
	  tablero_apuestas.c. */
#define RADIO_MESA 6.0f

void generar_perfil_bezier_rueda(void);
void construir_malla_rueda(void);
void dibujar_mesa(void);
void dibujar_rueda(void);
void dibujar_vidrio_protector(void);

/* Pista de 37 casillas con color y numero, sobre la superficie de la
   rueda. Se llama despues de dibujar_rueda(), con su misma matriz
   activa, para que gire junto con ella. */
void dibujar_pista_numerada(void);

/* Emblema decorativo (anillo + aspas) en el domo central. Se llama
   junto con dibujar_pista_numerada(). */
void dibujar_emblema_central(void);

/* Altura real de la superficie de la rueda en un radio dado,
   interpolada del perfil Bezier. La usan la pista y la bolita para
   apoyarse en la curva real en vez de una altura fija. */
float altura_superficie_en_radio(float radio);

#endif /* RULETA_GEOMETRIA_H */