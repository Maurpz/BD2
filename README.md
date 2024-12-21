## Proyecto BD2

# Creación de un Sistema de Gestión de Base de Datos (DBMS) en C++

## Introducción

Este proyecto implementa un sistema de gestión de base de datos (DBMS) en C++ con funcionalidades para manejar el almacenamiento y la gestión de páginas en disco, así como el manejo de buffers para mejorar la eficiencia. Las clases principales del sistema son `BufferManager`, `Disck`, y `DisckManager`. A continuación, se describen sus funciones principales y relaciones.

## Clase `Disck`
La clase `Disck` representa la estructura lógica y física de un disco, proporcionando funcionalidades para la gestión de bloques, platos, pistas y sectores.

### Componentes Clave

1. **Atributos Principales:**
   - Características del disco, como número de platos, pistas, bloques y sectores.
   - Propiedades derivadas, como el número total de bloques y el tamaño de cada bloque.
   - Directorio raíz (`root`) para almacenar los datos del disco.

2. **Estructuras de Datos:**
   - `FreeSpaceMap`: Almacena el tamaño de los sectores.
   - `EntryFreeSpaceMap`: Representa un bloque en el mapa de espacio libre.

3. **Métodos:**
   - **`createDirectories`**: Crea la estructura física del disco en el sistema de archivos.
   - **`getFreeBlock`**: Busca bloques libres para la asignación.
   - **`ubicacionDelBloque`**: Calcula la ubicación física (archivo y offset) de un bloque.
   - **`writeToDisck`**: Escribe datos en un bloque del disco.
   - **`updateStateBlock`**: Marca un bloque como ocupado en el mapa de espacio libre.

### Propósito

La clase `Disck` abstrae la representación física del disco y administra la asignación y el estado de los bloques. Interactúa con el sistema de archivos para mantener persistencia.

## Clase `DisckManager`

La clase `DisckManager` actúa como una capa de gestión que utiliza `Disck` para realizar operaciones de nivel superior, como la gestión de archivos y directorios.

### Componentes Clave

1. **Atributos Principales:**
   - `disck`: Puntero único a una instancia de `Disck`.

2. **Métodos:**
   - **`saveFile`**: Guarda un archivo en el disco.
   - **`updateFile`**: Actualiza los datos en un bloque específico.
   - **`indexingFile`**: Registra información sobre un archivo en un sistema de directorios.
   - **`createDirectories`**: Inicializa la estructura del disco y crea un sistema de directorios.
   - **`findFile`**: Busca un archivo por su nombre.
   - **`getBlockByNumber`**: Recupera datos de un bloque específico.

### Propósito

`DisckManager` encapsula operaciones como la gestión de archivos y la lectura/escritura de datos en bloques. Proporciona una interfaz de nivel superior para interactuar con el disco.

## Clase `BufferManager`

La clase `BufferManager` maneja la carga, almacenamiento y sincronización de páginas entre la memoria y el disco. Utiliza la política de reemplazo “Least Recently Used” (LRU) para optimizar el uso del buffer.

### Componentes Clave

1. **Estructuras:**
   - `bufferPool`: Mapa que almacena las páginas cargadas en el buffer.
   - `lruList` y `lruMap`: Lista doblemente enlazada y mapa para rastrear el orden de acceso reciente de las páginas.

2. **Métodos:**
   - **`loadPage`**: Carga una página desde el disco al buffer.
   - **`evictPage`**: Expulsa la página menos recientemente usada.
   - **`newPage`**: Crea una nueva página.
   - **`flushPage`**: Escribe páginas modificadas al disco.

### Propósito

La clase `BufferManager` optimiza el acceso a las páginas al reducir las operaciones de E/S mediante un buffer limitado. Es esencial para mejorar el rendimiento general del sistema.

## Relación entre las Clases

- **`Disck`:** Gestiona la representación física del disco y administra los bloques.
- **`DisckManager`:** Utiliza `Disck` para gestionar archivos y directorios.
- **`BufferManager`:** Optimiza el acceso a páginas mediante un buffer limitado y la política LRU.

## Ventajas

