/*
Proyecto 2. Sistemas Operativos 2019-3 MiTwitter
Archivo: cliente.c
Realizado por: Juan Camilo Chafloque, Cristobal Castrillon y Jorge Salgado
Contiene: Implementación del cliente con sus respectivas funciones
*/

#include "info.h"

typedef void (*sighandler_t)(int);
sighandler_t signalTweets (void){

    struct Mensaje tweet;
    read(fdSignal, &tweet, sizeof(struct Mensaje));
    printf("El usuario con Id %i envío un mensaje: \n", tweet.idEmisor);
    printf("%s \n", tweet.mensaje);
}

void abrirPipeR(char* nomPipe, int *fd);
void abrirPipeW(char* nomPipe, int *fd);

int main (int argc,char  *argv[]){

    int fd, fd1, cuantos, opcion, pId;
    char mensaje[200];
    struct Data misDatos;
    struct Mensaje miTweet;
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    if(argc != 5){
        perror("Número de argumentos invalidos. \n");
        perror("El comando para invocar al cliente es: $Cliente -i <id> -p <pipeNom> \n");
        exit(1);       
    }

    if(atoi(argv[2]) > 10 || atoi(argv[2]) < 1){
        perror("Id invalido.");
        perror("El Id debe ser un número entre 1 y 10.");
        exit(1);
    }

    if(strcmp(argv[1], "-i") != 0 ){
        perror("Comando invalido para la asociación de un Id. \n");
        perror("El comando es -i \n");
        exit(1);
    }

    if(strcmp(argv[3], "-p") != 0 ){
        perror("Comando invalido para la asociación del pipe. \n");
        perror("El comando es -p \n");
        exit(1);
    }

    abrirPipeW(argv[4], &fd);
    printf("Se abrió el pipe nominal de escritura %s \n", argv[4]);

    misDatos.idUsuario = atoi(argv[2]);
    misDatos.conectado = false;
    misDatos.recibido = false;
    misDatos.pid = getpid();
    misDatos.opcion = -1;
    misDatos.follow = 0;
    misDatos.unfollow = 0;
    misDatos.fd = 0;
    strcpy(misDatos.pipeId, "pipeId");
    strcat(misDatos.pipeId, argv[2]);

    write(fd, &misDatos, sizeof(struct Data));
    printf("Se enviaron los datos iniciales de inicio de sesión. \n");
    sleep(1);

    do{
        unlink(misDatos.pipeId);
    }while(mkfifo(misDatos.pipeId, fifo_mode) == - 1);

    abrirPipeR(misDatos.pipeId, &fd1);
    fdSignal = fd1;
    printf("Se abrió el pipe nominal de lectura %s \n", misDatos.pipeId);

    do{
        cuantos = read (fd1, &misDatos, sizeof(struct Data));
        if(cuantos == -1)
            perror("Cliente.");
    }while(cuantos == -1);

    if(misDatos.recibido == true){
        printf("Bienvenido. Su sesión ha iniciado. \n");
        signal(SIGUSR1, (sighandler_t) &signalTweets);
        do{
            printf("Su id de sesión es: %i \n", misDatos.idUsuario);
            printf("-----Bienvenido a miTwitter----- \n");
            printf("1. Seguir a un usuario. \n");
            printf("2. Dejar de seguir a un usuario. \n");
            printf("3. Enviar mensaje a seguidores. \n");
            printf("4. Desconectarse. \n");
            printf("Ingrese una opción. \n ");
            scanf("%i", &opcion);
            switch(opcion){

                case 1:
                    misDatos.opcion = 1;
                    printf("Digite el Id del usuario a seguir. \n");
                    scanf("%i", &pId);
                    if(pId < 1 || pId > 10){
                        printf("El Id ingresado no es valido. Tiene que ser un número entre 1 y 10. \n");
                    }
                    else{
                        misDatos.follow = pId;
                        write(fd, &misDatos, sizeof(struct Data));
                        do{
                            cuantos = read (fd1, &mensaje, 200);
                            if(cuantos == -1)
                            perror("Cliente.");
                        }while(cuantos == -1);
                        printf("%s \n", mensaje);
                    }

                    break;

                case 2:
                    misDatos.opcion = 2;
                    printf("Digite el Id del usuario a dejar de seguir. \n");
                    scanf("%i", &pId);
                    if(pId < 1 || pId > 10){
                        printf("El Id ingresado no es valido. Tiene que ser un número entre 1 y 10. \n");
                    }
                    else{
                        misDatos.unfollow = pId;
                        write(fd, &misDatos, sizeof(struct Data));
                        do{
                            cuantos = read (fd1, &mensaje, 200);
                            if(cuantos == -1)
                            perror("Cliente.");
                        }while(cuantos == -1);
                        printf("%s \n", mensaje);
                    }
                
                    break;

                case 3:
                    misDatos.opcion = 3;
                    miTweet.idEmisor = misDatos.idUsuario;
                    miTweet.enviado = false;
                    miTweet.idReceptor = -1;
                    printf("Digita el tweet a enviar. \n");
                    scanf("%s", miTweet.mensaje);
                    write(fd, &misDatos, sizeof(struct Data));
                    write(fd, &miTweet, sizeof(struct Mensaje));
                    do{
                        cuantos = read (fd1, &mensaje, 200);
                        if(cuantos == -1)
                            perror("Cliente.");
                    }while(cuantos == -1);
                    printf("%s \n", mensaje);
                
                    break;

                case 4:
                    misDatos.opcion = 4;
                    write(fd, &misDatos, sizeof(struct Data));
                    do{
                        cuantos = read (fd1, &mensaje, 200);
                        if(cuantos == -1)
                            perror("Cliente.");
                    }while(cuantos == -1);
                    printf("%s \n", mensaje);
                    close(fd);
                    close(fd1);
                    return 0;

                    break;

                default:
                    printf("Opción Invalida. Digite nuevamente. \n");
                    
                    break;
            }
        }while(opcion != 4);
    }
    else{
        printf("Error iniciando sesión. Ya se encuentra una sesión iniciada. \n");
        return 0;
    }
}

/*
 * Función: abrirPipeR
 * Abre un pipe en modo lectura
 */
void abrirPipeR(char *nomPipe, int *fd){

    int creado = 0;
    do{
        *fd = open(nomPipe, O_RDONLY);
        if (*fd != -1){
            creado = 1;
        }
    }while (creado == 0);
}

/*
 * Función: abrirPipeW
 * Abre un pipe en modo escritura
 */
void abrirPipeW(char *nomPipe, int *fd){

    int creado = 0;
    do{
        *fd = open(nomPipe, O_WRONLY);
        if (*fd != -1){
            creado = 1;
        }
    }while (creado == 0);
}