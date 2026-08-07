#include "dados_animacion.h"
#include "../utils/bezier.h"
#include <math.h>

#define TIEMPO_TIRO 4.0f

void inicializar_dados_animacion(EstadoDados* estado) {
    estado->girando = 0;
    estado->tiempo_transcurrido = 0.0f;
    estado->rotacion_x[0] = 0.0f;
    estado->rotacion_y[0] = 0.0f;
    estado->rotacion_z[0] = 0.0f;
    estado->rotacion_x[1] = 0.0f;
    estado->rotacion_y[1] = 0.0f;
    estado->rotacion_z[1] = 0.0f;
    estado->altura[0] = 0.5f;
    estado->altura[1] = 0.5f;
}

static void calcular_angulos_finales(int valor, float* rx, float* ry, float* rz) {
    /* La camara esta arriba (+Y).
       Caras locales: 1(+Z), 6(-Z), 2(+X), 5(-X), 3(+Y), 4(-Y).
       Para asegurar que la cara correcta quede hacia arriba (+Y):
       Usamos rotaciones simples sobre un solo eje cuando sea posible.
       Nota: OpenGL multiplica (Rx * Ry * Rz) * V
    */
    switch(valor) {
        case 1: *rx = -90.0f; *ry = 0.0f; *rz = 0.0f; break;  /* +Z a +Y */
        case 6: *rx = 90.0f;  *ry = 0.0f; *rz = 0.0f; break;  /* -Z a +Y */
        case 2: *rx = 0.0f;   *ry = 0.0f; *rz = 90.0f; break; /* +X a +Y */
        case 5: *rx = 0.0f;   *ry = 0.0f; *rz = -90.0f; break;/* -X a +Y */
        case 3: *rx = 0.0f;   *ry = 0.0f; *rz = 0.0f; break;  /* +Y a +Y */
        case 4: *rx = 180.0f; *ry = 0.0f; *rz = 0.0f; break;  /* -Y a +Y */
        default: *rx = 0.0f;  *ry = 0.0f; *rz = 0.0f; break;
    }
}

void iniciar_tiro_dados(EstadoDados* estado, int dado1, int dado2) {
    estado->girando = 1;
    estado->tiempo_transcurrido = 0.0f;
    estado->dado1_final = dado1;
    estado->dado2_final = dado2;

    /* Damos una velocidad inicial */
    estado->velocidad_x[0] = 360.0f;
    estado->velocidad_y[0] = 540.0f;
    estado->velocidad_x[1] = 450.0f;
    estado->velocidad_y[1] = 270.0f;
    
    estado->velocidad_z[0] = 180.0f;
    estado->velocidad_z[1] = 240.0f;
}

void actualizar_dados(EstadoDados* estado, float delta_tiempo) {
    float t, rx, ry, rz, factor;
    int i;
    Punto3D p0 = {0.0f, 1.0f, 0.0f};
    Punto3D p1 = {0.0f, 0.85f, 0.0f};
    Punto3D p2 = {0.0f, 0.25f, 0.0f};
    Punto3D p3 = {0.0f, 0.0f, 0.0f};
    Punto3D easing;

    if (!estado->girando) return;

    estado->tiempo_transcurrido += delta_tiempo;
    t = estado->tiempo_transcurrido / TIEMPO_TIRO;
    if (t >= 1.0f) {
        t = 1.0f;
        estado->girando = 0;
        
        calcular_angulos_finales(estado->dado1_final, &rx, &ry, &rz);
        estado->rotacion_x[0] = rx;
        estado->rotacion_y[0] = ry;
        estado->rotacion_z[0] = rz;
        estado->altura[0] = 0.5f;

        calcular_angulos_finales(estado->dado2_final, &rx, &ry, &rz);
        estado->rotacion_x[1] = rx;
        estado->rotacion_y[1] = ry;
        estado->rotacion_z[1] = rz;
        estado->altura[1] = 0.5f;
        return;
    }

    easing = evaluar_bezier_cubica(p0, p1, p2, p3, t);
    factor = easing.y;
    if (factor < 0.0f) factor = 0.0f;

    /* Aplicamos factor a la rotacion */
    for (i = 0; i < 2; i++) {
        estado->rotacion_x[i] += estado->velocidad_x[i] * factor * delta_tiempo;
        estado->rotacion_y[i] += estado->velocidad_y[i] * factor * delta_tiempo;
        estado->rotacion_z[i] += estado->velocidad_z[i] * factor * delta_tiempo;
        
        /* Simular rebote simple con sin() */
        estado->altura[i] = 0.5f + (float)fabs(sin(estado->tiempo_transcurrido * (10.0f + i * 2.0f))) * 2.0f * factor;
    }
}
