#include <windows.h>
#include <winuser.h>
#include <stdio.h> 
#include <assert.h>
#include <signal.h>
#include "falonso2.h"
#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock, std::defer_lock
  
typedef int (*DLL0Arg)(void);//funciones 0 argumentos
typedef int (*DLL1Arg)(int);//funciones 1 argumento INT
typedef int (*DLL1Argp)(int*);//funciones 1 argumento INT
typedef int (*DLL2Arg)(int,int);
typedef int (*DLL3Arg)(int,int,int);
typedef int (*DLL3ArgP)(int*,int*,int);
typedef void (*DLL1Argvoid)(const char *);

/*


// unique_lock::lock/unlock
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock, std::defer_lock

std::mutex mtx;           // mutex for critical section


*/



//---------------------------------------------------------------------------
//DEFINICIONES
    unsigned long *contador=NULL;
    typedef struct Coche {
        int posicion;
        int carril;
    }coche;
    HINSTANCE hinstLib = NULL;
    DLL1Arg inicio_falonso = NULL, estadoSem = NULL;
    DLL1Argp f_fin = NULL;
    DLL2Arg luzSem =NULL, posOcup =NULL;
    DLL3Arg velocidad = NULL;
    DLL3ArgP iniCoche = NULL, avanceCoche = NULL, cambioCarril = NULL;
    DLL0Arg pausa = NULL;
    DLL1Argvoid p_error = NULL;


