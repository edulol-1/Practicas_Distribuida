# Prácticas de la materia de Computación Distribuida.

Tres practicas realizadas en la materia de CD. 

El lenguage usado es C, junto con la biblioteca Open MPI.

Para poder usar la biblioteca debemos de hacer:
'''
sudo apt-get install openmpi-bin libopenmpi-dev 
'''

Para compilar los programas en linux:
''' 
mpicc <archivo>
'''
 y con
'''
gcc <archivo> -l mpi
'''

Para ejecutar:
'''
mpirun <archivo>
'''
de igual forma:
'''
mpiexec <archivo>
'''

Argumentos de ejecucion adicionales:
'''
mpirun --oversubscribe -np 4 <archivo>
'''
y
'''
mpirun -np 4 --hostfile file.txt <archivo>
'''

