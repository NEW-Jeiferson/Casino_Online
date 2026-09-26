# 🎰 Casino Online — Simulador Serious Game (MVP)

Este documento consolida y reemplaza la información del `README.md` ubicado en la raíz del proyecto para reflejar los roles, responsabilidades definitivas del equipo y el estado actual de este entregable en la carpeta `docs/`.

## 📝 ¿Qué es el proyecto?

**Casino Online** es un simulador de casino en 3D desarrollado en C89 utilizando la API gráfica OpenGL y la librería GLUT. 

## 🎯 Objetivo


El objetivo de esta herramienta **no es glorificar el juego ni enseñar a apostar**, sino servir como un **Serious Game (juego serio) para la concientización sobre la ludopatía**. 
El jugador inicia con saldo virtual y experimenta el rápido y destructivo ciclo del juego problemático: la euforia de ganar, la pérdida rápida de fondos, el endeudamiento progresivo para intentar recuperar las pérdidas y, finalmente, la bancarrota (Game Over). Este progreso está intercalado con pantallas educativas, quizzes y mensajes reflexivos que explican de manera realista las verdaderas probabilidades matemáticas en contra del jugador.


## 👥 Integrantes y Módulos Asignados

El equipo de desarrollo se encargó de los siguientes módulos:

- **Jeiferson:** Orquestación general, máquina de estados de la partida, lógica y render del módulo Ruleta, e integración final del proyecto.
- **Dubenny:** Lógica y tabla de pagos del módulo Tragamonedas.
- **Luis:** Geometría y animación del módulo Tragamonedas, y esqueleto base del módulo de Dados.

## ⌨️ Controles del Teclado y Ratón

La interfaz está diseñada para que el usuario interactúe usando el teclado y el ratón. A continuación, el detalle de controles completos:

### Generales y Navegación
- `ESC`: Salir del programa instantáneamente.
- `ENTER`: Confirmar/Avanzar en advertencias, menús, o cerrar mensajes reflexivos y de educación.
- `S`: Terminar sesión actual o cerrar ventanas educativas para volver al juego principal.
- `N`: Regresar al menú principal (si estás en confirmación de juego).
- `I`: Abrir pantallas de educación desde los menús.
- `Espacio`: Avanzar páginas dentro del módulo de educación.
- `P`: Pedir un préstamo (cuando estás sin dinero y en la pantalla de préstamo).

### Apuestas Generales
- `1`: Seleccionar ficha de **$10**.
- `2`: Seleccionar ficha de **$25**.
- `3`: Seleccionar ficha de **$50**.
- `4`: Seleccionar ficha de **$100**.

### 🎡 Módulo Ruleta
- **Ratón (Clic Izquierdo):** Colocar la ficha seleccionada en una casilla o zona especial del tablero.
- **Ratón (Clic Derecho):** Quitar (deshacer) la última apuesta realizada en la casilla apuntada.
- `Retroceso` (Backspace): Deshacer la última apuesta colocada en la mesa.
- `R`: Colocar una apuesta rápida al ROJO.
- `N`: Colocar una apuesta rápida al NEGRO.
- `Espacio`: Lanzar la bolita y girar la rueda (requiere que haya al menos una apuesta en la mesa).

### 🎰 Módulo Tragamonedas
- **Ratón (Clic Izquierdo):** Pulsar el botón "SPIN" (girar) desde el panel virtual en pantalla.
- `Espacio`: Tirar de la palanca / Girar los rodillos (requiere saldo suficiente).

### 🎲 Módulo Dados
- `P`: Apostar a número PAR.
- `I`: Apostar a número IMPAR.
- `7`: Apostar al SIETE.
- `E`: Apostar a los EXTREMOS (2, 3, 11 o 12).

### 📚 Quizzes Educativos
- `A`, `B`, `C`: Seleccionar la respuesta correspondiente durante las fases de Trivia/Quiz en medio de las rondas.

## 🚀 Cómo Compilar y Correr

El simulador está construido sobre la arquitectura de 32 bits debido a las limitaciones del clásico `glut32.dll`.

1. **Requisitos Previos:**
   - Sistema operativo Windows.
   - Entorno de desarrollo **Visual Studio 2022** (o versiones compatibles) con la carga de trabajo "**Desarrollo para escritorio con C++**".
   - Las librerías de GLUT de 32 bits (`glut.h`, `glut32.lib` y `glut32.dll`), que deben instalarse globalmente en la ruta de Visual Studio y System32/SysWOW64 de tu máquina, ya que no se adjuntan en el repositorio.

2. **Abrir el Proyecto:**
   - Clona este repositorio.
   - Navega a la raíz y abre el archivo `Casino_Online.sln` directamente con Visual Studio.

3. **Verificar Plataforma:**
   - En la barra de herramientas superior, asegúrate de que la plataforma activa sea **x86** (no x64).

4. **Compilar:**
   - Usa el atajo `Ctrl + Shift + B` (o ve a *Compilar* -> *Compilar solución*).

5. **Ejecutar:**
   - Presiona `F5` para correrlo con el depurador atado, o `Ctrl + F5` para ejecutarlo sin el depurador.

*Si experimentas algún fallo relacionado con GLUT, revisa la configuración de paths de la librería.*
