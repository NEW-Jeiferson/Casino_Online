/*
* Generacion de la geometria de la mesa y la rueda de ruleta a partir de
* un perfil de Bezier revolucionado (superficie de revolucion).
*/
#ifndef RULETA_GEOMETRIA_H
#define RULETA_GEOMETRIA_H


/* Define el radio exterior de la rueda (lado del cuadrado) */
#define RADIO_EXTERIOR_RUEDA 3.0f


/* Define el radio de la mesa (lado del cuadrado) */
#define RADIO_MESA 6.0f


/* Sirve para generar el perfil de Bezier de la rueda, esto para poder crear la superficie de revolucion */
void generar_perfil_bezier_rueda(void);


/* Sirve para construir la malla 3D de la rueda a partir del perfil de Bezier */
void construir_malla_rueda(void);


/* Sirve para construir la malla 3D de la mesa a partir del perfil de Bezier */
void dibujar_mesa(void);


/* Dibuja la rueda de la ruleta, con su malla 3D y su textura. */
void dibujar_rueda(void);


/* Dibuja el vidrio protector de la ruleta, con su malla 3D y su textura. */
void dibujar_vidrio_protector(void);


/* Dibuja la pista numerada de la ruleta, con su malla 3D y su textura. */
void dibujar_pista_numerada(void);
float altura_superficie_en_radio(float radio);


/* Define el radio interior y exterior de la pista numerada de la ruleta */
#define RADIO_INTERNO_PISTA 1.6f
#define RADIO_EXTERNO_PISTA 2.6f

#endif