/*
    std::mutex mtx;           // mutex for critical section
//---------------------------------------------------------------------------
//CAMBIO DE CARRIL
    int cambio_carril_cal(int desp, int carril){
        int dep_temp = desp;
            if ((!( carril) &&desp <= 28 &&desp >= 14) || ( carril &&desp >= 59 &&desp <= 60))
                    dep_temp++;
            else if ((!( carril) &&( desp == 61 || desp == 62 || desp == 135 || desp == 136)) || ( carril &&desp <= 28 &&desp >= 16))
                    dep_temp--;
            else if (!( carril) &&(( desp <= 65 &&desp >= 63) || ( desp >= 131 &&desp <= 134)))
                    dep_temp -= 2;
            else if (!( carril) &&(( desp <= 67 &&desp >= 66) || ( desp == 130)))
                    dep_temp -= 3;
            else if (!( carril) &&desp == 68)
                    dep_temp -= 4;
            else if (!( carril) &&( desp <= 129 &&desp >= 69))
                    dep_temp -= 5;
            else if ( carril &&(( desp <= 62 &&desp >= 61) || ( desp <= 133 &&desp >= 129)))
                    dep_temp += 2;
            else if ( carril &&( desp <= 128 &&desp >= 127))
                    dep_temp += 3;
            else if ( carril &&( desp == 63 || desp == 64 || desp == 126))
                    dep_temp += 4;
            else if ( carril &&( desp <= 125 &&desp >= 65))
                    dep_temp += 5;
            else if ( carril &&( desp <= 136 &&desp >= 134))
                    dep_temp = 136;

        return dep_temp;

    }
    /*
//---------------------------------------------------------------------------
//ENTER_CRITIC
void enterCritic(int semId, int semnum, int op){

    std::unique_lock<std::mutex> lck (mtx,std::defer_lock);
    lck.lock();
 

    assert(op<0);
    if (semop(semId, &buf, 1) == -1){
        perror("ERROR AL ENTRAR CRITICA");
            raise(SIGINT);
    }
    }
//---------------------------------------------------------------------------
//LEAVE_CRITIC
    void leaveCritic(int semId, int semnum, int op){
     lck.unlock();
    assert(op>0);
    if (semop(semId, &buf, 1) == -1){
        perror("ERROR AL SALIR CRITICA");
            raise(SIGINT);
    }
    }
//---------------------------------------------------------------------------
//SEMtoRed
    void semtoRed(int sem) {
        mensaje mg;
        //fprintf(stderr, " [%d] Revico mensajes residuales to Red\n", getpid());
        msgrcv(buzon, & mg, sizeof(mensaje) - sizeof(long), 303 - 2 * sem, IPC_NOWAIT);
        msgrcv(buzon, & mg, sizeof(mensaje) - sizeof(long), 303 - 2 * sem - 1, IPC_NOWAIT);
        luzSem(sem, AMARILLO);
        //fprintf(stderr, " PADRE Sem_cruze pre rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
        enterCritic(sem_cruze, 0, -6);
        luzSem(sem, ROJO);
        //fprintf(stderr, " PADRE Sem_cruze post rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
    }
//---------------------------------------------------------------------------
//SEMtoGreen
    void semtoGreen(int sem) {
        //fprintf(stderr, " PADRE Sem_cruze pre verde: %d\n", semctl(sem_cruze, 0, GETVAL));

        leaveCritic(sem_cruze, 0, 6);
        //fprintf(stderr, " PADRE Sem_cruze post verde: %d\n", semctl(sem_cruze, 0, GETVAL));

        mensaje mt;

        //fprintf(stderr, " [%d] Recivo mensajes residuales to Green \n", getpid());
        msgrcv(buzon, & mt, sizeof(mensaje) - sizeof(long), 303 - 2 * sem, IPC_NOWAIT);
        msgrcv(buzon, & mt, sizeof(mensaje) - sizeof(long), 303 - 2 * sem - 1, IPC_NOWAIT);
        luzSem(sem, VERDE);
        //fprintf(stderr, "               >>>SEM==%d\n",sem);


        mt.tipo = 303 - 2 * sem;
        //fprintf(stderr, "[%d] Envio mensaje tipo %d \n", getpid(),303 -2*sem);
        if (-1 == msgsnd(buzon, & mt, sizeof(mensaje) - sizeof(long), 0)) {
            perror("ERROR AL ENVIAR MENSAJE");
            raise(SIGINT);
        }

        mt.tipo = 303 - 2 * sem - 1;
        //fprintf(stderr, " [%d] Envio mensaje tipo %d \n", getpid(),303 -2*sem-1);


        if (-1 == msgsnd(buzon, & mt, sizeof(mensaje) - sizeof(long), 0)) {
            perror("ERROR AL ENVIAR MENSAJE");
            raise(SIGINT);
        }
    }
    //---------------------------------------------------------------------------
//AVANCE CONTROLADO
void avance_controlado(int * carril, int * desp, int color, int v) {
    enterCritic(critica, 0, -1);
    //fprintf(stderr, "Color (%d) [%d] Entro seccion critica avance\n", color, getpid());
    mensaje mt1, mt2;

    //fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
    if ( * desp > 137 || * desp < 0 || * carril < 0 || * carril > 1 || color < 0 || color > 7 || buzon == -1) {
        ////  fprintf(stderr, "Color (%d) [%d]ERROR ARGUMENTOS F-AVANCE_CONTROLADO d: %d c:%d co:%d b:%d\n",color, getpid(), *desp, *carril, color, buzon);
        leaveCritic(critica, 0, 1);
        raise(SIGINT);
    } //Error en el paso de argumentos

    int dep_temp = * desp, pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1, pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1;
    ////  fprintf(stderr, "Color (%d) [%d] Entro Funcion avance controlado\n",color, getpid());
    msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), pos_cambio + 1, IPC_NOWAIT);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_cambio);
    msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), pos_2 + 1, IPC_NOWAIT);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_2);
    // fprintf(stderr, "Color (%d) [%d] Limpio mensajes (rcv)\n",color, getpid());
    //fprintf(stderr, "Color (%d) [%d]  COMPRUEBO POSICION SIGUIENTE %d (%d+1%%137+%d*137)\n", color, getpid(), *desp + 1 % 137 + *carril *137, *desp, *carril);
    if (memoria[( * desp + 1) % 137 + * carril * 137] == ' ') {
        // fprintf(stderr, "Estado semaforo V: %d H: %d\n", memoria[275], memoria[274]);
        //fprintf(stderr, "Color (%d) [%d] Vacia posicion sig: %d\n", color, getpid(), * desp + 1 % 137 + * carril * 137);
        if ( * desp == 21 && * carril) {
            if ((memoria[275] == ROJO || memoria[275] == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (%d)\n", color, getpid(), 300);
                leaveCritic(critica, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 300, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }
                enterCritic(critica, 0, -1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[275] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

        } else if ( * desp == 20 && !( * carril)) {
            if ((memoria[275] == ROJO || memoria[275] == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (301)\n", color, getpid());

                leaveCritic(critica, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 301, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }
                enterCritic(critica, 0, -1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[275] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

            //fprintf(stderr, "Color (%d) [%d] Semáforo apagado\n", color, getpid());

        } else if ( * desp == 97 && * carril) { //233
            ////  fprintf(stderr, "                                           >>>>>>>POST-ELSE-IF---> %d\n", *desp);
            //          
            if (memoria[274] == ROJO || memoria[274] == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (302)\n", color, getpid());
                leaveCritic(critica, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 302, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }
                enterCritic(critica, 0, -1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[274] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

        } else if ( * desp == 102 && ! * carril) {
            if (memoria[274] == ROJO || memoria[274] == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (303)\n", color, getpid());
                leaveCritic(critica, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 303, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }
                enterCritic(critica, 0, -1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[274] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);
        }
        // fprintf(stderr, "Color (%d) [%d] Avanzo a la siguente posicion (%d)\n",color, getpid(), *desp + 1 % 137 + *carril *137);
        if (avance_coche(carril, desp, color) == -1) {
            perror("ERROR AL AVANZAR COCHE");
            raise(SIGINT);
        }
        //fprintf(stderr, "Color (%d) [%d] Modificada la pos: (nueva) %d\n", color, getpid(), *desp);
        if ((( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 102 && * carril) || ( * desp == 27 && * carril)) && (memoria[274] == ROJO || memoria[274] == AMARILLO || memoria[274] == VERDE)) {
            //fprintf(stderr, "Color (%d) [%d] Sem_cruze salida pre leave: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));

            leaveCritic(sem_cruze, 0, 1);
            //fprintf(stderr, "Color (%d) [%d] Sem_cruze salida post leave: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));

            //fprintf(stderr, "Color (%d) [%d] Suelto la pagita en el cruze %d\n", color, getpid(), *desp);
        }
        if (( * desp == 131 && * carril) || ( * desp == 133 && ! * carril)) {
            ( * contador) ++;
        }
        //fprintf(stderr, "Color (%d) [%d] check Carril-pos: %d\n", color, getpid(), pos_2);
        if (memoria[pos_2] != ' ') {
            mt1.tipo = pos_2 + 1;
            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_2);
            if (-1 == msgsnd(buzon, & mt1, sizeof(mensaje) - sizeof(long), 0)) {
                perror("ERROR AL MSGSND");
                raise(SIGINT);
            }
            //fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
        }

        //fprintf(stderr, "Color (%d) [%d] check Carril-pos: %d\n", color, getpid(), pos_cambio);
        if (memoria[pos_cambio] != ' ') {
            mt1.tipo = pos_cambio + 1;
            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_cambio);
            if (-1 == msgsnd(buzon, & mt1, sizeof(mensaje) - sizeof(long), 0)) {
                perror("ERROR AL MSGSND");
                raise(SIGINT);
            }
            //fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
        }
        // fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
        leaveCritic(critica, 0, 1);
        velocidad(70, * carril, * desp);

        // fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
    } else {
        //fprintf(stderr, "Color (%d) [%d] Posicion siguiente ocupada, compruebo posible cambio de carril pos: %d\n", color, getpid(), * desp);
        if (memoria[cambio_carril_cal( * desp, * carril) + ! * carril * 137] == ' ') {
            ////  fprintf(stderr, "Color (%d) [%d] Efectivo cambio de carril pos(nueva): %d\n",color, getpid(), dep_temp);
            if (cambio_carril(carril, desp, color) == -1) {
                perror("ERROR AL CAMBIAR CARRIL");
                raise(SIGINT);
            }
            //fprintf(stderr, "Color (%d) [%d] Cambio de carril efectuado pos: %d\n", color, getpid(), * desp);
            // fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
            leaveCritic(critica, 0, 1);
            // fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
        } else {
            ////  fprintf(stderr, "Color (%d) [%d] Imposible cambio de carril pos: %d\n",color, getpid(), *desp);
            //fprintf(stderr, "Color (%d) [%d] RECIBIR MENSAJE DE %d \n", color, getpid(), ( *desp + *carril *137) + 1);
            ////  fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
            leaveCritic(critica, 0, 1);
            //fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
            if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), ( * desp + * carril * 137) + 1, 0)) {
                perror("ERROR AL MSGRCV");
                raise(SIGINT);
            }
            //fprintf(stderr, "               ***MENSAJE RECIVIDO ---> %d\n", ( *desp + *carril *137) + 1);
        }
    }




} //fin avance_controlado
//---------------------------------------------------------------------------
//CREA_N_HIJOS
int creaNhijos(int n, int v) {
    mensaje m1, m2;
    int colores[] = {0,0,1,2,3,5,6,7};
    static int i = 0;
    int miIndice, miIndiceCarril;
    pid_t pid_child, pidPoceso;
    int b;

    for (i = 1; i <= n; i++) {
        ////  fprintf(stderr, "**Soy el padre creando al hijo--> %d\n", i);
        miIndice = i;
        miIndiceCarril = miIndice % 2;

        if ((pid_child = fork()) == -1) {
            perror("ERROR CREACION HIJOS");
            raise(SIGINT);
        }

        if (pid_child == 0) {
            //fprintf(stderr, "Hola soy el hijo %d PID: %d\n", i, getpid());
            enterCritic(critica_salida, 0, -1);
            //fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[miIndice], i);

            for (b = 136; b >= 0;) {
                //fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[miIndice], i, b);
                b -= 4;
                if (memoria[b + miIndiceCarril * 137] == ' ') {
                    ////  fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n",colores[miIndice],i);
                    if (inicio_coche( & miIndiceCarril, & b, colores[miIndice]) == -1) {
                        perror("ERROR INCIO COCHE");
                        raise(SIGINT);
                    } //miendice alterna Izquierdo y derecho
                    //fprintf(stderr, "Color (%d) [%d] Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[miIndice], i, miIndiceCarril, b, colores[miIndice]);
                    ////  fprintf(stderr, "               >>ASTERISCO  --->[   %C   ]\n", memoria[b + miIndiceCarril *137]);
                    ////   fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[miIndice],i);

                    break;
                }
            }

            //fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[miIndice], i);
            leaveCritic(critica_salida, 0, 1);

            //SINCRONIZACION SALIDA
            if (n != 1) {
                if (miIndice != n) {
                    ////  fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, i + 1);
                    if (miIndice != 1) {
                     

                        // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                        //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                        if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long), miIndice, 0)) {
                            perror("ERROR AL MSGRCV");
                            raise(SIGINT);
                        }

                    }
                     m1.tipo = miIndice+1;
                        if (-1 == msgsnd(buzon, & m1, sizeof(mensaje) - sizeof(long), 0)) {
                            perror("ERROR AL MSGSND");
                            raise(SIGINT);
                        }

                   if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long), 500+miIndice, 0)) {
                            perror("ERROR AL MSGRCV");
                            raise(SIGINT);
                        }

                    if (miIndice != 1) {
                     

                        // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                        //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                         m1.tipo = 500+miIndice-1;
                        if (-1 == msgsnd(buzon, & m1, sizeof(mensaje) - sizeof(long), 0)) {
                            perror("ERROR AL MSGSND");
                            raise(SIGINT);
                        }

                    }
                    if (miIndice==n){
                        if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long), 600, 0)) {
                            perror("ERROR AL MSGRCV");
                            raise(SIGINT);
                        }
                    }
                    // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld y Arranco \n",colores[miIndice], i , m1.tipo);

                } else { //i==n
                    if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long), miIndice, 0)) {
                        perror("ERROR AL MSGRCV");
                        raise(SIGINT);
                    }
                    m1.tipo = miIndice-1+500;
                    if (-1 == msgsnd(buzon, & m1, sizeof(mensaje) - sizeof(long), 0)) {
                        perror("ERROR AL MSGSND");
                        raise(SIGINT);
                    }
                    // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i , m1.tipo);
                    // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);

                }
            }
            // fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[miIndice],i);
            //

            //velocidad(1, miIndiceCarril, b);

            while (1) {
                //printf("%d",semctl(sem_cruze, 0 ,GETVAL ));
                avance_controlado( & miIndiceCarril, & b, colores[miIndice], v);
            }

            exit(0);
        } else if (pidPoceso == -1) {
            perror("ERROR FORK");
            raise(SIGINT);
        }
    }

    return 0;

} //Fin Nhijos

//---------------------------------------------------------------------------
    
    */
    
    
