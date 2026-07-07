# 🎰 Casino Online — Simulador de Ruleta (MVP)

> Simulador de casino en OpenGL/GLUT desarrollado como herramienta de concientización sobre ludopatía, y como proyecto integrador de Computación Gráfica.

**Práctica 1 — Computación Gráfica**
Equipo: 3 integrantes | Lenguaje: C | Gráficos: OpenGL + GLUT | IDE: Visual Studio

---

## Tabla de contenido

- [Descripción](#descripción)
- [Público objetivo y propósito](#público-objetivo-y-propósito)
- [Características (MVP)](#características-mvp)
- [Requisitos previos](#requisitos-previos)
- [Instalación y configuración](#instalación-y-configuración)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Cómo ejecutar](#cómo-ejecutar)
- [Documentación](#documentación)
- [Roadmap](#roadmap)
- [Equipo](#equipo)
- [Licencia](#licencia)

---

## Descripción

**Casino Online** es un simulador de casino desarrollado en OpenGL (pipeline de función fija) con GLUT, cuyo propósito no es enseñar a apostar ni glorificar el juego, sino funcionar como una **herramienta experiencial de concientización sobre la ludopatía**. El usuario juega con saldo virtual (nunca dinero real) y experimenta la escalada típica del juego problemático: ganar, perder, quedarse sin fondos, endeudarse para seguir jugando, y ver las consecuencias de una deuda impagable.

El proyecto también sirve como ejercicio integrador de los temas de Computación Gráfica vistos en el curso: transformaciones, jerarquía con pila de matrices, modelo de iluminación de Phong, blending/transparencia, curvas de Bézier, superficies paramétricas, optimización de pipeline (culling, z-buffer), algoritmo de Bresenham y antialiasing.

## Público objetivo y propósito

- **Público primario:** jóvenes y estudiantes, como audiencia de interés para programas de prevención de adicciones al juego.
- **Público secundario:** orientadores educativos, instituciones de salud mental que busquen un recurso didáctico interactivo.
- **Propósito:** mostrar de forma tangible y sin riesgo real cómo escala una deuda de juego, replicando a menor escala principios ya usados por organizaciones de juego responsable e investigación académica en el campo (ver [`docs/analisis-previo.md`](docs/analisis-previo.md)).

## Características (MVP)

El MVP implementado corresponde al **módulo de Ruleta**:

- 🎡 Rueda de ruleta generada como superficie de revolución a partir de curvas de **Bézier**.
- 🎱 Bolita animada con curva de desaceleración (easing) también basada en Bézier.
- 💡 Iluminación con modelo de **Phong** (point light) y materiales diferenciados (madera, fieltro, metal, vidrio).
- 🪟 Vidrio protector con **blending**/transparencia.
- 📏 Líneas del tablero de apuestas dibujadas con una implementación propia del **algoritmo de Bresenham**.
- 🧱 Jerarquía de escena con **pila de matrices** (mesa → rueda → casillas → bolita).
- 🎯 **Back-face culling** y **z-buffer** activados para la rueda como sólido cerrado.
- 🖥️ HUD con saldo, estadísticas de sesión (total apostado, préstamos activos) e interpolación de color.
- 💸 Sistema de préstamo/deuda con mensajes reflexivos y pantalla final de "Game Over".

Módulos de **Póker** y **Tragamonedas** están documentados como diseño conceptual/roadmap, no implementados en este entregable (ver [Roadmap](#roadmap)).


## Requisitos previos

- Windows (el proyecto usa GLUT clásico de 32 bits, específico de Windows).
- Visual Studio 2022 (o compatible) con la carga de trabajo **Desarrollo para escritorio con C++**.
- El paquete `glutdlls37beta.zip` (GLUT clásico para Win32) — no incluido en este repositorio, ver instrucciones de instalación.

## Instalación y configuración

La guía completa y detallada de instalación (paso a paso, con solución de problemas comunes) está en:

📄 **[`INSTALL.md`](INSTALL.md)**

Resumen rápido:
1. Clona este repositorio.
2. Descarga `glutdlls37beta.zip` e instala `glut.h`, `glut32.lib` y `glut32.dll` de forma **global** en tu instalación de Visual Studio (no viajan con el repo — cada integrante del equipo debe hacerlo en su propia máquina, ver `INSTALL.md`).
3. Abre `Casino_Online.sln` en Visual Studio.
4. Confirma que la plataforma esté en **x86**.
5. Compila con `Ctrl+Shift+B` y ejecuta con `Ctrl+F5`.

## Estructura del proyecto

```
Casino_Online/
├── docs/                      # Documentación del proyecto (ver más abajo)
│   ├── ARs/                   # Architecture Decision Records
│   ├── analisis-previo.md
│   ├── justificacion-tema.md
│   ├── descripcion-proyecto.md
│   └── trazabilidad-temas.md
├── src/
│   ├── main.c                 # Punto de entrada, ciclo de vida GLUT
│   ├── core/                  # Máquina de estados, datos del jugador
│   ├── ruleta/                # Geometría (Bézier/revolución) y animación
│   ├── render/                # Iluminación (Phong) y materiales
│   ├── ui/                    # HUD y pantallas (préstamo, game over)
│   └── utils/                 # Funciones compartidas: Bézier, Bresenham
├── Casino_Online.sln
├── Casino_Online.vcxproj
├── INSTALL.md
└── README.md
```

## Cómo ejecutar

1. Abre `Casino_Online.sln` con Visual Studio.
2. Verifica que la plataforma activa sea **x86** (barra superior del IDE).
3. Compila la solución (`Ctrl+Shift+B`).
4. Ejecuta con `F5` (con depurador) o `Ctrl+F5` (sin depurador).

Si algo falla al compilar o ejecutar, revisa primero la sección de solución de problemas en [`INSTALL.md`](INSTALL.md).

## Documentación

Este proyecto sigue un enfoque de documentación arquitectónica, requerido como parte de la práctica:

| Documento | Contenido |
|---|---|
| [`docs/analisis-previo.md`](docs/analisis-previo.md) | Estudio de campo, antecedentes y beneficios de la CG aplicada a este dominio |
| [`docs/justificacion-tema.md`](docs/justificacion-tema.md) | Por qué se eligió este tema e implementación |
| [`docs/descripcion-proyecto.md`](docs/descripcion-proyecto.md) | Descripción general y alcance detallado del MVP |
| [`docs/trazabilidad-temas.md`](docs/trazabilidad-temas.md) | Mapeo de temas del curso contra su cobertura en el proyecto |
| [`docs/ARs/`](docs/ARs/) | Registro de decisiones de arquitectura (Architecture Decision Records) tomadas durante el desarrollo |

## Roadmap

- [x] Módulo Ruleta (MVP)
- [ ] Módulo Póker — luz tipo spot, materiales de cartas/fichas
- [ ] Módulo Tragamonedas — luz direccional, interpolación de color tipo neón

## Equipo

| Integrante | Responsabilidad principal |
|---|---|
| Jeiferson | Geometría y jerarquía: rueda por revolución (Bézier), pila de matrices, normales |
| Dubenny | Iluminación y materiales: modelo de Phong, blending, MSAA |
| Luis| Sistema de juego y UI: máquina de estados, HUD, Bresenham, pantallas |

## Licencia

Proyecto académico desarrollado para la materia de Computación Gráfica. Sin fines comerciales.
