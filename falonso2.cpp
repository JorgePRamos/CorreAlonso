#include <windows.h>
#include <winuser.h>
#include <stdio.h> 
#include <assert.h>
#include <signal.h>
#include "falonso2.h"
#include <thread>        
#include <mutex>   
#include <iostream>        


#define TAM 100
typedef int (*DLL0Arg)(void);//funciones 0 argumentos
typedef int (*DLL1Arg)(int);//funciones 1 argumento INT
typedef int (*DLL1Argp)(int*);//funciones 1 argumento INT
typedef int (*DLL2Arg)(int,int);
typedef int (*DLL3Arg)(int,int,int);
typedef int (*DLL3ArgP)(int*,int*,int);
typedef void (*DLL1Argvoid)(const char *);

CRITICAL_SECTION sc1, critica_salida ,critica, critica_sem;
DWORD WINAPI funcionHilos (LPVOID pEstruct);
DWORD arrayPosiciones [274];
SECURITY_ATTRIBUTES test;
HANDLE sem_cruze = CreateSemaphore(
    NULL, // default security attributes
    6, // initial count
    6, // maximum count
    NULL);

HANDLE evento = CreateEvent(NULL, TRUE, FALSE, NULL);//Inicio Evento

HANDLE semH = CreateEvent(NULL, TRUE, FALSE,  NULL);
        
HANDLE semV = CreateEvent(NULL, TRUE, FALSE,  NULL);


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
MSG creaCola, uMsg, clMsg;
int num_coche=0;
DWORD idPadre;


//----------------------------------------------------------------------------------------------------------------------
//---------- Cambio_carril_cal
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
}//Fin Cambio_carril_cal

//----------------------------------------------------------------------------------------------------------------------
//---------- SemtoRed
void semtoRed(int sem) {

    if(sem==VERTICAL){   //vertical
    
    //fprintf(stderr, "Reset Vertical\n");

    ResetEvent(semV);
    }else if(sem==HORIZONTAL)
    {//horizontal
    //fprintf(stderr, "Reset Horizontal\n");

    ResetEvent(semH);
    }

    //fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas\n", GetCurrentThreadId());
    luzSem(sem, AMARILLO);
    LeaveCriticalSection(&critica_sem);
    //fprintf(stderr, "[%d]-Padre Salida Sem_Salida critica +1 pajitas\n", GetCurrentThreadId());
    for(int a= 0; a<6;a++){
            if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
            PERROR("WaitForSingleObject SEM RED");
    }
    //fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica -6 pajitas\n", GetCurrentThreadId());
    EnterCriticalSection(&critica_sem);
    luzSem(sem, ROJO);


}//Fin SemtoRed

//----------------------------------------------------------------------------------------------------------------------
//---------- semtoGreen
void semtoGreen(int sem) {
    if(!(ReleaseSemaphore(sem_cruze, 6, NULL))){
            PERROR("ReleaseSemaphore");
    } 
    //fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas\n", GetCurrentThreadId());

    luzSem(sem, VERDE);
    if(sem==VERTICAL){   //vertical
        SetEvent(semV);
    }else if(sem==HORIZONTAL)
    {//Horizontal
    SetEvent(semH);

    }
}//Fin semtoGreen

