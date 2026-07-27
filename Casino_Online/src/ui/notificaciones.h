/*
 * notificaciones.h
 * Sistema de notificaciones flotantes transitorias (non-blocking toast notifications).
 */
#ifndef NOTIFICACIONES_H
#define NOTIFICACIONES_H

#include <GL/glut.h>

#define MAX_NOTIFICACIONES 3

/* Inicializa el subsistema de notificaciones flotantes */
void inicializar_notificaciones(void);

/* Agrega una notificacion flotante a la cola con su mensaje ASCII y color RGB */
void agregar_notificacion(const char* texto, float r, float g, float b);

/* Renderiza las notificaciones flotantes activas en la esquina superior derecha */
void dibujar_notificaciones(void);

#endif /* NOTIFICACIONES_H */
