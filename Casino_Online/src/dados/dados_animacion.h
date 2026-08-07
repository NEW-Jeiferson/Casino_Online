#ifndef DADOS_ANIMACION_H
#define DADOS_ANIMACION_H

typedef struct {
    float rotacion_x[2];
    float rotacion_y[2];
    float rotacion_z[2];
    float velocidad_x[2];
    float velocidad_y[2];
    float velocidad_z[2];
    float altura[2];
    float velocidad_rebote[2];
    
    int girando;
    int dado1_final;
    int dado2_final;
    
    int fase_asentamiento;
    float rot_snap_x[2];
    float rot_snap_y[2];
    float rot_snap_z[2];
    
    float tiempo_transcurrido;
} EstadoDados;

void inicializar_dados_animacion(EstadoDados* estado);
void actualizar_dados(EstadoDados* estado, float delta_tiempo);
void iniciar_tiro_dados(EstadoDados* estado, int dado1, int dado2);

#endif /* DADOS_ANIMACION_H */
