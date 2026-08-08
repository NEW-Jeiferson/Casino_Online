# Documentación del Módulo Tragamonedas

## 1. Resumen del módulo
El módulo `tragamonedas` implementa el minijuego de máquina tragamonedas (slot machine) para un proyecto "Serious Game" de Casino Online en OpenGL (C89). Está diseñado bajo el principio de separación de responsabilidades:
- **Lógica (`tragamonedas_logica`)**: Reglas matemáticas, probabilidades, símbolos y cálculo de pagos.
- **Animación (`tragamonedas_animacion`)**: Estado de los rodillos, físicas de giro y desaceleración usando curvas de Bezier.
- **Geometría (`tragamonedas_geometria`)**: Construcción 3D del gabinete y sus partes mediante primitivas de OpenGL, respetando la estricta restricción de no importar modelos externos.
- **Texturas (`simbolos_tex`)**: Carga y renderizado de los símbolos visuales utilizando `stb_image`.

## 2. Estructuras de datos clave
- **`SimboloTragamonedas`**: Un `enum` que representa los distintos símbolos posibles (`SIMBOLO_CEREZA`, `SIMBOLO_CAMPANA`, `SIMBOLO_HERRADURA`, `SIMBOLO_DIAMANTE`, `SIMBOLO_BARRA`, `SIMBOLO_SIETE`).
- **`EstadoRodillo`**: Estructura que almacena la posición actual, velocidad, estado de giro, variables de la curva de Bezier (velocidad inicial, duración total, tiempo transcurrido) y la posición objetivo exacta para evitar errores de punto flotante.
- **`EstadoTragamonedas`**: Contenedor principal que agrupa el estado de los 3 rodillos, el resultado actual de los símbolos, banderas de estado (`todos_detenidos`, `hay_ganancia`), y variables relacionadas a la ganancia (`ganancia_ultima`, `monto_apuesta`, `tiempo_desde_parada`).

## 3. Funciones públicas

### `inicializar_tragamonedas_animacion(EstadoTragamonedas* estado)`
- **¿Para qué sirve?**: Prepara el estado inicial del tragamonedas, dejando los rodillos detenidos y listos para jugar.
- **¿Por qué se diseñó así?**: Permite tener un punto de partida determinista y seguro. Setea banderas como `todos_detenidos = 1` para que el bucle de actualización no dispare eventos de finalización falsos en el primer frame.
- **¿Cómo lo hace internamente?**: Itera sobre los rodillos poniendo posiciones, velocidades y tiempos a cero, e inicializa variables de ganancia y tiempos de parada.

### `actualizar_tragamonedas(EstadoTragamonedas* estado, float delta_tiempo)`
- **¿Para qué sirve?**: Actualiza la posición y velocidad de los rodillos frame a frame basados en el tiempo transcurrido.
- **¿Por qué se diseñó así?**: Utiliza **curvas de Bezier** para la desaceleración (easing) imitando el frenado físico de la ruleta del mismo proyecto, ofreciendo una respuesta visual fluida y consistente en el juego. Se evitan errores de acumulación flotante al forzar la "posición objetivo" al terminar.
- **¿Cómo lo hace internamente?**: Itera los rodillos activos calculando `t` (progreso). Evalúa la curva de Bezier cúbica para obtener un factor de velocidad. Suma a la posición. Al detenerse el último rodillo (detectado cuando `alguno_girando` pasa a 0), calcula las ganancias y activa banderas para UI.

### `iniciar_giro_tragamonedas(EstadoTragamonedas* estado)`
- **¿Para qué sirve?**: Desencadena la acción del juego arrancando la animación de los rodillos.
- **¿Por qué se diseñó así?**: El resultado se decide **antes** de que los rodillos comiencen a girar (usando un RNG). Esto garantiza que la parte visual solo represente el resultado matemático de la lógica de rodillos sin interferir, y permite calcular los tiempos exactos de giro.
- **¿Cómo lo hace internamente?**: Bloquea el giro si ya está en curso. Llama a la lógica para decidir el resultado y lo almacena. Calcula la distancia angular relativa para cada rodillo y su tiempo de animación necesario con un escalonamiento temporal (para que paren uno por uno). Asigna velocidades iniciales.

### `dibujar_tragamonedas(const EstadoTragamonedas* estado)`
- **¿Para qué sirve?**: Renderiza el modelo 3D del tragamonedas en pantalla.
- **¿Por qué se diseñó así?**: Debido a la **restricción C89** y directivas del proyecto, está estrictamente prohibido cargar modelos externos (OBJ, FBX). Por ende, el gabinete se moldea artesanalmente.
- **¿Cómo lo hace internamente?**: Usa llamadas directas de OpenGL (`GL_QUADS`, `GL_TRIANGLE_STRIP`, etc.) para dibujar la base, cuerpo y marquesina basándose en constantes predefinidas. 

### `obtener_simbolo_en_posicion(int rodillo, float posicion)`
- **¿Para qué sirve?**: Devuelve qué símbolo se encuentra en un punto exacto de la tira del rodillo.
- **¿Por qué se diseñó así?**: Encapsula el diseño de las tiras (array circular `ORDEN_TIRA_RODILLO`). La capa visual jamás sabe qué orden tiene el rodillo; solo consulta a la lógica para renderizar.
- **¿Cómo lo hace internamente?**: Toma la posición, le aplica un módulo flotante (`fmodf`) sobre la cantidad total de símbolos (manejando correctamente números negativos) y lo indexa contra el array constante.