- Abstracción clara entre las capas física (Disck), lógica (DisckManager) y de memoria (BufferManager).
- Eficiente reutilización de bloques mediante el mapa de espacio libre.
- Mejor rendimiento gracias a la optimización del buffer.

## Limitaciones

- Falta manejo de concurrencia para accesos simultáneos.
- Validaciones limitadas para errores en operaciones de lectura/escritura.

## Conclusión

El sistema implementa una gestión eficiente de almacenamiento en disco, con abstracciones claras para la manipulación física y lógica de los datos. Este diseño modular es adecuado para aplicaciones de bases de datos o sistemas de almacenamiento de datos, y puede ampliarse con soporte para concurrencia y validación avanzada.


# Descripción del Árbol B+

## Introducción

El código implementa un Árbol B+ con soporte para inserción, búsqueda, actualización y eliminación de claves. Los Árboles B+ son estructuras jerárquicas utilizadas principalmente en sistemas de bases de datos para mantener índices equilibrados y eficientes. Esta implementación está compuesta por nodos internos y nodos hoja, que almacenan claves y referencias a datos mediante la estructura `CTID`.

## Componentes Clave

### 1. Estructura `CTID`
- Representa una referencia a una tupla en un bloque del disco.

#### Atributos:
- `numBlock`: Número del bloque.
- `offsetTuple`: Desplazamiento dentro del bloque.

### 2. Clase `Node`
- Representa un nodo del Árbol B+.

#### Atributos Principales:
- `keys`: Claves almacenadas en el nodo.
- `parent`: Puntero al nodo padre.
- `children`: Vector de punteros a nodos hijos (solo para nodos internos).
- `values`: Vector de referencias (`CTID`) a datos (solo para hojas).
- `next` y `prev`: Enlaces a las hojas adyacentes.
- `isLeaf`: Indica si el nodo es una hoja.

#### Métodos Clave:
- `splitLeaf` y `splitInternal`: Dividen un nodo hoja o interno cuando exceden la capacidad.
- `setChild`: Inserta una clave y actualiza los hijos en nodos internos.
- `get` y `set`: Obtienen o actualizan valores asociados a claves.

### 3. Clase `BPlusTree`
- Representa la estructura completa del Árbol B+.

#### Atributos Principales:
- `root`: Nodo raíz del Árbol.
- `maxCapacity` y `minCapacity`: Límites superior e inferior de claves por nodo.
- `depth`: Profundidad del Árbol.

#### Métodos Clave:
- `findLeaf`: Encuentra el nodo hoja correspondiente a una clave.
- `set`: Inserta o actualiza una clave con su referencia (`CTID`).
- `insert`: Maneja la propagación tras una división.
- `remove`: Elimina una clave y ajusta los nodos según sea necesario.
- `print`: Muestra el Árbol en formato jerárquico.

## Flujo General

### 1. Inserción (`set`)
- Busca el nodo hoja correspondiente.
- Inserta la clave y su referencia (`CTID`).
- Si el nodo excede la capacidad, se divide.
- Propaga la división hacia arriba si es necesario.

### 2. Búsqueda (`get`)
- Navega por el Árbol hasta encontrar el nodo hoja que contiene la clave.
- Devuelve la referencia asociada (`CTID`).

### 3. Actualización (`update`)
- Encuentra la clave en el nodo hoja y actualiza su referencia.

### 4. Eliminación (`remove`)
- Elimina la clave de un nodo hoja o interno.
- Si un nodo queda por debajo de la capacidad mínima, intenta:
  - Tomar claves de nodos hermanos (préstamo).
  - Fusionarse con nodos hermanos (unión).
- Ajusta la raíz si queda vacía.

### 5. Impresión (`print`)
- Recorre el Árbol y muestra su contenido en un formato legible.

## Ventajas del Diseño

- **Eficiencia:** Inserciones, búsquedas y eliminaciones tienen una complejidad promedio de \(O(\log n)\).
- **Optimizado para Disco:** Los nodos hoja están enlazados, facilitando recorridos secuenciales.
- **Flexibilidad:** Permite divisiones y fusiones dinámicas para mantener balance.

## Limitaciones

- No incluye manejo de concurrencia para accesos simultáneos.
- Faltan verificaciones avanzadas para garantizar la integridad en operaciones fallidas.