//----------------------------------------------------------------------------------------------------------------------
//---------- Avance_controlado
void avance_controlado(int * carril, int * desp, int color, int veloc) {
        EnterCriticalSection( & critica);
        //fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);
        arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();
        if ( * desp > 137 || * desp < 0 || * carril < 0 || * carril > 1 || color < 0 || color > 7) {
            LeaveCriticalSection( & critica);
        } //Error en el paso de argumentos

        //fprintf(stderr, "[%d] Color (%d) *%d+(*%d)*137 = %d \n", GetCurrentThreadId(), color, * desp, * carril, * desp + ( * carril) * 137);

        int pos_2 = (((( * desp) + 136) % 137) + (( * carril) * 137));//calculo posicion anterior preAvance

    //if (!(posOcup( * carril, ( * desp + 1) % 137))) {
    if(0==arrayPosiciones[(*desp+1)%137+*carril*137]){
         //fprintf(stderr, " [%d] Color (%d)  #### Posicion LIBRE: [%d] ####\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje

            if ( * desp == 20 && * carril) {//dep =21 y CArril = Izquierdo

                EnterCriticalSection( & critica_sem);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica


                if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 300);//#semaforo

                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica 

                    if((WaitForSingleObject(semV, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObjectA");
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300);//#mensaje

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                    PERROR("WaitForSingleObject Vertical 1");

                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica


                } else if (estadoSem(VERTICAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color);//#semaforo

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId());//#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId());//#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject vertical 2");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    EnterCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                } else
                    raise(SIGINT);

            } else if ( * desp == 19 && !( * carril)) { //dep =19 y CArril = Derecho

                EnterCriticalSection( & critica_sem);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) { 
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 301);//#semaforo

                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica 

                    if((WaitForSingleObject(semV, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObjectB");
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject vertical 3");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                } else if (estadoSem(VERTICAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId());//#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId());//#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject vertical 4");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                } else
                    raise(SIGINT);

            } else if ( * desp == 96 && * carril) {//dep =96 y CArril = Izquierdo
                EnterCriticalSection( & critica_sem);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                    LeaveCriticalSection( & critica);

                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 

                    if((WaitForSingleObject(semH, INFINITE))==WAIT_FAILED)//Espera Evento VErdeH
                        PERROR("WaitForSingleObject C");
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300);//#mensaje

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject HORIZONTAL 1");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

                } else if (estadoSem(HORIZONTAL) == VERDE) {

                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject HORIZONTAL 2");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else
                    raise(SIGINT);

            } else if ( * desp == 101 && !(* carril)) {//dep =102 y CArril = Derecho
                EnterCriticalSection( & critica_sem);
                //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
                    //fprintf(stderr, "[%d] Color (%d) Espero semaforo HORIZONTAL (%d)\n", GetCurrentThreadId(), color, 303); //#semaforo

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 

                    if((WaitForSingleObject(semH, INFINITE))==WAIT_FAILED)
                    PERROR("WaitForSingleObjectD");
                    //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject HORIZONTAL 3");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else if (estadoSem(HORIZONTAL) == VERDE) {
                    //fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

                    LeaveCriticalSection( & critica_sem);
                    //fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    LeaveCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

                    //EnterCriticalSection( & sem_cruze);

                    if((WaitForSingleObject(sem_cruze, INFINITE))==WAIT_FAILED)
                        PERROR("WaitForSingleObject HORIZONTAL 4");
                    //fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                    EnterCriticalSection( & critica);
                    //fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

                } else
                    raise(2);
            }
            //fprintf(stderr, "[%d] Color (%d) Avanzo a posicion (%d)\n", GetCurrentThreadId(), color, * desp + 1 % 137 + * carril * 137); //#posicion
            arrayPosiciones[ * desp + ( * carril) * 137] = 0;//Limpia pos Array
            if (avanceCoche(carril, desp, color) == -1) {
                PERROR("ERROR AL AVANZAR COCHE");
                raise(SIGINT);
            }
            arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();//Guarda ID en Nueva Pos
            int pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137));//Obtencion pos -1 cambio de carril bien
            if (( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 106 && * carril) || ( * desp == 25 && * carril)) {//Salida del semaforo

            if(!(ReleaseSemaphore(sem_cruze, 1, NULL)))//Dejamos pagita
                PERROR("ReleaseSemaphore");  
            //fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

            }

            if (( * desp == 131 && * carril) || ( * desp == 133 && ! * carril)) {//Paso por meta
                (contador) ++;
            }


            //if (posOcup( * carril, ((( * desp) + 135) % 137))) {//Comprobamos Pos -2 Para mensaje Bien
            if(arrayPosiciones[pos_2]){
            
                fprintf(stderr, " [%d] Color (%d) Envio mensaje POS_2-----> %d | %d : [%d]\n", GetCurrentThreadId(), color, pos_2, ((( * desp) + 135) % 137)+137*(*carril),arrayPosiciones[pos_2]); //#mensaje
                //fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, GetCurrentThreadId(), pos_2);
                for(int i = 0;i<275;i++){
                    fprintf(stderr, "%d: %4d |",i, arrayPosiciones[i]);
                }
                 fprintf(stderr, "\n");
                if (PostThreadMessageA(arrayPosiciones[pos_2], WM_USER+3, 3, 3) == 0) {
                    PERROR("ERROR AL MSGSND (pos -2 )");
                    raise(SIGINT);
                }
            }

            //pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
            //if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {
            if(arrayPosiciones[pos_cambio]){


                fprintf(stderr, "Color (%d) [%d] Posicion cambio ocupada %d\n", color, GetCurrentThreadId(), pos_cambio);
                 for(int i = 0;i<275;i++){
                fprintf(stderr, "%2d: %4d |",i, arrayPosiciones[i]);
                }
                 fprintf(stderr, "\n");
                if (PostThreadMessageA(arrayPosiciones[pos_cambio], WM_USER+4, 4, 4) == 0) {
                    PERROR("ERROR AL MSGSND (pos carril opuesto ocupada)");
                }
                //fprintf(stderr, "Color (%d) [%d] Envio Mesaje pos_cambio: %d\n", color, GetCurrentThreadId(), pos_cambio);


                //Limpieza Cola

                //fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", GetCurrentThreadId(), color, pos_cambio); //#mensaje

            }
            while(PeekMessage( & clMsg, NULL, WM_USER, WM_USER+4, PM_REMOVE)){
                //fprintf(stderr,"Limpiando Cola...\n");
            }

            LeaveCriticalSection( & critica);
            //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica

            velocidad(veloc, * carril, * desp);

        } else {
            //fprintf(stderr, "[%d] Color (%d) Posicion ocupada, compruebo cambio de carril: %d\n", GetCurrentThreadId(), color, * desp); //#posicion
              //Limpieza Cola
                while(PeekMessage( & clMsg, NULL, WM_USER, WM_USER+4, PM_REMOVE)){
                    //fprintf(stderr,"Limpiando Cola...\n");
                }

            //if (!posOcup(! * carril, cambio_carril_cal( * desp, * carril))) {//Efectuo cambio carril Si es posible
            if(0==arrayPosiciones[cambio_carril_cal( * desp, * carril)+!(*carril)*137]){
                if (cambioCarril(carril, desp, color) == -1) {
                    PERROR("ERROR AL CAMBIAR CARRIL");
                }
                //fprintf(stderr, "[%d] Color (%d) Cambio Carril: %d\n", GetCurrentThreadId(), color, dep_temp); //#posicion
                arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();//Guarda ID en Nueva Pos

                LeaveCriticalSection( & critica);
                velocidad(veloc, * carril, * desp);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

            } else {
                arrayPosiciones[ * desp + ( * carril) * 137] = GetCurrentThreadId();//Guarda ID en Nueva Pos

                LeaveCriticalSection( & critica);
                //fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica 

                //fprintf(stderr, " [%d] Color (%d) ESPERANDO Mensaje [%d]\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje
                if (GetMessage( & uMsg, NULL,WM_USER+3, WM_USER+4) == -1) {
                    PERROR("GetMessage");
                }
                //fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje
            }
        }
} //Fin Avance_controlado

