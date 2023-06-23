# Práctica 4

Se implementa en C con Open MPI una versión distribuida del algoritmo de ordenamiento Merge Sort.

- Se genera un arreglo con un tamaño proporcional al número de nodos, la constante de proporcionalidad es un número entre 1 y 10.
- Los elementos del arreglo son números entre 0 y 100.
- Cada nodo es el encargado de ordenar un parte del arreglo. Al final se mezclan para presentar el arreglo original ordenado.
