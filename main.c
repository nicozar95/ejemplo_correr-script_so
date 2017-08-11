#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(void){

  #define SIZE 1024
  /*
  Este es un ejemplo de como hacer un proceso que cree un hijito y este corra un script
  Primero lo primero, hay que crear una comunicacion entre el proceso padre y el hijo, para eso
  creo el pipeline
  */

  int pipe_padreAHijo[2];
	int pipe_hijoAPadre[2];

	pipe(pipe_padreAHijo);
	pipe(pipe_hijoAPadre);

  /*¡Momento! ¿Porque 2 pipeline? El pipeline es unidireccional, osea, si quiero que
  mi proceso padre le envie algo al hijo y que tambien reciva del hijo tengo que tener 2 pipe.

  El pipe es un canal de datos unidireccional, lo que significa que tiene 2 fds por eso a la hora de setearlos lo declaro como
  un array de enteros de 2 elementos. Esto tiene que ver con que en la pocision 0 del pipe esta para
  lectura y la pocision 1 esta para escritura.

  Bueno eso es como se arma un pipe, ahora vamos a forkear
  */
  pid_t pid;
	int status;
  char* buffer=malloc(SIZE);

  if ((pid=fork()) == 0 )
  /*Con esa sentencia ya cree un proceso hijo, cuando lo crea generalmente lo que devuelve
  es el 0 si todo salio bien, no es el PID del proceso sino el PID para el padre*/

  /*Ambos ejecutan independientemente de uno y el otro*/

  {/*Aca estamos ejecutando lo que va a correr el proceso hijo*/

    dup2(pipe_padreAHijo[0],STDIN_FILENO);
		dup2(pipe_hijoAPadre[1],STDOUT_FILENO);

    /*Esto es super importante, lo que hace dup2() es duplicar un fd creandome una copia del mismo
    donde yo le diga, en este caso estoy cambiando el fd de la entrada estandar del proceso hijo (STDIN_FILENO)
    y lo estoy reemplazando por el pipe padre->hijo (pipe_padreAHijo[0]). Tambien le estoy cambiando
    el fd de la salida estadar del hijo (STDOUT_FILENO), por el pipe hijo -> padre (pipe_hijoAPadre[1])*/
    /*Acuerdense de la regla de oro para los pipes: 0 es lectura, 1 es escritura*/

    close( pipe_padreAHijo[1] );
		close( pipe_hijoAPadre[0] );
		close( pipe_hijoAPadre[1]);
		close( pipe_padreAHijo[0]);

    /*Como ya cree la copia puedo cerrar el resto de los fd, en este caso las puntas de los pipes.
    No tengan miedo de cerrar archivos en este parte del codigo porque estamos en el hijo, generalmente es de buena
    costumbre cerrar los fds en el hijo porque es una copia igual de lo que tiene el padre y puede
    generar problemas*/

    system("./script_preparacion.py");
    exit(1);

    /*Listo, fin del hijo.
    Un par de items a destacar:

      + Fijarse que el script tenga permisos de ejecucion: parece una boludez pero mas de una vez me
      paso que no me andaba porque no tenia ese permiso.
      Hay gente que cuando recive el script, ya le agrega los permisos de una. Si el script
      lo van a recivir por sockets lo mas probable es que ustedes tambien tengan que hacer eso.
      (No es nada complicado solo un chmod())

      + system(): ejecuta el comando que le pases como argumento, en este caso yo le digo que corra
      el script nada mas.
      ¿Pero el script no recive algo, un archivo a preparar no?
      Ahhhh ahi esta la magia de esto, el script recive el archivo por entrada estandar (STDIN).
      Ese archivo es el que el padre le va a mandar al hijo... mas adelante, cuando estemos en
      el padre muestro como se lo manda
      ¿Y donde va el resultado del script?
      ¡Por salida estandar! Despues el padre lo va a tener que recibir

      Todos los scripts que armemos van a ser por entrada/salida estandar.

      + exit(): Exit mata al proceso, generalmente cuando hacemos una funcion o algo parecido metemos
      un return, aca no estamos en una funcion ni en un hilo, estamos en un proceso hijo. Por eso es
      necesario matarlo con un exit al finalizar para no generar problemas.

      + execve(): Este es otra manera de hacer lo que hice arriba, funciona muy parecido a system() y mata
      al proceso cuando termina.
    */
  }else{
    /*Mucho hijo vamos con el padre*/

    close( pipe_padreAHijo[0] );
    close( pipe_hijoAPadre[1] );
    /*Aca solo cierro los fds que no me interesan porque son los que esta usando el hijo,
    me quedan los otros 2 que son los que uso para escribir en el proceso hijo y para leer
    lo que me tenga que devolver*/

    write( pipe_padreAHijo[1],"hola pepe",strlen("hola pepe"));
    /*Asi de sencillo es escribir en el proceso hijo, cuando el hijo lo reciva, lo va a recibir como
    entrada estandar*/

    close( pipe_padreAHijo[1]);
    /*Ya esta, como termine de escribir cierro esta parte del archivo*/

    waitpid(pid,&status,0);
    /*Esto es el "mata zombies", lo que hace waitpid es esperar a que el proceso hijo termine.
    Mientras tanto el padre se queda bloqueado. Si vos no haces esto, cuando el hijo termine la ejecucion
    por exit() no va a poder irse ya que la entrada del proceso hijo en la tabla de procesos del sistema operativo
    sigue estando para que el padre lea la salida estandar. El proceso hijo se encuentra en un estado de terminacion
    que se lo conoce como "zombie", tecnicamente termino porque hizo un exit pero sigue vivo en la tabla de procesos.
    */

    read( pipe_hijoAPadre[0], buffer, SIZE );
    close( pipe_hijoAPadre[0]);
    /*Listo asi de sencillo leo de un proceso hijo, ahora el resultado de mi script se encuentra en
    "buffer" y tiene un tamaño SIZE*/
  }

  /*Ya esta, tengo mi resultado en buffer ¿Que puedo hacer con eso? Escribirlo en un archivo por ejemplo ;)*/
  FILE* fd = fopen("/tmp/resultado","w");
  fputs(buffer,fd);
  fclose(fd);

  /*Eso es todo! Fijense que a la hora de agregacion de un archivo es exactamente igual:
  Crean 2 pipes, forkean, corren el script, guardan el resultado.*/
  return 0;
}
