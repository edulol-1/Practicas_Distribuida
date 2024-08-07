/**
 * Implementación del Algoritmo del Abusón.
 * @author Eduardo Montaño Gómez.
 * @since Laboratorio: Computación Distribuida.
 * Descripción: 
 * Este programa realiza una implementacion del algoritmo del abusón. Al inicio del programa se presupone que
 * el nodo que desencadena todo es el nodo con rango más alto, y que así permanecerá por el resto del programa.
 * 
 * Los demás nodos verificarán si alguno de ellos es el nodo con mayor rango que no es un nodo caído, si es así, 
 * se lo harán saber al resto de nodos no caidos. De otra forma, se esperará la respuesta del nuevo nodo coordinador.
 * Al final, cada nodo no caído establece su variable para el coordinador con el rango del nuevo nodo coordinador.
 * En el programa se hace uso de 0 para saber si un nodo es no caido, y 1 en otro caso. También se usa un valor numérico
 * desde 0 hasta 60 que representa que un nodo responde en tiempo, si excede este tiempo se considera un TIMEOUT.
 *
 * Cada nodo usa una función definida previamente al incio del programa para saber si él mismo es un nodo caído.
 * Dicha función siempre establece al nodo con rango size - 1 como caido. Los demás nodos tienen una probabilidad de un 20%
 * de que sean nodos caidos.
 *
 * Dicho programa no implementa el caso en que un nodo se recupere de una caida.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define TAG_ELECCION 0
#define TAG_RESPUESTA 1
#define TAG_COORDINADOR 2

/*
 * Decide si el nodo con rango rank es un nodo caido. 1 en caso verdadero,
 * 0 en caso contrario.
*/
int
regresa_si_es_caido(int rank, int size)
{
  // Cada nodo puede estar caido o no. Los nodos distintos a size-1 tienen un 20% de
  // probabilidad de ser un nodo caido.  
  int es_caido;
  if (rank == size-1)
    es_caido = 1;
  else {
    time_t t;
    srand(time (&t) + rank);
    double auxiliar = (double)rand() / RAND_MAX;
    es_caido = (auxiliar < 0.20)? 1: 0;
  } 
  return es_caido;
}

 /*
 * Calcula un tiempo de respuesta para el nodo que lo manda llamar.
 * Si es un nodo caido, su tiempo de respuesta es un entero entre 
 * 61 y 100. En caso contrario regresa un entero entre 0 y 60.
*/
int
tiempo_respuesta(int rank, int es_caido)
{
  time_t t;
  srand(time (&t) + rank);
  return (es_caido)? rand() % (100 + 1 - 61) + 61:
    rand() % (60 + 1 - 0) + 0;
}

int
main (int argc, char **argv)
{
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &size);
  if (size < 4) {
    printf("Deben haber al menos 4 nodos.\n");
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  int es_caido = regresa_si_es_caido(rank, size);
  int tiempo = tiempo_respuesta(rank, es_caido);
  int coordinador;
  printf("¿El nodo %i está caido?: %i\n", rank, es_caido);

  // Manda a los nodos inferiores su estado (si es caido o no).
  for (int i = rank - 1; i >= 0; i--) {
    MPI_Send(&tiempo, 1, MPI_INT, i, TAG_RESPUESTA, MPI_COMM_WORLD);
  }

  // Recibe las respuestas de los estados de los nodos superiores y deduce si es el nodo con el rango mas alto
  // y que no es un nodo caido.
  int es_id_no_fallido_mas_alto = (es_caido)? 0:1;
  for (int i = rank + 1; i < size; i++) {
    int tiempo_respuesta;
    MPI_Recv(&tiempo_respuesta, 1, MPI_INT, i, TAG_RESPUESTA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Si no es un timeout entonces si hay un nodo con mayor rango que no está caído.
    if (tiempo_respuesta <= 60) {
      es_id_no_fallido_mas_alto = 0;
      break;      
    }
  }

  if (es_id_no_fallido_mas_alto) {
    // Enviamos a los nodos con menor rango que ahora el nodo actual es el nuevo coordinador.
    printf("Nodo %i anuncia que es el nuevo coordinador a todos los nodos con rango menor.\n", rank);
    coordinador = rank;
    for (int i = rank - 1; i >= 0; i--) {
      MPI_Send(&coordinador, 1, MPI_INT, i, TAG_COORDINADOR, MPI_COMM_WORLD);
    }
  } else {
    // Como no es un nodo coordinador, mandamos -1 simulando que es caido o que no es coordinador.
    for (int i = rank - 1; i >= 0; i--) {
      int aux = -1;
      MPI_Send(&aux, 1, MPI_INT, i, TAG_COORDINADOR, MPI_COMM_WORLD);
    }
    int recibio_respuesta = 0;
    MPI_Status *status;
    for (int i = rank + 1; i < size; i++) {
      int mensaje_respuesta = -1;
      MPI_Recv(&mensaje_respuesta, 1, MPI_INT, i, TAG_COORDINADOR, MPI_COMM_WORLD, status);
      // Si recibimos respuesta del coordinador, entonces cambiamos nuestra variable del coordinador.
      if (mensaje_respuesta != -1) {
	coordinador = mensaje_respuesta;
	break;
      }
    }
  }

  if (!es_caido)
    printf("El nodo %i ahora sabe que el coordinador es el nodo: %i\n", rank, coordinador);
  MPI_Finalize();
  return 0;
}
