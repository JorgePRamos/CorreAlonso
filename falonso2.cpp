#include <windows.h>
#include <winuser.h>
#include <stdio.h> 
#include <assert.h>
#include <signal.h>
#include "falonso2.h"
#include <thread>         // std::thread
#include <mutex>   
#include <iostream>        // std::mutex, std::unique_lock, std::defer_lock





#define TAM 100
typedef int (*DLL0Arg)(void);//funciones 0 argumentos
typedef int (*DLL1Arg)(int);//funciones 1 argumento INT
typedef int (*DLL1Argp)(int*);//funciones 1 argumento INT
typedef int (*DLL2Arg)(int,int);
typedef int (*DLL3Arg)(int,int,int);
typedef int (*DLL3ArgP)(int*,int*,int);
typedef void (*DLL1Argvoid)(const char *);


DWORD WINAPI funcionHilos (LPVOID pEstruct);

HANDLE critica = CreateSemaphore(
    NULL, // default security attributes
    1, // initial count
    1, // maximum count
    NULL);
/*
std::mutex m_critica;
std::unique_lock<std::mutex> critica (m_critica,std::defer_lock);
*/
HANDLE critica_salida = CreateSemaphore(
    NULL, // default security attributes
    1, // initial count
    1, // maximum count
    NULL);
/*
std::mutex m_critica_salida;
std::unique_lock<std::mutex> critica_salida (m_critica_salida,std::defer_lock);
*/
HANDLE sem_cruze = CreateSemaphore(
    NULL, // default security attributes
    6, // initial count
    6, // maximum count
    NULL);
/*
std::mutex m_sem_cruze;
std::unique_lock<std::mutex> sem_cruze (m_sem_cruze,std::defer_lock);
*/

HANDLE sem_dad = CreateSemaphore(
    NULL, // default security attributes
    1, // initial count
    1, // maximum count
    NULL);
/*
std::mutex m_sem_dad;
std::unique_lock<std::mutex> sem_dad (m_sem_dad,std::defer_lock);
*/





//---------------------------------------------------------------------------
//DEFINICIONES
unsigned int  contador = 0;
typedef struct Coche {
    int posicion;
    int carril;
} coche;
typedef struct parametros {
    int nCoches;
    int velocidad;
    int indice;
} * pParam;
HINSTANCE hinstLib = NULL;
DLL1Arg inicio_falonso = NULL, estadoSem = NULL;
DLL1Argp f_fin = NULL;
DLL2Arg luzSem = NULL, posOcup = NULL;
DLL3Arg velocidad = NULL;
DLL3ArgP iniCoche = NULL, avanceCoche = NULL, cambioCarril = NULL;
DLL0Arg pausa = NULL;
DLL1Argvoid p_error = NULL;
MSG test_msg, uMsg;




//---------------------------------------------------------------------------
//CAMBIO DE CARRIL
int cambio_carril_cal(int desp, int carril) {
    int dep_temp = desp;
    if ((!(carril) && desp <= 28 && desp >= 14) || (carril && desp >= 59 && desp <= 60))
        dep_temp++;
    else if ((!(carril) && (desp == 61 || desp == 62 || desp == 135 || desp == 136)) || (carril && desp <= 28 && desp >= 16))
        dep_temp--;
    else if (!(carril) && ((desp <= 65 && desp >= 63) || (desp >= 131 && desp <= 134)))
        dep_temp -= 2;
    else if (!(carril) && ((desp <= 67 && desp >= 66) || (desp == 130)))
        dep_temp -= 3;
    else if (!(carril) && desp == 68)
        dep_temp -= 4;
    else if (!(carril) && (desp <= 129 && desp >= 69))
        dep_temp -= 5;
    else if (carril && ((desp <= 62 && desp >= 61) || (desp <= 133 && desp >= 129)))
        dep_temp += 2;
    else if (carril && (desp <= 128 && desp >= 127))
        dep_temp += 3;
    else if (carril && (desp == 63 || desp == 64 || desp == 126))
        dep_temp += 4;
    else if (carril && (desp <= 125 && desp >= 65))
        dep_temp += 5;
    else if (carril && (desp <= 136 && desp >= 134))
        dep_temp = 136;

    return dep_temp;

}

