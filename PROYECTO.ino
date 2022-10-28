//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <SPI.h>
#include <SD.h>
#include <stdint.h>
#include <stdbool.h>
#include "TM4C123GH6PM.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
//*** Definir los pines ***
#define PB1 PF_4
#define PB2 PF_0
#define LED PF_1
#define LED2 PF_3
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  
//*** Variables ***
int hola = 0;
int BOT1 = 0;
int BOT2 = 0;
int pos_actual = 0;
uint8_t gol = 1;
uint8_t dtiempo_anterior = 0;
uint8_t dtiempo_actual = 0;
uint8_t bandera_1s = 0;
uint8_t pais = 0;
uint8_t bandera_cambio = 0;
uint8_t J1_pais = 0;
uint8_t J2_pais = 0;
uint8_t J1_goles = 0;
uint8_t J2_goles = 0;
uint8_t modo = 0;
uint8_t bandera_SD = 0;
uint8_t bandera = 1;
uint8_t bandera2 = 1;
uint8_t bandera3 = 1;
uint8_t P_J1 = 0;
uint8_t P_J2 = 0;
String J1_ptxt;
String J2_ptxt;
unsigned long tiempo = 0;
unsigned long millisActual = 0;
unsigned long millisAnterior = 0;
char contenidoSD = 0;
unsigned char data[28000]={};
uint16_t caracter = 0;
uint16_t temp = 0;
unsigned long indice = 0;
String entradaSerial = "";
bool entradaCompleta = false;
bool b_UP = 0;
bool b_DOWN = 0;
bool b_LEFT = 0;
bool b_RIGHT = 0;
File myFile;

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
void Read_SD(void);
//***************************************************************************************************************************************
// Memoria Flash
//***************************************************************************************************************************************
extern uint8_t inicio[];
//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  // Frecuencia de reloj (utiliza TivaWare)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  // Serial
  Serial.begin(9600);
  Serial2.begin(9600);
  // Pin Mode
  pinMode(PB1, INPUT_PULLUP);
  pinMode(PB2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PF_4), b1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PF_0), b2, CHANGE);
  
  // Configuración del puerto (utiliza TivaWare)
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  
  // Inicialización de la pantalla
  LCD_Init();
  //LCD_Clear(0x9005);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  SPI.setModule(0);

  
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(PA_3, OUTPUT);

  if (!SD.begin(PA_3)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Empezar con la imagen de brasil guardada de la SD
  myFile = SD.open("Brasil.txt");
  Read_SD();
  
  
}

//***************************************************************************************************************************************
// Interrupciones botones de la tiva
//***************************************************************************************************************************************

void b1() {
  BOT1 = digitalRead(PB1);
  if (BOT1 == 0){
    digitalWrite(LED2, 1);
  }
  else{
    digitalWrite(LED2, 0);
  }
}
void b2() {
  BOT2 = digitalRead(PB2);
  if (BOT2 == 0){
    digitalWrite(LED, 1);
  }
  else{
    digitalWrite(LED, 0);
  }
}

