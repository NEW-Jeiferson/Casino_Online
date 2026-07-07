# Casino Online (MVP Ruleta) — Guía de configuración en Visual Studio

Este documento explica paso a paso cómo crear el proyecto en Visual Studio,
enlazar GLUT, e integrar la estructura de carpetas ya armada.

> **Nota de equipo sobre la librería usada:** el proyecto usa la librería
> **GLUT clásica** (paquete `glutdlls37beta`, de Nate Robins), **no
> freeGLUT**. Se decidió así porque el equipo ya tenía este paquete
> descargado y funcionando. Diferencias clave a tener en cuenta:
> - Es una librería de **32 bits únicamente** → la plataforma del
>   proyecto en Visual Studio debe estar siempre en **x86**, nunca x64.
> - Los archivos se llaman `glut32.lib` y `glut32.dll` (con "32" en el
>   nombre), no `freeglut.lib`/`freeglut.dll`.
> - Es una librería sin mantenimiento desde hace años. Las funciones
>   básicas de GLUT (`glutInit`, `glutDisplayFunc`, `glutMainLoop`,
>   `glutIdleFunc`, `glutKeyboardFunc`, etc.) funcionan igual que en
>   freeGLUT. Si en algún momento el proyecto necesita `MSAA`
>   (`GLUT_MULTISAMPLE`), probarlo cuanto antes: es el punto donde esta
>   librería es menos confiable y podría requerir documentar una
>   alternativa en una AR.

## 0. Estructura de carpetas de este paquete

```
Casino_Online/
├── docs/                     <- documentación (análisis, ARs, justificación)
│   ├── ARs/
│   ├── analisis-previo.md
│   ├── justificacion-tema.md
│   ├── descripcion-proyecto.md
│   └── trazabilidad-temas.md
├── include/                  <- ya no se usa (GLUT se instala global, ver paso 2)
├── lib/                      <- ya no se usa (GLUT se instala global, ver paso 2)
└── src/
    ├── main.c
    ├── core/                 <- Persona C: máquina de estados, jugador
    │   ├── estado_juego.h/.c
    │   └── jugador.h/.c
    ├── ruleta/                <- Persona A: geometría y animación
    │   ├── ruleta_geometria.h/.c
    │   └── ruleta_animacion.h/.c
    ├── render/                <- Persona B: luces y materiales
    │   ├── iluminacion.h/.c
    │   └── materiales.h/.c
    ├── ui/                    <- Persona C: HUD y pantallas
    │   ├── hud.h/.c
    │   └── pantallas.h/.c
    └── utils/                 <- compartido: Bézier y Bresenham
        ├── bezier.h/.c
        └── bresenham.h/.c
```

> Las carpetas `include/` y `lib/` quedaron del diseño original del
> proyecto (pensado para tener GLUT local al proyecto), pero el equipo
> decidió usar la instalación global de GLUT en Visual Studio en su
> lugar (ver paso 2), así que estas dos carpetas se pueden dejar vacías
> o eliminar sin afectar la compilación.

Cada archivo `.c`/`.h` ya trae comentarios con el responsable sugerido y
bloques `TODO` marcando exactamente qué falta implementar — esto también
les sirve como base para la documentación de código que pide el profesor
(cada bloque ya explica su propósito).

## 1. Descargar GLUT para Windows

1. Descarga el paquete `glutdlls37beta.zip` (GLUT clásico para Win32,
   versión 3.7 beta, de Nate Robins).
2. Descomprime el zip. Adentro vas a encontrar 5 archivos sueltos (sin
   subcarpetas): `glut.h`, `glut32.lib`, `glut32.dll`, `glut.lib`,
   `glut.dll`.
3. Solo se usan los que tienen **"32" en el nombre** (`glut32.lib`,
   `glut32.dll`) junto con `glut.h`. Los archivos `glut.lib` y `glut.dll`
   (sin el "32") son redundantes y no se usan.

## 2. Copiar los archivos de GLUT a Visual Studio (instalación global)

> **Nota de equipo:** se eligió esta opción (instalación global en Visual
> Studio) en vez de tenerla local al proyecto. La ventaja es que, una vez
> configurado, cualquier proyecto en esa misma computadora encuentra GLUT
> automáticamente, sin tener que tocar rutas en Propiedades. La
> desventaja es que **cada integrante del equipo debe repetir estos 3
> pasos en su propia computadora**, ya que estos archivos no viajan
> junto con el proyecto ni con el repositorio del código. Si alguien
> clona el proyecto en una PC nueva y le marca error de "no se encuentra
> glut.h" o "falta glut32.dll", lo primero que hay que revisar es si ya
> hizo este paso en esa máquina.

