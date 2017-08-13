#include <stdlib.h>

int main(void){
  /*
  Este es un ejemplo de como hacer un proceso que cree un hijito y este corra un script, utilizando las llamadas de system()
  El script que estamos corriendo es un script en python me cuenta la cantidad de palabras por entrada estandar. Cabe aclarar
  que no es objetivo para el desarrollo del TP la manera en la que el script funciona, sino como enviarle los datos y como recivirlos
  Mi objetivo en este ejemplo es que al enviar "hola pepe", este programa me cree un archivo con el resultado del script:

  "
  hola 1
  pepe 1
  "
  */
  system("echo hola pepe | ./script_transformacion.py > /tmp/resultado");
  /*Este es mucho mas sencillo, internamente system() hace una llamada a fork()/exec() y fijense bien
  que lo que yo le estoy pasando a system() es un comando un tanto mas complejo en bash para que me
  corra el script en python, son libres de usar algo parecido en su TP...

  */
  return 0;
}
