# Registro de Decisión de Arquitectura: Carga de Texturas con stb_image.h

* **Estado**: Aceptado
* **Contexto**: Para mejorar la presentación visual del proyecto (pantalla de carga y escena principal de la ruleta), necesitamos renderizar imágenes 2D como fondos de pantalla. OpenGL clásico requiere decodificar y extraer los píxeles de formatos comprimidos como PNG o JPG antes de subirlos a la GPU con `glTexImage2D`. El uso de librerías grandes como FreeImage o libpng introduce una complejidad excesiva en la distribución y configuración del compilador.
* **Decisión**: Usar la librería de un solo archivo de cabecera `stb_image.h` (de Sean Barrett, en el dominio público).
  - Copiamos `stb_image.h` directamente en la carpeta de código fuente `src/`.
  - Definimos `#define STB_IMAGE_IMPLEMENTATION` en un solo archivo C (`main.c`) para compilar su implementación una única vez.
  - Subimos las imágenes cargadas como texturas 2D bidimensionales lineales estándares de OpenGL.
  - Para renderizar el fondo de pantalla completa, entramos temporalmente en proyección ortográfica (`glOrtho`), dibujamos un Quad plano en el plano frontal deshabilitando la iluminación y el test de profundidad (`GL_DEPTH_TEST`), y luego limpiamos el búfer de profundidad con `glClear(GL_DEPTH_BUFFER_BIT)` para restaurar el z-buffer y que los objetos 3D reales se pinten encima.
* **Consecuencias**:
  - **Ventajas**: Extrema facilidad de compilación y portabilidad. Cero configuraciones de enlazador (`.lib`) o copias de DLLs adicionales. Soporta PNG, JPG y BMP nativamente de forma muy ligera.
  - **Desventajas**: No proporciona optimizaciones complejas de compresión de texturas en GPU, lo cual es innecesario para los propósitos académicos y de rendimiento de este juego.
