# Proyecto 2. Sistemas Operativos 2019-3 miTwitter - Makefile para generar los ejecutables
# Juan Camilo Chafloque, Jorge Salgado y Cristobal Castrillon
# 

all: gestor cliente

cliente: cliente.o 
	gcc cliente.o -o cliente 

cliente.o: cliente.c
	gcc -c cliente.c 

gestor: gestor.o
	gcc gestor.o -o gestor

gestor.o: gestor.c
	gcc -c gestor.c

clean: rm *.o gestor cliente
