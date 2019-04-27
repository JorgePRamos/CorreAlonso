/*
 gcc -o myprog testdll.cpp -L./ -lfalonso2

gcc -o myprog myprog.c -L./ -lmydll
gcc object1.o object2.o -lMyLib2 -Wl,-Bstatic -lMyLib1 -Wl,-Bdynamic -o output
*/
/////ddsdsdsdsdsd
// A simple program that uses LoadLibrary and 
// GetProcAddress to access myPuts from Myputs.dll. 

#include <windows.h> 
#include <stdio.h> 
int FALONSO2_inicio(int ret);

extern "C"{
#define DLL_EXPORTS 1
#include "falonso2.h"
}
typedef int (*DLL1Arg)(int);
typedef int (*DLL0Arg)(void);
typedef int (__cdecl *MYPROC)(LPWSTR); 

int main( void ) 
{ 
    HINSTANCE hinstLib=NULL; 
    DLL1Arg f_ini = NULL; 
    DLL0Arg f_pausa = NULL; 
    //FARPROC ProcAdd;
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE; 
 
    // Get a handle to the DLL module.
 
    hinstLib = LoadLibrary(TEXT("falonso2.dll")); //cargas la libreria en memoria del proceso
    //HINSTANCE__ *hModule= LoadLibrary(L"C:\\Windows\\System32\\user32.dll");
 
    // If the handle is valid, try to get the function address.
 
    if (hinstLib != NULL) 
    { 
       f_ini = (DLL1Arg) GetProcAddress(hinstLib, "FALONSO2_inicio"); //obtienes y añades la direccion de la funcion
        f_pausa = (DLL0Arg) GetProcAddress(hinstLib, "FALONSO2_pausa");
        // If the function address is valid, call the function.
 
        if (NULL != f_ini) 
        {          printf("entro\n");

            fRunTimeLinkSuccess = TRUE;
            //(ProcAdd) (L"2"); 
            printf("entro");
            f_ini(1);
        }
        // Free the DLL module.
 
        fFreeResult = FreeLibrary(hinstLib); 
    } 

    // If unable to call the DLL function, use an alternative.
    if (! fRunTimeLinkSuccess) 
        printf("Message printed from executable\n"); 
    while (1)
    {
        f_pausa;
    }
    
    return 0;

}
/*

The gcc can link against SUSI4.dll. Use FIND_LIBRARY(DLL_SUSI4 Susi4.dll ${CMAKE_SOURCE_DIR}/lib/susi4) and
 TARGET_LINK_LIBRARIES(proggi ${DLL_SUSI4}) commands in your cmake script. An executable will be created. When you execute your program be shure that the dll is in the same path.*/

 using namespace std;
 
typedef int(__stdcall MESS)(unsigned int);


/*

// A simple program that uses LoadLibrary and 
// GetProcAddress to access myPuts from Myputs.dll. 
 
#include <windows.h> 
#include <stdio.h> 
 
typedef int (__cdecl *MYPROC)(LPWSTR); 
 
int main( void ) 
{ 
    HINSTANCE hinstLib; 
    MYPROC ProcAdd; 
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE; 
 
    // Get a handle to the DLL module.
 
    hinstLib = LoadLibrary(TEXT("MyPuts.dll")); 
 
    // If the handle is valid, try to get the function address.
 
    if (hinstLib != NULL) 
    { 
        ProcAdd = (MYPROC) GetProcAddress(hinstLib, "myPuts"); 
 
        // If the function address is valid, call the function.
 
        if (NULL != ProcAdd) 
        {
            fRunTimeLinkSuccess = TRUE;
            (ProcAdd) (L"Message sent to the DLL function\n"); 
        }
        // Free the DLL module.
 
        fFreeResult = FreeLibrary(hinstLib); 
    } 

    // If unable to call the DLL function, use an alternative.
    if (! fRunTimeLinkSuccess) 
        printf("Message printed from executable\n"); 

    return 0;

}

*/