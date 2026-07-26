/*
 * materiales.h
 * -----------------------------------------------------------------------
 * Definicion de los materiales usados en la escena (madera, metal,
 * fieltro, vidrio), cada uno con sus componentes ambient/diffuse/
 * specular/shininess para el modelo de Phong.
 *
 * Responsable sugerido: Persona B
 * -----------------------------------------------------------------------
 */
#ifndef MATERIALES_H
#define MATERIALES_H

typedef enum {
    MATERIAL_MADERA,
    MATERIAL_METAL,
    MATERIAL_FIELTRO,
    MATERIAL_VIDRIO,
    MATERIAL_MADERA_OSCURA
} TipoMaterial;

/* Aplica los parametros glMaterialfv correspondientes al tipo de
   material indicado, antes de dibujar la geometria asociada */
void aplicar_material(TipoMaterial tipo);

#endif /* MATERIALES_H */