**2.1 Copiar `glut.h`**
1. Localiza la carpeta de includes de Visual Studio, normalmente en una
   ruta parecida a:
   `C:\Program Files\Microsoft Visual Studio\<version>\Community\VC\Auxiliary\VS\include\`
   (la versión y edición —Community, Professional, etc.— pueden variar
   según la instalación de cada quien).
2. Dentro de esa carpeta, crea (si no existe) una carpeta `GL` y copia
   `glut.h` adentro, quedando como:
   `...\VC\Auxiliary\VS\include\GL\glut.h`

**2.2 Copiar `glut32.lib`**
1. Localiza la carpeta de librerías de Visual Studio para **x86**. Para
   confirmarla exactamente en tu instalación: Herramientas → Opciones →
   Proyectos y soluciones → Directorios de VC++ → dropdown "Directorios
   de biblioteca". Normalmente algo como:
   `C:\Program Files\Microsoft Visual Studio\<version>\Community\VC\Auxiliary\VS\lib\x86\`
2. Copia `glut32.lib` dentro de esa carpeta.

**2.3 Copiar `glut32.dll`**
1. Copia `glut32.dll` a `C:\Windows\SysWOW64\` (carpeta de DLLs de 32
   bits en sistemas Windows de 64 bits). Vas a necesitar permisos de
   administrador para pegar ahí.
2. **Recomendado además:** copia también `glut32.dll` directamente en la
   carpeta de salida del proyecto (`Casino_Online/Debug/`, después de
   compilar al menos una vez) — así el proyecto queda autocontenido y no
   depende únicamente de la copia en `SysWOW64` si comparten el proyecto
   o lo suben a un repositorio.

## 3. Crear el proyecto en Visual Studio

1. Abre Visual Studio → **Crear un proyecto nuevo**.
2. Busca la plantilla **"Proyecto vacío"** (Empty Project) de C++ — sí, aunque
   el código sea C, se usa la plantilla de C++ y luego Visual Studio
   compila como C automáticamente por la extensión del archivo (`.c`).
3. Como ubicación, elige la carpeta que contiene tu `Casino_Online/` (o
   crea el proyecto directamente ahí), y dale un nombre a la solución,
   por ejemplo `Casino_Online`.

## 4. Agregar los archivos existentes al proyecto

1. Crea los filtros (carpetas virtuales) en el Explorador de soluciones
   que reflejen la estructura física: clic derecho sobre el proyecto →
   **Agregar → Nuevo filtro** → nómbralo `core`. Repite para `ruleta`,
   `render`, `ui`, `utils`.
2. Clic derecho sobre cada filtro → **Agregar → Elemento existente...**,
   navega hasta la carpeta física correspondiente (`src/core/`,
   `src/ruleta/`, etc.) y selecciona todos los `.h`/`.c` de esa carpeta
   a la vez (Ctrl+clic).
3. `main.c` se agrega directo sobre el proyecto (no dentro de ningún
   filtro): clic derecho sobre `Casino_Online` → **Agregar → Elemento
   existente...**

> Recuerda: los filtros son carpetas *virtuales* en el Explorador de
> soluciones, no mueven archivos físicos. Si usas "Nuevo elemento" en
> vez de "Elemento existente", revisa siempre el campo "Ubicación" en el
> diálogo antes de crear el archivo, para que caiga en la carpeta física
> correcta y no en la raíz del proyecto.

## 5. Confirmar que la plataforma esté en x86

Como GLUT clásico es de 32 bits, el proyecto **debe compilarse en x86**,
nunca en x64.

1. En la barra superior de Visual Studio, revisa el dropdown de
   plataforma (al lado de "Debug").
2. Debe decir **x86**. Si dice x64 o "Cualquier CPU", cámbialo a x86.
3. Si no aparece esa opción, ve a **Compilación → Administrador de
   configuración** y en "Plataforma de la solución activa" selecciona
   x86, o créala con `<Nuevo...>` si no existe.

## 6. Configurar las dependencias del vinculador

1. Clic derecho sobre el proyecto → **Propiedades**.
2. Asegúrate de que la configuración arriba diga **"Todas las
   configuraciones"** y la plataforma **x86**.
3. Ve a **Propiedades de configuración → Vinculador → Entrada →
   Dependencias adicionales**, agrega al inicio de la lista:
   `glut32.lib;opengl32.lib;glu32.lib;`

> Si al compilar te marca "no se encuentra glut.h", revisa el paso 2.1.
> Si marca "unresolved external symbol", revisa el paso 2.2 y que este
> paso 6 esté bien escrito (sin errores de tipeo). Si marca algo sobre
> "machine type conflict", revisa el paso 5 (plataforma x86).

## 7. Configurar el subsistema (evitar el error de "WinMain")

Como el programa usa `int main()` y no `WinMain`, hay que decirle al
vinculador que use el subsistema de consola:

1. **Propiedades de configuración → Vinculador → Sistema → Subsistema** →
   selecciona **Consola (/SUBSYSTEM:CONSOLE)**.

## 8. Compilar y ejecutar

1. Presiona **Ctrl+Shift+B** para compilar.
2. Presiona **F5** o **Ctrl+F5** para ejecutar. Debería abrir una ventana
   negra vacía (todavía no hay geometría dibujada, solo el ciclo de vida
   básico) — eso confirma que GLUT está correctamente enlazado.
3. Si al ejecutar Windows dice que falta `glut32.dll`, revisa el paso
   2.3 (que esté copiado en `SysWOW64` y/o en la carpeta `Debug/` del
   proyecto).

## 9. Siguientes pasos de desarrollo

Con el esqueleto compilando, cada integrante puede trabajar sobre su
carpeta asignada sin pisar el código de los demás:

- **Persona A** → `src/ruleta/` (geometría de la rueda con Bézier,
  animación de la bolita)
- **Persona B** → `src/render/` (iluminación, materiales)
- **Persona C** → `src/core/` y `src/ui/` (máquina de estados, HUD,
  pantallas de préstamo/game over)

Los `TODO` dentro de cada archivo marcan exactamente qué falta por
implementar en cada bloque.

Recuerda: los pasos 2.1, 2.2 y 2.3 (instalación global de GLUT) deben
repetirse en la computadora de **cada integrante del equipo**, ya que no
viajan con el proyecto.
