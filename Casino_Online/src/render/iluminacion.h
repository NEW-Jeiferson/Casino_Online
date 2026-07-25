/*
 * iluminacion.h
 * -----------------------------------------------------------------------
 * Configuracion del modelo de iluminacion de Phong: fuentes de luz,
 * componentes ambiental/difusa/especular.
 *
 * Responsable sugerido: Persona B
 * -----------------------------------------------------------------------
 */
#ifndef ILUMINACION_H
#define ILUMINACION_H

 /* Habilita GL_LIGHTING y configura la luz principal (point light) sobre
	la mesa, con sus componentes ambiental, difusa y especular */
void inicializar_iluminacion(void);

#endif /* ILUMINACION_H */