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

CRITICAL_SECTION sc1, critica_salida ,critica;
DWORD WINAPI funcionHilos (LPVOID pEstruct);
DWORD arrayPosiciones [272];
HANDLE evento, pistola;
/*
HANDLE critica = CreateSemaphore(
    NULL, // default security attributes
    1, // initial count
    1, // maximum count
    NULL);

HANDLE critica_salida = CreateSemaphore(
    NULL, // default security attributes
    1, // initial count
    1, // maximum count
    NULL);

*/
HANDLE sem_cruze = CreateSemaphore(
    NULL, // default security attributes
    6, // initial count
    6, // maximum count
    NULL);






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
/*
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
    

    exit(1);
}*/

/*
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
    

    } else {
        raise(SIGINT);
    }

}
*/
//---------------------------------------------------------------------------
//SEMtoRed
void semtoRed(int sem) {


    luzSem(sem, AMARILLO);

    LeaveCriticalSection(&critica_salida);
    //fprintf(stderr, "[%d]-Padre Salida Sem_Salida critica +1 pajitas: %d\n", GetCurrentThreadId(),semctl(critica_salida, 0, GETVAL));//#getval
    //fprintf(stderr, "[%d]-Padre Salida Sem_Salida critica +1 pajitas\n", GetCurrentThreadId());


    //enterCritic("sem_cruze", 6);
    for(int a= 0; a<6;a++)
            //EnterCriticalSection(&sem_cruze);

            if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject");


    //fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica -6 pajitas: %d\n", GetCurrentThreadId(),semctl(sem_cruze, 0, GETVAL));//#getval
    //fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica -6 pajitas\n", GetCurrentThreadId());


    EnterCriticalSection(&critica_salida);
    //fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas: %d\n", GetCurrentThreadId(),semctl(critica_salida, 0, GETVAL));//#getval
    //fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas\n", GetCurrentThreadId());


    luzSem(sem, ROJO);
}

//---------------------------------------------------------------------------
//SEMtoGreen
void semtoGreen(int sem) {
    //for(int a= 0; a<6;a++)
        //LeaveCriticalSection(&sem_cruze);
 if(!(ReleaseSemaphore(sem_cruze, 6, NULL)))
            PERROR("ReleaseSemaphore");
        
        

    //leaveCritic("sem_cruze", 6);
    //fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas: %d\n", GetCurrentThreadId(),semctl(sem_cruze, 0, GETVAL));//getval
    //fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas\n", GetCurrentThreadId());



    luzSem(sem, VERDE);


    if (PostThreadMessageA(idPadre,WM_APP + 303 - 2 * sem, 0, 0) == FALSE) {
        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }
    //fprintf(stderr, " [%d] Envio mensaje tipo %d \n", GetCurrentThreadId(),303 -2*sem-1);
    if (PostThreadMessageA(idPadre,WM_APP + 303 - 2 * sem - 1, 0, 0) == FALSE) {

        PERROR("ERROR AL MSGSND");
        raise(SIGINT);
    }

}

