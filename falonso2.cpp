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
}
coche;
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

    fprintf(stderr, " [%d] Quito mensajes residuales de SemRed [%d]\n", getpid(), sem);
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem, 303 - 2 * sem, PM_REMOVE);//ipc_Nowait
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem - 1, 303 - 2 * sem - 1, PM_REMOVE);//ipc_Nowait
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


    fprintf(stderr, " [%d] Quito mensajes residuales de SemRed [%d]\n", getpid(), sem);
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem, 303 - 2 * sem, PM_REMOVE);
    PeekMessageA( & test_msg, NULL,WM_USER+ 303 - 2 * sem - 1, 303 - 2 * sem - 1, PM_REMOVE);
    luzSem(sem, VERDE);

    fprintf(stderr, "[%d] Envio mensaje [ %d ]\n", getpid(),303 -2*sem);
    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 303 - 2 * sem, 0, 0) == FALSE) {
        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }

    fprintf(stderr, "[%d] Envio mensaje [ %d ]\n", getpid(),303 -2*sem-1);
    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 303 - 2 * sem - 1, 0, 0) == FALSE) {
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


    PeekMessageA( & test_msg, NULL,WM_USER+ pos_cambio + 1, pos_cambio + 1, PM_REMOVE);//IPC_NOWAIT
    fprintf(stderr, " [%d] Color (%d) Quito mensajes residuales de [%d]\n", getpid(), color, pos_cambio + 1);//#mensaje

    
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
            if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + pos_2 + 1, 0, 0) == 0) {
                PERROR("ERROR AL MSGSND (pos -2 ocupada post avance)");
                raise(SIGINT);
            }
            fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", getpid(), color, pos_2);//#mensaje
        }

        //pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
        if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {
            if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + pos_cambio + 1, 0, 0) == 0) {
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

            if (GetMessage( & uMsg, NULL,WM_USER+ ( * desp + * carril * 137) + 1, ( * desp + * carril * 137) + 1) == -1) {
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

    int colores[] = {0,0,1,2,3,5,6,7};
    static int i = 0;
    int miIndice, miIndiceCarril;
    pid_t pid_child, pidPoceso;
    int b;

    for (i = 1; i <= n; i++) {
        ////  fprintf(stderr, "**Soy el padre creando al hijo--> %d\n", i);
        miIndice = i;
        miIndiceCarril = miIndice % 2;

        /*if ((pid_child = fork()) == -1) {
            PERROR("ERROR CREACION HIJOS");
            raise(SIGINT);
            
        }
    */
        if (pid_child == 0) {
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

            //SINCRONIZACION SALIDA
            if (n != 1) {
                if (miIndice != n) {
                    //  fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, i + 1);
                    if (miIndice != 1) {
                        //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                        //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                        if (GetMessage( & uMsg, NULL,WM_USER+ miIndice, 100+miIndice) == -1) {
                            PERROR("[GetMessage] pausa Sem");
                            raise(SIGINT);
                        }

                    }
                    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 100+miIndice + 1, 0, 0) == FALSE)
                        PERROR("Error PostMsg");
                    raise(SIGINT);

                    if (GetMessage( & uMsg, NULL,WM_USER+ 500 + miIndice, 500 + miIndice) == -1) {
                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }

                    if (miIndice != 1) {
                        // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i,  m1.tipo);
                        // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                        if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + 500 + miIndice - 1, 0, 0) == FALSE)
                            PERROR("Error PostMsg");
                        raise(SIGINT);

                    }
                    if (miIndice == n) {
                        if (GetMessage( & uMsg, NULL,WM_USER+ 600, 600) == -1) {
                            PERROR("[GetMessage] pausa Sem");
                            raise(SIGINT);
                        }
                    }
                    // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld y Arranco \n",colores[miIndice], i , m1.tipo);
                } else { //i==n
                    if (GetMessage( & uMsg, NULL,WM_USER+ miIndice,100+miIndice) == -1) {
                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                    if (PostThreadMessageA(GetCurrentThreadId(),WM_USER + miIndice - 1 + 500, 0, 0) == FALSE)
                        PERROR("Error PostMsg");
                    raise(SIGINT);

                    // fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n",colores[miIndice], i , m1.tipo);
                    // fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[miIndice],i, 2*n+i);
                }
            }
            // fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[miIndice],i);
            while (1) {
                //printf("%d",semctl(sem_cruze, 0 ,GETVAL ));
                avance_controlado( & miIndiceCarril, & b, colores[1+(miIndice-1)%6], v);
            }

            exit(0);
        } else if (pidPoceso == -1) {
            PERROR("ERROR FORK");
            raise(SIGINT);
        }
    }

    return 0;

    } //Fin Nhijos

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