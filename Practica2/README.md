# Práctica 2 · Sistema de reglas con factores de certeza

Sistema basado en reglas que intenta demostrar un objetivo a partir de una base de hechos y una base de conocimiento. Utiliza encadenamiento hacia atrás y factores de certeza para representar y combinar evidencias.

Las reglas pueden relacionar antecedentes mediante conjunción o disyunción. El programa propaga la certeza a través de las reglas y combina el apoyo obtenido para una conclusión.

## Archivos

- `sbr-fc.cpp` y `sbr-fc.exe`: código C++ y ejecutable de la entrega.
- `BC-*.txt`: bases de conocimiento con reglas y sus factores de certeza.
- `BH-*.txt`: bases de hechos y objetivo de la consulta.
- `logEjecucion_*.txt`: registros de ejecuciones de ejemplo.
- [Manual](Manual%20de%20Uso%20del%20Software.pdf), [informe](Practica2-Informe.pdf) y [cuestiones](Practica2-Cuestiones.pdf): documentación.

## Compilación y ejecución

Desde esta carpeta, con un compilador C++:

```bash
g++ -std=c++11 sbr-fc.cpp -o sbr-fc
./sbr-fc BC-1.txt BH-1.txt
```

El primer argumento es la base de conocimiento y el segundo la base de hechos. Ejecuta el programa desde esta carpeta con los nombres de archivo, tal como muestra el ejemplo.

El programa presenta la conclusión y genera un registro de la inferencia. Una ejecución puede sustituir el registro existente con el mismo nombre. Los archivos incluidos permiten consultar ejemplos sin ejecutar el programa.

## Resultado académico

Calificación: **8,15/10**.

[Volver a SSII](../README.md)
