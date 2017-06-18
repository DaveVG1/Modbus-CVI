/*********************************************************************** */
/**/
/**/
/*   Aplicacion docente para prueba de los modulos de adquisicion ICPCON */
/*   M7060 y M7017, mediante protocolo Modbus-TRU                        */
/**/
/*                                      Dpto. Tecnologia Electronica     */
/*************************************************************************/
 
#include <formatio.h>
#include <rs232.h>
#include <utility.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "modbus.h"
#include <stdlib.h>
#include <string.h>

#include "CRC.h"

#define ADD_7017 1
#define ADD_7060D 0
#define TAMANIO 257
#define TRUE 1
#define FALSE 0
#define FE 32767

static int panelHandle;

unsigned char COM=4;
int abierto=0;
unsigned char direcciones[2] = {0x02,0x04};
char *logname;
double tiempo_enviado = 0;
unsigned char recibido[TAMANIO],recibido_l[TAMANIO];
int selector_l=FALSE;
int actualizado_l = FALSE;
unsigned char funcion;
int enviado_l = FALSE; 
char nomcom[7];
char nom_fichero[261] = "LOG.TXT";
int cargado;
int autoactualizar=0;

void CVICALLBACK CBReceive (int portNumber, int eventMask, void *callbackdata);
 
int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "modbus.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	abierto=0;
	
	RunUserInterface ();
	
	DiscardPanel (panelHandle);
	return 0;
}


int CVICALLBACK Conectar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char descriptor[256];
	unsigned char puerto;
	unsigned char origen =0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle,PANEL_PUERTO,&COM);
			
			//GetCtrlVal(panelHandle,PANEL_ADDR_7060D,&direcciones[origen]);
			//origen++;
			//GetCtrlVal(panelHandle,PANEL_ADDR_7017,&direcciones[origen]);
			
			
            /*  Open and Configure Com port 1 */
		    OpenComConfig (COM, "", 9600, 0, 8, 1, 512, 512);
    
    		/*  Turn off Hardware handshaking (loopback test will not function with it on) */
			SetCTSMode (COM, LWRS_HWHANDSHAKE_CTS_RTS);
    
    		/*  Make sure Serial buffers are empty */
    		FlushInQ (COM);
    		FlushOutQ (COM);
 
			// Aqui se podrian configurar los modulos de adquisicion (setting) con otras direcciones
			
			Fmt(nomcom,"%s%d","COM",COM);
    		/*  Install a callback such that if the event character appears at the 
        	receive buffer, our function will be notified.  */
    		//InstallComCallback (COM, LWRS_RXFLAG, 0, STOP_FRAME_CHAR , Event_Char_Detect_Func, 0);
			abierto=1;
			break;
		}
	return 0;
}

/*
int CVICALLBACK Enviar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char paquete[256];
	char mensaje[256];
	char numero[256];
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (abierto)
			{
				GetCtrlVal(panelHandle,PANEL_TELEFONO,numero);
				GetCtrlVal(panelHandle,PANEL_MENSAJE,mensaje);
				sprintf(paquete,"AT+CMGF=1\r\n");
				ComWrt(COM,paquete,strlen(paquete));
				Delay(0.5);
				sprintf(paquete,"AT+CMGS=\"+34%s\"\r\n",numero);
				ComWrt(COM,paquete,strlen(paquete));
				//Delay(1);
				sprintf(paquete,"%s%c\r\n",mensaje,0x1A);
				ComWrt(COM,paquete,strlen(paquete));
				//Delay(1);
			}
			else
			{
				MessagePopup ("Error", "Abra primero la conexión");
			}
			break;
		}
	return 0;
}

  */