//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  
  entradaSerial = "";       // Limpiar lo recibido constantemente
  // Cambios de modo por el boton 1
  if(BOT1 == 1){
    BOT1 = 0;
    bandera = 1;
    if(modo == 1){
     J1_pais = pais;
    }
    else if(modo == 2){
     J2_pais = pais;
     bandera_cambio = 1;
    }
    if((modo == 0)||(modo == 1)||(modo == 2)){
      modo++;
    }
  }
  // Cambios de pais en el modo 2 o 3
  if(BOT2 == 1){
    BOT2 = 0;
    //bandera = 1;
    if((modo == 1)||(modo == 2)){
      pais++;
      if(pais == 4){
        pais = 0;
      }
      if(pais == 0){
        myFile = SD.open("Brasil.txt");
        Read_SD();
      }
      else if(pais == 1){
        myFile = SD.open("Portugal.txt");
        Read_SD();
      }
      else if(pais == 2){
        myFile = SD.open("Arg.txt");
        Read_SD();
      }
      else if(pais == 3){
        myFile = SD.open("GER.txt");
        Read_SD();
      }
    }
  }

  // Si es el modo 3 o 4 imprimir el partido que es
  if((modo == 3)||(modo == 4)){
    if(J1_pais == 0){
      J1_ptxt = "BRA";
    }
    else if(J1_pais == 1){
      J1_ptxt = "POR";
    }
    else if(J1_pais == 2){
      J1_ptxt = "ARG";
    }
    else if(J1_pais == 3){
      J1_ptxt = "GER";
    }
    
    if(J2_pais == 0){
      J2_ptxt = "BRA";
    }
    else if(J2_pais == 1){
      J2_ptxt = "POR";
    }
    else if(J2_pais == 2){
      J2_ptxt = "ARG";
    }
    else if(J2_pais == 3){
      J2_ptxt = "GER";
    }
    LCD_Print(J1_ptxt+" VS "+J2_ptxt,20,200,2,0xFFFF,0x0567);
  }

  // Modo 0 - Pantalla de inicio 
  if(modo == 0){
    if(bandera == 1){
      LCD_Clear(0x9005);
      // Fondo
      LCD_Bitmap(0,0,320,240,inicio);
      bandera = 0;
      J1_goles = 0;
      J2_goles = 0;
      pais = 0;
      myFile = SD.open("Brasil.txt");
      Read_SD();
    }
    // Texto de inicio
    LCD_Print("Catar 2022",125,160,2,0xFFFF,0x9005);
    LCD_Print("Penales",125,176,2,0xFFFF,0x9005);
    LCD_Print("Presione para iniciar",130, 210, 1, 0xFFFF, 0x9005); 
  }
  // Modo 1 - Selección de equipo jugador 1
  else if(modo == 1){
    if(bandera == 1){
      // Limpia la pantalla
      LCD_Clear(0x9005);
      bandera = 0;
    }
    // Texto
    LCD_Print("Seleccion equipo",20,50,2,0xFFFF,0x9005);
    LCD_Print("Jugador 1",20,67,2,0xFFFF,0x9005);
    // Imprime la bandera del pais a seleccionar
    LCD_Bitmap(100, 100, 114, 80, data);
  }
  // Modo 1 - Selección de equipo jugador 1
  else if(modo == 2){
    if(bandera == 1){
      // Limpia la pantalla
      LCD_Clear(0x9005);
      bandera = 0;
    }
    // Texto
    LCD_Print("Seleccion equipo",20,50,2,0xFFFF,0x9005);
    LCD_Print("Jugador 2",20,67,2,0xFFFF,0x9005);
    // Imprime la bandera del pais a seleccionar
    LCD_Bitmap(100, 100, 114, 80, data);
  }
  // Patear penal jugador 1
  else if(modo == 3){
    if(bandera == 1){
      // Lo ejecuta 1 vez
      if(bandera_cambio == 1){
        // Escenario
        LCD_Clear(0x0567);
        FillRect(0,75+60,320,3,0xFFFF);
        FillRect(0,75+60+46+3,320,3,0xFFFF);
        bandera_cambio = 0;
      }
      bandera = 0;
      bandera2 = 1;
      // Aumenta el penal
      P_J1++;
      // Imprime que penal 
      LCD_Print("Penal "+String(P_J1)+" Jugador 1",15,20,2,0xFFFF,0x0567);

      // Conteo con milis
      tiempo = 10000;
      dtiempo_anterior = tiempo/1000;
      millisActual = millis();
      millisAnterior = millisActual;
    }
    
    millisActual = millis();
    if ((millisActual-millisAnterior) >= 0){
      // Conteo con milis cuando llega al tiempo
      if ((millisActual-millisAnterior) >= tiempo){
         millisAnterior = millisActual;
         LCD_Clear(0x0567);
         myFile = SD.open("G1.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         // Imprime al jugador corriendo del pais correspondiente
         // El jugador se va moviendo por el ciclo for
         for (int x = 0; x < 140-1-54; x++){
           if (3 > bandera3){
             bandera3++;
             if(J1_pais == 0){
               myFile = SD.open("JC01.txt");
             }
             else if(J1_pais == 1){
               myFile = SD.open("JC11.txt");
             }
             else if(J1_pais == 2){
               myFile = SD.open("JC21.txt");
             }
             else if(J1_pais == 3){
               myFile = SD.open("JC31.txt");
             }
             Read_SD();
             LCD_Bitmap(x, 158, 54, 65, data);
             V_line(x - 1, 158, 65, 0x0567);
           }
           else if ((6 > bandera3)&&(bandera3 > 2)){
             bandera3++;
             if(J1_pais == 0){
               myFile = SD.open("JC02.txt");
             }
             else if(J1_pais == 1){
               myFile = SD.open("JC12.txt");
             }
             else if(J1_pais == 2){
               myFile = SD.open("JC22.txt");
             }
             else if(J1_pais == 3){
               myFile = SD.open("JC32.txt");
             }
             Read_SD();
             LCD_Bitmap(x, 158, 54, 65, data);
             V_line(x - 1, 158, 65, 0x0567);
           }
            else{
             bandera3 = 0;
           } 
         }
          
         // Imprimir la secuencia del balon
         myFile = SD.open("G2.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G3.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G4.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G5.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G6.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G7.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G8.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);

         // Evalua si es gol o no gol
         // GOL impime la secuencia del balon         
         if(gol == 1){
          myFile = SD.open("G9.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G10.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G11.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G12.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          FillRect(0, 50, 320, 30, 0xFFFF);
          myFile = SD.open("gool.txt");
          Read_SD();
          // Animacion GOL moviendose
          for (int x = 0; x < 320-148; x++){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          for (int x = 320-148; x > 0; x--){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          J1_goles++; // Aumenta el contador de goles 
         }
         // Si es NO GOL 
         else {
          // Imprime secuencia no gol del balon
          myFile = SD.open("NG9.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          FillRect(0, 50, 320, 30, 0x3007);
          // Muestra animacion no gol moviendose
          myFile = SD.open("nogol.txt");
          Read_SD();
          for (int x = 0; x < 320-148; x++){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          for (int x = 320-148; x > 0; x--){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
         }
         delay(2000);   // tiempo de espera          
         modo = 4;      // Cambio de modo
         LCD_Clear(0x0567);
         FillRect(0,75+60,320,3,0xFFFF);
         FillRect(0,75+60+46+3,320,3,0xFFFF);
         bandera = 1;
         bandera2 = 0;
      }
    }
    // Cambia cada 1s
    if (bandera2){
      if(dtiempo_actual != dtiempo_anterior){
        bandera_1s = !bandera_1s;
        dtiempo_anterior = dtiempo_actual;
      }
      // Imprime al futbolista y arquero de una manera
      if(bandera_1s == 0){
        myFile = SD.open("p1.txt");
        Read_SD();
        LCD_Bitmap(200,80,90,56,data);
    
        if(J1_pais == 0){
          myFile = SD.open("VJR1.txt");
        }
        else if(J1_pais == 1){
          myFile = SD.open("CR7.txt");
        }
        else if(J1_pais == 2){
          myFile = SD.open("M1.txt");
        }
        else if(J1_pais == 3){
          myFile = SD.open("TM1.txt");
        }
      }
      // Cambia la manera para que se vea animacion
      else {
        myFile = SD.open("p2.txt");
        Read_SD();
        LCD_Bitmap(200,80,90,56,data);
        if(J1_pais == 0){
          myFile = SD.open("VJR2.txt");
        }
        else if(J1_pais == 1){
          myFile = SD.open("CR72.txt");
        }
        else if(J1_pais == 2){
          myFile = SD.open("M2.txt");
        }
        else if(J1_pais == 3){
          myFile = SD.open("TM2.txt");
        }
      }
      Read_SD();
      LCD_Bitmap(50,75,100,114,data);
      // Muestra el tiempo restante
      dtiempo_actual = tiempo/1000-(millisActual-millisAnterior)/1000;
      LCD_Print("Tiempo restante = "+String(dtiempo_actual)+"    ",23,37,1,0xFFFF,0x0567);  
    }
    
  }
  // El modo 4 es similar al modo 3 con la variante que ahora patea el Jugador 2 
  // La variante es que cuando el J1 tiene 3 penales y el J2 cumple los 3 penales, el juego acaba y pasa al modo 5
  else if(modo == 4){
    if(bandera == 1){
      bandera = 0;
      bandera2 = 1;
      P_J2++;
      LCD_Print("Penal "+String(P_J2)+" Jugador 2",15,20,2,0xFFFF,0x0567);
      tiempo = 10000;
      dtiempo_anterior = tiempo/1000;
      millisActual = millis();
      millisAnterior = millisActual;
    }

    millisActual = millis();
    if ((millisActual-millisAnterior) >= 0){
      if ((millisActual-millisAnterior) >= tiempo){
         millisAnterior = millisActual;
         LCD_Clear(0x0567);
         myFile = SD.open("G1.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);

        for (int x = 0; x < 140-1-54; x++){
          
          if (3 > bandera3){
            bandera3++;
            if(J2_pais == 0){
              myFile = SD.open("JC01.txt");
            }
            else if(J2_pais == 1){
              myFile = SD.open("JC11.txt");
            }
            else if(J2_pais == 2){
              myFile = SD.open("JC21.txt");
            }
            else if(J2_pais == 3){
              myFile = SD.open("JC31.txt");
            }
            Read_SD();
            LCD_Bitmap(x, 158, 54, 65, data);
            V_line(x - 1, 158, 65, 0x0567);
          }
          else if ((6 > bandera3)&&(bandera3 > 2)){
            bandera3++;
            if(J2_pais == 0){
              myFile = SD.open("JC02.txt");
            }
            else if(J2_pais == 1){
              myFile = SD.open("JC12.txt");
            }
            else if(J2_pais == 2){
              myFile = SD.open("JC22.txt");
            }
            else if(J2_pais == 3){
              myFile = SD.open("JC32.txt");
            }
            Read_SD();
            LCD_Bitmap(x, 158, 54, 65, data);
            V_line(x - 1, 158, 65, 0x0567);
          }
           else{
            bandera3 = 0;
          } 
        }
            
         myFile = SD.open("G2.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G3.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G4.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G5.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G6.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G7.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         myFile = SD.open("G8.txt");
         Read_SD();
         LCD_Bitmap(140,150,174,73,data);
         
         if(gol == 1){
          myFile = SD.open("G9.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G10.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G11.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          myFile = SD.open("G12.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          //LCD_Print("GOOOOL",15,20,2,0xFFFF,0x0567);
          FillRect(0, 50, 320, 30, 0xFFFF);
          myFile = SD.open("gool.txt");
          Read_SD();
          for (int x = 0; x < 320-148; x++){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          for (int x = 320-148; x > 0; x--){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          J2_goles++;
         }
         else {
          myFile = SD.open("NG9.txt");
          Read_SD();
          LCD_Bitmap(140,150,174,73,data);
          //LCD_Print("Fallaste",15,20,2,0xFFFF,0x0567);
          FillRect(0, 50, 320, 30, 0x3007);
          myFile = SD.open("nogol.txt");
          Read_SD();
          for (int x = 0; x < 320-148; x++){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
          for (int x = 320-148; x > 0; x--){
            delay(2);
            LCD_Bitmap(x, 50, 148, 30, data);
          }
         }
          
         delay(2000);
          
         modo = 3;
         LCD_Clear(0x0567);
         FillRect(0,75+60,320,3,0xFFFF);
         FillRect(0,75+60+46+3,320,3,0xFFFF);
         if((P_J1 == 3)&&(P_J2 == 3)){
          P_J1 = 0;
          P_J2 = 0;
          modo = 5;
         }
         bandera = 1;
         bandera2 = 0;
      }
    }
    
    if (bandera2){
      if(dtiempo_actual != dtiempo_anterior){
        bandera_1s = !bandera_1s;
        dtiempo_anterior = dtiempo_actual;
      }
      
      if(bandera_1s == 0){
        myFile = SD.open("p1.txt");
        Read_SD();
        LCD_Bitmap(200,80,90,56,data);
        if(J2_pais == 0){
          myFile = SD.open("VJR1.txt");
        }
        else if(J2_pais == 1){
          myFile = SD.open("CR7.txt");
        }
         if(J2_pais == 2){
          myFile = SD.open("M1.txt");
        }
        else if(J2_pais == 3){
          myFile = SD.open("TM1.txt");
        }
      }
      else {
        myFile = SD.open("p2.txt");
        Read_SD();
        LCD_Bitmap(200,80,90,56,data);
        if(J2_pais == 0){
          myFile = SD.open("VJR2.txt");
        }
        else if(J2_pais == 1){
          myFile = SD.open("CR72.txt");
        }
        else if(J2_pais == 2){
          myFile = SD.open("M2.txt");
        }
        else if(J2_pais == 3){
          myFile = SD.open("TM2.txt");
        }
      }
      Read_SD();
      LCD_Bitmap(50,75,100,114,data);
  
      dtiempo_actual = tiempo/1000-(millisActual-millisAnterior)/1000;
      LCD_Print("Tiempo restante = "+String(dtiempo_actual)+"    ",23,37,1,0xFFFF,0x0567);
    }
    
  }
  // Mostrar el ganador y el resultado
  else if(modo == 5){
    if(bandera == 1){
      LCD_Clear(0x9005);
      // Si J1 tiene mas goles
      if(J1_goles > J2_goles){
        // Campeon J1 y muestra el resultado
        pais = J1_pais;
        LCD_Print("CAMPEON "+J1_ptxt,100,140,2,0xFFFF,0x9005);
        LCD_Print(J1_ptxt+String(J1_goles)+"-"+String(J2_goles)+J2_ptxt,100,156,2,0xFFFF,0x9005);
      }
      else if(J2_goles > J1_goles){
        // Si J1 tiene mas goles
        pais = J2_pais;
        // Campeon J2 y muestra el resultado
        LCD_Print("CAMPEON "+J2_ptxt,100,140,2,0xFFFF,0x9005);
        LCD_Print(J1_ptxt+String(J1_goles)+"-"+String(J2_goles)+J2_ptxt,100,156,2,0xFFFF,0x9005);
      }
      // Empate
      else{
        LCD_Print("EMPATE",100,140,2,0xFFFF,0x9005);
        LCD_Print(J1_ptxt+String(J1_goles)+"-"+String(J2_goles)+J2_ptxt,100,156,2,0xFFFF,0x9005);
      }
      myFile = SD.open("Trof.txt");
      Read_SD();
      LCD_Bitmap(20, 45, 67, 150, data);
      // Imprime bandera ganador
      if(J2_goles != J1_goles){
        if(pais == 0){
          myFile = SD.open("Brasil.txt");
        }
        else if(pais == 1){
          myFile = SD.open("Portugal.txt");
        }
        else if(pais == 2){
          myFile = SD.open("Arg.txt");
        }
        else if(pais == 3){
          myFile = SD.open("GER.txt");
        }
        Read_SD();
        LCD_Bitmap(100, 45, 114, 80, data);
      }
      
      bandera = 0;
      millisActual = millis();
      millisAnterior = millisActual;
    }
    // Tiempos
    tiempo = 10000;
    millisActual = millis();
    if ((millisActual-millisAnterior) >= 0){
      if ((millisActual-millisAnterior) >= tiempo){
         millisAnterior = millisActual;
         modo = 0;
         bandera = 1;
      }
    }
    //LCD_Print(String((millisActual-millisAnterior)/1000),0,0,2,0x0000,0xFFFF);
  }

  
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}

//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 

//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  //Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    //Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}

//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Funcion READ SD
//***************************************************************************************************************************************

void Read_SD(void){  
  memset(data, 0, 28000);
  indice = 0;
  bandera_SD = 0;
  if (myFile){    
    while (myFile.available()) {
       contenidoSD = myFile.read();
       if (contenidoSD == 'f'){
          caracter = 15;
       }
       else if (contenidoSD == 'a'){
          caracter = 10;
       }
       else if (contenidoSD == 'b'){
          caracter = 11;
       }
       else if (contenidoSD == 'c'){
          caracter = 12;
       }
       else if (contenidoSD == 'd'){
          caracter = 13;
       }
       else if (contenidoSD == 'e'){
          caracter = 14;
       }
       else if (contenidoSD == '0'){
          caracter = 0;
       }
       else if (contenidoSD == '1'){
          caracter = 1;
       }
       else if (contenidoSD == '2'){
          caracter = 2;
       }
       else if (contenidoSD == '3'){
          caracter = 3;
       }
       else if (contenidoSD == '4'){
          caracter = 4;
       }
       else if (contenidoSD == '5'){
          caracter = 5;
       }
       else if (contenidoSD == '6'){
          caracter = 6;
       }
       else if (contenidoSD == '7'){
          caracter = 7;
       }
       else if (contenidoSD == '8'){
          caracter = 8;
       }
       else if (contenidoSD == '9'){
          caracter = 9;
       }
       if (bandera_SD == 0){
        temp = (caracter*16);        
       }
       if (bandera_SD == 1){
        temp = (temp + (caracter));
        data[indice] = temp;
        indice++;
       }
       bandera_SD = !bandera_SD;
    }
    myFile.close();
  }
}
//***************************************************************************************************************************************
// Serial Event
//***************************************************************************************************************************************

void serialEvent(){
  entradaSerial = "";
  while(Serial2.available()){
    char inChar = (char)Serial2.read();
    // GOL
    if(inChar == 'B'){
      Serial.println("Que golazo"); 
      gol = 1;
    }
    // NO GOL
    else if(inChar == 'A'){
      Serial.println("Que pena :(");      
      gol = 0;
    }
    // JOYSTICK ARRIBA
    else if(inChar == 'U'){
      if(b_UP == 0){
        // Lo hace una sola vez
        if((modo == 0)||(modo == 1)||(modo == 2)){
          BOT1 = 1;
        }
        b_UP = 1;
      }
    }
    // JOYSTICK ABAJO
    else if(inChar == 'D'){
      if(b_DOWN == 0){
        // Lo hace una sola vez
        if(modo == 0){
          BOT1 = 1;
        }
        else if(modo == 2){
          modo--;
        }
        b_DOWN = 1;
      }
    }
    // JOYSTICK IZQUIERDA
    else if(inChar == 'L'){
      if(b_LEFT == 0){
        // Lo hace una sola vez
        if(modo == 0){
          BOT1 = 1;
        }
        if((modo == 1)||(modo == 2)){
          if(pais == 0){
            pais = 3;
          }
          else{
            pais--;
          }
          if(pais == 0){
            myFile = SD.open("Brasil.txt");
            Read_SD();
          }
          else if(pais == 1){
            myFile = SD.open("Portugal.txt");
            Read_SD();
          }
          else if(pais == 2){
            myFile = SD.open("Arg.txt");
            Read_SD();
          }
          else if(pais == 3){
            myFile = SD.open("GER.txt");
            Read_SD();
          }
        }
        b_LEFT = 1;
      }
    }
    // JOYSTICK DERECHA
    else if(inChar == 'R'){
      if(b_RIGHT == 0){
        // Lo hace una sola vez
        if(modo == 0){
          BOT1 = 1;
        }
        if((modo == 1)||(modo == 2)){
          BOT2 = 1;
        }
        b_RIGHT = 1;
      }
    }
    else if(inChar == 'M'){
      b_UP = 0;
      b_DOWN = 0;
      b_LEFT = 0;
      b_RIGHT = 0;
    }
    else{
      Serial.println("El dato no es correcto");
    }
  }
}
