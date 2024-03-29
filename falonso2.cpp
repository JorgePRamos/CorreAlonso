#include <windows.h>
#include <winuser.h>
#include <stdio.h> 
#include <assert.h>
#include <signal.h>
#include "falonso2.h"
#include <iostream>        


#define TAM 100
typedef int (*DLL0Arg)(void);//funciones 0 argumentos
typedef int (*DLL1Arg)(int);//funciones 1 argumento INT
typedef int (*DLL1Argp)(int*);//funciones 1 argumento INT
typedef int (*DLL2Arg)(int, int);
typedef int (*DLL3Arg)(int, int, int);
typedef int (*DLL3ArgP)(int*, int*, int);
typedef void (*DLL1Argvoid)(const char*);

CRITICAL_SECTION  critica_salida, critica, critica_sem;
DWORD WINAPI funcionHilos(LPVOID pEstruct);
DWORD WINAPI hiloContador(LPVOID pContador);
DWORD arrayPosiciones[274];
SECURITY_ATTRIBUTES test;
HANDLE sem_cruze = CreateSemaphore(
	NULL, // default security attributes
	6, // initial count
	6, // maximum count
	NULL);

HANDLE evento = CreateEvent(NULL, TRUE, FALSE, NULL);//Inicio Evento

HANDLE semH = CreateEvent(NULL, TRUE, FALSE, NULL);

HANDLE semV = CreateEvent(NULL, TRUE, FALSE, NULL);