//----------------------------------------------------------------------------------------------------------------------
//---------- CreaNhijos
int creaNhijos(int n, int v) {
    HANDLE hThreadArray [n];
    DWORD idHilo [n];
    static int i;
    num_coche=n;
    pParam arrayParam [n];
    srand(time(NULL));
    
    for (i = 1; i <= n; i++) {//CreaciOn N Hilos
        //fprintf(stderr, "%d %d**Soy el padre creando al hijo--> %d\n", (sizeof(hThreadArray)/sizeof(* hThreadArray )) ,n, i);
        arrayParam[i]= (pParam)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Param));
        arrayParam[i]->indice=i;
        arrayParam[i]->velocidad=rand()%100+1;
        arrayParam[i]->nCoches=n;
        if((hThreadArray[i]=CreateThread(NULL,0, funcionHilos,arrayParam[i], 0 , &idHilo[i]))==NULL){
            PERROR("Create Hilo");
        }
    }
    SetEvent(evento);
    return 0;
    } //Fin Nhijos

//----------------------------------------------------------------------------------------------------------------------
//---------- FuncionHilos
DWORD WINAPI funcionHilos (LPVOID pEstruct_2){
    pParam pEstruct = (pParam) pEstruct_2;
    int colores[] = {0,0,1,2,3,5,6,7};
    int n = pEstruct -> nCoches;
    int miIndice = pEstruct -> indice;
    int v = pEstruct -> velocidad;
    int miIndiceCarril = miIndice % 2;
    int b;
    //fprintf(stderr, "\t\tHola soy el hijo %d PID: %d\n", miIndice, GetCurrentThreadId());
    //EnterCriticalSection( & critica_salida);
    PeekMessage( & creaCola, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    //arrayPosiciones[miIndice] = GetCurrentThreadId();
    //LeaveCriticalSection( & critica_salida);
    //fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
    //EnterCriticalSection( & critica_salida);

    EnterCriticalSection( & critica);
    for (b = 136; b >= 0;) {//Busqueda de Sitio
        //fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[1 + (miIndice - 1) % 6], miIndice, b);
        b -= 2;
        //if (!(posOcup(miIndiceCarril, b))) {
        if (arrayPosiciones[b + (miIndiceCarril) * 137]==0) {

            //fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n", colores[1 + (miIndice - 1) % 6], miIndice);
            if (iniCoche( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6]) != 0) {
                //fprintf(stderr, "Color (%d) [%d] ERROR INICIO COCHE +++++++++++++++++++++++++++++++++\n", colores[1 + (miIndice - 1) % 6], miIndice);
                PERROR("ERROR INCIO COCHE");
                raise(SIGINT);
            } //Alterna Carril
            //fprintf(stderr, "Color (%d) [%d]        Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[1 + (miIndice - 1) % 6], miIndice, miIndiceCarril, b, colores[1 + (miIndice - 1) % 6]);
            //fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[1 + (miIndice - 1) % 6],i);
            arrayPosiciones[b + (miIndiceCarril) * 137] = GetCurrentThreadId();
            break;//Encontrado Sitio
        }
    }
    LeaveCriticalSection( & critica);
    //LeaveCriticalSection( & critica_salida);
    //fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
    // //fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[1 + (miIndice - 1) % 6],i);
    if(WaitForSingleObject(evento, INFINITE)==WAIT_FAILED)
                        PERROR("WaitForSingleObject vertical INI");
        
    while (1) {
        avance_controlado( & miIndiceCarril, & b, colores[1 + (miIndice - 1) % 6], v);
    }
}//Fin funcionHilos

