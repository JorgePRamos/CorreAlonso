//Version correctamente sincronizada.

#include <assert.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "falonso.h"
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
//---------------------------------------------------------------------------
//DEFINICIONES
typedef struct Coche {
    int posicion;
    int carril;
}coche;

union semun 
    {int             val;
    struct semid_ds *buf;
    unsigned short        *array;
 };
typedef struct tipoMensaje {
    long tipo;
}mensaje;

//---------------------------------------------------------------------------
union semun semaforo;
sigset_t mask_c, mask_z;
struct sigaction s1;
struct sigaction s2;
int semaforos=-1, mem=-1, buzon=-1,critica=-1,critica_salida=-1, sem_cruze=-1, sem_dad =-1;
char *memoria;
unsigned long *contador=NULL;
int flag = 0;

//---------------------------------------------------------------------------
//MANEJADORA CTR+C
void manejadora(int s){
    printf("Salto a la Manejadora\n");

    flag = 1;
    semaforo.val = 0;
      //LIBERAMOS RECURSOS IPC
    if((semctl(critica, 0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(semaforos, 0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(critica_salida,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(sem_cruze,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((shmctl(mem,  IPC_RMID,NULL))==-1)
        perror("Liberacion recursos");
    if((msgctl(buzon,IPC_RMID,NULL))==-1)
        perror("Liberacion recursos");
     if((semctl(sem_dad,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    fin_falonso((int *)contador);
    if (kill(0, SIGKILL))
        {
            perror("ERROR KILL PID MANEJADORA C");
        }
}
//---------------------------------------------------------------------------
//MANEJADORA CTR+Z
void pistolero(int s){
    //fprintf(stderr, "SOY EL PISTOLERO, PREPARATE PARA MORIR...PIUM PIUM CONTADOR: %ull\n",*contador);
    semaforo.val = 0;

    //LIBERAMOS RECURSOS IPC
    if((semctl(critica, 0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(semaforos, 0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(critica_salida,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((semctl(sem_cruze,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
    if((shmctl(mem,  IPC_RMID,NULL))==-1)
        perror("Liberacion recursos");
    if((msgctl(buzon,IPC_RMID,NULL))==-1)
        perror("Liberacion recursos");

     if((semctl(sem_dad,0,IPC_RMID))==-1)
        perror("Liberacion recursos");
     if (kill(0, SIGKILL))
        {
            perror("ERROR KILL PID MANEJADORA Z");
        }
}

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
//---------------------------------------------------------------------------
//ENTER_CRITIC
void enterCritic(int semId, int semnum, int op){
   struct sembuf buf;
   assert(op<0);
   buf.sem_num = semnum;//1
   buf.sem_op = op;
   buf.sem_flg = 0;
   if (semop(semId, &buf, 1) == -1){
      perror("ERROR AL ENTRAR CRITICA");
        raise(SIGINT);
   }
}
//---------------------------------------------------------------------------
//LEAVE_CRITIC
void leaveCritic(int semId, int semnum, int op){
   struct sembuf buf;
   assert(op>0);
   buf.sem_num = semnum;
   buf.sem_op = op;
   buf.sem_flg = 0;
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
    luz_semAforo(sem, AMARILLO);
    leaveCritic(critica_salida, 0, 1);
    //fprintf(stderr, " PADRE Sem_cruze pre rojo: %d\n", semctl(sem_cruze, 0, GETVAL));
    enterCritic(sem_cruze, 0, -6);
    enterCritic(critica_salida, 0, -1);
    luz_semAforo(sem, ROJO);
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
    luz_semAforo(sem, VERDE);
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
    msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long),  * desp + (( * carril) * 137) + 1, IPC_NOWAIT);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_cambio);
    //msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), pos_2 + 1, IPC_NOWAIT);
    //fprintf(stderr, "Color (%d) [%d]  MENSAJE RECOGIDO CON EXITO ------> [%d]\n", color, getpid(), pos_2);
    // fprintf(stderr, "Color (%d) [%d] Limpio mensajes (rcv)\n",color, getpid());
    //fprintf(stderr, "Color (%d) [%d]  COMPRUEBO POSICION SIGUIENTE %d (%d+1%%137+%d*137)\n", color, getpid(), *desp + 1 % 137 + *carril *137, *desp, *carril);
    if (memoria[( * desp + 1) % 137 + * carril * 137] == ' ') {
        // fprintf(stderr, "Estado semaforo V: %d H: %d\n", memoria[275], memoria[274]);
        //fprintf(stderr, "Color (%d) [%d] Vacia posicion sig: %d\n", color, getpid(), * desp + 1 % 137 + * carril * 137);
        if ( * desp == 21 && * carril) {
            enterCritic(critica_salida, 0, -1);
            if ((memoria[275] == ROJO || memoria[275] == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (%d)\n", color, getpid(), 300);
                leaveCritic(critica, 0, 1);
                leaveCritic(critica_salida, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 300, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[275] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic(critica_salida, 0, 1);
                leaveCritic(critica, 0, 1);
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

        } else if ( * desp == 20 && !( * carril)) {
            enterCritic(critica_salida, 0, -1);
            if ((memoria[275] == ROJO || memoria[275] == AMARILLO)) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (301)\n", color, getpid());

                leaveCritic(critica, 0, 1);
                leaveCritic(critica_salida, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 301, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[275] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic(critica_salida, 0, 1);
                leaveCritic(critica, 0, 1);
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

            //fprintf(stderr, "Color (%d) [%d] Semáforo apagado\n", color, getpid());

        } else if ( * desp == 97 && * carril) { //233
            ////  fprintf(stderr, "                                           >>>>>>>POST-ELSE-IF---> %d\n", *desp);
            //   
            enterCritic(critica_salida, 0, -1);
            if (memoria[274] == ROJO || memoria[274] == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (302)\n", color, getpid());
                leaveCritic(critica, 0, 1);
                leaveCritic(critica_salida, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 302, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));

                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[274] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic(critica_salida, 0, 1);
                leaveCritic(critica, 0, 1);
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            }else
                raise(SIGINT);

        } else if ( * desp == 102 && ! * carril) {
            enterCritic(critica_salida, 0, -1);
            if (memoria[274] == ROJO || memoria[274] == AMARILLO) {
                //fprintf(stderr, "Color (%d) [%d] Espero al semaforo (303)\n", color, getpid());
                leaveCritic(critica, 0, 1);
                leaveCritic(critica_salida, 0, 1);
                if (-1 == msgrcv(buzon, & mt2, sizeof(mensaje) - sizeof(long), 303, 0)) {
                    perror("ERROR AL MSGRCV");
                    raise(SIGINT);
                }
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

                //fprintf(stderr, "Color (%d) [%d] Sem_cruze post enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
            } else if (memoria[274] == VERDE) {
                //fprintf(stderr, "Color (%d) [%d] Sem_cruze pre enter: %d\n", color, getpid(), semctl(sem_cruze, 0, GETVAL));
                leaveCritic(critica_salida, 0, 1);
                leaveCritic(critica, 0, 1);
                enterCritic(sem_cruze, 0, -1);
                enterCritic(critica, 0, -1);

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
        if ((( * desp == 111 && ! * carril) || ( * desp == 24 && ! * carril) || ( * desp == 106 && * carril) || ( * desp == 25 && * carril)) && (memoria[274] == ROJO || memoria[274] == AMARILLO || memoria[274] == VERDE)) {
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
        velocidad(10, * carril, * desp);

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
                b -= 2;
                if (memoria[b + miIndiceCarril * 137] == ' ') {
                    ////  fprintf(stderr, "Color (%d) [%d] Carril libre encontrado\n",colores[miIndice],i);
                    if (inicio_coche( & miIndiceCarril, & b,
                                      colores[1+(miIndice-1)%6]) == -1) {
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
                        if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long), 100+miIndice, 0)) {
                            perror("ERROR AL MSGRCV");
                            raise(SIGINT);
                        }

                    }
                     m1.tipo =100+ miIndice+1;
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
                    if (-1 == msgrcv(buzon, & m2, sizeof(mensaje) - sizeof(long),100+ miIndice, 0)) {
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
                avance_controlado( & miIndiceCarril, & b, colores[1+(miIndice-1)%6], v);
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
//MAIN
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
        int vel = atoi(argv[2]);

        ////  fprintf(stderr,"numCoche: %d\n", numCoches);

        //VAR's CONTROLADORA
        sigemptyset( & mask_c); //Mascara Ctr+c
        sigemptyset( & mask_z); //Mascara Ctr+c

        sigaddset( & mask_c, SIGINT);
        sigaddset( & mask_z, SIGTSTP);

        s2.sa_handler = pistolero;
        sigemptyset( & s2.sa_mask);
        s2.sa_flags = 0;

        s1.sa_handler = manejadora;
        sigemptyset( & s1.sa_mask);
        s1.sa_flags = 0;

        if (sigaction(SIGINT, & s1, NULL)) {
            perror("ERROR CREACION HERRAMIENTAS MANEJADORA \n");
            raise(SIGINT);
        } //Salto a KillMain COntroladora.

        if (sigaction(SIGTSTP, & s2, NULL) == -1) {
            perror("ERROR CREACION HERRAMIENTAS MANEJADORA \n");
            raise(SIGINT);
        } //Salto a pistolero

        if ((critica = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
            perror("ERROR CREACION SECCION CRITICA\n");
            raise(SIGINT);
        }
        semaforo.val = 1;
        if (semctl(critica, 0, SETVAL, semaforo) == -1) {
            perror("ERROR AL PONER EL SEMAFOR_CRITICO A 1\n");
            raise(SIGINT);
        }
        if ((critica_salida = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
            perror("ERROR CREACION SECCION CRITICA\n");
            raise(SIGINT);
        }
        semaforo.val = 1;
        if (semctl(critica_salida, 0, SETVAL, semaforo) == -1) {
            perror("ERROR AL PONER EL SEMAFOR_CRITICO_salida A 1\n");
            raise(SIGINT);
        }

        if ((sem_cruze = semget(IPC_PRIVATE, 4, IPC_CREAT | 0666)) == -1) {
            perror("ERROR CREACION SECCION CRITICA\n");
            raise(SIGINT);
        }
        semaforo.val = 6;

        if (semctl(sem_cruze, 0, SETVAL, semaforo) == -1) {
            perror("ERROR AL PONER EL SEMAFOR_CRITICO_salida A 1\n");
            raise(SIGINT);
        }


        if ((semaforos = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666)) == -1) {
            perror("semaforo create error");
            raise(SIGINT);
        }

        semaforo.val = 1;

        if ((sem_dad = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666)) == -1) {
            perror("semaforo create error");
            raise(SIGINT);
        }
        if (semctl(sem_dad, 0, SETVAL, semaforo) == -1) {
            perror("ERROR AL PONER EL SEMAFOR_CRITICO_salida A 1\n");
            raise(SIGINT);
        }


        if ((mem = shmget(IPC_PRIVATE, 400 * sizeof(char), IPC_CREAT | 0666)) == -1) {
            perror("ERROR CREACION MEMORIA_COMPARTIDA\n");
            raise(SIGINT);
        }
        if ((buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
            perror("ERROR CREACION BUZON\n");
            raise(SIGINT);
        }
        if ((memoria = (char * ) shmat(mem, 0, 0)) == NULL) {
            perror("Null pointer");
            raise(SIGINT);
        }

        if ((inicio_falonso(atoi(argv[2]), semaforos, memoria)) == -1) {
            perror("ERROR INICIO_FALONSO");
            raise(SIGINT);
        }
        contador = (unsigned long * ) memoria + 324;
        * contador = 0;
        enterCritic(critica_salida, 0, -1);
            semtoRed(VERTICAL);
            semtoGreen(HORIZONTAL);
            leaveCritic(critica_salida, 0, 1);
        creaNhijos(numCoches, vel);
        sigaddset( & mask_c, SIGKILL);
        while (!flag) {
              mensaje mt;

 

            enterCritic(critica_salida, 0, -1);

            semtoRed(HORIZONTAL);
            semtoGreen(VERTICAL);
            leaveCritic(critica_salida, 0, 1);
            int b = 0;
            for (; b < 7; b++) {
                pausa();
            }
            enterCritic(critica_salida, 0, -1);
            semtoRed(VERTICAL);
            semtoGreen(HORIZONTAL);
            leaveCritic(critica_salida, 0, 1);
            for (b = 0; b < 7; b++) {
                pausa();
            }
        }
        //El padre espera a los hijos
        int stat;
        while ((waitpid(-1, & stat, 0)) > 0) {}
        sigemptyset( & mask_c); //Mascara Ctr+c
        return 0;
    }

}