### `decidir_resultado_tragamonedas(SimboloTragamonedas resultado[NUM_RODILLOS])`
- **¿Para qué sirve?**: Determina los símbolos ganadores de una ronda de manera aleatoria.
- **¿Por qué se diseñó así?**: Se centraliza el **generador de números aleatorios (RNG)** para que funcione igual que la ruleta. Al llamarlo una única vez por tirada se evitan manipulaciones por frame.
- **¿Cómo lo hace internamente?**: Un bucle itera sobre los 3 rodillos asignando un símbolo al azar usando `rand() % NUM_SIMBOLOS`. (El estado inicial del RNG lo maneja `main.c`).

### `calcular_ganancia_tragamonedas(const SimboloTragamonedas resultado[NUM_RODILLOS], float monto)`
- **¿Para qué sirve?**: Calcula el dinero ganado (o perdido) en base a los resultados del giro.
- **¿Por qué se diseñó así?**: Maneja lógicas de recompensas duras (Trío vs. Par), con multiplicadores estáticos en un array escalado, permitiendo balancear la economía y probabilidad independientemente de la visual. 
- **¿Cómo lo hace internamente?**: Cuenta las ocurrencias de cada símbolo. Si un símbolo aparece en los 3 rodillos, paga mediante la tabla `MULTIPLICADOR_TRIO`. Si aparece exactamente 2 veces, paga el `MULTIPLICADOR_PAR`. Si no, retorna la apuesta en negativo.

### `resolver_ronda_tragamonedas(Jugador* jugador, const SimboloTragamonedas resultado[NUM_RODILLOS], float monto)`
- **¿Para qué sirve?**: Impacta los resultados de la tirada en las finanzas globales del jugador.
- **¿Por qué se diseñó así?**: Se integra con el sistema `core/jugador.h` para que métricas de deudas, dinero y mensajes reflexivos de ludopatía sean universales en el casino, respetando las reglas de concientización.
- **¿Cómo lo hace internamente?**: Llama a `calcular_ganancia_tragamonedas`, aplica el saldo mediante `aplicar_resultado_apuesta` y chequea si es necesario un mensaje mediante `verificar_mensaje_reflexivo`.

### `simbolos_tex_inicializar()`
- **¿Para qué sirve?**: Carga en memoria gráfica los PNGs de los símbolos.
- **¿Por qué se diseñó así?**: Al usar texturas en un entorno de **C89** y sin herramientas avanzadas, se integró `stb_image.h`. Debido a que las imágenes disponibles son JPEG disfrazadas de PNG (sin canal alpha nativo), la función aplica un procesamiento post-carga para crear la transparencia mediante luminancia, guardando un reborde neon sin recortes abruptos. 
- **¿Cómo lo hace internamente?**: Intenta abrir las rutas predefinidas. Extrae datos, usa `aplicar_recorte_por_brillo()` para generar el canal Alpha según el brillo del píxel, y sube el array con `glTexImage2D`.

### `simbolos_tex_liberar()`
- **¿Para qué sirve?**: Libera la memoria de video de las texturas.
- **¿Cómo lo hace internamente?**: Llama a `glDeleteTextures` en un bloque.

### `simbolos_tex_dibujar(SimboloTragamonedas simbolo, float tamano)`
- **¿Para qué sirve?**: Dibuja el Quad (cuadrado) 3D con la textura del símbolo especificado.
- **¿Por qué se diseñó así?**: Para mantener los colores originales del neón sin que el sistema de iluminación del motor los altere o los oscurezca.
- **¿Cómo lo hace internamente?**: Deshabilita iluminación (`glDisable(GL_LIGHTING)`), habilita blending alpha, bindea la textura generada y dibuja los vértices con `glTexCoord2f`. Si falla al cargar, dibuja un cuadro de respaldo magenta solido.

### `simbolos_tex_tiene_textura(SimboloTragamonedas simbolo)`
- **¿Para qué sirve?**: Comprueba si el símbolo actual depende de una textura o si se renderiza con modelos poligonales a mano (ej. Herradura).
- **¿Cómo lo hace internamente?**: Revisa si el ID de textura almacenado es distinto a `0`.

## 4. Dependencias
- **Módulo Core (`../core/jugador.h`)**: Gestión central del dinero, apuestas y comportamiento reflexivo de ludopatía (Serious Game).
- **Módulo Utils (`../utils/bezier.h`)**: Curvas de Bezier matemáticas para el easing y desaceleración de giro.
- **Carga de Texturas (`stb_image.h`)**: Biblioteca de un solo archivo para procesar las imágenes PNG/JPG en crudo hacia memoria OpenGL.
- **OpenGL/GLUT (`<GL/glut.h>`)**: Renderizado 3D de geometría fija (API antigua).
- **Librerías estándar de C**: `<math.h>`, `<stdlib.h>`, `<stdio.h>`, `<string.h>`.
