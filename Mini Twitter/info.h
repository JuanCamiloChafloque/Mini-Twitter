/*
Proyecto 2. Sistemas Operativos 2019-3 MiTwitter
Archivo: info.h
Realizado por: Juan Camilo Chafloque, Cristobal Castrillon y Jorge Salgado
Contiene: Estructuras necesarias para correr la aplicación Cliente-Servidor
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

int contConectados;
int contPendientes;
int relaciones[10][10];
struct Data *informacionUsuarios;
struct Buffer buffer;
int fdSignal;

struct Data{
    int idUsuario;
    int pid;
    int fd;
    int opcion;
    bool conectado;
    int follow;
    int unfollow;
    bool recibido;
    char pipeId[10];
};

struct Mensaje{
    char mensaje[200];
    int idReceptor;
    int idEmisor;
    bool enviado;
};

struct Buffer{
    struct Mensaje *mensajes;
};