//---------------------------------------------------------------------------
//ENTER_CRITIC
void enterCritic(const char semId[], int op) {
    int f = 0;


    if (strcmp(semId, "critica")) {
        for (; f < op; f++) {
            WaitForSingleObject(critica, INFINITE);
        }
    } else if (strcmp(semId, "critica_salida")) {
        for (; f < op; f++) {
            WaitForSingleObject(critica_salida, INFINITE);
        }
    } else if (strcmp(semId, "sem_cruze")) {
        for (; f < op; f++) {
            WaitForSingleObject(sem_cruze, INFINITE);
        }
    } else if (strcmp(semId, "sem_dad")) {
        for (; f < op; f++) {
            WaitForSingleObject(sem_dad, INFINITE); //problema
        }
    } else {
        raise(SIGINT);
    }





}
//---------------------------------------------------------------------------
//LEAVE_CRITIC
void leaveCritic(const char semId[], int op) {
    if (strcmp(semId, "critica")) {
        ReleaseSemaphore(critica, op, NULL);
    } else if (strcmp(semId, "critica_salida")) {
        ReleaseSemaphore(critica_salida, op, NULL);
    } else if (strcmp(semId, "sem_cruze")) {
        ReleaseSemaphore(sem_cruze, op, NULL);
    } else if (strcmp(semId, "sem_dad")) {
        ReleaseSemaphore(sem_dad, op, NULL);
    } else {
        raise(SIGINT);
    }

}
//---------------------------------------------------------------------------
//SEMtoRed
void semtoRed(int sem) {

    //fprintf(stderr, " [%d] Revico mensajes residuales to Red\n", getpid());
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem, 303 - 2 * sem, PM_REMOVE);//ipc_Nowait
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem - 1, 303 - 2 * sem - 1, PM_REMOVE);//ipc_Nowait
    luzSem(sem, AMARILLO);
    //fprintf(stderr, " PADRE Sem_cruze pre rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
    enterCritic("sem_cruze", 6);

    leaveCritic("critica_salida", 1);
    //fprintf(stderr, " PADRE Sem_cruze pre rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
    enterCritic("sem_cruze", 6);
    enterCritic("critica_salida", 1);

    luzSem(sem, ROJO);
    //fprintf(stderr, " PADRE Sem_cruze post rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
}
//---------------------------------------------------------------------------
//SEMtoGreen
void semtoGreen(int sem) {
    //fprintf(stderr, " PADRE Sem_cruze pre verde: %d\n", semctl(sem_cruze, 0, GETVAL));

    leaveCritic("sem_cruze", 6);
    //fprintf(stderr, " PADRE Sem_cruze post verde: %d\n", semctl(sem_cruze, 0, GETVAL));



    //fprintf(stderr, " [%d] Recivo mensajes residuales to Green \n", getpid());
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem, 303 - 2 * sem, PM_REMOVE);
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem - 1, 303 - 2 * sem - 1, PM_REMOVE);
    luzSem(sem, VERDE);
    //fprintf(stderr, "               >>>SEM==%d\n",sem);



    //fprintf(stderr, "[%d] Envio mensaje tipo %d \n", getpid(),303 -2*sem);


    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 303 - 2 * sem, 0, 0) == FALSE) {
        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }
    //fprintf(stderr, " [%d] Envio mensaje tipo %d \n", getpid(),303 -2*sem-1);
    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 303 - 2 * sem - 1, 0, 0) == FALSE) {
        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }

}
//---------------------------------------------------------------------------
//AVANCE CONTROLADO
void avance_controlado(int * carril, int * desp, int color, int v) {
    enterCritic("critica", 1);
    //fprintf(stderr, "Color (%d) [%d] Entro seccion critica avance\n", color, getpid());


    ////fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
    if ( * desp > 137 || * desp < 0 || * carril < 0 || * carril > 1 || color < 0 || color > 7) {
        ////  fprintf(stderr, "Color (%d) [%d]ERROR ARGUMENTOS F-AVANCE_CONTROLADO d: %d c:%d co:%d b:%d\n",color, getpid(), *desp, *carril, color, buzon);
        leaveCritic("critica", 1);
        raise(SIGINT);
    } //Error en el paso de argumentos

    int dep_temp = * desp, pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1, pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1;
    //  fprintf(stderr, "Color (%d) [%d] Entro Funcion avance controlado\n",color, getpid());

    PeekMessageA( & test_msg, NULL,WM_USER+ pos_cambio + 1, pos_cambio + 1, PM_REMOVE);//IPC_NOWAIT
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_cambio);

   // PeekMessageA( & test_msg, NULL,WM_USER+ pos_2 + 1, pos_2 + 1, PM_REMOVE);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_2);
    //fprintf(stderr, "Color (%d) [%d] Limpio mensajes (rcv)\n",color, getpid());
    //fprintf(stderr, "Color (%d) [%d]  COMPRUEBO POSICION SIGUIENTE %d (%d+1%%137+%d*137)\n", color, getpid(), *desp + 1 % 137 + *carril *137, *desp, *carril);

    if (!(posOcup( * carril, ( * desp + 1) % 137))) {
        // fprintf(stderr, "Estado semaforo V: %d H: %d\n", memoria[275], memoria[274]);
        //fprintf(stderr, "Color (%d) [%d] Vacia posicion sig: %d\n", color, getpid(), * desp + 1 % 137 + * carril * 137);
        if ( * desp == 21 && * carril) {
            if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (%d)\n", color, getpid(), 300);
                leaveCritic("critica", 1);
                if (GetMessage( & uMsg, NULL,WM_USER+ 300, 300) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic("critica", 1);
                enterCritic("sem_cruze", 1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (estadoSem(VERTICAL) == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic("critica_salida", 1);
                leaveCritic("critica", 1);
                enterCritic("sem_cruze",1);
                enterCritic("critica",1);


                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else
                raise(SIGINT);

        } else if ( * desp == 20 && !( * carril)) {
            enterCritic("critica_salida",1);

            if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (301)\n", color, getpid());

                leaveCritic("critica", 1);
                if (GetMessage( & uMsg, NULL,WM_USER+ 301, 301) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                
                leaveCritic("critica_salida",1);
                enterCritic("critica", 1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic("sem_cruze", 1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (estadoSem(VERTICAL) == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic("sem_cruze", 1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else
                raise(SIGINT);

            //fprintf(stderr, "Color (%d) [%d] Semáforo apagado\n", color, getpid());

        } else if ( * desp == 97 && * carril) { //233
            //  fprintf(stderr, "                                           >>>>>>>POST-ELSE-IF---> %d\n", *desp);

            if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (302)\n", color, getpid());
                leaveCritic("critica", 1);
                if (GetMessage( & uMsg, NULL,WM_USER+ 302, 302) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
              
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic("sem_cruze", 1);
                enterCritic("critica", 1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (estadoSem(HORIZONTAL) == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic("critica_salida", 1);
                leaveCritic("critica", 1);
                enterCritic("sem_cruze", 1);
                enterCritic("critica", 1);


                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else
                raise(SIGINT);

        } else if ( * desp == 97 && ! * carril) {
            enterCritic("critica_salida", 1);
            if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (303)\n", color, getpid());
                leaveCritic("critica", 1);
                leaveCritic("critica_salida", 1);
                if (GetMessage( & uMsg, NULL,WM_USER+ 303, 303) == -1) {//302¿?
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                enterCritic("sem_cruze", 1);
                enterCritic("critica", 1);
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (estadoSem(HORIZONTAL) == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic("critica_salida", 1);
                leaveCritic("critica", 1);
                enterCritic("sem_cruze", 1);
                enterCritic("critica", 1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else
                raise(SIGINT);
        }
        // fprintf(stderr, "Color (%d) [%d] Avanzo a la siguente posicion (%d)\n",color, getpid(), *desp + 1 % 137 + *carril *137);
        if (avanceCoche(carril, desp, color) == -1) {
            PERROR("ERROR AL AVANZAR COCHE");
            raise(SIGINT);
        }
        //fprintf(stderr, "Color (%d) [%d] Modificada la pos: (nueva) %d\n", color, getpid(), *desp);
        if (( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 106 && * carril) || ( * desp == 25 && * carril)) {
            //fprintf(stderr, "Color (%d) [%d] Sem_cruze salida pre leave: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));

            leaveCritic("sem_cruze", 1);
            //fprintf(stderr, "Color (%d) [%d] Sem_cruze salida post leave: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            //fprintf(stderr, "Color (%d) [%d] Suelto la pagita en el cruze %d\n", color, getpid(), *desp);
        }
        if (( * desp == 131 && * carril) || ( * desp == 133 && ! * carril)) {
            (contador) ++;
        }
        //fprintf(stderr, "Color (%d) [%d] check Carril-pos: %d\n", color, getpid(), pos_2);
        //pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1
        if (posOcup( * carril, ((( * desp) + 135) % 137))) {
            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_2);
            if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + pos_2 + 1, 0, 0) == 0) {
                PERROR("ERROR AL MSGSND (pos -2 ocupada post avance)");
                raise(SIGINT);
            }
            ////fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
        }

        //fprintf(stderr, "Color (%d) [%d] check Carril-pos: %d\n", color, getpid(), pos_cambio);
        //pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
        if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {
            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_cambio);
            if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + pos_cambio + 1, 0, 0) == 0) {
                ("ERROR AL MSGSND (pos carril opuesto ocupada)");
                raise(SIGINT);
            }
            ////fprintf(stderr, "Color (%d) [%d] ENVIADO MENSAJE --> %ld \n", color, getpid(), mt1.tipo);
        }
        // fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
        leaveCritic("critica", 1);
        velocidad(50, * carril, * desp);

        // fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
    } else {
        //fprintf(stderr, "Color (%d) [%d] Posicion siguiente ocupada, compruebo posible cambio de carril pos: %d\n", color, getpid(), * desp);

        if (!posOcup(! * carril, cambio_carril_cal( * desp, * carril))) {
            //  fprintf(stderr, "Color (%d) [%d] Efectivo cambio de carril pos(nueva): %d\n",color, getpid(), dep_temp);
            if (cambioCarril(carril, desp, color) == -1) {
                PERROR("ERROR AL CAMBIAR CARRIL");
                raise(SIGINT);
            }
            //fprintf(stderr, "Color (%d) [%d] Cambio de carril efectuado pos: %d\n", color, getpid(), * desp);
            // fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
            leaveCritic("critica", 1);
            // fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
        } else {
            //  fprintf(stderr, "Color (%d) [%d] Imposible cambio de carril pos: %d\n",color, getpid(), *desp);
            //fprintf(stderr, "Color (%d) [%d] RECIBIR MENSAJE DE %d \n", color, getpid(), ( *desp + *carril *137) + 1);
            //  fprintf(stderr, "Color (%d) [%d] Voy a soltar la seccion critica \n",color, getpid());
            leaveCritic("critica", 1);
            //fprintf(stderr, "Color (%d) [%d] Suelto la seccion critica \n",color, getpid());
            if (GetMessage( & uMsg, NULL,WM_USER+ ( * desp + * carril * 137) + 1, ( * desp + * carril * 137) + 1) == -1) {
                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }
            //fprintf(stderr, "               ***MENSAJE RECIVIDO ---> %d\n", ( *desp + *carril *137) + 1);
        }
    }




} //fin avance_controlado
//---------------------------------------------------------------------------
//CREA_N_HIJOS
int creaNhijos(int n, int v) {

    DWORD idHilo [n];
    static int i;




    for (i = 1; i <= n; i++) {
        ////  fprintf(stderr, "**Soy el padre creando al hijo--> %d\n", i);

        pParam nParam;
        nParam->indice=i;
        nParam->velocidad=v;
        nParam->nCoches=n;

        if((CreateThread(NULL,0, funcionHilos,nParam, 0 , &idHilo[i]))==NULL){
            PERROR("Create Hilo");
            raise(SIGINT);
        }
        

    }

    return 0;

    } //Fin Nhijos

DWORD WINAPI funcionHilos (LPVOID pEstruct){
    int colores[] = {0,0,1,2,3,5,6,7};
    int n = pEstruct->nCoches;
    int  miIndice=pEstruct->indice;
    int v = pEstruct->velocidad;
    int b;
       //fprintf(stderr, "Hola soy el hijo %d PID: %d\n", i, getpid());
            enterCritic("critica_salida", 1);
            //fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[miIndice], i);
            for (b = 136; b >= 0;) {
                //fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[miIndice], i, b);
                b -= 2;
                //if (memoria[b + miIndiceCarril * 137] == ' ') {
                if (!(posOcup(miIndiceCarril, b))) {
                    //  fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n",colores[miIndice],i);
                    if (iniCoche( & miIndiceCarril, & b,  colores[1+(miIndice-1)%6]) == -1) {
                        PERROR("ERROR INCIO COCHE");
                        raise(SIGINT);
                    } //miendice alterna Izquierdo y derecho
                    // fprintf(stderr, "Color (%d) [%d] Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[miIndice], i, miIndiceCarril, b, colores[miIndice]);
                    // fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[miIndice],i);

                    break;
                }
            }

            //fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[miIndice], i);
            leaveCritic("critica_salida", 1);
        if (n != 1) {
            if (miIndice != n) {
                //  fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, i + 1);
                if (miIndice != 1) {
                    //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                    //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                    if (GetMessage( & uMsg, NULL, WM_USER + miIndice, 100 + miIndice) == -1) {
                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }

                }
                if (PostThreadMessageA(GetCurrentThreadId(), WM_USER + 100 + miIndice + 1, 0, 0) == FALSE)
                    PERROR("Error PostMsg");
                raise(SIGINT);

                if (GetMessage( & uMsg, NULL, WM_USER + 500 + miIndice, 500 + miIndice) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }

                if (miIndice != 1) {
                    // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                    // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                    if (PostThreadMessageA(GetCurrentThreadId(), WM_USER + 500 + miIndice - 1, 0, 0) == FALSE)
                        PERROR("Error PostMsg");
                    raise(SIGINT);

                }
                if (miIndice == n) {
                    if (GetMessage( & uMsg, NULL, WM_USER + 600, 600) == -1) {
                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                }
                // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld y Arranco \n",colores[miIndice], i , m1.tipo);
            } else { //i==n
                if (GetMessage( & uMsg, NULL, WM_USER + miIndice, 100 + miIndice) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                if (PostThreadMessageA(GetCurrentThreadId(), WM_USER + miIndice - 1 + 500, 0, 0) == FALSE)
                    PERROR("Error PostMsg");
                raise(SIGINT);

                // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i , m1.tipo);
                // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
            }
        }
    // fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[miIndice],i);
    while (1) {
        //printf("%d",semctl(sem_cruze, 0 ,GETVAL ));
        avance_controlado( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6], v);
    }

    exit(0);
}
//---------------------------------------------------------------------------
void manejadora(int param) {
    printf("Salto a Manejadora\n");
    FreeLibrary(hinstLib);
    exit(1);
}
int main(void) { //Punteros funciones

    PeekMessage(&test_msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    signal(SIGINT, manejadora);

    //---------------------------------------------------------------------------
    //Var's


        //CreateThread(NULL,0,avance_controlado,NULL,0);

    if ((hinstLib = LoadLibrary(TEXT("falonso2.dll"))) == NULL) { //cargas la libreria en memoria del proceso
        PERROR("Error cargar DLL");
        return (1);
    }


    if (!(inicio_falonso = (DLL1Arg) GetProcAddress(hinstLib, "FALONSO2_inicio"))) {
        PERROR("Error getProc FALONSO2_inicio");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(estadoSem = (DLL1Arg) GetProcAddress(hinstLib, "FALONSO2_estado_semAforo"))) {
        PERROR("Error getProc FALONSO2_estado_semAforo");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(f_fin = (DLL1Argp) GetProcAddress(hinstLib, "FALONSO2_fin"))) {
        PERROR("Error getProc FALONSO2_fin");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(luzSem = (DLL2Arg) GetProcAddress(hinstLib, "FALONSO2_luz_semAforo"))) {
        PERROR("Error getProc FALONSO2_luz_semAforo");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(posOcup = (DLL2Arg) GetProcAddress(hinstLib, "FALONSO2_posiciOn_ocupada"))) {
        PERROR("Error getProc FALONSO2_posiciOn_ocupada");
        FreeLibrary(hinstLib);
        return (2);
    }

    if (!(velocidad = (DLL3Arg) GetProcAddress(hinstLib, "FALONSO2_velocidad"))) {
        PERROR("Error getProc FALONSO2_velocidad");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(iniCoche = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_inicio_coche"))) {
        PERROR("Error getProc FALONSO2_inicio_coche");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(avanceCoche = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_avance_coche"))) {
        PERROR("Error getProc FALONSO2_avance_coche");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(cambioCarril = (DLL3ArgP) GetProcAddress(hinstLib, "FALONSO2_cambio_carril"))) {
        PERROR("Error getProc FALONSO2_cambio_carril");
        FreeLibrary(hinstLib);
        return (2);
    }
    if (!(p_error = (DLL1Argvoid) GetProcAddress(hinstLib, "pon_error"))) {
        PERROR("Error getProc  pon_error");
        FreeLibrary(hinstLib);
        return (2);
    }

    if (!(pausa = (DLL0Arg) GetProcAddress(hinstLib, "FALONSO2_pausa"))) {
        PERROR("Error getProc FALONSO2_pausa");
        FreeLibrary(hinstLib);
        return (2);
    }





    inicio_falonso(1);
  
    int d = 1, p = 30, e=1, f=29;
    iniCoche( & d, & p, 7);
    iniCoche(&e,&f, 4 );
    luzSem(1, 2);
    luzSem(0, 2);
    for(;;){
        avance_controlado(&d, &p, 7, 1);
        avance_controlado(&e,&f, 4, 1 );
        pausa;

    }

    FreeLibrary(hinstLib); //esto va a la manejadora (BOOL)

    return 0;

}
/*
  for( i=0; i < THREADCOUNT; i++ )
    {
        aThread[i] = CreateThread( 
                     NULL,       // default security attributes
                     0,          // default stack size
                     (LPTHREAD_START_ROUTINE) ThreadProc, 
                     NULL,       // no thread function arguments
                     0,          // default creation flags
                     &ThreadID); // receive thread identifier

        if( aThread[i] == NULL )
        {
            printf("CreateThread error: %d\n", GetLastError());
            return 1;
        }
    }

    // Wait for all threads to terminate

    WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);
    
*/      