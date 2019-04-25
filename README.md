# CorreAlonso
El programa propuesto constará de un único fichero fuente, falonso2.cpp, cuya adecuada compilación producirá el ejecutable falonso2.exe. Por favor, mantened los nombres tal cual, incluida la extensión. Se trata de realizar una práctica casi idéntica a la primera práctica de este año, pero mediante un programa que realice llamadas a la API de WIN32. 

Las principales diferencias respecto a la práctica primera son:
Se proporcionará una Biblioteca de Enlazado Dinámico (DLL) en lugar de la biblioteca estática libfalonso.a. La biblioteca se llamará falonso2.dll.
Se realizará la práctica mediante hilos, uno para cada coche, además del hilo principal.
En lugar del array de semáforos o buzones de mensajes se usarán los semáforos independientes u otros mecanismos de sincronización de WIN32 que se estimen convenientes.
Aparece dos nuevas funciones de la biblioteca de prototipo:
int FALONSO2_estado_semAforo(int direcciOn): Esta función devuelve el estado del semáforo que se le especifica como argumento (HORIZONTAL o VERTICAL). El estado devuelto será ROJO, AMARILLO, VERDE o NEGRO.
int FALONSO2_posiciOn_ocupada(int carril, int desp): Verdadera, si la posición correspondiente al carril y desplazamiento está ocupada por un coche. Falsa, en caso contrario.
La práctica no funcionará indefinidamente hasta que se pulse CTRL+C, sino que durará 30 segundos, transcurridos los cuales, acabará, no sin antes comprobar que la cuenta de vueltas de la biblioteca coincide con la realizada por vuestro programa.
No estará permitida la semiespera ocupada. Se deben bloquear completamente a los hilos que no puedan avanzar.