/*==============================================================================*/
int Enviar(char *nomcom, int puerto, unsigned char *mensaje, int longitud, char *logfile){
	
	unsigned char original_CRC[2];
	unsigned short new_CRC;
	unsigned char high, low = 0;
	char aux[50];
	int errnum, i;
	char *errort;
	unsigned short CRC;
	
	FILE *fichero;
	logname = logfile;
	
	
	//Calculo del CRC
	CRC = checkcrc(mensaje, longitud);
	mensaje[longitud] = getCRCHighBit(CRC);
	mensaje[longitud+1] = getCRCLowBit(CRC);
	
	DisableBreakOnLibraryErrors ();
	errnum=OpenComConfig (puerto, nomcom, 9600, 0, 8, 1, 512, 512);		// Configurar y abrir el puerto;
    EnableBreakOnLibraryErrors ();
	if (errnum) {
		MessagePopup("Error", GetRS232ErrorString(errnum));
//		FuncionSalirAuto(NULL, NULL, EVENT_COMMIT, NULL, NULL, NULL);
		
	}else{
		FlushInQ (puerto);
    	FlushOutQ (puerto);
		funcion = mensaje[1];
		
		SetComTime (puerto, 0.035);  //time out
		InstallComCallback (puerto, LWRS_RECEIVE, 5, 0, CBReceive, NULL);// Programamos un Callback para que avise cuando lleguen datos
		
		ComWrt (puerto, mensaje,longitud + 2);								// Escribimos en el puerto
		enviado_l = TRUE;
		actualizado_l = FALSE;  // actualizacion de datos
		GetCurrentDateTime (&tiempo_enviado);
		if (ReturnRS232Err()) {
			MessagePopup("Error", GetRS232ErrorString(errnum));
		}else {
			

		}
		while (enviado_l){
			ProcessSystemEvents();		  //Nos esperamos a recibir el comando de respuesta y habilitamos los eventos de programa
			
		}

	}
	return 0;
}
/*==============================================================================*/

/*==============================================================================*/
/* Detección de datos recibidos													*/												
void CVICALLBACK CBReceive (int portNumber, int eventMask, void *callbackdata){
	unsigned char original_CRC[2];
	unsigned short new_CRC;
	unsigned char high, low = 0;
	char aux[50];
	int longitud,i,j;
	double current; 
	FILE *fichero;
	
	
	if (selector_l) {
			longitud=ComRd (portNumber, recibido, TAMANIO);
			//Detección de errores
			original_CRC[0]= recibido[longitud -2];
			original_CRC[1]= recibido[longitud -1];
			
			new_CRC = checkcrc(recibido,longitud-2);
			high = new_CRC >> 8;
			low = new_CRC & 0x00FF;
	
			actualizado_l =TRUE;
			if(original_CRC[0] != high && original_CRC[1] != low){
				actualizado_l=FALSE;
			}else if((funcion | 0x80) == recibido[1]){
				actualizado_l=FALSE;
			}
			
					
	}else{
			longitud=ComRd (portNumber, recibido_l, TAMANIO);
			//Detección de errores
			original_CRC[0]= recibido_l[longitud -2];
			original_CRC[1]= recibido_l[longitud -1];
		
			new_CRC = checkcrc(recibido_l,longitud-2);
			high = new_CRC >> 8;
			low = new_CRC & 0x00FF;
	
			actualizado_l =TRUE;
			if(original_CRC[0] != high && original_CRC[1] != low){
				actualizado_l=FALSE;
			}else if((funcion | 0x80) == recibido_l[1]){
				actualizado_l=FALSE;
			}
			
			
	}
	CloseCom(portNumber);	
	if(enviado_l){
		enviado_l = FALSE;
	}		

	
}
/*==============================================================================*/ 


/*==============================================================================*/ 
int EstadoEnvio(){
	return enviado_l;
}
/*==============================================================================*/ 


/*==============================================================================*/ 
double TiempoUltimoEnvio(){
	return tiempo_enviado;
}
/*==============================================================================*/