## Conclusión

El Árbol B+ implementado es una estructura eficiente y flexible para almacenamiento jerárquico, adecuada para sistemas de bases de datos o aplicaciones que requieran índices equilibrados y operaciones rápidas. Su diseño modular permite futuras extensiones, como soporte para concurrencia o manejo de errores más robusto.



# Descripción del Lexer y Parser SQL en C++

## Introducción

Este código implementa un analizador léxico (*lexer*) y un analizador sintáctico (*parser*) para procesar consultas SQL en C++. El *lexer* convierte una cadena de texto SQL en una lista de *tokens*, mientras que el *parser* valida y analiza estos *tokens* para verificar la correcta estructura de la consulta.

---

## Componentes Clave

### 1. Enumeración `TokenType`
Define los tipos de *tokens* posibles en las consultas SQL:
- **KEYWORD:** Palabras clave como `SELECT`, `FROM`, `CREATE`.
- **IDENTIFIER:** Nombres de tablas o columnas.
- **NUMBER:** Valores numéricos.
- **SYMBOL:** Caracteres especiales como paréntesis, comas o punto y coma.
- **OPERATOR:** Operadores como `=`, `<`, `>`.
- **STRING:** Cadenas de texto entre comillas.
- **UNKNOWN:** Cualquier elemento no identificado.

### 2. Clase `Token`
Representa un *token* individual con dos atributos:
- **type:** El tipo del *token* (usando `TokenType`).
- **value:** El valor del *token*.

### 3. Clase `Lexer`
El *lexer* procesa una cadena de entrada y la convierte en una lista de *tokens*. Los métodos principales incluyen:
- **`tokenize`:** Divide la entrada en *tokens*, categorizándolos en función de su tipo.
- Métodos auxiliares como:
  - **`readKeywordOrIdentifier`:** Identifica palabras clave o nombres de tablas/columnas.
  - **`readNumber`:** Procesa valores numéricos.
  - **`readString`:** Lee cadenas de texto delimitadas por comillas.
  - **`readOperator`:** Identifica operadores como `=`, `<`, `>=`.

### 4. Clase `Parser`
El *parser* utiliza los *tokens* generados por el *lexer* para validar y analizar la consulta SQL. Sus métodos principales incluyen:
- **`parseSelect`:** Procesa consultas `SELECT`, verificando las columnas, tablas y cláusulas `WHERE`.
- **`parseCreate`:** Analiza consultas `CREATE TABLE`, asegurando que los nombres de columnas y sintaxis sean válidos.
- **`parseInsert`:** Valida y analiza consultas `INSERT INTO`, revisando columnas y valores.
- **`parseDelete`:** Procesa consultas `DELETE`, incluyendo cláusulas opcionales `WHERE`.

---

## Flujo General
1. El *lexer* convierte una cadena SQL en una lista de *tokens*.
2. El *parser* toma los *tokens* y los analiza según el tipo de consulta:
   - Verifica la estructura general de la consulta.
   - Valida nombres de tablas, columnas, operadores y valores.
   - Detecta errores de sintaxis y lanza excepciones si es necesario.
3. Si la consulta es válida, el programa imprime un mensaje de éxito.

---

## Ejemplo de Uso
**Consulta de ejemplo:**
```sql
SELECT id, name FROM alumnos WHERE id < 10;
```

**Salida esperada:**
- Lista de *tokens* generados.
- Mensaje: `Consulta SQL válida.`

---

## Ventajas del Diseño
- **Separación clara:** Entre el análisis léxico y sintáctico.
- **Extensibilidad:** Permite agregar nuevos tipos de consultas o palabras clave fácilmente.
- **Manejo de errores:** Con mensajes detallados y específicos.

---

## Limitaciones
- El *parser* no genera un árbol de sintaxis abstracta (AST).
- Soporte limitado para consultas SQL avanzadas.

---

## Conclusión
Este analizador SQL en C++ es una implementación básica que demuestra cómo procesar y validar consultas de forma estructurada. Es una base sólida para extender las funcionalidades hacia un sistema más completo que soporte AST o traducción a otros lenguajes.
