# CorreAlonso
El programa propuesto constará de un único fichero fuente, falonso2.cpp, cuya adecuada compilación producirá el ejecutable falonso2.exe. 

Las principales diferencias respecto a la práctica primera son:
Se proporcionará una Biblioteca de Enlazado Dinámico (DLL) en lugar de la biblioteca estática libfalonso.a. La biblioteca se llamará falonso2.dll.

Se realizará la práctica mediante hilos, uno para cada coche, además del hilo principal.

En lugar del array de semáforos o buzones de mensajes se usarán los semáforos independientes u otros mecanismos de sincronización de WIN32 que se estimen convenientes.

*Aparece dos nuevas funciones de la biblioteca de prototipo:

  int FALONSO2_estado_semAforo(int direcciOn): Esta función devuelve el estado del semáforo que se le especifica como argumento (HORIZONTAL     o VERTICAL). El estado devuelto será ROJO, AMARILLO, VERDE o NEGRO.

  int FALONSO2_posiciOn_ocupada(int carril, int desp): Verdadera, si la posición correspondiente al carril y desplazamiento está ocupada       por un coche. Falsa, en caso contrario.
  
  
La práctica no funcionará indefinidamente hasta que se pulse CTRL+C, sino que durará 30 segundos, transcurridos los cuales, acabará, no sin antes comprobar que la cuenta de vueltas de la biblioteca coincide con la realizada por vuestro programa.
No estará permitida la semiespera ocupada. Se deben bloquear completamente a los hilos que no puedan avanzar.

## LPEs
- [ ] No debéis usar la función TerminateThread para acabar con los hilos o TerminateProcess para acabar con los procesos. El problema de estas funciones es que están diseñadas para ser usadas solo en condiciones excepcionales y los hilos mueren abruptamente. Pueden dejar estructuras colgando, ir llenando la memoria virtual del proceso con basura o no invocar adecuadamente las funciones de descarga de la DLL. 

- [ ] Al ejecutar la práctica, no puedo ver lo que pasa, porque la ventana se cierra justo al acabar.

- [ ] Para evitar esto, ejecutad la práctica desde el "Símbolo del sistema", que se encuentra en el menú de "Accesorios". También es necesario que la ventana que uséis tenga un tamaño de 80x25 caracteres. Si no lo tenéis así, cambiadlo en el menú de propiedades de la ventana. 

- [ ] Los caracteres de la práctica no aparecen correctamente.

- [ ] La DLL puede usar algunos caracteres gráficos para la presentación. Para que funcionen correctamente, la ventana del "Símbolo del sistema" debe usar una fuente que los incluya. Probad a cambiarla en la ventana de propiedades hasta hallar una correcta en vuestro sistema.
- [ ] Al ejecutar la función LoadLibrary, en lugar de aparecer la pantalla de presentación, aparece un mensaje que pone "En DllMain".

- [ ] Es necesario que la ventana que uséis tenga un tamaño de 80x25 caracteres. Si no lo tenéis así, cambiadlo en el menú de propiedades de la ventana. 

- [ ] Cuando ejecuto la práctica depurando la pantalla se emborrona. ¿Cómo lo puedo arreglar?

- [ ] Mejor depurad la práctica enviando la información de trazado escrita con fprintf(stderr,...) a un fichero, añadiendo al final de la línea de órdenes 2>salida. De este modo, toda la información aparecerá en el fichero salida para su análisis posterior. No os olvidéis de incluir el identificador del hilo que escribe el mensaje. 

- [ ] Tengo muchos problemas a la hora de llamar a la función XXXX de la biblioteca. No consigo de ningún modo acceder a ella.