/*==============================================================================*/
int CVICALLBACK FuncionEncenderLeds (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned char comando[8];
	unsigned char presionado = 0;
	int on =0;
	int aux;

	switch (event)
	{
		case EVENT_COMMIT:
			switch(control){
				case PANEL_LED_ROJO:
					presionado = 0x00;
					break;
				case PANEL_LED_NARANJA:
					presionado = 0x01;
					break;
				case PANEL_LED_AMARILLO:
					presionado = 0x02;
					break;
				case PANEL_LED_VERDE:
					presionado = 0x03;
					break;
				
			}
			GetCtrlVal (panelHandle, control, &on);
			
			// Preparar mensaje de escribir un unico rele
			comando[0] = direcciones[ADD_7060D];
			comando[1] = 0x05;		// Comando escribir un unico Rele
			comando[2] = 0x00;
			comando[3] = presionado;
			if(on == 1)
				comando[4] = 0xff;	
			else
				comando[4] = 0x00;
			comando[5] = 0x00;
			selector_l=FALSE;	//En principio no necesito leer la respuesta del modulo
			Enviar(nomcom, COM, comando, 6, nom_fichero);
			break;
	}
	return 0;
}
/*==============================================================================*/ 
/*==============================================================================*/
int CVICALLBACK FuncionActualizar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned char comando[8];
	unsigned int aux;
	double aux2;
	FILE *fichero;
	int i;
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
					if (autoactualizar){
						
						// Lectura del estado de los reles
						comando[0] = direcciones[ADD_7060D];
						comando[1] = 0x01;  
						comando[2] = 0x00;
						comando[3] = 0x00;
						comando[4] = 0x00;
						comando[5] = 0x04;
						selector_l=TRUE;
						Enviar(nomcom, COM, comando, 6, nom_fichero);
						if (actualizado_l){
							SetCtrlVal(panelHandle, PANEL_LED_ROJO, 		recibido[3]&0x01);
							SetCtrlVal(panelHandle, PANEL_LED_NARANJA, 	recibido[3]>>1&0x01);
							SetCtrlVal(panelHandle, PANEL_LED_AMARILLO, 	recibido[3]>>2&0x01);
							SetCtrlVal(panelHandle, PANEL_LED_VERDE, 		recibido[3]>>3&0x01);
							actualizado_l=FALSE;
						}	
						
						
						//  Lectura de las entradas digitales
							comando[0] = direcciones[ADD_7060D];
							comando[1] = 0x02;
							comando[2] = 0x00;
							comando[3] = 0x00;
							comando[4] = 0x00;
							comando[5] = 0x04;
							selector_l=TRUE;
							Enviar(nomcom, COM, comando, 6, nom_fichero);
							if (actualizado_l) {
								SetCtrlVal(panelHandle, PANEL_PULSO_1,recibido[3]&0x01);
								SetCtrlVal(panelHandle, PANEL_PULSO_2,recibido[3]>>1&0x01);
								SetCtrlVal(panelHandle, PANEL_PULSO_3,recibido[3]>>2&0x01);	
								SetCtrlVal(panelHandle, PANEL_PULSO_4,recibido[3]>>3&0x01);	
								actualizado_l=FALSE;
							}   
							
						
							// Lectura de las entradas analogicas
							comando[0] = direcciones[ADD_7017];
							comando[1] = 0x04;
							comando[2] = 0x00;
							comando[3] = 0x00;
							comando[4] = 0x00;
							comando[5] = 0x04;
							selector_l=TRUE;
							Enviar(nomcom, COM, comando, 6, nom_fichero);
							if (actualizado_l) {
								aux=recibido[3]<<8;
								aux=aux+recibido[4];
								aux2=(float)aux;
								SetCtrlVal(panelHandle, PANEL_ALIMENTACION,aux2);
								aux=recibido[5]<<8;
								aux=aux+recibido[6];
								aux2=(float)aux;
								SetCtrlVal(panelHandle, PANEL_POTENCIOMETRO,aux2);	 
								aux=recibido[7]<<8;
								aux=aux+recibido[8];
								aux2=(float)aux;
								SetCtrlVal(panelHandle, PANEL_LDR,aux2);
								aux=recibido[9]<<8;
								aux=aux+recibido[10];
								aux2=(float)aux;
								SetCtrlVal(panelHandle, PANEL_PULSADOR,aux2);
								actualizado_l=FALSE;	
							} 	
							
					}
					
			break;
	}

	return 0;
}

/*==============================================================================*/ 

/*==============================================================================*/ 
int CVICALLBACK FuncionReset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
		unsigned char comando[8];
	switch (event)
	{
		case EVENT_COMMIT:
			
/*			comando[0] = direcciones[ADD_7060D];
			comando[1] = 0x05;
			comando[2] = 0x02;
			comando[3] = 0x00;
			comando[4] = 0xFF;
			comando[5] = 0x00;
			selector_l=FALSE;			
			Enviar(nomcom, COM, comando, 6, nom_fichero);
			
			comando[3] = 0x01;
			selector_l=FALSE;
			Enviar(nomcom, COM, comando, 6, nom_fichero);
			
			comando[3] = 0x02;
			selector_l=FALSE;						
			Enviar(nomcom, COM, comando, 6, nom_fichero);
*/			
			break;
	}
	return 0;
}
/*==============================================================================*/ 

/*==============================================================================*/
int CVICALLBACK FuncionAutoactualizar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_AUTOACTUALIZAR,&autoactualizar);
			break;
	}
	return 0;
}
/*==============================================================================*/ 

int CVICALLBACK Salir (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
		}
	return 0;
}
