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
} * pParam, Param;
HINSTANCE hinstLib = NULL;
DLL1Arg inicio_falonso = NULL, estadoSem = NULL;
DLL1Argp f_fin = NULL;
DLL2Arg luzSem = NULL, posOcup = NULL;
DLL3Arg velocidad = NULL;
DLL3ArgP iniCoche = NULL, avanceCoche = NULL, cambioCarril = NULL;
DLL0Arg pausa = NULL;
DLL1Argvoid p_error = NULL;
MSG test_msg, uMsg;
int num_coche=0;
DWORD idPadre;


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

void manejadora(int param) {
    printf("Salto a Manejadora\n");
    FreeLibrary(hinstLib);
    /*
    for(int i=0; i<num_coche; i++)
    {
        CloseHandle(hThreadArray[i]);

        if(arrayParam[i] != NULL)
        {
            HeapFree(GetProcessHeap(), 0, arrayParam[i]);
            arrayParam[i] = NULL;    // Ensure address is not reused.
        }

    }
    delete [] hThreadArray;
    delete [] arrayParam;
    */

    exit(1);
}


//---------------------------------------------------------------------------
//ENTER_CRITIC
void enterCritic(const char semId[], int op) {
    int f = 0;

        //fprintf(stderr, "Pre-IF enterCritc()\n");

    if (!strcmp(semId, "critica")) {
        for (; f < op; f++) {
            if((WaitForSingleObject(critica, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject");
        }
    } else if (!strcmp(semId, "critica_salida")) {//WAIT_FAILED
            //fprintf(stderr, "Post strcmp\n");
        for (; f < op; f++) {
            //fprintf(stderr, "Entro en critica salida\n");
            if((WaitForSingleObject(critica_salida, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject");
        }
    } else if (!strcmp(semId, "sem_cruze")) {
        for (; f < op; f++) {
            if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject");
        }
    } else if (!strcmp(semId, "sem_dad")) {
        for (; f < op; f++) {
            if((WaitForSingleObject(sem_dad, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject"); //problema
        }
    } else {
        raise(SIGINT);
    }

}
//--------------------------------------------------------------------------
//LEAVE_CRITIC
void leaveCritic(const char semId[], int op) {

    if (!strcmp(semId, "critica")) {
        if(!(ReleaseSemaphore(critica, op, NULL)))
            PERROR("ReleaseSemaphore");
    } else if (!strcmp(semId, "critica_salida")) {
        if(!(ReleaseSemaphore(critica_salida, op, NULL)))
            PERROR("ReleaseSemaphore");
    } else if (!strcmp(semId, "sem_cruze")) {
        if(!(ReleaseSemaphore(sem_cruze, op, NULL)))
            PERROR("ReleaseSemaphore");
    } else if (!strcmp(semId, "sem_dad")) {
        if(!(ReleaseSemaphore(sem_dad, op, NULL)))
            PERROR("ReleaseSemaphore");

    } else {
        raise(SIGINT);
    }

}
//---------------------------------------------------------------------------
//SEMtoRed
void semtoRed(int sem) {


    luzSem(sem, AMARILLO);

    leaveCritic("critica_salida", 1);
    //fprintf(stderr, "[%d]-Padre Salida Sem_Salida critica +1 pajitas: %d\n", getpid(),semctl(critica_salida, 0, GETVAL));//#getval
    fprintf(stderr, "[%d]-Padre Salida Sem_Salida critica +1 pajitas\n", getpid());


    enterCritic("sem_cruze", 6);
    //fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica -6 pajitas: %d\n", getpid(),semctl(sem_cruze, 0, GETVAL));//#getval
    fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica -6 pajitas\n", getpid());


    enterCritic("critica_salida", 1);
    //fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas: %d\n", getpid(),semctl(critica_salida, 0, GETVAL));//#getval
    fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas\n", getpid());


    luzSem(sem, ROJO);
}

//---------------------------------------------------------------------------
//SEMtoGreen
void semtoGreen(int sem) {

    leaveCritic("sem_cruze", 6);
    //fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas: %d\n", getpid(),semctl(sem_cruze, 0, GETVAL));//getval
    fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas\n", getpid());



    luzSem(sem, VERDE);


    if (PostThreadMessageA(idPadre,WM_APP + 303 - 2 * sem, 0, 0) == FALSE) {
        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }
    //fprintf(stderr, " [%d] Envio mensaje tipo %d \n", getpid(),303 -2*sem-1);
    if (PostThreadMessageA(idPadre,WM_APP + 303 - 2 * sem - 1, 0, 0) == FALSE) {

        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }

}

//---------------------------------------------------------------------------
//AVANCE CONTROLADO
void avance_controlado(int * carril, int * desp, int color, int v) {
    enterCritic("critica", 1);
    //fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas: %d\n", getpid(),color, semctl(critica, 0, GETVAL));//#getval
    fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas\n", getpid(),color);


    if ( * desp > 137 || * desp < 0 || * carril < 0 || * carril > 1 || color < 0 || color > 7) {
        leaveCritic("critica", 1);
        raise(SIGINT);
    } //Error en el paso de argumentos

    int dep_temp = * desp, pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1, pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1;


    PeekMessageA( & test_msg, NULL,WM_APP+ pos_cambio + 1, pos_cambio + 1, PM_REMOVE);//IPC_NOWAIT
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_cambio);

   // PeekMessageA( & test_msg, NULL,WM_APP+ pos_2 + 1, pos_2 + 1, PM_REMOVE);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_2);
    //fprintf(stderr, "Color (%d) [%d] Limpio mensajes (rcv)\n",color, getpid());
    //fprintf(stderr, "Color (%d) [%d]  COMPRUEBO POSICION SIGUIENTE %d (%d+1%%137+%d*137)\n", color, getpid(), *desp + 1 % 137 + *carril *137, *desp, *carril);

    
    if (!(posOcup( * carril, ( * desp + 1) % 137))) {

        if ( * desp == 21 && * carril) {
            enterCritic("critica_salida", 1);
            //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", getpid(), color);//#critica


            if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", getpid(), color, 300);//#semaforo
                
                leaveCritic("critica", 1);

                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid(), color);//#critica


                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", getpid(), color);//#critica 


                if (GetMessage( & uMsg, NULL,WM_USER+ 300, 300) == -1) {

                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", getpid(), color, 300);//#mensaje
                
                enterCritic("sem_cruze", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica


                enterCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

                
            } else if (estadoSem(VERTICAL) == VERDE) {
                fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", getpid(), color);//#semaforo

                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", getpid());//#critica

                leaveCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid());//#critica

                enterCritic("sem_cruze",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica

                enterCritic("critica",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica


            } else
                raise(SIGINT);

        } else if ( * desp == 20 && !( * carril)) { 
            enterCritic("critica_salida", 1);
            //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", getpid(), color);//#critica

            if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", getpid(), color, 301);//#semaforo

                leaveCritic("critica", 1);

                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid(), color);//#critica


                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", getpid(), color);//#critica 


                if (GetMessage( & uMsg, NULL,WM_USER+ 300, 300) == -1) {

                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", getpid(), color, 300);//#mensaje
                
                enterCritic("sem_cruze", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica


                enterCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

                
            } else if (estadoSem(VERTICAL) == VERDE) {
                fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", getpid(), color);//#semaforo

                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", getpid());//#critica

                leaveCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid());//#critica

                enterCritic("sem_cruze",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica

                enterCritic("critica",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

            } else
                raise(SIGINT);

        } else if ( * desp == 97 && * carril) {
            enterCritic("critica_salida", 1);
            //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", getpid(), color);//#critica

            if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                leaveCritic("critica", 1);

                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid(), color);//#critica


                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", getpid(), color);//#critica 


                if (GetMessage( & uMsg, NULL,WM_USER+ 300, 300) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", getpid(), color, 300);//#mensaje
                

                enterCritic("sem_cruze", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica


                enterCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

                
            } else if (estadoSem(HORIZONTAL) == VERDE) {
                fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", getpid(), color);//#semaforo

                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", getpid());//#critica

                leaveCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid());//#critica

                enterCritic("sem_cruze",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica

                enterCritic("critica",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica


            } else
                raise(SIGINT);

        } else if ( * desp == 97 && ! * carril) {
            enterCritic("critica_salida", 1);
            //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", getpid(), color);//#critica

            if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                fprintf(stderr, "[%d] Color (%d) Espero semaforo HORIZONTAL (%d)\n", getpid(), color, 303);//#semaforo
                
                leaveCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid(), color);//#critica


                leaveCritic("critica_salida", 1);

                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", getpid(), color);//#critica 


                if (GetMessage( & uMsg, NULL,WM_USER+ 300, 300) == -1) {

                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", getpid(), color, 300);//#mensaje
                
                enterCritic("sem_cruze", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica


                enterCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

                } else if (estadoSem(HORIZONTAL) == VERDE) {
            } else if (estadoSem(VERTICAL) == VERDE) {
                fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", getpid(), color);//#semaforo

                leaveCritic("critica_salida", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", getpid());//#critica

                leaveCritic("critica", 1);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid());//#critica

                enterCritic("sem_cruze",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", getpid(), color);//#critica

                enterCritic("critica",1);
                //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
                fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", getpid(), color);//#critica

            } else
                raise(SIGINT);
        }
        fprintf(stderr, "[%d] Color (%d) Avanzo a posicion (%d)\n", getpid(), color, *desp + 1 % 137 + *carril *137);//#posicion
        if (avanceCoche(carril, desp, color) == -1) {
            PERROR("ERROR AL AVANZAR COCHE");
            raise(SIGINT);
        }
        if (( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 106 && * carril) || ( * desp == 25 && * carril)) {

            leaveCritic("sem_cruze", 1);
            //fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas\n", getpid(), color);//#critica

        }
        if (( * desp == 131 && * carril) || ( * desp == 133 && ! * carril)) {
            (contador) ++;
        }

        //pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1
        if (posOcup( * carril, ((( * desp) + 135) % 137))) {

            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_2);
            if (PostThreadMessageA(idPadre,WM_APP + pos_2 + 1, 0, 0) == 0) {

                PERROR("ERROR AL MSGSND (pos -2 ocupada post avance)");
                raise(SIGINT);
            }
            fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", getpid(), color, pos_2);//#mensaje
        }

        //pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
        if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {

            //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, getpid(), pos_cambio);
            if (PostThreadMessageA(idPadre,WM_APP + pos_cambio + 1, 0, 0) == 0) {

                ("ERROR AL MSGSND (pos carril opuesto ocupada)");
                raise(SIGINT);
            }
            fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", getpid(), color, pos_cambio);//#mensaje

        }
        leaveCritic("critica", 1);
        //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
        fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color);//#critica


        velocidad(50, * carril, * desp);

    } else {
        fprintf(stderr, "[%d] Color (%d) Posicion ocupada, compruebo cambio de carril: %d\n", getpid(), color, *desp);//#posicion

        if (!posOcup(! * carril, cambio_carril_cal( * desp, * carril))) {
            if (cambioCarril(carril, desp, color) == -1) {
                PERROR("ERROR AL CAMBIAR CARRIL");
                raise(SIGINT);
            }
            fprintf(stderr, "[%d] Color (%d) Cambio Carril: %d\n", getpid(), color, dep_temp);//#posicion

            leaveCritic("critica", 1);
            //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", getpid(), color);//#critica

        } else {
            
            
            leaveCritic("critica", 1);

            //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color, semctl(critica, 0, GETVAL));//#critica getval
            fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", getpid(), color);//#critica 

            if (GetMessage( & uMsg, NULL,WM_APP+ ( * desp + * carril * 137) + 1, ( * desp + * carril * 137) + 1) == -1) {

                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }
            fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", getpid(), color, ( * desp + * carril * 137) + 1);//#mensaje
        }
    }
} //fin avance_controlado


//---------------------------------------------------------------------------
//CREA_N_HIJOS
int creaNhijos(int n, int v) {
    HANDLE hThreadArray [n];

    DWORD idHilo [n];
    static int i;
    num_coche=n;
    pParam arrayParam [n];




    for (i = 1; i <= n; i++) {
        //fprintf(stderr, "%d %d**Soy el padre creando al hijo--> %d\n", (sizeof(hThreadArray)/sizeof(* hThreadArray )) ,n, i);

        arrayParam[i]= (pParam)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Param));

        arrayParam[i]->indice=i;
        arrayParam[i]->velocidad=v;
        arrayParam[i]->nCoches=n;

        if((hThreadArray[i]=CreateThread(NULL,0, funcionHilos,arrayParam[i], 0 , &idHilo[i]))==NULL){
            PERROR("Create Hilo");
            //raise(SIGINT);
        }
        //fprintf(stderr, "Post creacion Hijo\n");

        WaitForMultipleObjects(n,hThreadArray, TRUE, INFINITE);
    }

    return 0;

    } //Fin Nhijos

DWORD WINAPI funcionHilos (LPVOID pEstruct_2){
    pParam pEstruct = (pParam) pEstruct_2;
    int colores[] = {0,0,1,2,3,5,6,7};
    int n = pEstruct -> nCoches;
    int miIndice = pEstruct -> indice;
    int v = pEstruct -> velocidad;
    int miIndiceCarril = miIndice % 2;
    int b;
    //fprintf(stderr, "\t\tHola soy el hijo %d PID: %d\n", miIndice, GetCurrentThreadId());
    enterCritic("critica_salida", 1);
    //fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);

    for (b = 136; b >= 0;) {
        //fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[1 + (miIndice - 1) % 6], miIndice, b);
        b -= 2;
        //if (memoria[b + miIndiceCarril * 137] == ' ') {
        if (!(posOcup(miIndiceCarril, b))) {
            //fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n", colores[1 + (miIndice - 1) % 6], miIndice);
            if (iniCoche( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6]) != 0) {
                //fprintf(stderr, "Color (%d) [%d] ERROR INICIO COCHE +++++++++++++++++++++++++++++++++\n", colores[1 + (miIndice - 1) % 6], miIndice);
                PERROR("ERROR INCIO COCHE");
                raise(SIGINT);
            } //miendice alterna Izquierdo y derecho
            //fprintf(stderr, "Color (%d) [%d] Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[1 + (miIndice - 1) % 6], miIndice, miIndiceCarril, b, colores[1 + (miIndice - 1) % 6]);
            // fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[1 + (miIndice - 1) % 6],i);

            break;
        }
    }
    fflush(stderr);

    leaveCritic("critica_salida", 1);
    //fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
    /*
    if (n != 1) {
        if (miIndice != n) {
            //  fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, i + 1);
            if (miIndice != 1) {
                fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, WM_APP + miIndice);
                if (GetMessage( & uMsg, NULL, WM_APP + miIndice, WM_APP + miIndice) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }

            }
            fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[1 + (miIndice - 1) % 6], miIndice,  WM_APP + miIndice + 1);
            if (PostThreadMessageA(idPadre, WM_APP + miIndice + 1, 0, 0) == FALSE) {
                PERROR("Error PostMsg");
                raise(SIGINT);
            }

            if (GetMessage( & uMsg, NULL, WM_APP + 500 + miIndice,WM_APP + 500 + miIndice) == -1) {
                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }

            if (miIndice != 1) {
               fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[1 + (miIndice - 1) % 6], miIndice,  WM_APP + 500 + miIndice - 1);
                // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, 2*n+i);
                if (PostThreadMessageA(idPadre, WM_APP + 500 + miIndice - 1, 0, 0) == FALSE) {
                    PERROR("Error PostMsg");
                    raise(SIGINT);
                }

            }

            // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld y Arranco \n",colores[1 + (miIndice - 1) % 6], miIndice , m1.tipo);
        } else { //i==n
            fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, WM_APP + miIndice);
            if (GetMessage( & uMsg, NULL, WM_APP + miIndice, WM_APP + miIndice) == -1) {
                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }
            fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[1 + (miIndice - 1) % 6], miIndice , WM_APP + miIndice - 1 + 500);
            if (PostThreadMessageA(idPadre, WM_APP + miIndice - 1 + 500, 0, 0) == FALSE) {
                PERROR("Error PostMsg");
                raise(SIGINT);
            }

        }
    }*/
    // fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[1 + (miIndice - 1) % 6],i);
    while (1) {
        //printf("%d",semctl(sem_cruze, 0 ,GETVAL ));
        avance_controlado( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6], v);

    }
    


}

int main(int argc, char const * argv[]) {
    if (argc != 3) {
        perror("arg:");
        exit(4);
        //fprintf(stderr, "Error numero de argumentos:%d\n", argc);
    } else if (atoi(argv[1]) < 1 && atoi(argv[1]) > 20) {
        perror("arg:");
        exit(4);
        //fprintf(stderr, "Error numero de coches invalido\n");
    } else if (!(!atoi(argv[2]) || atoi(argv[2]) == 1)) {
        perror("Velocidad");
        exit(4);
    } else {
            int u = 0; //variable para bucles For
            int numCoches = atoi(argv[1]);
            int vel = atoi(argv[2]); //Punteros funciones

        PeekMessage(&test_msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);



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





    fflush(stderr);
    inicio_falonso(1);
  
    luzSem(1, 2);
    luzSem(0, 2);

    //fprintf(stderr, "PRE-CreaHijos\n");
    creaNhijos(3, 1);
    //fprintf(stderr, "POST-CreaHijos\n");

    while(1){
        pausa();
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