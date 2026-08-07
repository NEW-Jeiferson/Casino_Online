# Registro de Decisión de Arquitectura: Reproducción de Audio con Windows MCI

* **Estado**: Aceptado
* **Contexto**: El proyecto "Casino Online" requiere música de fondo para intensificar la atmósfera del casino virtual. Se necesita soporte para reproducción continua (loop/repeat), detención, control básico y una distribución limpia que no requiera descargar librerías externas o añadir DLLs al ejecutable final (como SDL_mixer o FMOD).
* **Decisión**: Usar la API nativa de Windows Multimedia (cabecera `mmsystem.h` y librería `winmm.lib`) a través de la interfaz de control de medios (MCI) y la función `mciSendString`.
  - Vinculamos la librería de forma portable usando `#pragma comment(lib, "winmm.lib")` en el código fuente.
  - La música se carga y reproduce en bucle con el comando `"play musica repeat"`.
  - Para asegurar una liberación limpia del dispositivo al cerrar la ventana (ya que GLUT clásico no posee un callback confiable de cierre), registramos la función de apagado `atexit()`.
* **Consecuencias**:
  - **Ventajas**: Cero DLLs de terceros a distribuir. Funciona directamente en cualquier equipo Windows con el SDK nativo.
  - **Desventajas**: Específico de la plataforma Windows (aceptable ya que la materia exige el uso de GLUT clásico de 32 bits en Windows).
