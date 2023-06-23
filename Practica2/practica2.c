#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

float
get_diferencia (float correcto, float ajustado)
{
  float diferencia = correcto - ajustado;
  return (diferencia < 0) ? diferencia * (-1) : diferencia;
}

int
main (int argc, char **argv)
{
  int rank, size;
  float tiempo_local, tiempo_correcto, tiempo_ajustado, tiempo_comunicacion;
  float *tiempos_locales, *tiempos_ajustados, *tiempos_comunicacion;
  int i;
  float start_time, end_time;
  tiempos_locales = (float *) malloc (size * sizeof (float));
  tiempos_ajustados = (float *) malloc (size * sizeof (float));
  tiempos_comunicacion = (float *) malloc (size * sizeof (float));

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  // Usamos el tiempo del sistema y el rango del nodo para la semilla.
  time_t t;
  srand (time (&t) + rank);

  // Cada nodo determina su tiempo local, limite de 100.
  tiempo_local = rand () % 100;
  printf ("Nodo %d, tiempo local = %f segundos\n", rank, tiempo_local);

  // Nodo maestro solicita el tiempo local de cada nodo esclavo.
  if (rank == 0) {
    start_time = MPI_Wtime ();
    for (i = 1; i < size; i++) {
      MPI_Recv (&tiempos_locales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD,
		MPI_STATUS_IGNORE);
      end_time = MPI_Wtime ();
      //  Calculamos tiempos de comunicacion y ajustamos por tiempo de transmisión
      // (restamos el tiempo de comuniacion al tiempo local de cada nodo).
      tiempos_comunicacion[i] = (end_time - start_time) / 2;
      tiempos_locales[i] -= tiempos_comunicacion[i];
    }
    tiempos_locales[0] = tiempo_local;
  } else {
    // Los nodos esclavos responden con su tiempo local.
    MPI_Send (&tiempo_local, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  // Nodo maestro calcula el tiempo correcto
  if (rank == 0) {
    float promedio = 0;
    for (i = 0; i < size; i++) {
      promedio += tiempos_locales[i];
    }
    promedio = (promedio / size);
    tiempo_correcto = promedio;
    printf ("Tiempo correcto: %f segundos \n", tiempo_correcto);
    // El nodo maestro envia la diferencia de tiempo al nodo esclavo.
    for (i = 1; i < size; i++) {
      tiempos_ajustados[i] = get_diferencia (tiempo_correcto, tiempos_locales[i]);
      MPI_Send (&tiempos_ajustados[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
    tiempos_ajustados[0] = get_diferencia(tiempo_correcto, tiempos_locales[0]);

  } else {
    MPI_Recv (&tiempo_ajustado, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
	      MPI_STATUS_IGNORE);
  }

  // Cada nodo ajusta su tiempo local y lo muestra en pantalla
  if (rank == 0) {
    for (i = 0; i < size; i++) {
      tiempos_locales[i] = tiempo_correcto;
      printf ("Proceso %d, nuevo tiempo = %f segundos (ajuste = %f segundos)\n", i,
	      tiempos_locales[i], ((tiempos_ajustados[i] < 0) ? tiempos_ajustados[i] * (-1) : tiempos_ajustados[i]));
    }
  }
  free (tiempos_locales);
  free (tiempos_comunicacion);
  free (tiempos_ajustados);
  MPI_Finalize ();

  return 0;
}
