# Limitaciones y Alcance del Proyecto: Análisis Técnico Riguroso

Este documento expone con detalle forense y rigor académico las decisiones arquitectónicas, las limitaciones impuestas y el alcance definido para el desarrollo del proyecto "Casino Online". Las determinaciones aquí documentadas se alinean estrechamente con las especificaciones de requisitos de software (SRS) regidas por el estándar IEEE-830, priorizando los objetivos pedagógicos sobre la viabilidad comercial o la modernización técnica.

## 1. Análisis de Conflictos de Arquitectura: Visual Studio y `glutdlls37beta`

Durante la fase de configuración del entorno de desarrollo integrado (IDE) Microsoft Visual Studio, se identificó un conflicto crítico de arquitectura (x86 vs. x64) asociado a la biblioteca gráfica `glutdlls37beta`. 

### 1.1 Naturaleza del Conflicto Linker (x64 vs x86)
La distribución `glutdlls37beta` contiene binarios precompilados (`glut32.lib` y `glut32.dll`) generados exclusivamente para arquitecturas de 32 bits (x86). Por defecto, las configuraciones recientes de Visual Studio (MSBuild) inicializan los proyectos bajo la plataforma `x64`. Cuando el compilador de Microsoft (MSVC) traduce el código fuente a archivos objeto de 64 bits y el vinculador (Linker) intenta enlazar estas dependencias con `glut32.lib` (32 bits), se produce una incompatibilidad en la Interfaz Binaria de Aplicaciones (ABI).

Esto resulta en errores fatales del tipo:
`LNK2019: unresolved external symbol __imp_glutInit referenced in function main`
`LNK1112: module machine type 'x86' conflicts with target machine type 'x64'`

### 1.2 Resolución Forense
Para solventar esta limitación técnica, fue imperativo modificar las propiedades del proyecto a nivel de solución:
1. **Target Platform:** Reconfigurar la plataforma activa a `Win32` (x86).
2. **Library Linkage:** Asegurar que el *Linker* estuviera apuntando correctamente al directorio de `glut32.lib`.
3. **Dynamic Linking:** Ubicar `glut32.dll` en el directorio de trabajo (`SysWOW64` o directamente en el directorio local de salida del ejecutable) para evitar el error `0xc000007b` (STATUS_INVALID_IMAGE_FORMAT) en tiempo de ejecución.

## 2. Restricciones del Estándar ANSI C (C89/C90) y su Impacto Paradigmático

El proyecto adoptó estrictamente el estándar ISO/IEC 9899:1990 (comúnmente conocido como C89 o ANSI C). Esta decisión introdujo limitaciones sintácticas rigurosas que moldearon profundamente la mentalidad de desarrollo y la estructuración arquitectónica del software.

### 2.1 Declaración Estricta de Variables
En C89, todas las variables deben ser declaradas exclusivamente al inicio de un bloque léxico, antes de cualquier sentencia ejecutable. Esto prohíbe la declaración de variables *inline* o dentro de la declaración de bucles `for`.

**Ejemplo de Código Moderno (C99/C++) - Inválido en C89:**
```c
// Bucle iterativo moderno
for (int i = 0; i < 10; i++) {
    int temp = i * 2;
    printf("%d\n", temp);
}
```

**Refactorización Obligatoria bajo C89:**
```c
/* Bucle iterativo bajo estándar ANSI C */
int i;
int temp; /* Obligatorio declarar en la cabecera del bloque */

for (i = 0; i < 10; i++) {
    temp = i * 2;
    printf("%d\n", temp);
}
```

### 2.2 Ausencia de Comentarios de Línea Única
El estándar C89 no reconoce la sintaxis `//` introducida en C++ y adoptada posteriormente en C99. Toda la documentación interna debió encapsularse en bloques `/* ... */`.

**Impacto en la Mentalidad del Desarrollador:**
Estas restricciones obligaron a adoptar un enfoque altamente disciplinado:
- **Gestión Cognitiva del Alcance:** Al forzar la declaración previa de variables, los desarrolladores se vieron obligados a planificar meticulosamente el uso de memoria local y el ciclo de vida de los datos antes de escribir la lógica.
- **Modularidad Extrema:** Para evitar la acumulación de docenas de variables al inicio de funciones complejas (el "síndrome de la función monolítica"), se promovió la refactorización continua en subrutinas más pequeñas, cohesivas y acopladas de manera laxa.

## 3. Justificación Técnica de Exclusiones en el Alcance (IEEE-830)

En conformidad con las especificaciones del documento IEEE-830, el alcance del proyecto se delimitó rigurosamente para maximizar el valor pedagógico en torno a los fundamentos de la computación gráfica. Como resultado, componentes estándar en la industria de los videojuegos modernos fueron excluidos deliberadamente.

### 3.1 Multijugador en Red
La implementación de una arquitectura cliente-servidor para partidas multijugador habría requerido la integración de bibliotecas de sockets (ej. Winsock), mecanismos de serialización de datos, y manejo de hilos (threading) para evitar el bloqueo del renderizado. 
**Justificación:** Estas tecnologías representan una desviación sustancial del objetivo primario. La complejidad inherente a la sincronización de estado, la latencia de red y la resolución de condiciones de carrera (race conditions) diluiría los esfuerzos orientados al dominio del Pipeline Fijo de OpenGL (transformaciones geométricas, proyecciones ortogonales/perspectivas, y la máquina de estados de OpenGL).

### 3.2 Motores de Física Externos
Integrar motores de física de terceros (como Box2D o Bullet Physics) habría simplificado enormemente el cálculo de colisiones y cinemática.
**Justificación:** El uso de bibliotecas externas actúa como una "caja negra" que oculta las matemáticas subyacentes. El objetivo académico dictó que toda detección de colisiones (por ejemplo, intersecciones AABB - Axis-Aligned Bounding Box o esferas) y cinemática debía ser programada manualmente. Esto garantizó que el equipo de desarrollo asimilara sólidamente conceptos de álgebra lineal, cálculo de vectores normales, y resolución espacial en sistemas de coordenadas 3D, consolidando la base matemática indispensable para cualquier ingeniero de gráficos por computadora.

### 3.3 Uso Exclusivo del Pipeline Fijo (Legacy OpenGL)
En lugar de utilizar OpenGL moderno basado en *shaders* programables (GLSL) e *Immediate Mode* obsoleto, se emplearon primitivas de pipeline fijo (`glBegin`, `glEnd`, `glMatrixMode`, `glPushMatrix`). Esto permitió una visualización directa e inmediata de los efectos matemáticos sobre el modelo, eliminando la abstracción que introducen las tuberías modernas basadas en memoria VRAM y búferes (VBOs/VAOs), ideal para una curva de aprendizaje inicial robusta.
