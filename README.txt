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

	-b, --benchmark		 		Genera un archivo con tiempos de ejecución

	-pq, --partials-quantity		El siguiente argumento indica la cantidad de
						soluciones parciales a guardar. (default: 1)

						El nombre de los archivos de salida sera:

						<nombre_instancia>_partial_<numero_solution>.out

Ejemplos:
	./main -d nott_data -i nott1d			Ejecuta la instancia <nott1d> del
							dataset <nott_data>.

	./main -d example -i test1 -io 1 -b -pq 6	Ejecuta la instancia <test1> del
							dataset <example> con un orden
							de instanciación de variables de
							la <variable más conectada primero>
							realiza <benchmark> y guarda 6
							soluciones parciales.


########################################################
## Consideraciones
########################################################

Es necesario contar con la siguiente estructura de carpetas en la carpeta raiz.

/out -
     |_/ partials

Dentro de <out> se guardará la solución óptima y dentro de <partials> las soluciones
parciales.



















