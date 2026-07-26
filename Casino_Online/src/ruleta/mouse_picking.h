/*
* Conversion de un clic de mouse (coordenadas 2D de pantalla) a un punto
* 3D sobre el plano de la mesa (Y = 0), usando gluUnProject y la
* interseccion de un rayo con un plano.
*/
#ifndef MOUSE_PICKING_H
#define MOUSE_PICKING_H


/* Obtiene el punto de interseccion del rayo que pasa por la camara y el punto del mouse en la pantalla con el plano de la mesa (Y = 0). */
int obtener_punto_clic_en_mesa(int mouse_x, int mouse_y, float* out_x, float* out_z);

#endif