- [ ] El proceso detallado viene en la última sesión. De todos modos, soléis tener problemas en una conversión de tipos, aunque no os deis cuenta de ello. No se os dirá aquí qué es lo que tenéis que poner para que funcione, pues lo pondríais y no aprenderíais nada. Sin embargo y dada la cantidad de personas con problemas, a continuación viene una pequeña guía:
- [ ] Primero debéis definir una variable puntero a función. El nombre de la variable es irrelevante, pero podemos llamarle igual que a la función (XXXX) por lo que veremos más abajo. Para definir el tipo de esta variable correctamente, debéis conocer cómo son los punteros a función. En esta sesión, se describe una función, atexit. Dicha función en sí no es importante para lo que nos traemos entre manos, pero sí el argumento que tiene. Ese argumento es un puntero a función. Fijándoos en ese argumento, no os resultará difícil generalizarlo para poner un puntero a funciones que admiten otro tipo de parámetros y devuelve otra cosa. Notad, además, que, al contrario que ocurre con las variables normales, la definición de una variable puntero a función es especial por cuanto su definición no va solo antes del nombre de la variable, sino que lo rodea. Tenéis que poner algo similar a: #$%&%$ XXXX $%&$@;, es decir, algo por delante y algo por detrás.
- [ ] Después de cargar la biblioteca como se dice en la última sesión, debéis dar valor al puntero de función. Dicho valor lo va a proporcionar GetProcAddress. Pero, ¡cuidado!, GetProcAddress devuelve un FARPROC, que sólo funciona con punteros a funciones que devuelven int y no se les pasa nada (void). Debéis hacer el correspondiente casting. Para ello, de la definición de vuestro puntero, quitáis el nombre, lo ponéis todo entre paréntesis y lo añadís delante de GetProcAddress, como siempre.
- [ ] Ya podéis llamar a la función como si de una función normal se tratara. Ponéis el nombre del puntero y los argumentos entre paréntesis. Como os advertí más arriba, si habéis puesto XXXX como nombre al puntero, ahora no se diferenciarán en nada vuestras llamadas a la función respecto a si dicha función no perteneciera a una DLL y la hubierais programado vosotros.


- [ ] Os puede dar errores en el fichero de cabecera .h si llamáis a vuestro fichero fuente con extensión .c. Llamadlo siempre con extensión .cpp. 

- [ ] Tened mucho cuidado si usáis funciones de memoria dinámica de libc (malloc y free). Son funciones que no están sincronizadas, es decir, no se comportan bien en entornos multihilo. O bien las metéis en una sección crítica o, mejor aún, tratad de evitarlas. 

- [ ] En algunas versiones de Visual Studio os puede dar un error del tipo: error XXXXX: 'FuncionW': no se puede convertir de 'const char\[X\]' a 'LPCWSTR'. El motivo del error es que, por defecto, esa versión de Visual Studio supone que deseáis usar UNICODE (caracteres de 16 bits) en lugar de los normales (caracteres de 8 bits). La solución pasa por transformar el código fuente para que se ajuste a la programación en UNICODE de Microsoft o decirle a Visual Studio que no, que no queréis trabajar con UNICODE. Unos compañeros vuestros escriben diciendo que si en la configuración del proyecto seleccionáis "Juego de Caracteres->Sin establecer", se soluciona. 

### Pasos para que no dé problemas en Visual Studio 2008:
- [ ] Es importante que seleccionéis, al crear un proyecto nuevo, "Aplicación de consola" de Win32
- [ ] Dejad la función main tal cual la pone él: int _tmain(int argc, _TCHAR* argv[])
- [ ] En las propiedades del Proyecto, propiedades de configuración, general, a la derecha, poned "Juego de caracteres", sin establecer.
- [ ] Probad un "Hola, mundo" y, si no os da errores, los errores que aparezcan a continuación podéis confiar que provienen de que no estáis haciendo bien el acceso a la DLL. En general, o que no declaráis bien los punteros para acceder a las funciones o que no hacéis el correspondiente casting cuando llamáis a GetProAddress. 

- [ ] Tenéis que incluir el fichero de cabeceras windows.h antes que el fichero de cabeceras de la DLL
