# Documentación del Módulo: Juego de Dados

## 1. Resumen del módulo
El módulo de `dados` implementa un minijuego interactivo de lanzamiento de dados para un simulador de casino, utilizando C89 y la tubería fija de OpenGL. El módulo separa sus responsabilidades en tres capas fundamentales: 
1. **Lógica**: Resolución de apuestas, generación de números aleatorios y cálculo de pagos.
2. **Animación**: Simulación pseudo-física de los dados basándose en el tiempo y rotaciones precalculadas, garantizando que los dados caigan en el resultado lógico ya determinado.
3. **Geometría/Renderizado**: Creación procedural de los dados (caras, puntos) y la mesa utilizando primitivas básicas de OpenGL (`glBegin`, `glEnd`) con materiales.

## 2. Estructuras de datos clave

### `EstadoDados` (dados_animacion.h)
Mantiene el estado de la animación de ambos dados de manera persistente durante el tiro.
- Controla velocidades angulares y de rebote (`velocidad_x/y/z`, `velocidad_rebote`, `altura`).
- Almacena rotaciones actuales y el objetivo al que deben ajustarse durante el asentamiento (`rotacion_x/y/z`, `rot_snap_x/y/z`).
- Guarda los valores finales de cada dado que fueron determinados por la lógica (`dado1_final`, `dado2_final`).
- Sincroniza las fases de animación usando tiempo continuo (`tiempo_transcurrido`, `girando`, `fase_asentamiento`).

### `TipoApuestaDados` (dados_logica.h)
Enumeración para los diferentes tipos de apuestas que el jugador puede realizar: `APUESTA_DADOS_PAR`, `APUESTA_DADOS_IMPAR`, `APUESTA_DADOS_SIETE`, `APUESTA_DADOS_EXTREMOS`.

## 3. Funciones públicas

### `inicializar_dados_animacion` (dados_animacion.h)
- **¿Para qué sirve?**: Establece los valores iniciales y neutros (reposo) de la estructura de animación antes de cualquier tirada de dados.
- **¿Por qué se diseñó así?**: Se usa el patrón de pasar un puntero a una estructura de estado por parámetro para evitar variables globales y cumplir con las mejores prácticas en C89. Centralizar los estados garantiza consistencia.
- **¿Cómo lo hace internamente?**: Asigna ceros a los ángulos de rotación, tiempo y fases, y pone los dados a una altura base (`0.5f`).

### `iniciar_tiro_dados` (dados_animacion.h)
- **¿Para qué sirve?**: Inicia el evento de lanzar los dados visualmente. Toma el resultado numérico ya definido por la lógica de juego y configura las velocidades iniciales.
- **¿Por qué se diseñó así?**: La animación debe llegar inexorablemente al resultado generado por `decidir_resultado_dados`. Es por ello que recibe `dado1` y `dado2` para preasignarlos (`dado1_final`, `dado2_final`). En lugar de depender de la suerte en un motor de física 3D complejo, es una simulación visual que se acopla al azar matemático estricto.
- **¿Cómo lo hace internamente?**: Marca la bandera `girando` a 1, reinicia el tiempo y asigna velocidades iniciales y caóticas (`360.0f` a `540.0f` grados por segundo) a los diferentes ejes (X, Y, Z) de cada dado para que el inicio parezca violento y aleatorio.

### `actualizar_dados` (dados_animacion.h)
- **¿Para qué sirve?**: Actualiza la posición y rotación de los dados basándose en el tiempo real (`delta_tiempo`), haciendo que los dados rueden, boten y finalmente se asienten mostrando la cara ganadora hacia arriba (+Y).
- **¿Por qué se diseñó así? (Matemática de asentamiento / Restricción C89)**: Se evitó implementar física de cuerpos rígidos real que podría ser muy inestable o costosa. En lugar de ello, la animación está pre-guionada matemáticamente en base a un tiempo total `TIEMPO_TIRO`. Dividir la rotación en dos fases (rotación libre y asentamiento - *ease-out*) genera un tiro muy realista en la experiencia del usuario. Todo ello cumpliendo con la limitación de declarar iteradores al inicio del bloque en C89.
- **¿Cómo lo hace internamente?**: 
  - Si el tiempo (normalizado `t` de 0 a 1) está por debajo de `0.7f` (Fase 1), aplica un decaimiento parabólico a las velocidades y añade un bote (variación senoidal absoluta en la altura). 
  - Si `t > 0.7f` (Fase 2), entra en la fase de asentamiento. Calcula los ángulos `rx, ry, rz` que posicionan el valor deseado mirando a la cámara a través de `calcular_angulos_finales`. Calcula las revoluciones acumuladas para saber el camino angular más corto para encajar los dados y finalmente usa una interpolación cuadrática (Ease-out) para que suavemente lleguen al reposo perfecto antes de llegar a `t = 1.0f`.

