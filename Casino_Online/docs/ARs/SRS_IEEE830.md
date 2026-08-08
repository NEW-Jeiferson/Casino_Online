# Especificación de Requisitos de Software (SRS) - Estándar IEEE-830
## Proyecto: Casino Online (Serious Game)

---

## 1. Introducción

### 1.1 Propósito del documento
El propósito de este documento es realizar una especificación de requisitos mediante ingeniería inversa para el proyecto "Casino Online". Dado que la aplicación ya fue desarrollada, este documento documenta retrospectivamente las decisiones de diseño arquitectónico y de usabilidad, con especial énfasis en su naturaleza de "Serious Game". Se busca responder al "por qué" detrás de las fricciones intencionales, la estricta adherencia al estándar C89, y las interrupciones reflexivas implementadas en la experiencia de usuario.

### 1.2 Alcance del sistema
El sistema es un casino virtual educativo diseñado no para el entretenimiento puro, sino como una herramienta de concientización sobre la ludopatía. El jugador interactúa con minijuegos clásicos de casino (Ruleta, Tragamonedas, Dados) mientras el sistema monitorea su comportamiento, niveles de deuda, y rachas de pérdida. El alcance incluye una economía virtual cerrada con un sistema de préstamos depredador que desencadena progresivamente advertencias y penalizaciones (fricción cognitiva) para educar al usuario sobre los riesgos del juego compulsivo.

### 1.3 Glosario
*   **Serious Game**: Un juego diseñado con un propósito principal distinto al del puro entretenimiento (en este caso, educativo y de concientización psicológica).
*   **Fricción Cognitiva**: Obstáculos intencionales en la interfaz o flujo de juego diseñados para forzar al jugador a detenerse y reflexionar sobre sus acciones (ej. retrasos en apuestas, pop-ups que no se pueden cerrar inmediatamente).
*   **C89 / ANSI C**: El estándar original del lenguaje de programación C, adoptado por ANSI en 1989.
*   **Legacy OpenGL (1.x)**: Modo de renderizado gráfico de funciones fijas, sin el uso de shaders modernos.
*   **HUD (Heads-Up Display)**: Interfaz gráfica en pantalla que muestra información vital (saldo, deudas, estado de ánimo).

---

## 2. Descripción General

### 2.1 Perspectiva del producto
El software opera como una aplicación de escritorio independiente, renderizada íntegramente por software utilizando llamadas gráficas de OpenGL 1.x legacy. Funciona en un solo hilo (single-threaded) sin dependencias a bases de datos externas ni librerías complejas. Actúa como un entorno aislado donde todas las métricas psicológicas e interacciones se calculan en tiempo real basándose en el historial de sesión del usuario.

### 2.2 Funciones del producto
*   **Simulación de Juegos de Azar**: Participación en Ruleta, Máquina Tragamonedas y Dados.
*   **Sistema de Economía y Préstamos**: Gestión de saldo de jugador, capacidad de adquirir deudas con intereses abusivos simulados.
*   **Motor de Concientización (Serious Game)**: Evaluación continua de las acciones del jugador para inyectar advertencias reflexivas, mensajes de impacto psicológico y penalizaciones de tiempo.
*   **Renderizado de Interfaz (HUD)**: Representación visual retro y estricta de las estadísticas de juego y navegación por menús.

### 2.3 Características de los usuarios
El público objetivo son adultos jóvenes y estudiantes que son susceptibles a la adicción a las apuestas en línea. Se asume que el usuario tiene familiaridad básica con mecánicas de casino y espera una experiencia rápida y sin fricción. El diseño subvierte intencionalmente estas expectativas para maximizar el impacto educativo.

### 2.4 Restricciones del Sistema y Diseño (El "Por Qué")
*   **Estándar C89 Estricto**: Se eligió C89 para garantizar máxima portabilidad, reproducibilidad histórica y forzar una disciplina de gestión de memoria explícita. Actúa como un ejercicio de diseño minimalista.
*   **Legacy OpenGL 1.x sin Shaders**: Se prescinde de gráficos modernos de alto rendimiento (shaders) para evitar distracciones visuales (el "glamour" del casino). El renderizado básico, de funciones fijas (software rendering), subraya la crudeza y seriedad de la simulación.
*   **Ausencia de Librerías Externas Complejas**: Mantiene la arquitectura transparente, permitiendo un control granular de los eventos de entrada para inyectar fricción deliberadamente.

---

## 3. Requisitos Específicos

### 3.1 Requisitos Funcionales

#### 3.1.1 Navegación por HUD
*   El sistema debe mostrar continuamente el saldo actual, la deuda acumulada y un indicador de estrés/riesgo del jugador.
*   La interfaz debe permitir la navegación entre las salas de juego y el banco a través de menús bloqueantes.

#### 3.1.2 Ruleta
*   El sistema debe permitir apuestas a números individuales, colores o paridad.
*   La resolución del giro debe tener un retraso (fricción) proporcional a la deuda del jugador antes de mostrar el resultado.

#### 3.1.3 Tragamonedas
*   El sistema debe simular múltiples carretes y líneas de pago.
*   Las ganancias pequeñas deben mostrarse rápidamente, pero las rachas de pérdida consecutivas deben ralentizar la máquina, forzando al jugador a leer un mensaje sobre la "falacia del apostador".

#### 3.1.4 Dados (Craps/Sic Bo)
*   Debe permitir lanzar dados con físicas simplificadas, donde el sistema loguee la impulsividad de la apuesta (tamaño de la apuesta respecto al saldo total).

#### 3.1.5 Préstamos y Bancarrota
*   El jugador debe poder pedir préstamos al sistema cuando el saldo sea cero.
*   Si la deuda supera un umbral crítico (ej. $10,000) sin capacidad de pago, el sistema debe declarar la bancarrota virtual, bloqueando todos los minijuegos temporalmente y obligando a la lectura de un sumario de la sesión (concientización).

### 3.2 Requisitos No Funcionales

#### 3.2.1 Rendimiento
*   El sistema debe operar íntegramente en un **solo hilo (single-threaded)**, garantizando sincronismo determinista de los estados del juego y las inyecciones de advertencias.

#### 3.2.2 Portabilidad
*   El código debe compilar sin advertencias bajo compiladores modernos cuando se fuerza el flag `-std=c89`. No se permiten declaraciones en medio del código ni comentarios `//`.

### 3.3 Reglas de Negocio / Educativas (Mecánicas de "Serious Game")

#### 3.3.1 Prevención de Ludopatía y Cálculo del Nivel de Riesgo
*   El sistema mantendrá un "Nivel de Riesgo" oculto que aumenta con apuestas impulsivas (>50% del saldo), pérdidas consecutivas rápidas, y préstamos sucesivos.

#### 3.3.2 Interrupciones y Fricción Cognitiva Intencional
*   A diferencia de los casinos reales que diseñan UX para retener al usuario, este sistema introducirá **obstáculos artificiales**. Ejemplo: Si el jugador pierde 5 veces seguidas rápido, el botón de "Apostar" se desactivará por 10 segundos.

#### 3.3.3 Escalonamiento Dinámico de Advertencias
*   **Fase 1 (Riesgo Bajo):** Mensajes sutiles en el HUD ("Has estado jugando por 20 minutos").
*   **Fase 2 (Riesgo Medio / Deuda Leve):** Pop-ups reflexivos entre giros ("El 90% de los jugadores no recuperan sus pérdidas").
*   **Fase 3 (Riesgo Alto / Deuda Crítica):** Bloqueo total de la interfaz por periodos de 30-60 segundos con testimonios simulados o estadísticas crudas de ludopatía.
