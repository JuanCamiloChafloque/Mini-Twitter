/*
Proyecto 2. Sistemas Operativos 2019-3 MiTwitter
Archivo: gestor.c
Realizado por: Juan Camilo Chafloque, Cristobal Castrillon y Jorge Salgado
Contiene: Implementación del gestor con sus respectivas funciones
*/

#include "info.h"

void leerRelaciones(char* nomArchivo);
void abrirPipeR(char* nomPipe, int *fd);
void abrirPipeW(char* nomPipe, int *fd);
int buscarUsuario(int pId);
void conectarUsuario(struct Data *datos);
int follow(struct Data *datos);
int unfollow(struct Data *datos);
void desconectar(int pId);
void enviarMensajeSeguidores(struct Data datosDe, struct Data datosPara, struct Mensaje tweet);
void guardarMensajeBuffer(struct Mensaje *tweet);
void verificarBuffer(struct Data datos);

int main (int argc,char  *argv[]){

    int fd, fd1, cuantos, fol, unf;
    char mensaje[200];
    struct Data datos;
    struct Mensaje tweet;
    buffer.mensajes = (struct Mensaje *) calloc(100, sizeof(struct Mensaje));
    informacionUsuarios = (struct Data *) calloc(10, sizeof(struct Data));
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    contConectados = 0;
    contPendientes = 0;

    if(argc != 5){
        perror("Número de argumentos invalidos. \n");
        perror("El comando para invocar al gestor es: $Gestor -r <relaciones> -p <pipeNom> \n");
        exit(1);
    }

    if(strcmp(argv[1], "-r") != 0 ){
        perror("Comando invalido para la matriz de relaciones. \n");
        perror("El comando es -r \n");
        exit(1);
    }

    if(strcmp(argv[3], "-p") != 0 ){
        perror("Comando invalido para la creación del pipe. \n");
        perror("El comando es -p \n");
        exit(1);
    }

    leerRelaciones(argv[2]);
    printf("Se cargaron las relaciones correctamente. \n");

    do{
        unlink(argv[4]);
    }while(mkfifo(argv[4], fifo_mode) == - 1);

    abrirPipeR(argv[4], &fd);
    printf("Se abrió el pipe nominal de lectura %s \n", argv[4]);

    while(1){

        cuantos = read(fd, &datos, sizeof(struct Data));

        if( cuantos == -1 ){
            perror("Gestor.");
            exit(1);
        }
        //Si el pipe leido tiene como opción el número -1 significa que no hay peticiones y el 
        //usuario se quiere conectar al sistema
        if(datos.opcion == -1){
            printf("Gestor intentando conectar al usuario con Id %i \n", datos.idUsuario);
            unlink(datos.pipeId);
            abrirPipeW(datos.pipeId, &fd1);
            printf("Se abrió el pipe nominal de escritura %s \n", datos.pipeId);
            datos.fd = fd1;
            conectarUsuario(&datos);
            write(datos.fd, &datos, sizeof(struct Data));
            verificarBuffer(datos);
        }
        //Si el pipe leido tiene como opción el número 1 significa que la petición es para hacer 
        //un follow
        else if(datos.opcion == 1){
            printf("El usuario con Id %i solicitó seguir al usuario con Id %i. \n", datos.idUsuario, datos.follow);
            fol = follow(&datos);
            if(fol == 0){
                strcpy(mensaje, "Warning: No se pudó seguir al usuario. Ya sigues a este usuario \n");
            }else{
                strcpy(mensaje, "Has comenzado a seguir al usuario. \n");
            }
            write(datos.fd, &mensaje, strlen(mensaje) + 1);
        }
        //Si el pipe leido tiene como opción el número 2 significa que la petición es para hacer 
        //un unfollow
        else if(datos.opcion == 2){
            printf("El usuario con Id %i solicitó dejar de seguir al usuario con Id %i. \n", datos.idUsuario, datos.unfollow);
            unf = unfollow(&datos);
            if(unf == 0){
                strcpy(mensaje, "Warning: No se pudó dejar de seguir al usuario. Tu no sigues a este usuario \n");
            }else{
                strcpy(mensaje, "Has dejado de seguir al usuario. \n");
            }
            write(datos.fd, &mensaje, strlen(mensaje) + 1);
            
        }
        //Si el pipe leido tiene como opción el número 3 significa que la petición es para enviar 
        //un tweet
        else if(datos.opcion == 3){
            read(fd, &tweet, sizeof(struct Mensaje));
            printf("El usuario con Id %i solicitó enviar un mensaje a sus seguidores. \n", tweet.idEmisor);
            printf("El mensaje a enviar es: %s \n", tweet.mensaje);
            for(int i = 0; i < 10; i++){
                if(relaciones[i][datos.idUsuario - 1] == 1){
                    int posActual = buscarUsuario(i + 1);
                    if(informacionUsuarios[posActual].conectado == true){
                        enviarMensajeSeguidores(datos, informacionUsuarios[posActual], tweet);
                    }
                    else{
                        tweet.idReceptor = i + 1;
                        guardarMensajeBuffer(&tweet);
                    }
                }
            }
        }
        //Si el pipe leido tiene como opción el número 4 significa que la petición es para 
        //desconectarse del sistema
        else if(datos.opcion == 4){
            printf("El usuario con Id %i solicitó desconectarse. \n", datos.idUsuario);
            desconectar(datos.idUsuario);
            strcpy(mensaje, "Su sesión se ha cerrado correctamente. \n");
            write(datos.fd, &mensaje, strlen(mensaje) + 1);
        }
    }
}