int  contador = 0;
typedef struct Coche {
	int posicion;
	int carril;
} coche;
typedef struct parametros {
	int nCoches;
	int velocidad;
	int indice;
} *pParam, Param;
HINSTANCE hinstLib = NULL;
DLL1Arg inicio_falonso = NULL, estadoSem = NULL;
DLL1Argp f_fin = NULL;
DLL2Arg luzSem = NULL, posOcup = NULL;
DLL3Arg velocidad = NULL;
DLL3ArgP iniCoche = NULL, avanceCoche = NULL, cambioCarril = NULL;
DLL0Arg pausa = NULL;
DLL1Argvoid p_error = NULL;
MSG creaCola, uMsg, clMsg;
int num_coche = 0;
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
/*
#define CARRIL_DERECHO       0
#define CARRIL_IZQUIERDO     1
*/
void sendMess(int desp, int carril) {
		if (arrayPosiciones[(desp+136)%137 + carril * 137])
			if (PostThreadMessageA(arrayPosiciones[(desp + 136) % 137 + carril * 137], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (post cambio carril)");
				raise(SIGINT);
			}
	if ((carril && desp == 15) || (!carril && desp == 60)) {
		if (arrayPosiciones[desp - 1 + !carril * 137])
			if (PostThreadMessageA(arrayPosiciones[desp - 1 + !carril * 137], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )a");
				raise(SIGINT);
			}
	}
	else if (carril && desp == 134) {
		if (arrayPosiciones[desp + 1 + !carril * 137])
			if (PostThreadMessageA(arrayPosiciones[135], WM_USER + 3, 3, 3) == 0) { //135 0
				PERROR("ERROR AL MSGSND (pos -2 )b");
				raise(SIGINT);
			}
	}
	else if ((!carril && desp == 14) || (carril && desp == 59)) {
		if (arrayPosiciones[desp + !carril * 137])
			if (PostThreadMessageA(arrayPosiciones[desp + !carril * 137], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )c");
				raise(SIGINT);
			}
	}
	else if (carril && desp == 63) {
		if (arrayPosiciones[65])
			if (PostThreadMessageA(arrayPosiciones[65], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )d");
				raise(SIGINT);
			}
		if (arrayPosiciones[66])
			if (PostThreadMessageA(arrayPosiciones[66], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )e");
				raise(SIGINT);
			}
	}
	else if (!carril && desp == 67) {
		if (arrayPosiciones[63 + 137])
			if (PostThreadMessageA(arrayPosiciones[63 + 137], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )f");
				raise(SIGINT);
			}
	}
	else if (!carril && desp == 135) {
		if (arrayPosiciones[133 + 137])
			if (PostThreadMessageA(arrayPosiciones[133 + 137], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )g");
				raise(SIGINT);
			}
	}
	else if (carril && desp == 134) {
		if (arrayPosiciones[135])
			if (PostThreadMessageA(arrayPosiciones[135], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (pos -2 )h");
				raise(SIGINT);
			}
	}
	else {
		int t_cambio = cambio_carril_cal(desp, carril) + (!carril) * 137;
		if (arrayPosiciones[t_cambio])
			if (PostThreadMessageA(arrayPosiciones[t_cambio], WM_USER + 3, 3, 3) == 0) {
				PERROR("ERROR AL MSGSND (post cambio carrilA)");
				raise(SIGINT);
			}
		if ((!carril && (desp == 15) || (desp == 131) || (desp == 130)) || (desp == 136) || (carril && (desp == 29) || (desp == 60) || (desp == 64) || (desp == 61) || (desp == 31))) {
			if (arrayPosiciones[t_cambio - 1])
				if (PostThreadMessageA(arrayPosiciones[t_cambio - 1], WM_USER + 3, 3, 3) == 0) {//
					PERROR("ERROR AL MSGSND (post cambio carrilB)");
					raise(SIGINT);
				}
			if ((!carril && (desp == 136))) {
				if (arrayPosiciones[136 + 137])
					if (PostThreadMessageA(arrayPosiciones[136 + 137], WM_USER + 3, 3, 3) == 0) {
						//fprintf(stderr, "\n[%d] ",t_cambio+1 ); //#critica
						PERROR("ERROR AL MSGSND (post cambio carril)");
						raise(SIGINT);
					}
			}
			if (((carril && (desp == 64)))) {
				if (arrayPosiciones[69])
					if (PostThreadMessageA(arrayPosiciones[69], WM_USER + 3, 3, 3) == 0) {
						//fprintf(stderr, "\n[%d] ",t_cambio+1 ); //#critica
						PERROR("ERROR AL MSGSND (post cambio carrilC)");
						raise(SIGINT);
					}
			}
			if ((!carril && (desp == 130))) {
				if (arrayPosiciones[125 + 137])
					if (PostThreadMessageA(arrayPosiciones[125 + 137], WM_USER + 3, 3, 3) == 0) {
						PERROR("ERROR AL MSGSND (post cambio carrilD)");
						raise(SIGINT);
					}
			}
		}

	}

}
//----------------------------------------------------------------------------------------------------------------------
//---------- SemtoRed
void semtoRed(int sem) {

	if (sem == VERTICAL) {   //vertical

	//fprintf(stderr, "Reset Vertical\n");

		ResetEvent(semV);
	}
	else if (sem == HORIZONTAL)
	{//horizontal
	//fprintf(stderr, "Reset Horizontal\n");

		ResetEvent(semH);
	}

	//fprintf(stderr, "[%d]-Padre Enter critica_salida critica -1 pajitas\n", GetCurrentThreadId());
	luzSem(sem, AMARILLO);
	LeaveCriticalSection(&critica_sem);
	fprintf(stderr, "--------------      Comienzo a pillar pajitas\n");
	for (int a = 0; a < 6; a++) {
		switch (WaitForSingleObject(sem_cruze, INFINITE))
		{
		case WAIT_FAILED:
			PERROR("WaitForSingleObject SEM RED");
			break;
		case WAIT_OBJECT_0:
			PERROR("WaitForSingleObject W0 SEM RED");
			break;

		default:
			break;
		}
		//if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
			//fprintf(stderr,"PAJITA %d\n", a+1);
	}
	fprintf(stderr, "[%d]-Padre Enter Sem_Cruze -6 pajitas\n", GetCurrentThreadId());
	EnterCriticalSection(&critica_sem);
	fprintf(stderr, "[%d]-Padre Enter Sem_Cruze critica \n", GetCurrentThreadId());
	luzSem(sem, ROJO);


}//Fin SemtoRed

//----------------------------------------------------------------------------------------------------------------------
//---------- semtoGreen
void semtoGreen(int sem) {
	if (!(ReleaseSemaphore(sem_cruze, 6, NULL))) {
		PERROR("ReleaseSemaphore");
	}
	//fprintf(stderr, "[%d]-Padre Salida Sem_Cruze critica +6 pajitas\n", GetCurrentThreadId());

	luzSem(sem, VERDE);
	if (sem == VERTICAL) {   //vertical
		SetEvent(semV);
	}
	else if (sem == HORIZONTAL)
	{//Horizontal
		SetEvent(semH);

	}
}//Fin semtoGreen

//----------------------------------------------------------------------------------------------------------------------
//---------- Avance_controlado
void avance_controlado(int* carril, int* desp, int color, int v) {
	//int flag = 0;
	EnterCriticalSection(&critica);
	//fprintf(stderr, "[%d] Color (%d)Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);
	arrayPosiciones[*desp + (*carril) * 137] = GetCurrentThreadId();


	//fprintf(stderr, "[%d] Color (%d) *%d+(*%d)*137 = %d \n", GetCurrentThreadId(), color, * desp, * carril, * desp + ( * carril) * 137);

	int pos_2 = ((((*desp) + 136) % 137) + ((*carril) * 137));//calculo posicion anterior preAvance

	if (!(posOcup(*carril, (*desp + 1) % 137))) {
		//if (!(arrayPosiciones[(((*desp) + 1) % 137) + ((*carril) * 137)])) {
			/*for(int a=0;a<=274;a++){
				fprintf(stderr, "%d: %6d |", a, arrayPosiciones[a]);
			}
			*/
			//fprintf(stderr, "%d:%d: %6d |\n", (((*desp) + 1) % 137) + ((*carril) * 137), ((*desp + 1) % 137) + ((*carril) * 137), arrayPosiciones[(((*desp) + 1) % 137) + ((*carril) * 137)]);
		//fprintf(stderr, " [%d] Color (%d)  #### Posicion LIBRE: [%d] ####\n", GetCurrentThreadId(), color, (((*desp) + 1) % 137) + ((*carril) * 137)); //#mensaje

		if (*desp == 20 && *carril) {//dep =21 y CArril = Izquierdo
			LeaveCriticalSection(&critica);
			EnterCriticalSection(&critica_sem);
			fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica
			EnterCriticalSection(&critica);


			if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
				//fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 300);//#semaforo

				LeaveCriticalSection(&critica);

				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica 

				if ((WaitForSingleObject(semV, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObjectA");
				//fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300);//#mensaje

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject Vertical 1");

				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica


			}
			else if (estadoSem(VERTICAL) == VERDE) {
				//fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color);//#semaforo

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId());//#critica

				LeaveCriticalSection(&critica);
				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId());//#critica

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject vertical 2");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

				EnterCriticalSection(&critica);

				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

			}
			else
				raise(SIGINT);

		}
		else if (*desp == 19 && !(*carril)) { //dep =19 y CArril = Derecho
			LeaveCriticalSection(&critica);
			EnterCriticalSection(&critica_sem);
			fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica
			EnterCriticalSection(&critica);

			if ((estadoSem(VERTICAL) == ROJO || estadoSem(VERTICAL) == AMARILLO)) {
				//fprintf(stderr, "[%d] Color (%d) Espero semaforo VERTICAL (%d)\n", GetCurrentThreadId(), color, 301);//#semaforo

				LeaveCriticalSection(&critica);

				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color);//#critica 

				if ((WaitForSingleObject(semV, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObjectB");
				//fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject vertical 3");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

			}
			else if (estadoSem(VERTICAL) == VERDE) {
				//fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId());//#critica

				LeaveCriticalSection(&critica);
				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId());//#critica

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject vertical 4");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

			}
			else
				raise(SIGINT);

		}
		else if (*desp == 96 && *carril) {//dep =96 y CArril = Izquierdo
			LeaveCriticalSection(&critica);
			EnterCriticalSection(&critica_sem);
			fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica
			EnterCriticalSection(&critica);

			if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
				LeaveCriticalSection(&critica);

				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 

				if ((WaitForSingleObject(semH, INFINITE)) == WAIT_FAILED)//Espera Evento VErdeH
					PERROR("WaitForSingleObject C");
				//fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300);//#mensaje

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject HORIZONTAL 1");
				//flag = 1;
				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color);//#critica

			}
			else if (estadoSem(HORIZONTAL) == VERDE) {

				//fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

				LeaveCriticalSection(&critica);
				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject HORIZONTAL 2");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

			}
			else
				raise(SIGINT);

		}
		else if (*desp == 101 && !(*carril)) {//dep =102 y CArril = Derecho
			LeaveCriticalSection(&critica);
			EnterCriticalSection(&critica_sem);
			fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica
			EnterCriticalSection(&critica);

			if (estadoSem(HORIZONTAL) == ROJO || estadoSem(HORIZONTAL) == AMARILLO) {
				//fprintf(stderr, "[%d] Color (%d) Espero semaforo HORIZONTAL (%d)\n", GetCurrentThreadId(), color, 303); //#semaforo

				LeaveCriticalSection(&critica);
				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Entrada critica_salida critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica 

				if ((WaitForSingleObject(semH, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObjectD");
				//fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, 300); //#mensaje

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject HORIZONTAL 3");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

			}
			else if (estadoSem(HORIZONTAL) == VERDE) {
				//fprintf(stderr, "[%d] Color (%d) Semaforo VERTICAL (VERDE)\n", GetCurrentThreadId(), color); //#semaforo

				LeaveCriticalSection(&critica_sem);
				//fprintf(stderr, "[%d] Color (%d) Salida critica_salida critica +1 pajitas\n", GetCurrentThreadId()); //#critica

				LeaveCriticalSection(&critica);
				//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId()); //#critica

				//EnterCriticalSection( & sem_cruze);

				if ((WaitForSingleObject(sem_cruze, INFINITE)) == WAIT_FAILED)
					PERROR("WaitForSingleObject HORIZONTAL 4");
				//flag = 1;
				fprintf(stderr, "[%d] Color (%d) Entrada sem_cruze critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

				EnterCriticalSection(&critica);
				fprintf(stderr, "[%d] Color (%d) Entrada Critica critica -1 pajitas\n", GetCurrentThreadId(), color); //#critica

			}
			else { raise(SIGINT); }
		}


		arrayPosiciones[*desp + (*carril) * 137] = 0;//Limpia pos Array
		//fprintf(stderr, "%d:%d: %6d |\n", (((*desp) + 1) % 137) + ((*carril) * 137), ((*desp + 1) % 137) + ((*carril) * 137), arrayPosiciones[(((*desp) + 1) % 137) + ((*carril) * 137)]);
		//if (!(arrayPosiciones[(((*desp) + 1) % 137) + ((*carril) * 137)])) {
		if (!(posOcup(*carril, (*desp + 1) % 137))) {
			int tdesp = *desp;
			//fprintf(stderr, "[%d] Color (%d) Avanzo a posicion (%d)\n", GetCurrentThreadId(), color, * desp + 1 % 137 + * carril * 137); //#posicion
			if (avanceCoche(carril, desp, color) == -1) {
				PERROR("ERROR AL AVANZAR COCHE");
				raise(SIGINT);
			}
			arrayPosiciones[*desp + (*carril) * 137] = GetCurrentThreadId();//Guarda ID en Nueva Pos

			int pos_cambio = (cambio_carril_cal(((*desp) + 136) % 137, *carril) + ((!*carril) * 137));//Obtencion pos -1 cambio de carril bien




			int check = ((*desp) == 111 && !(*carril)) || ((*desp) == 24 && !(*carril)) || (*desp == 106 && *carril) || (*desp == 25 && *carril);
			fprintf(stderr, "[%d] Color (%d) posicion (%d, %d) if.check (%d)\n", GetCurrentThreadId(), color, *desp, *carril, check); //#posicion

	
			if (check) {
				long varsem;
				if (!(ReleaseSemaphore(sem_cruze, 1, &varsem))) {//Dejamos pagita
					PERROR("ReleaseSemaphore");
					raise(SIGINT);
				}
				fprintf(stderr, "[%d] Color (%d) Salida sem_cruze critica +1 pajitas (%ld)\n", GetCurrentThreadId(), color,varsem); //#critica

				//flag = 0;

			}







			if ((*desp == 131 && *carril) || (*desp == 133 && !*carril)) {//Paso por meta
				(contador)++;
			}


			//if (posOcup( * carril, ((( * desp) + 135) % 137))) {//Comprobamos Pos -2 Para mensaje Bien
				sendMess(tdesp, *carril);/*
			if (arrayPosiciones[pos_2]) {

				//fprintf(stderr, " [%d] Color (%d) Envio mensaje POS_2-----> %d | %d : [%d]\n", GetCurrentThreadId(), color, pos_2, (((*desp) + 135) % 137) + 137 * (*carril), arrayPosiciones[pos_2]); //#mensaje
					//fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, GetCurrentThreadId(), pos_2);

			}

			//pos_cambio = (cambio_carril_cal((( * desp) + 136) % 137, * carril) + ((! * carril) * 137)) + 1
			//if (posOcup(! * carril, cambio_carril_cal((( * desp) + 136) % 137, * carril))) {
			if (arrayPosiciones[pos_cambio]) {


				//fprintf(stderr, "Color (%d) [%d] 2 posiciones atras ocupada %d\n", color, GetCurrentThreadId(), pos_cambio);

				if (PostThreadMessageA(arrayPosiciones[pos_cambio], WM_USER + 4, 4, 4) == 0) {
					PERROR("ERROR AL MSGSND (pos carril opuesto ocupada)");
				}*/
				//fprintf(stderr, "Color (%d) [%d] Envio Mesaje pos_cambio: %d\n", color, GetCurrentThreadId(), pos_cambio);


				//Limpieza Cola
				/*while(PeekMessage( & clMsg, NULL, WM_USER, WM_USER+4, PM_REMOVE)){
					fprintf(stderr,"Limpiando Cola...\n");
				}
				}*/

				//fprintf(stderr, " [%d] Color (%d) Envio mensaje [%d]\n", GetCurrentThreadId(), color, pos_cambio); //#mensaje

			
			LeaveCriticalSection(&critica);
			if ((velocidad(v, *carril, *desp)) != -1) {
				//fprintf(stderr, "##");
			}
		}
		else {

			LeaveCriticalSection(&critica);

		}

		//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica


	}
	else {//Entra la que SIG pos esta ocupada


		//fprintf(stderr, "[%d] Color (%d) Posicion ocupada  , compruebo cambio de carril:        %d\n", GetCurrentThreadId(), color, * desp); //#posicion
	   //Limpieza Cola
		 /*while(PeekMessage( & clMsg, NULL, WM_USER, WM_USER+4, PM_REMOVE)){
			 //fprintf(stderr,"Limpiando Cola...\n");
		 }*/

		if (!posOcup(!*carril, cambio_carril_cal(*desp, *carril))) {//Efectuo cambio carril Si es posible
			arrayPosiciones[*desp + (*carril) * 137] = 0;//Guarda ID en Nueva Pos
			//fprintf(stderr, "\nCAMBIO%d:%d: %6d |\n", cambio_carril_cal(*desp, *carril) + !*carril * 137, ((*desp + 1) % 137) + ((*carril) * 137), arrayPosiciones[cambio_carril_cal(*desp, *carril) + !*carril * 137]);
			int tdesp = *desp;
			if (cambioCarril(carril, desp, color) == -1) {
				PERROR("ERROR AL CAMBIAR CARRIL");
			}
			//fprintf(stderr, "[%d] Color (%d) Cambio Carril: %d\n", GetCurrentThreadId(), color, dep_temp); //#posicion
			arrayPosiciones[*desp + (*carril) * 137] = GetCurrentThreadId();//Guarda ID en Nueva Pos
			sendMess(tdesp, !*carril);
			LeaveCriticalSection(&critica);
			if ((velocidad(v, *carril, *desp)) != -1) {
				fprintf(stderr, "##");
			}

			//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas\n", GetCurrentThreadId(), color); //#critica

		}
		else {

			//fprintf(stderr, "[%d] Color (%d) Salida Critica critica +1 pajitas: %d\n", GetCurrentThreadId(), color); //#critica 
			LeaveCriticalSection(&critica);

			//fprintf(stderr, " [%d] Color (%d) ESPERANDO Mensaje [%d]\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje
			if (GetMessage(&uMsg, NULL, WM_USER + 3, WM_USER + 4) == -1) {
				PERROR("GetMessage");
			}
			EnterCriticalSection(&critica);
			while (PeekMessage(&clMsg, NULL, WM_USER, WM_USER + 4, PM_REMOVE)) {
				//fprintf(stderr, "Limpiando Cola...\n");
			}
			LeaveCriticalSection(&critica);
			//fprintf(stderr, " [%d] Color (%d) Recojo mensaje [%d]\n", GetCurrentThreadId(), color, ( * desp + * carril * 137) + 1); //#mensaje
		}
	}
} //Fin Avance_controlado

//----------------------------------------------------------------------------------------------------------------------
//---------- CreaNhijos
int creaNhijos(int n, int v) {
	//HANDLE hThreadArray[20];
	DWORD idHilo[100];
	static int i;
	//num_coche = n;
	pParam arrayParam[100];
	srand(GetCurrentThreadId());

	for (i = 1; i <= n; i++) {//CreaciOn N Hilos
		//fprintf(stderr, "%d %d**Soy el padre creando al hijo--> %d\n", (sizeof(hThreadArray)/sizeof(* hThreadArray )) ,n, i);
		arrayParam[i] = (pParam)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Param));
		arrayParam[i]->indice = i;
		arrayParam[i]->velocidad = (rand() + 10) % 100;
		arrayParam[i]->nCoches = n;
		//if ((hThreadArray[i] = CreateThread(NULL, 0, funcionHilos, arrayParam[i], 0, &idHilo[i])) == NULL) {
		if ((CreateThread(NULL, 0, funcionHilos, arrayParam[i], 0, &idHilo[i])) == NULL) {
			PERROR("Create Hilo");
		}
	}
	SetEvent(evento);
	return 0;
} //Fin Nhijos

//----------------------------------------------------------------------------------------------------------------------
//---------- FuncionHilos
DWORD WINAPI funcionHilos(LPVOID pEstruct_2) {
	pParam pEstruct = (pParam)pEstruct_2;
	int colores[] = { 0,0,2,2,3,5,6,7 };
	int n = pEstruct->nCoches;
	int miIndice = pEstruct->indice;
	int v = pEstruct->velocidad;
	int miIndiceCarril = miIndice % 2;
	int b;

	fprintf(stderr, "\t\tHola soy el hijo %d PID: %d\n", miIndice, GetCurrentThreadId());
	//EnterCriticalSection( & critica_salida);
	PeekMessage(&creaCola, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	//arrayPosiciones[miIndice] = GetCurrentThreadId();
	//LeaveCriticalSection( & critica_salida);
	//fprintf(stderr, "Color (%d) [%d] Entro seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
	//EnterCriticalSection( & critica_salida);

	EnterCriticalSection(&critica);
	for (b = 136; b >= 0;) {//Busqueda de Sitio
		//fprintf(stderr, "Color (%d) [%d] Iteracion b = %d\n", colores[1 + (miIndice - 1) % 6], miIndice, b);
		b -= 2;
		if (!(posOcup(miIndiceCarril, b))) {
			//if (arrayPosiciones[b + (miIndiceCarril) * 137]==0) {

				//fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n", colores[1 + (miIndice - 1) % 6], miIndice);
			if (iniCoche(&miIndiceCarril, &b, colores[1 + (miIndice - 1) % 6]) != 0) {
				//fprintf(stderr, "Color (%d) [%d] ERROR INICIO COCHE +++++++++++++++++++++++++++++++++\n", colores[1 + (miIndice - 1) % 6], miIndice);
				PERROR("ERROR INCIO COCHE");
				raise(SIGINT);
			} //Alterna Carril
			fprintf(stderr, "Color (%d) [%d]        Inicio coche con Carril %d **Posicion %d **Color %d \n", colores[1 + (miIndice - 1) % 6], miIndice, miIndiceCarril, b, colores[1 + (miIndice - 1) % 6]);
			//fprintf(stderr, "Color (%d) [%d] Salgo del bucle...\n",colores[1 + (miIndice - 1) % 6],i);
			arrayPosiciones[b + (miIndiceCarril) * 137] = GetCurrentThreadId();
			break;//Encontrado Sitio
		}
	}
	LeaveCriticalSection(&critica);
	//LeaveCriticalSection( & critica_salida);
	//fprintf(stderr, "Color (%d) [%d] Salgo de la seccion critica\n", colores[1 + (miIndice - 1) % 6], miIndice);
	// //fprintf(stderr, "Color (%d) [%d] Arranco\n",colores[1 + (miIndice - 1) % 6],i);
	if (WaitForSingleObject(evento, INFINITE) == WAIT_FAILED)
		PERROR("WaitForSingleObject vertical INI");

	while (1) {
		avance_controlado(&miIndiceCarril, &b, colores[1 + (miIndice - 1) % 6], v);

	}
}//Fin funcionHilos

//----------------------------------------------------------------------------------------------------------------------
//---------- Main
int main(int argc, char const* argv[]) {
	//Check atributos
	if (argc != 3) {
		perror("arg:");
		exit(4);
	}
	else if (atoi(argv[1]) < 1 && atoi(argv[1]) > 20) {
		perror("arg:");
		exit(4);
	}
	else if (!(!atoi(argv[2]) || atoi(argv[2]) == 1)) {
		perror("Velocidad");
		exit(4);
	}
	else {
		int u = 0; //variable para bucles For
		int numCoches = atoi(argv[1]); //Ncoches
		int vel = atoi(argv[2]); //Punteros funciones

		PeekMessage(&creaCola, NULL, WM_USER, WM_USER, PM_NOREMOVE); //Creacion cola  mensaje

		if ((hinstLib = LoadLibrary(TEXT("falonso2.dll"))) == NULL) { //Carga libreria en memoria del proceso
			PERROR("Error cargar DLL");
			return (1);
		}
		//Secciones Criticas
		//LanzarA STATUS_NO_MEMORY si no hay memoria
		InitializeCriticalSection(&critica_sem);
		InitializeCriticalSection(&critica_salida);
		InitializeCriticalSection(&critica);

		//Prototipos Funciones
		if (!(inicio_falonso = (DLL1Arg)GetProcAddress(hinstLib, "FALONSO2_inicio"))) {
			PERROR("Error getProc FALONSO2_inicio");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(estadoSem = (DLL1Arg)GetProcAddress(hinstLib, "FALONSO2_estado_semAforo"))) {
			PERROR("Error getProc FALONSO2_estado_semAforo");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(f_fin = (DLL1Argp)GetProcAddress(hinstLib, "FALONSO2_fin"))) {
			PERROR("Error getProc FALONSO2_fin");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(luzSem = (DLL2Arg)GetProcAddress(hinstLib, "FALONSO2_luz_semAforo"))) {
			PERROR("Error getProc FALONSO2_luz_semAforo");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(posOcup = (DLL2Arg)GetProcAddress(hinstLib, "FALONSO2_posiciOn_ocupada"))) {
			PERROR("Error getProc FALONSO2_posiciOn_ocupada");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(velocidad = (DLL3Arg)GetProcAddress(hinstLib, "FALONSO2_velocidad"))) {
			PERROR("Error getProc FALONSO2_velocidad");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(iniCoche = (DLL3ArgP)GetProcAddress(hinstLib, "FALONSO2_inicio_coche"))) {
			PERROR("Error getProc FALONSO2_inicio_coche");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(avanceCoche = (DLL3ArgP)GetProcAddress(hinstLib, "FALONSO2_avance_coche"))) {
			PERROR("Error getProc FALONSO2_avance_coche");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(cambioCarril = (DLL3ArgP)GetProcAddress(hinstLib, "FALONSO2_cambio_carril"))) {
			PERROR("Error getProc FALONSO2_cambio_carril");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(p_error = (DLL1Argvoid)GetProcAddress(hinstLib, "pon_error"))) {
			PERROR("Error getProc  pon_error");
			FreeLibrary(hinstLib);
			return (2);
		}
		if (!(pausa = (DLL0Arg)GetProcAddress(hinstLib, "FALONSO2_pausa"))) {
			PERROR("Error getProc FALONSO2_pausa");
			FreeLibrary(hinstLib);
			return (2);
		}


		//Inicio comportamiento
		inicio_falonso(vel); //Inicio Circuito

		EnterCriticalSection(&critica_sem);
		semtoRed(HORIZONTAL); //Configuracion Incial Semaforos
		semtoGreen(VERTICAL);
		LeaveCriticalSection(&critica_sem);

		//fprintf(stderr, "PRE-CreaHijos\n");
		creaNhijos(numCoches, vel); //DESCOMENTAR CUANDO ESTE DEPURADO
		if (CreateThread(NULL, 0, hiloContador, NULL, 0, NULL) == NULL)
			PERROR("Create Hilo Contador");
		//fprintf(stderr, "POST-CreaHijos\n");

		while (1) { //Alterdor Semaforos
			EnterCriticalSection(&critica_sem);
			semtoRed(VERTICAL);
			semtoGreen(HORIZONTAL);
			LeaveCriticalSection(&critica_sem);
			int b = 0;
			for (; b < 5; b++) {
				Sleep(300);
			}
			EnterCriticalSection(&critica_sem);
			semtoRed(HORIZONTAL);
			semtoGreen(VERTICAL);
			LeaveCriticalSection(&critica_sem);
			for (b = 0; b < 5; b++) {
				Sleep(300);
			}
		}

		FreeLibrary(hinstLib); //LiberaciOn de DLl
		return 0;
	}
} //Fin Main
DWORD WINAPI hiloContador(LPVOID pContador) {
	Sleep(30000);
	EnterCriticalSection(&critica);
	f_fin(&contador);
	raise(SIGINT);
	return 0;
}