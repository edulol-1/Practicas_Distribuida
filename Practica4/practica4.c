/**
 * Implementación en su versión distribuida del algoritmo MERGE SORT.
 * @author Eduardo Montaño Gómez.
 * @date 25 de mayo del 2023.
 * @since Laboratorio Computacion Distribuida.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define TAG_LENGTH 1
#define TAG_DATA 2

void merge_sort (int *arr, int l, int r);
void merge (int *arr, int l, int m, int r);
int *obtener_array (int longitud);
void poblar_array (int *arr, int length_arr);
void print_array (int *arr, int length_arr);
void merge_modificado (int *arr1, int *arr2, int arr1_size, int arr2_size);
int *obtener_array_a_enviar (int *arr, int i, int j, int size, int constante);

int
main (int argc, char **argv)
{
  int size, rank;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &size);
    if (size < 4)
      {
        printf ("Deben haber al menos 4 nodos.\n");
        MPI_Abort (MPI_COMM_WORLD, EXIT_FAILURE);
      }
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  
  if (rank == 0)
    {
      // El nodo maestro se encarga de crear el array.
      time_t t;
      srand (time (&t) + size);
      int constante = (rand () % 10) + 1;
      int longitud = constante * size;
      int *arr = obtener_array (longitud);
      printf ("Nodos: %i, arreglo: ", size);
      print_array (arr, longitud);
      printf ("\n");
      // Mandamos el arreglo correspondiente al arreglo que le corresponde a cada nodo.
      for (int r = 1; r < size; r++)
	{
	  int *array_a_enviar = obtener_array_a_enviar (arr, r * constante,
							r * constante +
							constante,
							longitud,
							constante);
	  MPI_Send (&constante, 1, MPI_INT, r, TAG_LENGTH, MPI_COMM_WORLD);
	  MPI_Send (array_a_enviar, constante, MPI_INT, r, TAG_DATA,
		    MPI_COMM_WORLD);
	  free (array_a_enviar);
	}
      // El nodo maestro ordena su parte del arreglo que le corresponde.
      int *array_a_ordenar = (int *) malloc (constante * sizeof (int));
      for (int i = 0; i < constante; i++)
	array_a_ordenar[i] = arr[i];

      free (arr);
      printf("El nodo 0 ordenará el arreglo: ");
      print_array(array_a_ordenar, constante);
      printf("\n");      
      merge_sort (array_a_ordenar, 0, constante - 1);
      int *resultado = (int *) malloc (longitud * sizeof (int));
      for (int i = 0; i < constante; i++)
	resultado[i] = array_a_ordenar[i];

      free (array_a_ordenar);
      // Recibe los arreglos ordenados de los demás nodos y los ordena usando
      // una versión modificada del método auxilar para Merge Sort.
      for (int r = 1; r < size; r++)
	{
	  int *array_recibido = (int *) malloc (constante * sizeof (int));
	  MPI_Recv (array_recibido, constante, MPI_INT, r, TAG_DATA,
		    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  // Usamos Merge Sort secuencial para ordnar el arreglo correspondiente.
	  merge_modificado (resultado, array_recibido, r * constante,
			    constante);
	  free (array_recibido);
	}
      printf ("Arreglo final: ");
      print_array (resultado, longitud);
      printf("\n");
      free (resultado);
    }
  else
    {
      // Los demas nodos reciben el arreglo que les toca ordenar. Una vez ordenado, lo mandan de regreso al
      // nodo maestro.
      int constante;
      MPI_Recv (&constante, 1, MPI_INT, 0, TAG_LENGTH, MPI_COMM_WORLD,
		MPI_STATUS_IGNORE);
      int *array_a_ordenar = (int *) malloc (constante * sizeof (int));
      printf ("El nodo %i ordenará el arreglo: ", rank);
      MPI_Recv (array_a_ordenar, constante, MPI_INT, 0, TAG_DATA,
		MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      print_array (array_a_ordenar, constante);
      printf("\n");
      merge_sort (array_a_ordenar, 0, constante - 1);
      MPI_Send (array_a_ordenar, constante, MPI_INT, 0, TAG_DATA,
		MPI_COMM_WORLD);
      free (array_a_ordenar);
    }
  MPI_Finalize ();
  return 0;
}

/**
 * Obtiene un apuntador con al menos un elemento, dos indices, y una constante.
 * Crea un nuevo arreglo dado esos dos índices y la constante.
 * Regresamos dicho arreglo.
 */