//---------------------------------------------------------------------------

int main(void) { //Punteros funciones





   

    //---------------------------------------------------------------------------
    //Var's
   
  

    if ((hinstLib = LoadLibrary(TEXT("falonso2.dll"))) == NULL) { //cargas la libreria en memoria del proceso
        perror("Error cargar DLL");
        return (1);
    }


    if (!(inicio_falonso = (DLL1Arg) GetProcAddress(hinstLib, "FALONSO2_inicio")) ) { 
        perror("Error getProc FALONSO2_inicio");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(estadoSem = (DLL1Arg) GetProcAddress(hinstLib, "FALONSO2_estado_semAforo"))) { 
        perror("Error getProc FALONSO2_estado_semAforo");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(f_fin = (DLL1Argp) GetProcAddress(hinstLib, "FALONSO2_fin"))) { 
        perror("Error getProc FALONSO2_fin");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(luzSem = (DLL2Arg) GetProcAddress(hinstLib, "FALONSO2_luz_semAforo"))) { 
        perror("Error getProc FALONSO2_luz_semAforo");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(posOcup = (DLL2Arg) GetProcAddress(hinstLib, "FALONSO2_posiciOn_ocupada"))) { 
        perror("Error getProc FALONSO2_posiciOn_ocupada");
        FreeLibrary(hinstLib);
        return (2);
    }

    if (!(velocidad = (DLL3Arg) GetProcAddress(hinstLib, "FALONSO2_velocidad"))) { 
        perror("Error getProc FALONSO2_velocidad");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(iniCoche = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_inicio_coche"))) { 
        perror("Error getProc FALONSO2_inicio_coche");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(avanceCoche = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_avance_coche"))) { 
        perror("Error getProc FALONSO2_avance_coche");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(cambioCarril = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_cambio_carril"))) { 
        perror("Error getProc FALONSO2_cambio_carril");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!( p_error = (DLL1Argvoid) GetProcAddress(hinstLib, "pon_error"))) { 
        perror("Error getProc  pon_error");
        FreeLibrary(hinstLib);
        return (2);
    }
    
    if (!(pausa = (DLL0Arg) GetProcAddress(hinstLib, "FALONSO2_pausa")) ) {
        perror("Error getProc FALONSO2_pausa");
        FreeLibrary(hinstLib);
        return (2);
    }
    




    inicio_falonso(1);
    int d = 0, p = 30;
    iniCoche(&d,&p,1 );
    luzSem(1, 3);





    while (1) {
        avanceCoche(&d, &p, 6);
         velocidad(10, d, p);
        avanceCoche(&d, &p, 4);
         velocidad(10, d, p);
        avanceCoche(&d, &p, 2);
         velocidad(10, d, p);
        avanceCoche(&d, &p, 4);
         velocidad(10, d, p);
        avanceCoche(&d, &p, 7);
        pausa;
        velocidad(20, d, p);
        cambioCarril(&d, &p, 4);
       
    }
    
    FreeLibrary(hinstLib);//esto va a la manejadora (BOOL)

    return 0;

}
