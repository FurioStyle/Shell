# Guia de Uso rapido para la shell.

## Para compilar: 
**g++ shell.cpp miprof.cpp**

luego:
**./a.out**

## Comando exit.

al escribir *exit* la shell se terminara.

## Comando cd

**cd** volvera al home.
**cd ..** subira un nivel en los directorios.
**cd <nombre>** abrira el directorio con ese nombre.

## Comando miprof

**miprof ejec <comando> <argumentos>** ejecutara el comando dado, mostrara en la terminal el tiempo utilizado para ejecutar el comando.

**miprof ejcsave <nombre> <comando> <argumentos>** ejecutara el comando dado, mostrara en la terminal el tiempo utilizado para ejecutar el comando y lo guardara en un archivo.

**miprof ejecutar <tiempo> <comando> <argumentos>** ejecutara el comando solo dentro de la ventana de tiempo, si lo completa antes se mostrar el tiempo que se tardo, si lo sobrepasa miprof terminara el proceso y el tiempo total sera el del valor entregado.

## Análisis de Rendimiento con Comando Sort

### Resultados Obtenidos:

| Archivo | Elementos | Tiempo Real | Tiempo Usuario | Memoria |
|----------------|-----------|-------------|----------------|---------|
| archivo_pequeno.txt | 10 | 0.004s | 0.022s | 3808 KB |
| archivo_mediano.txt | 1,000 | 0.006s | 0.003s | 3424 KB |
| archivo_grande.txt | 10,000 | 0.036s | 0.018s | 3808 KB |

### Analisis:

1. **Complejidad Temporal**: 
   - Crecimiento de 10 a 10,000 elementos (×1000) resulta en ×9 de tiempo (0.004s → 0.036s)
   - Confirma complejidad O(n log n) del algoritmo sort

2. **Uso de Memoria**:
   - Se mantiene alrededor de 3.5-3.8 MB para todos los tamaños
   - Demuestra eficiencia en manejo de memoria

3. **Escalabilidad**:
   - El algoritmo escala eficientemente para entradas grandes
   - Tiempo de sistema se mantiene bajo incluso con 10,000 elementos