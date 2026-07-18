/*
* Animacion de la bolita: giro y desaceleracion sobre la rueda, usando
* una curva de Bezier como funcion de easing para el frenado.
*/
#ifndef RULETA_ANIMACION_H
#define RULETA_ANIMACION_H


/* Constantes de configuracion de la animacion de la bolita */
#define VELOCIDAD_RUEDA_DURANTE_GIRO 162.0f


/* Constantes de configuracion de la animacion de la bolita */
typedef struct {
    float angulo_actual;      
    float velocidad;          
    float velocidad_inicial;  
    float duracion_total;     
    int   girando;            
    float tiempo_transcurrido;
} EstadoBolita;

/* Inicializa el estado de la bolita (detenida, angulo 0) */
void inicializar_bolita(EstadoBolita* bolita);


/* Inicia el giro de la bolita hacia un angulo absoluto deseado, considerando vueltas extra */
void iniciar_giro_bolita_hacia_absoluto(EstadoBolita* bolita, float delta_absoluto_deseado, int vueltas_extra);


/* Actualiza la posicion de la bolita en su orbita, considerando el tiempo transcurrido */
void actualizar_bolita(EstadoBolita* bolita, float delta_tiempo);


/* Dibuja la bolita en su posicion actual sobre la rueda */
void dibujar_bolita(const EstadoBolita* bolita);

#endif