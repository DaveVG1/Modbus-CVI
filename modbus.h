/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2010. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1
#define  PANEL_LED_VERDE                  2       /* callback function: FuncionEncenderLeds */
#define  PANEL_LED_AMARILLO               3       /* callback function: FuncionEncenderLeds */
#define  PANEL_LED_NARANJA                4       /* callback function: FuncionEncenderLeds */
#define  PANEL_LED_ROJO                   5       /* callback function: FuncionEncenderLeds */
#define  PANEL_DECORATION_6               6
#define  PANEL_DECORATION                 7
#define  PANEL_SALIR                      8       /* callback function: Salir */
#define  PANEL_CONECTAR                   9       /* callback function: Conectar */
#define  PANEL_ADDR_7017                  10
#define  PANEL_ADDR_7060D                 11
#define  PANEL_PUERTO                     12
#define  PANEL_DECORATION_3               13
#define  PANEL_TEXTMSG_5                  14
#define  PANEL_DECORATION_5               15
#define  PANEL_TEXTMSG                    16
#define  PANEL_POTENCIOMETRO              17
#define  PANEL_TEXTMSG_4                  18
#define  PANEL_LDR                        19
#define  PANEL_PULSADOR                   20
#define  PANEL_ALIMENTACION               21
#define  PANEL_TEXTMSG_3                  22
#define  PANEL_ACTUALIZAR                 23      /* callback function: FuncionActualizar */
#define  PANEL_AUTOACTUALIZAR             24      /* callback function: FuncionAutoactualizar */
#define  PANEL_RESET                      25      /* callback function: FuncionReset */
#define  PANEL_PULSO_1                    26
#define  PANEL_PULSO_2                    27
#define  PANEL_PULSO_4                    28
#define  PANEL_PULSO_3                    29


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK Conectar(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncionActualizar(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncionAutoactualizar(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncionEncenderLeds(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FuncionReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Salir(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