int *
obtener_array_a_enviar (int *arr, int i, int j, int size, int constante)
{
  int *array_temporal = (int *) malloc (constante * sizeof (int));
  for (int k = 0; k < constante; k++)
    {
      array_temporal[k] = arr[i + k];
    }
  return array_temporal;
}

/**
 * Recibe dos apuntadores con sus elementos ordenados, y sus respectivos tamaños.
 * Aplica el método auxilar de Merge Sort, Merge, para integrarlos en un
 * solo apuntador.
 */
void
merge_modificado (int *arr1, int *arr2, int arr1_size, int arr2_size)
{
  int i = 0;
  int j = 0;
  int k = 0;
  int nueva_longitud = arr1_size + arr2_size;
  int *resultado = (int *) malloc (nueva_longitud * sizeof (int));
  while (i < arr1_size && j < arr2_size)
    {
      resultado[k++] = (arr1[i] <= arr2[j]) ? arr1[i++] : arr2[j++];
    }

  while (i < arr1_size)
    {
      resultado[k++] = arr1[i++];
    }

  while (j < arr2_size)
    {
      resultado[k++] = arr2[j++];
    }
  for (int i = 0; i < nueva_longitud; i++)
    arr1[i] = resultado[i];
  free (resultado);
}

/* Algoritmo Mezcla auxiliar para Merge Sort.*/
void
merge (int *arr, int l, int m, int r)
{
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  int L[n1], R[n2];

  for (i = 0; i < n1; i++)
    L[i] = arr[l + i];
  for (j = 0; j < n2; j++)
    R[j] = arr[m + 1 + j];

  i = 0;
  j = 0;
  k = l;
  while (i < n1 && j < n2)
    {
      if (L[i] <= R[j])
	{
	  arr[k++] = L[i++];
	}
      else
	{
	  arr[k++] = R[j++];
	}
    }

  while (i < n1)
    {
      arr[k++] = L[i++];
    }

  while (j < n2)
    {
      arr[k++] = R[j++];
    }
}

/** Recibimos un apuntador y dos indices, cado uno representa un extremo
 * de la parte del arreglo a ordenar.
 * Implementa el algoritmo de Merge Sort de forma secuencial.
 */
void
merge_sort (int *arr, int l, int r)
{
  if (l < r)
    {
      int m = l + (r - l) / 2;

      merge_sort (arr, l, m);
      merge_sort (arr, m + 1, r);

      merge (arr, l, m, r);
    }
}

/**
 * Obtenemos un array con elemetos aleatorios dada una longitud.
 */
int *
obtener_array(int longitud)
{
  int *arr = (int *) malloc (longitud * sizeof (int));
  poblar_array (arr, longitud);
  return arr;
}

/**
 *  Dado un array y su longitud, se pobla dicho array con numeros
 * generados al azar, cuyos valores van desde 0 hasta 100.
 */
void
poblar_array (int *arr, int length_arr)
{
  time_t t;
  srand (time (&t) + length_arr);
  for (int i = 0; i < length_arr; i++)
    {
      arr[i] = rand () % 101;
    }
}

/** 
 * Dado un apuntador y su longitud, imprime todos los elementos.
 */
void
print_array (int *arr, int length_arr)
{
  printf ("[ ");
  for (int i = 0; i < length_arr; i++)
    {
      printf ("%i ", arr[i]);
    }
  printf ("]");
}
