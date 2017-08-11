# Ejemplo de fork
Este es un ejemplo de un programa en C que levanta un proceso hijo para ejecutar un script en python.

## Setup
Agregen los permisos de ejecucion al script de python con:
```
chmod +x script_preparacion.py
```
Compilen ejecutando:
```
gcc main.c -o fork
```
Y para ejecutarlo corran:
```
./fork
```
El resultado de correr el script se encuentra en el ```/tmp/resultado```

