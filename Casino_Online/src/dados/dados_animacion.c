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
    estado->fase_asentamiento = 0;
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
    float t, rx, ry, rz;
    int i, vueltas;

    if (!estado->girando) return;

    estado->tiempo_transcurrido += delta_tiempo;
    t = estado->tiempo_transcurrido / TIEMPO_TIRO;

    if (t >= 1.0f) {
        t = 1.0f;
        estado->girando = 0;
        
        calcular_angulos_finales(estado->dado1_final, &rx, &ry, &rz);
        estado->rotacion_x[0] = rx; estado->rotacion_y[0] = ry; estado->rotacion_z[0] = rz;
        estado->altura[0] = 0.5f;

        calcular_angulos_finales(estado->dado2_final, &rx, &ry, &rz);
        estado->rotacion_x[1] = rx; estado->rotacion_y[1] = ry; estado->rotacion_z[1] = rz;
        estado->altura[1] = 0.5f;
        return;
    }

    if (t < 0.7f) {
        /* --- FASE 1: GIRO LIBRE Y CAOTICO (Primer 70% del tiempo) --- */
        float t_libre = t / 0.7f;
        float factor = 1.0f - (t_libre * t_libre * 0.5f); /* Decaimiento suave sin detenerse del todo */
        
        for (i = 0; i < 2; i++) {
            estado->rotacion_x[i] += estado->velocidad_x[i] * factor * delta_tiempo;
            estado->rotacion_y[i] += estado->velocidad_y[i] * factor * delta_tiempo;
            estado->rotacion_z[i] += estado->velocidad_z[i] * factor * delta_tiempo;
            estado->altura[i] = 0.5f + (float)fabs(sin(estado->tiempo_transcurrido * (10.0f + i * 2.0f))) * 2.0f * factor;
        }
    } else {
        /* --- FASE 2: ASENTAMIENTO AL OBJETIVO (Ultimo 30% del tiempo) --- */
        float t_asentamiento = (t - 0.7f) / 0.3f;
        float ease_out = 1.0f - (1.0f - t_asentamiento) * (1.0f - t_asentamiento); /* Ease-Out Cuadratico */
        
        if (!estado->fase_asentamiento) {
            estado->fase_asentamiento = 1;
            for (i = 0; i < 2; i++) {
                estado->rot_snap_x[i] = estado->rotacion_x[i];
                estado->rot_snap_y[i] = estado->rotacion_y[i];
                estado->rot_snap_z[i] = estado->rotacion_z[i];
            }
        }
        
        for (i = 0; i < 2; i++) {
            int dado_final = (i == 0) ? estado->dado1_final : estado->dado2_final;
            calcular_angulos_finales(dado_final, &rx, &ry, &rz);
            
            /* Ajustar meta para tomar el camino mas corto basandonos en las vueltas acumuladas */
            vueltas = (int)(estado->rot_snap_x[i] / 360.0f); rx += vueltas * 360.0f;
            if (rx - estado->rot_snap_x[i] < -180.0f) rx += 360.0f;
            else if (rx - estado->rot_snap_x[i] > 180.0f) rx -= 360.0f;
            
            vueltas = (int)(estado->rot_snap_y[i] / 360.0f); ry += vueltas * 360.0f;
            if (ry - estado->rot_snap_y[i] < -180.0f) ry += 360.0f;
            else if (ry - estado->rot_snap_y[i] > 180.0f) ry -= 360.0f;
            
            vueltas = (int)(estado->rot_snap_z[i] / 360.0f); rz += vueltas * 360.0f;
            if (rz - estado->rot_snap_z[i] < -180.0f) rz += 360.0f;
            else if (rz - estado->rot_snap_z[i] > 180.0f) rz -= 360.0f;
            
            /* Interpolacion hacia el objetivo calculado */
            estado->rotacion_x[i] = estado->rot_snap_x[i] + (rx - estado->rot_snap_x[i]) * ease_out;
            estado->rotacion_y[i] = estado->rot_snap_y[i] + (ry - estado->rot_snap_y[i]) * ease_out;
            estado->rotacion_z[i] = estado->rot_snap_z[i] + (rz - estado->rot_snap_z[i]) * ease_out;
            
            /* Disminucion controlada de los ultimos botes */
            estado->altura[i] = 0.5f + (estado->altura[i] - 0.5f) * (1.0f - ease_out);
        }
    }
}
