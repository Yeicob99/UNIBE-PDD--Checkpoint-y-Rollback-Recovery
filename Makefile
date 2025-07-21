all:
	mpicc main.c -o checkpoint_app

run:
	@mpirun -np 3 ./checkpoint_app | tee salida_completa.txt


