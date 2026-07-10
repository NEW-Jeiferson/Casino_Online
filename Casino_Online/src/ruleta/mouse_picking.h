/*
 * mouse_picking.h
 * -----------------------------------------------------------------------
 * Conversion de un clic de mouse (coordenadas 2D de pantalla) a un punto
 * 3D sobre el plano de la mesa (Y = 0), usando gluUnProject y la
 * interseccion de un rayo con un plano.
 *
 * Esta extension (manejo de mouse con glutMouseFunc + gluUnProject) NO
 * esta en el temario visto en clase -- es una ampliacion voluntaria del
 * scope del proyecto para acercarse a una experiencia de casino real
 * (apostar con clics sobre el tablero, en vez de solo teclado). Ver el
 * AR correspondiente en docs/ARs/ que documenta esta decision.
 *
 * Responsable: Jeiferson
 * -----------------------------------------------------------------------
 */
#ifndef MOUSE_PICKING_H
#define MOUSE_PICKING_H

/* Convierte la posicion de un clic de mouse (mouse_x, mouse_y, en
   coordenadas de ventana tal como las entrega GLUT) a un punto sobre
   el plano de la mesa (Y = 0) en coordenadas del mundo.
   Devuelve 1 y escribe el resultado en *out_x, *out_z si la interseccion
   es valida (el rayo de la camara efectivamente cruza el plano de la
   mesa hacia adelante). Devuelve 0 si no hay interseccion valida (por
   ejemplo, si se hizo clic apuntando hacia arriba del horizonte). */
int obtener_punto_clic_en_mesa(int mouse_x, int mouse_y, float* out_x, float* out_z);

#endif /* MOUSE_PICKING_H */