/* 
 * Función: leerRelaciones
 * Inicializa la matriz de relaciones dado un archivo de texto con los seguidores iniciales de cada
 * usuario.
 */
void leerRelaciones(char* nomArchivo){

    int row = 0;
    int col = 0;
    char cadena[10];
    FILE *stream;
    stream = fopen(nomArchivo, "r");

    if(stream == NULL){
        perror("No se pudo abrir el archivo de relaciones.");
        exit(1);
    }

    while( !feof(stream) ){
        fscanf(stream, "%s", cadena);
        if(cadena != " "){
            relaciones[row][col] = atoi(cadena);
            col++;
            if(col == 10){
                row++;
                col = 0;
            }
        }
    }
    fclose(stream);
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

/*
 * Función: buscarUsuario
 * Busca la posición actual que ocupa el usuario y retorna dicha posición 
 */
int buscarUsuario(int pId){

    int pos = -1;

    for(int i = 0; i < 10; i++){
        if( informacionUsuarios[i].idUsuario == pId ){
            pos = i;
            break;
        }
    }

    return pos;
}

/*
 * Función: conectarUsuario
 * Verifica si el usuario existe o no, en dado caso que no exista, lo agrega a la lista de usuarios
 * del sistema y lo conecta al sistema. Si existe pero no está conectado al sistema lo vuelve a
 * conectar y si existe y está conectado, muestra un mensaje que el usuario ya está conectado
 */
void conectarUsuario(struct Data *datos){

    int encontrado = 0;

    if(contConectados == 0){
        datos->recibido = true;
        datos->conectado = true;
        informacionUsuarios[contConectados] = *datos;
        contConectados++;
        printf("El usuario con el id %i se conectó por primera vez al sistema. \n", datos->idUsuario);
    }
    else{
        for(int i = 0; i < contConectados; i++){
            if(informacionUsuarios[i].idUsuario == datos->idUsuario && informacionUsuarios[i].conectado == true){
                encontrado++;
                datos->recibido = false;
                printf("El usuario con el id %i ya se encuentra conectado en el sistema. \n", datos->idUsuario);
            }
            else if(informacionUsuarios[i].idUsuario == datos->idUsuario && informacionUsuarios[i].conectado == false){
                encontrado++;
                datos->recibido = true;
                datos->conectado = true;
                informacionUsuarios[i] = *datos;
                printf("El usuario con el id %i se conectó al sistema correctamente. \n", datos->idUsuario);
            }
        }
        if(encontrado == 0){
            datos->recibido = true;
            datos->conectado = true;
            informacionUsuarios[contConectados] = *datos;
            contConectados++;
            printf("El usuario con el id %i se conectó por primera vez al sistema. \n", datos->idUsuario);
        }
    }

    printf("Usuarios conectados en el sisema: \n");
    for(int i = 0; i < contConectados; i++){
        if(informacionUsuarios[i].conectado == true)
            printf("Id: %i \n", informacionUsuarios[i].idUsuario);
    }
}

/*
 * Función: desconectar
 * Desconecta a un determinado usuario temporalmente del sistema.
 */
void desconectar(int pId){

    int pos = -1;
    pos = buscarUsuario(pId);
    if(pos != -1){
        informacionUsuarios[pos].conectado = false;
        printf("El usuario con el Id %i se desconectó correctamente. \n", pId);
    }
    else{
        printf("El usuario con el Id %i no existe en el sistema. \n", pId);
    }
}

/*
 * Función: follow
 * Verifica que el usuario no este siguiendo al usuario que quiere seguir. Si no lo está siguiendo
 * se cambia la relación en la matriz. Si si lo está siguiendo se le informa dicho evento.
 */
int follow(struct Data *datos){

    int correcto = -1;
    int pos = -1;
    int idSeguir = -1;
    idSeguir = datos->follow - 1;
    pos = datos->idUsuario - 1;

    if(relaciones[pos][idSeguir] == 1){
        printf("Warning: El usuario con Id %i ya está siguiendo al usuario con Id %i \n", datos->idUsuario, datos->follow);
        correcto = 0;
    }
    else{
        relaciones[pos][idSeguir] = 1;
        printf("El usuario con Id %i comenzó a seguir al usuario con Id %i \n", datos->idUsuario, datos->follow);
        correcto = 1;
    }

    return correcto;
}

/*
 * Función: unfollow
 * Verifica que el usuario este siguiendo al usuario que quiere dejar de seguir. Si lo está siguiendo
 * se cambia la relación en la matriz. Si no lo está siguiendo se le informa dicho evento.
 */
int unfollow(struct Data *datos){

    int correcto = -1;
    int pos = -1;
    int idNoSeguir = -1;
    idNoSeguir = datos->unfollow - 1;
    pos = datos->idUsuario - 1;

    if(relaciones[pos][idNoSeguir] == 0){
        printf("Warning: El usuario con Id %i no está siguiendo al usuario con Id %i \n", datos->idUsuario, datos->unfollow);
        correcto = 0;
    }
    else{
        relaciones[pos][idNoSeguir] = 0;
        printf("El usuario con Id %i dejó de seguir al usuario con Id %i \n", datos->idUsuario, datos->unfollow);
        correcto = 1;
    }

    return correcto;
}

/*
 * Función: enviarMensajeSeguidores
 * Se envía un mensaje de 200 caracteres al usuario que sigue al usuario determinado
 */
void enviarMensajeSeguidores(struct Data datosDe, struct Data datosPara, struct Mensaje tweet){

    char mensaje[200];
    printf("Enviando mensaje al usuario conectado con Id %i \n", datosPara.idUsuario);
    kill(datosPara.pid, SIGUSR1);
    write(datosPara.fd, &tweet, sizeof(struct Mensaje));
    strcpy(mensaje, "Se envío el mensaje a tu seguidor. \n");
    write(datosDe.fd, &mensaje, strlen(mensaje) + 1);
}

/*
 * Función: guardarMensajeBuffer
 * Si se quiere enviar a un usuario que se encuentra desconectado entonces se guarda en el buffer de
 * mensajes pendientes.
 */
void guardarMensajeBuffer(struct Mensaje *tweet){

    printf("Guardando mensaje al usuario desconectado con Id %i \n", tweet->idReceptor);
    buffer.mensajes[contPendientes] = *tweet;
    contPendientes++;
}

/*
 * Función: verificarBuffer
 * Al conectarse un usuario al sistema se verifica si dicho usuario no tiene mensajes pendientes de 
 * algún seguidor
 */
void verificarBuffer(struct Data datos){

    int pId = datos.idUsuario;
    int posRec = buscarUsuario(pId);

    for(int i = 0; i < contPendientes; i++){
        if(buffer.mensajes[i].idReceptor == pId && buffer.mensajes[i].enviado == false){
            printf("Enviando mensaje guardado al usuario con Id %i \n", pId);
            int posEmi = buscarUsuario(buffer.mensajes[i].idEmisor);
            enviarMensajeSeguidores(informacionUsuarios[posEmi], informacionUsuarios[posRec], buffer.mensajes[i]);
            buffer.mensajes[i].enviado = true;
        }
    }
}