### `dibujar_dados` (dados_geometria.h)
- **¿Para qué sirve?**: Renderiza todo el contexto visual de la mesa y los dos dados en pantalla utilizando las matrices y el estado de rotaciones/alturas pasados.
- **¿Por qué se diseñó así? (Renderizado manual)**: En lugar de cargar mallas 3D complejas (.obj) y texturas mapeadas (UVs), se prefirió modelar geométricamente mediante polígonos simples (`GL_QUADS`) y puntos de los dados (círculos creados mediante `GL_TRIANGLE_FAN`), manteniendo el juego puro con primitivas inmediatas de OpenGL clásico. Además, utiliza colores difusos, ambientes y especulares en los vértices (`glMaterialfv`) para simular materiales realistas (madera, fieltro verde y plástico rojo).
- **¿Cómo lo hace internamente?**: Llama a funciones estáticas auxiliares para dibujar primero la mesa (cubos para la madera, caras de fieltro verde). Luego hace un `glPushMatrix()`, traslada cada dado a su posición en pantalla y aplica las rotaciones de `estado` usando `glRotatef`. Internamente `dibujar_un_dado` llama a `dibujar_cara` seis veces, orientando polígonos y ubicando la correcta cantidad y posición de puntos dependiendo del valor de esa cara utilizando compensaciones (offsets).

### `decidir_resultado_dados` (dados_logica.h)
- **¿Para qué sirve?**: Genera los números finales que caerán en ambos dados. 
- **¿Por qué se diseñó así?**: Se separa de la animación explícitamente para que la lógica de casino decida la ganancia antes de iniciar la representación visual.
- **¿Cómo lo hace internamente?**: Usa `rand() % 6 + 1` de la librería estándar de C (`stdlib.h`) sobre cada dado suministrado por referencia mediante punteros.

### `calcular_ganancia_dados` (dados_logica.h)
- **¿Para qué sirve?**: Evalúa si el jugador gana la apuesta según las reglas matemáticas y los valores caídos.
- **¿Por qué se diseñó así?**: Aísla puramente las matemáticas del pago. Recibe un multiplicador (`monto`) para devolver la ganancia en base a probabilidades ajustadas.
- **¿Cómo lo hace internamente?**: Suma `dado1` y `dado2` y usa un bloque `switch` basado en `TipoApuestaDados`. Modifica una bandera de victoria, asigna el pago respectivo (ej: `13.0f` para extremos, `0.9f` para pares/impares), y retorna si el jugador gana (positivo) o pierde el monto (negativo).

### `resolver_ronda_dados` (dados_logica.h)
- **¿Para qué sirve?**: Es el puente principal entre el subsistema de dados y el Core del casino (específicamente la abstracción del Jugador).
- **¿Por qué se diseñó así?**: Centraliza la resolución de una ronda, actualiza el balance de la billetera del jugador y maneja eventos (como mensajes reflexivos cuando el jugador pierde de más), logrando encapsulamiento.
- **¿Cómo lo hace internamente?**: Llama a `calcular_ganancia_dados`, luego invoca la función core `aplicar_resultado_apuesta` inyectándole al jugador su nueva situación financiera, y finaliza evaluando `verificar_mensaje_reflexivo`.

## 4. Dependencias

- Lógicas y estándares C89: `<math.h>` (para funciones trigonométricas y `fabs`), `<stdlib.h>` (para `rand()`).
- Gráficas: `<GL/glut.h>` (provee primitivas y funciones de estado como las pilas de matrices `glPushMatrix`, materiales e iluminación).
- Core/Propias:
  - `"../core/jugador.h"`: Para las estructuras de billetera e interacciones del balance final.
  - `"../render/materiales.h"`: Para el uso de variables uniformes de material como `MATERIAL_FIELTRO`, `MATERIAL_MADERA` o `MATERIAL_METAL` de manera coherente con otros mini-juegos.
