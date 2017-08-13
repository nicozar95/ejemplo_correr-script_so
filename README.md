# Ejemplo de fork/exec y system
Este es un ejemplo de un programa en C que levanta un proceso hijo para ejecutar un script en python.
Esta hecho de 2 maneras diferentes, una con la llamada de fork/exec y otro con la llamada de system

## Setup
Agregen los permisos de ejecucion al script de python, si el script no es ejecutable con:
```
chmod +x script_transformacion.py
```
Para compilar el ejemplo de fork/exec y ejecutarlo:
```
gcc ejemplo_fork-pipe.c -o fork
./fork
```

Para compilar el ejemplo de system y ejecutarlo:
```
gcc ejemplo_system.c -o system
./system
```
El resultado de correr el script se encuentra en el archivo```/tmp/resultado``` y es el mismo para ambos ejemplos