//---------------------------------------------------------------------------
//AVANCE CONTROLADO
void avance_controlado(int * carril, int * desp, int color, int v) {
        EnterCriticalSection( & critica);
        arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();

        //fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(),color, semctl(critica, 0, GETVAL));//#getval
        //fprintf(stderr, "[%d] Color (%d) *%d+(*%d)*137 = %d \n", GetCurrentThreadId(), color, * desp, * carril, * desp + ( * carril) * 137);
        //fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);


        if ( * desp > 137 || * desp < 0 || * carril < 0 || * carril > 1 || color < 0 || color > 7) {
            LeaveCriticalSection( & critica);
            raise(SIGINT);
        } //Error en el paso de argumentos

        int dep_temp = * desp, pos_2 = (((( * desp) + 136) % 137) + (( * carril) * 137));

        //PeekMessageA( & test_msg, NULL,WM_APP+ pos_cambio + 1, pos_cambio + 1, PM_REMOVE);//IPC_NOWAIT
        //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, GetCurrentThreadId(), pos_cambio);

        // PeekMessageA( & test_msg, NULL,WM_APP+ pos_2 + 1, pos_2 + 1, PM_REMOVE);
        //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, GetCurrentThreadId(), pos_2);
        //fprintf(stderr, "Color (%d) [%d] Limpio mensajes (rcv)\n",color, GetCurrentThreadId());
        //fprintf(stderr, "Color (%d) [%d]  COMPRUEBO POSICION SIGUIENTE %d (%d+1%%137+%d*137)\n", color, GetCurrentThreadId(), *desp + 1 % 137 + *carril *137, *desp, *carril);


        if (!(posOcup( * carril, ( * desp + 1) % 137))) {

            if ( * desp == 21 && * carril) {
                EnterCriticalSection( & critica_salida);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 300); //#semaforo

                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 


                    if (GetMessage( & uMsg, NULL, WM_USER + 300, 300) == -1) {

                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

                    //EnterCriticalSection( & sem_cruze);

            if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject");

                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                } else if (estadoSem(VERTICAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                } else
                    raise(SIGINT);

            } else if ( * desp == 20 && !( * carril)) {
                EnterCriticalSection( & critica_salida);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 301); //#semaforo

                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 


                    if (GetMessage( & uMsg, NULL, WM_USER + 300, 300) == -1) {

                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                } else if (estadoSem(VERTICAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else
                    raise(SIGINT);

            } else if ( * desp == 97 && * carril) {
                EnterCriticalSection( & critica_salida);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 


                    if (GetMessage( & uMsg, NULL, WM_USER + 300, 300) == -1) {
                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje


                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                } else if (estadoSem(HORIZONTAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                } else
                    raise(SIGINT);

            } else if ( * desp == 97 && ! * carril) {
                EnterCriticalSection( & critica_salida);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo HORIZONTAL (%d)\n", GetCurrentThreadId(), color, 303); //#semaforo

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    LeaveCriticalSection( & critica_salida);

                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 


                    if (GetMessage( & uMsg, NULL, WM_USER + 300, 300) == -1) {

                        PERROR("[GetMessage] pausa Sem");
                        raise(SIGINT);
                    }
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(sem_cruze, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else if (estadoSem(HORIZONTAL) == VERDE) {} else if (estadoSem(VERTICAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_salida);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else
                    raise(SIGINT);
            }
            //fprintf(stderr, "[%d] Color (%d) Avanzo a posicion (%d)\n", GetCurrentThreadId(), color, * desp + 1 % 137 + * carril * 137); //#posicion
            arrayPosiciones[ * desp + ( * carril) * 137] = 0;
            if (avanceCoche(carril, desp, color) == -1) {
                PERROR("ERROR AL AVANZAR COCHE");
                raise(SIGINT);
            }
            arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();
            int pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137));
            if (( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 106 && * carril) || ( * desp == 25 && * carril)) {

                //LeaveCriticalSection( & sem_cruze);
 if(!(ReleaseSemaphore(sem_cruze, 1, NULL)))
            PERROR("ReleaseSemaphore");
                
                //fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

            }
            if (( * desp == 131 && * carril) || ( * desp == 133 && ! * carril)) {
                (contador) ++;
            }

            //pos_2 = (((( * desp) + 135) % 137) + (( * carril) * 137)) + 1
            if (posOcup( * carril, ((( * desp) + 135) % 137))) {

                //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, GetCurrentThreadId(), pos_2);
                EnterCriticalSection( & sc1);
                if (PostThreadMessageA(arrayPosiciones[pos_2], WM_APP + 3, 0, 0) == 0) {
                    PERROR("ERROR AL MSGSND (pos -2 ocupada post avance)");
                    raise(SIGINT);
                }
                LeaveCriticalSection( & sc1);

                //fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", GetCurrentThreadId(), color, pos_2); //#mensaje
            }

            //pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
            if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {

                //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, GetCurrentThreadId(), pos_cambio);
                EnterCriticalSection( & sc1);
                if (PostThreadMessageA(arrayPosiciones[pos_cambio], WM_APP + 3, 0, 0) == 0) {
                    //fprintf(stderr, " [%d] Pos actual: %d carril %d pos cambio %d [%d]=%d\n", GetCurrentThreadId(), * desp, * carril, pos_cambio, cambio_carril_cal((( * desp) + 136) % 137, * carril), arrayPosiciones[cambio_carril_cal((( * desp) + 136) % 137, * carril)]); //#mensaje
                    PERROR("ERROR AL MSGSND (pos carril opuesto ocupada)");
                    raise(SIGINT);
                }
                LeaveCriticalSection( & sc1);

                //fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", GetCurrentThreadId(), color, pos_cambio); //#mensaje

            }
            LeaveCriticalSection( & critica);
            //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
            //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica


            velocidad(50, * carril, * desp);

        } else {
            //fprintf(stderr, "[%d] Color (%d) Posicion ocupada, compruebo cambio de carril: %d\n", GetCurrentThreadId(), color, * desp); //#posicion

            if (!posOcup(! * carril, cambio_carril_cal( * desp, * carril))) {
                if (cambioCarril(carril, desp, color) == -1) {
                    PERROR("ERROR AL CAMBIAR CARRIL");
                    raise(SIGINT);
                }
                //fprintf(stderr, "[%d] Color (%d) Cambio Carril: %d\n", GetCurrentThreadId(), color, dep_temp); //#posicion

                LeaveCriticalSection( & critica);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

            } else {


                LeaveCriticalSection( & critica);

                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color, semctl(critica, 0, GETVAL));//#critica getval
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica 

                if (GetMessage( & uMsg, NULL, WM_APP + ( * desp + * carril * 137) + 1, ( * desp + * carril * 137) + 1) == -1) {

                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }
                //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje
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

    }
    SetEvent(evento);
    //WaitForMultipleObjects(n,hThreadArray, TRUE, INFINITE);

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
    EnterCriticalSection( & critica_salida);
    PeekMessage( & test_msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    arrayPosiciones[miIndice] = GetCurrentThreadId();
    LeaveCriticalSection( & critica_salida);
    //fprintf(stderr, "\t\tHola soy el hijo %d PID: %d\n", miIndice, GetCurrentThreadId());
    WaitForSingleObject(evento, INFINITE);
    //fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
    EnterCriticalSection( & critica_salida);

    for (b = 136; b >= 0;) {
        //fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[1 + (miIndice - 1) % 6], miIndice, b);
        b -= 2;
        //if (memoria[b + miIndiceCarril * 137] == ' ') {
             EnterCriticalSection( & critica);
        if (!(posOcup(miIndiceCarril, b))) {
            //fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n", colores[1 + (miIndice - 1) % 6], miIndice);
            if (iniCoche( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6]) != 0) {
                //fprintf(stderr, "Color (%d) [%d] ERROR INICIO COCHE +++++++++++++++++++++++++++++++++\n", colores[1 + (miIndice - 1) % 6], miIndice);
                PERROR("ERROR INCIO COCHE");
                raise(SIGINT);
            } //miendice alterna Izquierdo y derecho
            //fprintf(stderr, "Color (%d) [%d] Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[1 + (miIndice - 1) % 6], miIndice, miIndiceCarril, b, colores[1 + (miIndice - 1) % 6]);
            // //fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[1 + (miIndice - 1) % 6],i);
            LeaveCriticalSection( & critica);
            break;
        }
        LeaveCriticalSection( & critica);
    }

    LeaveCriticalSection( & critica_salida);
    //fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
/*
    if (n != 1) {
        if(miIndice==n)
            SetEvent(pistola);
        else
            WaitForSingleObject(pistola, INFINITE);

        /*
        if (miIndice != n) {
            //  //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, i + 1);
            if (miIndice != 1) {
                //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n", colores[1 + (miIndice - 1) % 6], miIndice, WM_APP + miIndice);
                if (GetMessage( & uMsg, NULL, WM_APP + 1, WM_APP + 1) == -1) {
                    PERROR("[GetMessage] pausa Sem");
                    raise(SIGINT);
                }

            }
            //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n", colores[1 + (miIndice - 1) % 6], miIndice, WM_APP + miIndice + 1);
            EnterCriticalSection( & sc1);
            if (PostThreadMessageA(arrayPosiciones[miIndice + 1], WM_APP + 1, 0, 0) == FALSE) {
                PERROR("Error PostMsg");
                raise(SIGINT);
            }
            LeaveCriticalSection( & sc1);


            if (GetMessage( & uMsg, NULL, WM_APP + 2, WM_APP + 2) == -1) {
                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }

            if (miIndice != 1) {
                //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n", colores[1 + (miIndice - 1) % 6], miIndice, WM_APP + 500 + miIndice - 1);
                // //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n",colores[1 + (miIndice - 1) % 6],miIndice, 2*n+i);
                EnterCriticalSection( & sc1);
                if (PostThreadMessageA(arrayPosiciones[miIndice - 1], WM_APP + 2, 0, 0) == FALSE) {
                    PERROR("Error PostMsg");
                    raise(SIGINT);
                }
                LeaveCriticalSection( & sc1);


            }

            // //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld y Arranco \n",colores[1 + (miIndice - 1) % 6], miIndice , m1.tipo);
        } else { //i==n
            //fprintf(stderr, "Color (%d) [%d] Espero al mensaje %d\n", colores[1 + (miIndice - 1) % 6], miIndice, WM_APP + miIndice);
            if (GetMessage( & uMsg, NULL, WM_APP + 1, WM_APP + 1) == -1) {
                PERROR("[GetMessage] pausa Sem");
                raise(SIGINT);
            }
            //fprintf(stderr, "Color (%d) [%d] Envio mensaje %ld \n", colores[1 + (miIndice - 1) % 6], miIndice, WM_APP + miIndice - 1 + 500);
            EnterCriticalSection( & sc1);
            if (PostThreadMessageA(arrayPosiciones[miIndice - 1], WM_APP + 2, 0, 0) == FALSE) {
                PERROR("Error PostMsg");
                raise(SIGINT);
            }
            LeaveCriticalSection( & sc1);
        }
        
    }
    */
    // //fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[1 + (miIndice - 1) % 6],i);
    arrayPosiciones[b + (miIndiceCarril) * 137] = GetCurrentThreadId();
    while (1) {
        //printf("%d",semctl(sem_cruze, 0 ,GETVAL ));
        avance_controlado( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6], v);

    }



    }
int main(int argc, char
        const * argv[]) {
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
        }
        PeekMessage( & test_msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);



        //---------------------------------------------------------------------------
        //Var's


        //CreateThread(NULL,0,avance_controlado,NULL,0);

        if ((hinstLib = LoadLibrary(TEXT("falonso2.dll"))) == NULL) { //cargas la libreria en memoria del proceso
            PERROR("Error cargar DLL");
            return (1);
        }

        InitializeCriticalSection( & sc1); //Control de errores?
        InitializeCriticalSection( & critica_salida); //Control de errores?
        InitializeCriticalSection( & critica); //Control de errores?


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



        if ((evento = CreateEvent(NULL, TRUE, FALSE, TEXT("Evento"))) == NULL) {
            PERROR("Creacion evento");
            exit(1);
        }
        if ((pistola = CreateEvent(NULL, TRUE, FALSE, TEXT("Evento"))) == NULL) {
            PERROR("Creacion evento");
            exit(1);
        }


        inicio_falonso(1);


            luzSem(1, 2);
            luzSem(0, 2);
            /*
        EnterCriticalSection( & critica_salida);
        semtoRed(HORIZONTAL);
        semtoGreen(VERTICAL);
        LeaveCriticalSection( & critica_salida);
    */
        //fprintf(stderr, "PRE-CreaHijos\n");
        creaNhijos(3, 1);
        //fprintf(stderr, "POST-CreaHijos\n");

        while (1) {
    pausa();
}
/*
        while (!0) {

            EnterCriticalSection( & critica_salida);
            semtoRed(HORIZONTAL);
            semtoGreen(VERTICAL);
            LeaveCriticalSection( & critica_salida);



            int b = 0;
            for (; b < 7; b++) {
                pausa();
            }
            EnterCriticalSection( & critica_salida);
            semtoRed(VERTICAL);
            semtoGreen(HORIZONTAL);
            LeaveCriticalSection( & critica_salida);
            for (b = 0; b < 7; b++) {
                pausa();
            }
        }*/

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