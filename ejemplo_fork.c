#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(void){

  #define SIZE 1024
  /*
  Este es un ejemplo de como hacer un proceso que cree un hijito y este corra un script.
  El script que estamos corriendo es un script en python me cuenta la cantidad de palabras por entrada estandar.
  Mi objetivo en este ejemplo es que al enviar "hola pepe", este programa me cree un archivo con el resultado del script:

  "
  hola 1
  pepe 1
  "

  Es un script de python pero tranquilamente puede ser un script de bash, pearl, etc.

  Primero lo primero, hay que crear una comunicacion entre el proceso padre y el hijo, para eso
  creo el pipeline

  El pipeline o pipe es un canal de datos unidireccional donde de un lado tengo un fd que lee o el fd en la pocision 0
  y del otro lado un fd que escribe o el fd en la pocision 1, todo lo que escriba en el lado 1 del pipe se queda en un buffer
  hasta ser leido en el lado 0. Esta es la manera que tengo de comunicar un procesos por medio de estos fd raros.
  */

  int pipe_padreAHijo[2];
  int pipe_hijoAPadre[2];

  pipe(pipe_padreAHijo);
  pipe(pipe_hijoAPadre);

  /*¡Momento! ¿Porque 2 pipeline? El pipeline es unidireccional, osea, si quiero que
  mi proceso padre le envie algo al hijo y que tambien reciva del hijo tengo que tener 2 pipes.

  El pipe es un canal de datos unidireccional, lo que significa que tiene 2 fds por eso a la hora de setearlos lo declaro
  como un array de enteros de 2 elementos. Acuerdense que la pocision 0 del pipe es de lectura y la pocision 1 es de escritura

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
    y lo estoy reemplazando por el pipe padre->hijo (pipe_padreAHijo[0], osea donde debe leer de lo que el padre le escribe).
    Tambien le estoy cambiando el fd de la salida estadar del hijo (STDOUT_FILENO), por el pipe hijo -> padre
    (pipe_hijoAPadre[1], donde el hijo escribe para que el padre despues lea)*/

    /*Acuerdense de la regla de oro para los pipes: 0 es lectura, 1 es escritura*/

   	close( pipe_padreAHijo[1] );
  	close( pipe_hijoAPadre[0] );
	close( pipe_hijoAPadre[1]);
	close( pipe_padreAHijo[0]);

    /*¿Porque cerramos todos los pipes? Generalmente cuando forkeamos un proceso, el proceso hijo "hereda" una copia de
    todos los fds del padre, esto puede llegar a traer problemas si no sabes que fds heredaste.
    En este caso, el pipe se define antes de correr el proceso hijo (se debe definir antes de correr el proceso hijo), por lo tanto
    acabamos de duplicar la cantidad de fds que tenemos en el programa, porque tenemos una copia del mismo en el hijo y el
    original en el padre, osea cada proceso tiene acceso a ambos lados del pipe, lo cual no es lo deseable, si quiero escribir al padre
    quiero que solo el padre lo lea.
    Tambien hay otros tipos de problemas:

    + El menos grave es el caso de que el sistema operativo no nos deje tener mas archivos abiertos, pero es muy raro tener
    ese error hoy en dia.

    + El otro problema que es el mas grave es con la señal de error SIGPIPE, ese error va de la mano de que se cayo un lado del pipe
    por X razon, entonces cuando quiere notificar el error no sabe en que fd mandarlo y va a traer problemas.

    Es mas sencillo cerrar los fds que el hijo no este usando para evitar confusiones.
    */

      char *argv[] = {NULL};
      char *envp[] = {NULL};
      execve("./script_preparacion.py", argv, envp);
    	exit(1);

    /*Listo, fin del hijo.
    Un par de items a destacar:

      + Fijarse que el script tenga permisos de ejecucion: parece una boludez pero mas de una vez me
      paso que no me andaba porque no tenia ese permiso.
      Hay gente que cuando recive el script, ya le agrega los permisos de una. Si el script
      lo van a recivir por sockets lo mas probable es que ustedes tambien tengan que hacer eso.
      (No es nada complicado solo un chmod())

      + execve(): Miembro de la familia exec() de funciones de Linux. Ejecuta el comando que le pases como argumento,
      en este caso yo le digo que corra el script nada mas, hay muchas otras funciones de la familia exec() son
      libres de usar la que quieran. Los 2 argumentos hacen uso de las variables que recive el script tanto como argumentos y globales,
      Fijense que no pueden ser NULL pero pueden ser un un sring terminado en NULL. Hay otras funciones de la familia exec(),
      son libres de elegir la que quieran.

      ¿Pero el script no recive algo, un archivo a preparar o una cantidad de datos no?
      Ahhhh ahi esta la magia de esto, el script recive algo por entrada estandar (STDIN).
      Esos datos son los que el padre le va a mandar al hijo... mas adelante, cuando estemos en el padre muestro como se lo manda
      ¿Y donde va el resultado del script?
      ¡Por salida estandar! Despues el padre lo va a tener que recibir

      Todos los scripts que armemos van a ser por entrada/salida estandar.

      + exit(): Exit mata al proceso, generalmente cuando hacemos una funcion o algo parecido metemos
      un return, aca no estamos en una funcion ni en un hilo, estamos en un proceso hijo. Por eso es
      necesario matarlo con un exit al finalizar para no generar problemas.

      + system(): Este es otra manera de hacer lo que hice arriba, funciona muy parecido a exec(), pero a diferencia
      de exec(), no me reemplaza el proceso, sino que me genera otro. Puede causar mas overhead y blah blah pero son libres
      de usarlo porque tambien tiene un par de trucos bajo la manga si son cancheros con bash y los comandos de linux.
    */
  }else{
    /*Mucho hijo vamos con el padre*/
	close( pipe_padreAHijo[0] ); //Lado de lectura de lo que el padre le pasa al hijo.
    	close( pipe_hijoAPadre[1] ); //Lado de escritura de lo que hijo le pasa al padre.
    /*Aca cierro los fds que no me interesan porque son los que esta usando el hijo, tambien los cierros por el tema del
    duplicado explicado mas arriba. Me quedan los otros 2 que son:

    + pipe_padreAHijo[1] -> Lado de escritura de lo que el padre le pasa al hijo

    + pipe_hijoAPadre[0] -> Lado de lectura de lo que el hijo le pasa al padre
    */

    	write( pipe_padreAHijo[1],"hola pepe",strlen("hola pepe"));
    /*Asi de sencillo es escribir en el proceso hijo, cuando el hijo lo reciva, lo va a recibir como
    entrada estandar. En este caso le estoy mandando "hola pepe" al script de python*/

    	close( pipe_padreAHijo[1]);
    /*Ya esta, como termine de escribir cierro esta parte del pipe*/

    	waitpid(pid,&status,0);
    /*Esto es el "mata zombies", lo que hace waitpid es esperar a que el proceso hijo termine. Se logra el mismo resultado con wait()

    Tecnicamente espera el cambio de estado del proceso hijo (pid) y guarda el trace en ejecucion (lo guarda en status, segun lo
    que yo quiero consultar puedo consultar, por ejemplo, si el hijo recibio una señal, cual fue su exitcode, etc... a nosotros no nos interesa nada
    de eso por eso no lo vamos a usar y ponemos en el ultimo argumento, que son las opciones de trace, en 0 para decirle eso).

    Mientras tanto el padre se queda bloqueado. Si vos no haces esto, cuando el hijo termine la ejecucion
    por exit() no va a poder irse ya que la entrada del proceso hijo en la tabla de procesos del sistema operativo
    sigue estando para que el padre lea el codigo de salida. El proceso hijo se encuentra en un estado de terminacion
    que se lo conoce como "zombie", tecnicamente esta muerto porque hizo un exit pero sigue vivo en la tabla de procesos.

    Esto se debe que la llamada de wait() o waitpid() permite al padre leer el exitcode o codigo de salida del hijo. Si el padre nunca se entero
    que el hijo esta muerto, el SO no lo puede sacar (salvo a la fuerza).

    ¿Que problemas trae un proceso zombie? No muchos en cuestion de memoria porque no usan recursos del systema, sin embargo tienen un PID
    asignado por el SO, el cual el sistema operativo tiene un numero finitos de estos. Un zombie no causa muchos problemas.
    Varios zombies me limitan la cantidad de procesos que puedo ejecutar, si estan debuggeando un programa que te cree
    zombies sin querer te puede limitar el numero de procesos disponibles. Por eso es importante matarlos

    Sigamos con el ejemplo.
    */
    	read( pipe_hijoAPadre[0], buffer, SIZE );
    	close( pipe_hijoAPadre[0]);
    /*Listo asi de sencillo leo de un proceso hijo, ahora el resultado de mi script se encuentra en
    "buffer" y tiene un tamaño SIZE. Como termine de leer cierro el extremo del pipe*/
  }

  /*Ya esta, tengo mi resultado en buffer ¿Que puedo hacer con eso? Escribirlo en un archivo por ejemplo ;)*/
  FILE* fd = fopen("/tmp/resultado","w");
  fputs(buffer,fd);
  fclose(fd);

  /*Eso es todo! Fijense que a la hora de agregacion de un archivo es exactamente igual:
  Crean 2 pipes, forkean, corren el script, guardan el resultado.*/
  free(buffer);
  return 0;
}
