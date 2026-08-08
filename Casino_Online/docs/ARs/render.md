# Documentación del Módulo Render (Casino Online)

## 1. Resumen del módulo
El módulo `render` forma la base del sistema visual en 3D para este proyecto de casino. Escrito en estándar C89 y construido sobre el *fixed-function pipeline* de OpenGL (Legacy 1.x / 2.x), este módulo orquesta de forma desacoplada la iluminación de la escena, la aplicación de diferentes materiales fotorrealistas y la gestión/dibujado de texturas (como fondos 2D). Este diseño provee un nivel aceptable de realismo visual sin los requisitos de hardware y programación requeridos por los *shaders* modernos.

## 2. Estructuras de datos clave
* **`TipoMaterial`**: Una enumeración (`enum`) declarada en `materiales.h` que contiene identificadores explícitos para las diversas superficies presentes en los juegos del casino: `MATERIAL_MADERA`, `MATERIAL_METAL`, `MATERIAL_FIELTRO`, `MATERIAL_VIDRIO`, y `MATERIAL_MADERA_OSCURA`.
* **Variables estáticas (Handles) y Fallbacks (`textura.c`)**: 
  * `g_textura_fondo`: Guarda el identificador OpenGL de textura entera o `0` si no ha sido cargada. 
  * `RUTAS_FONDO`: Arreglo estático de cadenas de texto usado para intentar resolver las rutas de archivos de textura dependientes del directorio de trabajo actual (útil para solucionar discrepancias entre ejecución desde el IDE vs Ejecutable compilado).

## 3. Funciones públicas

### `void inicializar_iluminacion(void)`
* **¿Para qué sirve?**: Configura e inicializa el modelo de iluminación de Phong iluminando la escena global, posicionando focos de luz como la luz cenital principal (`LIGHT0`) y una de relleno (`LIGHT1`).
* **¿Por qué se usa este enfoque?**: Emplea el modelo de iluminación basado en vértices del pipeline fijo de OpenGL. Al definir las luces globalmente usando estados fijos de OpenGL, se evitan los shaders programables y se garantiza la retrocompatibilidad propia de proyectos desarrollados en C89 para sistemas heredados.
* **¿Cómo lo hace internamente?**: Activa el estado `GL_LIGHTING` y normalización (`GL_NORMALIZE`). Posteriormente, llama a funciones como `glLightfv` para asignar los componentes (ambiental, difusa, especular, y vector de posición) de las macros `GL_LIGHT0` y `GL_LIGHT1`.

### `void aplicar_material(TipoMaterial tipo)`
* **¿Para qué sirve?**: Prepara la máquina de estado gráfica de OpenGL para que la geometría dibujada a continuación reaccione correctamente ante la luz simulando el material solicitado (metal cromado, madera, fieltro de la mesa, etc.).
* **¿Por qué se usa este enfoque?**: En un entorno de *pipeline* fijo, los colores y reflexiones de la malla son producto directo de pasar los parámetros al renderizador en tiempo real a través de directivas de estado. Esto se hace en vez de enviar _uniform variables_ en fragment shaders.
* **¿Cómo lo hace internamente?**: Contiene una estructura `switch (tipo)` que asigna arreglos de 4 componentes de punto flotante para variables (Ambient, Diffuse, Specular) e intensidad de brillo (Shininess). Finalmente, inyecta estos parámetros mediante sucesivas llamadas a `glMaterialfv` y `glMaterialf` afectando las caras frontales (`GL_FRONT`).

