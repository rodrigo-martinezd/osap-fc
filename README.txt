# osap-fc

################################################################
## Instrucciones de compilación
################################################################

Para compilar ejecutar makefile en la carpeta raíz del proyecto.

e.j:  make

Para limpiar archivos de compilación ejecutar commando <clean>


################################################################
## Instrucciones de uso
################################################################


Al momento de ejecutar el programa se pueden pasar múltiples opciones
para su configuración.

Las opciones disponibles son:

	-d, --dataset				El siguiente argumento luego de este parámetro
						debe ser el nombre del dataset a utilizar

	-i, --instance				El siguiente argumento luego de este parámetro
						debe ser el nombre de la instancia a utilizar
						sin incluir la extensión.

	** Ambas opciones son requeridas por el programa.

	-io, --instantiation-order		Orden de instanciacion:
				 			- 0 : secuencial (ej: 0,1,2,...,n)
				 			- 1 : variable más conectada primero
				 			- 2 : variable menos conectada primero

	-pq, --partials-quantity		El siguiente argumento indica la cantidad de
						soluciones parciales a guardar. (default: 1)

						El nombre de los archivos de salida sera:

						<nombre_instancia>_partial_<numero_solution>.out

Ejemplos:
	./main -d nott_data -i nott1d			Ejecuta la instancia <nott1d> del
							dataset <nott_data>.

	./main -d example -i test1 -io 1  -pq 6	Ejecuta la instancia <test1> del
							dataset <example> con un orden
							de instanciación <variable más conectada primero>
							y guarda 6 soluciones parciales.


########################################################
## Benchmark
########################################################

Para ejecutar el algoritmo y obtener métricas de su eficiencia y otros aspectos.
Es necesario modificar el archivo makefile.

Se debe sustituir:

CFLAGS = -Wall -c $(DEBUG)

por:

CFLAGS = -Wall -c $(DEBUG) -std=c++14

Es decir, es necesario tener instalado gcc o g++ en su v14Y hay que forzar a que el
compilador compile el programa con esa versión. Usando -std=c++14

También se deben descomentar las lineas 7 y 487 en el archivo main.cpp para importar
la libreria chronos que se encarga de realizar mediciones sobre el tiempo de ejecución(linea 7)
y setear el bench_time inicial (linea 487)

Luego de realizar estos cambios se agrega una nueva opción al momento de ejecutar el programa.

	--benchmark, -b			El siguiente argumento luego de esta opción debe ser alguno de los
							siguientes enteros (default 0, no hacer nada):

							- 1 :  Experimento 1 (timing)
							- 2 :  Experimento 2 (checks)
							- 3 :  Experimento 3 (quality)

							* Para conocer en que consiste cada experimento revisar el informe.


Al ejecutar el programa con esta opción se crear un archivo csv con los resultados dentro de la
carpeta out. Con el nombre de archivo:

<nombre_instancia>_benchmarck_<nombre_experimento>.csv

Donde nombre experimento puede ser:

- timing
- checks
- quality

** Dependiendo de cuanto tiempo se ejecute el benchmark el archivo csv puede superar la cantidad
de filas leidas por excel. En este caso considerar ejecutar por menos tiempo o importar a R project.


########################################################
## Consideraciones
########################################################

Es necesario contar con la siguiente estructura de carpetas en la carpeta raiz.

/out -
     |_/ partials

Dentro de <out> se guardará la solución óptima y dentro de <partials> las soluciones
parciales.

** Si no se cuenta con dicha estructura el algoritmo arrojará error.