//----------------------------------------------------------------------------------------------------------------------
//---------- Main
int main(int argc, char const * argv[]) {
    //Check atributos
    if (argc != 3) {
        perror("arg:");
        exit(4);
    } else if (atoi(argv[1]) < 1 && atoi(argv[1]) > 20) {
        perror("arg:");
        exit(4);
    } else if (!(!atoi(argv[2]) || atoi(argv[2]) == 1)) {
        perror("Velocidad");
        exit(4);
    } else {
        int u = 0; //variable para bucles For
        int numCoches = atoi(argv[1]); //Ncoches
        int vel = atoi(argv[2]); //Punteros funciones

        PeekMessage( & creaCola, NULL, WM_USER, WM_USER, PM_NOREMOVE); //Creacion cola  mensaje

        if ((hinstLib = LoadLibrary(TEXT("falonso2.dll"))) == NULL) { //Carga libreria en memoria del proceso
            PERROR("Error cargar DLL");
            return (1);
        }
        //Secciones Criticas
        InitializeCriticalSection( & sc1); //LanzarA STATUS_NO_MEMORY si no hay memoria
        InitializeCriticalSection( & critica_sem);
        InitializeCriticalSection( & critica_salida);
        InitializeCriticalSection( & critica);

        //Prototipos Funciones
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


        //Inicio comportamiento
        inicio_falonso(vel); //Inicio Circuito

        EnterCriticalSection( & critica_sem);
        semtoRed(HORIZONTAL); //Configuracion Incial Semaforos
        semtoGreen(VERTICAL);
        LeaveCriticalSection( & critica_sem);

        //fprintf(stderr, "PRE-CreaHijos\n");
        //creaNhijos(3, 1);
        creaNhijos(numCoches, vel); //DESCOMENTAR CUANDO ESTE DEPURADO

        //fprintf(stderr, "POST-CreaHijos\n");

        while (1) { //Alterdor Semaforos
            EnterCriticalSection( & critica_sem);
            semtoRed(VERTICAL);
            semtoGreen(HORIZONTAL);
            LeaveCriticalSection( & critica_sem);
            int b = 0;
            for (; b < 7; b++) {
                pausa();
            }
            EnterCriticalSection( & critica_sem);
            semtoRed(HORIZONTAL);
            semtoGreen(VERTICAL);
            LeaveCriticalSection( & critica_sem);
            for (b = 0; b < 7; b++) {
                pausa();
            }
        }

        FreeLibrary(hinstLib); //LiberaciOn de DLl
        return 0;
    }
} //Fin Main