### `GLuint cargar_textura_gl(const char* ruta)`
* **¿Para qué sirve?**: Realiza la carga en memoria de un archivo de imagen en crudo y lo sube como textura a la GPU, devolviendo el ID.
* **¿Por qué se usa este enfoque?**: El manejo directo usando `stb_image` provee un flujo nativo ligero sin la complejidad de vincular librerías masivas como *libpng*. Facilita mantener un sistema estrictamente en C89 y controlable a nivel de manejo de memoria.
* **¿Cómo lo hace internamente?**: Utiliza `stbi_load` para extraer los píxeles (intentando primero con la ruta y luego con un prefijo `../`). Luego invoca `glGenTextures` para crear un "handle" local, acopla los filtros `GL_TEXTURE_MIN_FILTER`, etc., inyecta la memoria VRAM usando `glTexImage2D` (con soporte para 3 o 4 canales de color), y finalmente libera la memoria RAM al llamar a `stbi_image_free`.

### `int cargar_textura_fondo_tragamonedas(void)`
* **¿Para qué sirve?**: Carga específicamente la textura necesaria para dibujar el ambiente/fondo detrás de la máquina tragamonedas de manera resiliente.
* **¿Por qué se usa este enfoque?**: Como en C89 bajo MSVC el comportamiento del directorio actual en modo _debug_ varía enormemente según si se corre el .exe o desde Visual Studio, se encapsula en una búsqueda heurística.
* **¿Cómo lo hace internamente?**: Repite el proceso iterando una lista de _fallbacks_ de direcciones relativas (`RUTAS_FONDO`). Fuerza que `stb_image` devuelva imágenes boca arriba usando `stbi_set_flip_vertically_on_load(1)` ya que las coordenadas cartesianas de OpenGL empiezan desde la esquina inferior. Mantiene esta textura estáticamente en `g_textura_fondo`.

### `void dibujar_fondo_tragamonedas(int ancho, int alto)`
* **¿Para qué sirve?**: Pinta de fondo la textura de la tragamonedas (o un gradiente en caso de fallo de textura) para que actúe de papel tapiz detrás de la escena en 3D.
* **¿Por qué se usa este enfoque?**: Dibujar un ambiente en 2D dentro de una escena tridimensional heredada requiere deshabilitar temporeramente pruebas de profundidad y perspectivas calculadas para sobreescribir la capa de la pantalla de forma estática en modo 2D ortográfico.
* **¿Cómo lo hace internamente?**: Apila los estados de transformación (`glPushMatrix`). Cambia el estado hacia la matriz `GL_PROJECTION` para generar un render ortográfico plano (`glOrtho`) coincidente con el ancho y alto del *viewport*. Desactiva temporeramente `GL_LIGHTING` y `GL_DEPTH_TEST`, enciende la textura (`GL_TEXTURE_2D`), pinta un rectángulo mapeando los cuadrantes en sus bordes usando `GL_QUADS`, opcionalmente pinta encima un rectángulo oscuro semi-transparente como overlay con `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`, y después regresa a las matrices de perspectiva previas (`glPopMatrix`).

### `void liberar_textura_fondo_tragamonedas(void)`
* **¿Para qué sirve?**: Libera limpiamente la memoria VRAM retenida por OpenGL correspondiente al fondo.
* **¿Por qué se usa este enfoque?**: Práctica estricta y tradicional en manual de C para evitar fugas de memoria al momento de recargar o de apagar los procesos o escenarios.
* **¿Cómo lo hace internamente?**: Si el puntero estático (`g_textura_fondo`) es diferente de 0, llama a `glDeleteTextures` para destruir el recurso dentro del contexto OpenGL.

## 4. Dependencias
* **`GL/glut.h` (y derivados nativos de OpenGL):** Proveen acceso a los contextos de visualización, creación de ventanas, primitivas 3D, fixed-function pipeline, materiales e luces globales.
* **`stb_image.h`:** Librería modular e independiente *header-only* usada específicamente en `textura.c` para decodificar con alta compatibilidad archivos PNG/JPG, sin engordar los requerimientos de compilación del proyecto.
* **`<stdio.h>`:** Se emplea para el reporteo básico en consola ante casos de *warnings* (por ejemplo, al mostrar alertas o fallos si los directorios del *asset* del tragamonedas cambian).
