//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo SPI
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************

//***************************************************************************************************************************************
//                                            L I B R E R I A S
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include <SD.h>
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

//***************************************************************************************************************************************
//                                         D I R E C T I V A S  D E  P R O G R A M A
//***************************************************************************************************************************************
// El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2
#define LCD_RST PD_0
#define LCD_DC PD_1
#define LCD_CS PA_3

#define SD_CS PD_7
//***************************************************************************************************************************************
//                                  P R O T O T I P O S  D E  F U N C I O N E S
//***************************************************************************************************************************************
void nivel1(void);
void nivel2(void);
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

int ascii2hex(int a);
void mapeo_SD(char doc[], int y, int h);
void tile_maker(int t_ini, int t_fin, int t_paso, int posx, int posy, int width, int height, unsigned char bitmap[]);
void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
//***************************************************************************************************************************************
//                                                   V A R I A B L E S
//***************************************************************************************************************************************
File myFile;
extern unsigned char nientiendo[];
extern unsigned char DK_1[];
extern unsigned char DK_2[];
extern unsigned char menu[];
extern unsigned char blue_sqr[];
extern unsigned char banana[];
extern unsigned char arrow[];
extern unsigned char tube[];
extern unsigned char tube1[];
extern unsigned char tube2[];
extern unsigned char tube3[];
extern unsigned char bola[];
extern unsigned char peach[];
extern unsigned char donkey[];

const int btnPin1 = PUSH1; //START, OK
const int btnPin2 = PUSH2; //Manejo Menú 
const int btnPin3 = PC_4; //Izquierda - Luigi - Flecha izquierda
const int btnPin4 = PC_5; //Arriba - Luigi - L1
const int btnPin5 = PC_6; //Derecha - Luigi - Flecha derecha
const int btnPin6 = PC_7; //Izquierda - Mario - Cuadrado
const int btnPin7 = PB_6; //Arriba - Mario - R1
const int btnPin8 = PB_7; //Derecha - Mario - Circulo
uint8_t btnState1;
uint8_t btnState2;
uint8_t btnState3;
uint8_t btnState4;
uint8_t btnState5;
uint8_t btnState6;
uint8_t btnState7;
uint8_t btnState8;

bool audioState = 1;
bool TwoPlayers = 0;
int selector = 0;               // <---------- SELECTOR
int arrow_PosY = 10;
int arrow_PosY2=10;
// Variables de Mario
int Mario_posX=75;
int Mario_posY=215;
int Mario_posX2;
int Mario_posY2;
int m_hammer = 0;
int m_lives = 3;
// Variables de Luigi
int Luigi_posX=30;
int Luigi_posY=215;
int Luigi_posX2;
int Luigi_posY2;
int l_hammer = 0;
int l_lives = 3;
//ENEMIGOS NIVEL 1
//Barril 1
int timer_enemy1 = 0;
int barrel1 = 280;
bool activeBarrel1 = 1;
bool indicadorB1 = 0;
//Barril 2
int timer_enemy2 = 0;
int barrel2 = 26;
bool activeBarrel2 = 1;
bool indicadorB2 = 0;
//Barril 3
int timer_enemy3 = 0;
int barrel3 = 275;
bool activeBarrel3 = 1;
bool indicadorB3 = 0;
//Barril 4
int timer_enemy4 = 0;
int barrel4 = 50;
bool activeBarrel4 = 1;
bool indicadorB4 = 0;
//ENEMIGOS NIVEL 2
int bola_posX=160-16;
int bola_posX2=160-16;
int bola_posX3=20;
int bolaa_posX=160-16; //plat 2
int bolaa_posX2=160-16; //plat 2
int bolaa_posX3=160-16; //plat 3
int bolaa_posX4=160-16; //plat 3
int bolaa_posX5=20; //plat 4
int bolaLD=1;
int bolaLI=1;
int bolaLD2=1;
int bolaLI2=1;
int bolaLD3=1;
int bolaLI3=1;
int bolaLD4=1;
int bolaLI4=1;
int bolaLD5=1;
int bolaLI5=1;
int hpcont=3;
int hpcontM=3;
int hpcontL=3;

//***************************************************************************************************************************************
//                                                I N I C I A L I Z A C I O N
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);
  pinMode(LCD_CS, OUTPUT);    //definiendo salida para lcd cs
  pinMode(SD_CS, OUTPUT);     //definiendo salida para sd cs
  
  SPI.setModule(0);
  //Pantalla LCD
  digitalWrite(LCD_CS, LOW);
  delay(10);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);
  digitalWrite(LCD_CS, HIGH);
  delay(1000);

  //Memoria SD
  digitalWrite(SD_CS, LOW);
  delay(10);
  Serial.print("Inicializando SD...");
  if(!SD.begin(32)){
    Serial.println("inicializacion fallida");
    return;
    }
  Serial.println("inicializacion completada");
  delay(10);
  digitalWrite(SD_CS, HIGH);
  delay(1000);

  //initialize the pushbutton pin as an input
  pinMode(btnPin1, INPUT_PULLUP);
  pinMode(btnPin2, INPUT_PULLUP);
  pinMode(btnPin3, INPUT_PULLUP);
  pinMode(btnPin4, INPUT_PULLUP);
  pinMode(btnPin5, INPUT_PULLUP);
  pinMode(btnPin6, INPUT_PULLUP);
  pinMode(btnPin7, INPUT_PULLUP);
  pinMode(btnPin8, INPUT_PULLUP);

  //Pantalla de inicio, muestra logo Nientiendo
  LCD_Bitmap(33, 90, 263, 52, nientiendo);
  delay(2000);
  LCD_Clear(0x00);
  //Pantalla de Inicio - muestra el logo de DK
  mapeo_SD("DK.txt", 0, 240);
  
}
//***************************************************************************************************************************************
//                                            L O O P   I N F I N I T O
//***************************************************************************************************************************************
void loop() {

  //*************************************************************************************************************************************
  // BOTONES
  //*************************************************************************************************************************************
  uint8_t btnState1 = digitalRead(btnPin1);
  uint8_t btnState2 = digitalRead(btnPin2);
  uint8_t btnState3 = digitalRead(btnPin3);
  uint8_t btnState4 = digitalRead(btnPin4);
  uint8_t btnState5 = digitalRead(btnPin5);
  uint8_t btnState6 = digitalRead(btnPin6);
  uint8_t btnState7 = digitalRead(btnPin7);
  uint8_t btnState8 = digitalRead(btnPin8);

  Serial.print("Selector: ");
  Serial.println(selector);

  //*************************************************************************************************************************************
  // PANTALLA DE INICIO | Selector = 0
  //*************************************************************************************************************************************
  if (selector==0){ //Animacion PUSH START
        //Dejar presionado el boton para que pase de pantalla de inicio
        if(btnState4 == HIGH){
        selector = 1; //Selector = 1 para avanzar
        }
        
        LCD_Bitmap(79, 86, 160, 14, blue_sqr);
        delay(500);
        LCD_Bitmap(79, 86, 160, 14, DK_1); //PUSH START
        delay(500);
  }
  //*************************************************************************************************************************************
  // TRANSICIÓN | Selector = 1
  //*************************************************************************************************************************************
  else if (selector==1){ //Pantalla de transición Amarilla
            FillRect(0, 0, 320, 240, 0xff00);
            selector = 2; //Selector = 2 para avanzar
  }
  //*************************************************************************************************************************************
  // MENÚ PRINCIPAL | Selector = 2
  //*************************************************************************************************************************************
  else if (selector==2){//
            String text1 = "1 PLAYER"; //Opción 1
            LCD_Print(text1, 85, 15, 2, 0x0000, 0xff00);
        
            String text2 = "2 PLAYERS"; //Opción 2
            LCD_Print(text2, 85, 40, 2, 0x0000, 0xff00);
        
            String text3 = " OPTIONS"; //Opción 3
            LCD_Print(text3, 85, 65, 2, 0x0000, 0xff00);
        
            LCD_Bitmap(174, 99, 146, 142, banana);
            LCD_Bitmap(35, 10, 33, 25, arrow);
            FillRect(35, 60, 33, 25, 0xff00);
            selector = 3;   //coloca el selector en 3 para avanzar
  }
  //*************************************************************************************************************************************
  // ELECCIÓN EN MENÚ PRINCIPAL | Selector = 3
  //*************************************************************************************************************************************
   else if (selector==3){
              if (btnState7==HIGH){//Presionar botón 7 para moverse
                delay(200);     //"antirebote"
                arrow_PosY = arrow_PosY + 25;     //La flecha se desplaza hacia abajo 25 unidades
                if (arrow_PosY != arrow_PosY2){   //Si la posición previa es distinta a la actual se realiza lo siguiente
                  if (arrow_PosY > 60){           //Si la posición actual pasa de la última opción --> dar un wrap-around en el menu
                     arrow_PosY = 10;              //Flecha regresa a posicion inicial de 10 unidades  
                    }
                  
                   delay(5);
                  
                  LCD_Bitmap(35, arrow_PosY, 33, 25, arrow);    //Se renderiza la imagen en la nueva posicion
                  FillRect(35, arrow_PosY2, 33, 25, 0xff00);    //Se cubre la posicion antigua
                  arrow_PosY2 = arrow_PosY;                     //se actualiza la posicion2
                }
              }

              //OPCIÓN "options"  Selector --> 4
              if (btnState4==HIGH && arrow_PosY == 60) {     //Si se presiona el botón 1 y la posicion de la flecha es 60
                  selector = 4;                             //Entonces el selector toma el valor de 4
                  arrow_PosY = 40;                          //Se le da una nueva posicion a la flecha para el siguiente menu
              }
              //OPCIÓN "1 player"  Selector --> 6
              if (btnState4==HIGH && arrow_PosY == 10){      //Si se presiona el botón 1 y la posicion de la flecha es 10
                  selector = 6;                             //Entonces el selector toma el valor de 6
                  TwoPlayers = 0;                           //1 jugador
                  arrow_PosY = 10;   
                  arrow_PosY2=10;
              }
              //OPCIÓN "2 players"  Selector --> 6
              if (btnState4==HIGH && arrow_PosY == 35){      //Si se presiona el botón 1 y la posicion de la flecha es 35
                  selector = 6;                             //Entonces el selector toma el valor de 6
                  TwoPlayers = 1;                           //2 jugadores
                  arrow_PosY = 10;   
                  arrow_PosY2=10;
              }
  }
  //*************************************************************************************************************************************
  // TRANSICIÓN a "OPTIONS" | Selector = 4 | Selector --> 5
  //*************************************************************************************************************************************    
   else if (selector==4){//Sub-menu de opciones
              FillRect(0,0,320,240,0xff00);         //Se pinta un canva amarillo
              LCD_Bitmap(35, 10, 33, 25, arrow);    //Se renderiza una flecha 
              selector = 5;                         //el selector toma un valor de 5
  }
  //*************************************************************************************************************************************
  // MENÚ  "OPTIONS" | Selector = 4 | Selector --> 5
  //*************************************************************************************************************************************       
   else if (selector==5){
                if (btnState7==HIGH){//Si se presiona el botón 2
                  delay(200);     //"antirebote"
                  arrow_PosY = arrow_PosY + 25;     //La flecha se desplaza hacia abajo 25 unidades
                  if (arrow_PosY != arrow_PosY2){   //Si la posición previa es distinta a la actual
                    if (arrow_PosY > 35){           //Logica para dar un wrap-arpund en el menu
                      arrow_PosY = 10;              //Flecha regresa a posicion inicial de 10 unidades
                    }
                    delay(5);
                    
                    LCD_Bitmap(35, arrow_PosY, 33, 25, arrow);    //Se renderiza la imagen en la nueva posicion
                    FillRect(35, arrow_PosY2, 33, 25, 0xff00);    //Se cubre la posicion antigua
                    arrow_PosY2 = arrow_PosY;                     //se actualiza la posicion2 
                  }
                 }

                //VALOR DE AUDIO --> ON/OFF
                String text4 = "AUDIO: ";                         //Opciones del nuevo sub-menu
                LCD_Print(text4, 40+50, 15, 2, 0x0, 0xff00);      //Se imprime audio
                
                //AUDIO=ON
                if (audioState == 1){                             //Si el estado de audio tiene un valor de 1
                  String text5 = "ON  ";                          //Volumen ON (por default)
                  LCD_Print(text5, 150+50, 15, 2, 0x0, 0xff00);
                }
                //AUDIO=OFF
                else if (audioState == 0){                        //Si el estado de audio tiene un valor de 0
                  String text6 = "OFF";                           //el audio se apaga
                  LCD_Print(text6, 150+50, 15, 2, 0x0, 0xff00);
                }

                //CAMBIAR AUDIO - FLECHA en AUDIO  
                if (btnState4==HIGH && arrow_PosY == 10){          //Si la flecha tiene la posicion 10 y se presiona el botón 1 
                  delay(200);                                     //antirrebote
                  audioState = !audioState;                       //Se cambia el estado del audio (se niega) (OFF)
                }
                  
                String text7 = "BACK";                            //Se impreme la opcion de regreso al menu principal en pantalla
                LCD_Print(text7, 40+50, 40, 2, 0x0, 0xff00);

                //REGRESAR AL MENU - FLECHA en BACK
                if (btnState4==HIGH && arrow_PosY == 35){          //Si el botón1 se presiona y la flecha tiene la posicon de 35
                  selector = 1;                                   //El selector tiene valor de uno para regresar al menu principal
                  arrow_PosY = 65;                                //Se cambia la posicion de la flecha para el menu principal
                }
  }
  //*************************************************************************************************************************************
  // TRANSICIÓN a NIVELES| Selector = 6 | Selector --> 7 
  //*************************************************************************************************************************************
  else if (selector==6){ //Pantalla de transición Amarilla
            FillRect(0, 0, 320, 240, 0xff00);
            selector = 7; //Selector = 7 para avanzar
  }
  //*************************************************************************************************************************************
  // MENÚ DE NIVELES | Selector = 7 | Selector --> 8 
  //*************************************************************************************************************************************
  else if (selector==7){//
            String text1 = "LEVEL 1"; //Nivel 1
            LCD_Print(text1, 85, 15, 2, 0x0000, 0xff00);
        
            String text2 = "LEVEL 2"; //Nivel 2
            LCD_Print(text2, 85, 40, 2, 0x0000, 0xff00);
        
            String text3 = "BACK"; //Opción de regresar
            LCD_Print(text3, 85, 65, 2, 0x0000, 0xff00);
        
            LCD_Bitmap(174, 99, 146, 142, banana);
            LCD_Bitmap(35, 10, 33, 25, arrow);
            selector = 8;   //coloca el selector en 8 para avanzar
  }
  //*************************************************************************************************************************************
  // ELECCIÓN DE NIVEL | Selector = 8 
  //*************************************************************************************************************************************
   else if (selector==8){
              if (btnState7==HIGH){//Presionar botón 2 para moverse
                delay(200);     //"antirebote"
                arrow_PosY = arrow_PosY + 25;     //La flecha se desplaza hacia abajo 25 unidades
                if (arrow_PosY != arrow_PosY2){   //Si la posición previa es distinta a la actual se realiza lo siguiente
                  if (arrow_PosY > 60){           //Si la posición actual pasa de la última opción --> dar un wrap-around en el menu
                    arrow_PosY = 10;              //Flecha regresa a posicion inicial de 10 unidades  
                  }
                  
                  delay(5);
                  
                  LCD_Bitmap(35, arrow_PosY, 33, 25, arrow);    //Se renderiza la imagen en la nueva posicion
                  FillRect(35, arrow_PosY2, 33, 25, 0xff00);    //Se cubre la posicion antigua
                  arrow_PosY2 = arrow_PosY;                     //se actualiza la posicion2
                }
              }

              //OPCIÓN "BACK"  Selector --> 4
              if (btnState4==HIGH && arrow_PosY == 60) {     //Si se presiona el botón 1 y la posicion de la flecha es 60
                  selector = 1;                             //Entonces el selector toma el valor de 1, de regreso a menú principal
                  arrow_PosY = 40;                          //Se le da una nueva posicion a la flecha para el siguiente menu
              }
              //OPCIÓN "LEVEL 1"  Selector --> 9
              if (btnState4==HIGH && arrow_PosY == 10){      //Si se presiona el botón 1 y la posicion de la flecha es 10
                  selector = 9;                             //Entonces el selector toma el valor de 9
                                           
              }
              //OPCIÓN "LEVEL 2"  Selector --> 10
              if (btnState4==HIGH && arrow_PosY == 35){      //Si se presiona el botón 1 y la posicion de la flecha es 35
                  selector = 10;                             //Entonces el selector toma el valor de 10
                  
              }
  }
  //*************************************************************************************************************************************
  // NIVEL 1 | Selector = 9 |
  //*************************************************************************************************************************************   
  else if (selector==9){
                nivel1(); //Dibujamos el nivel
                
                //*************************************************************************************************************************************
                // FILTRO DE NO. de JUGADORES
                //*************************************************************************************************************************************   
                //FILTRO PARA 1 JUGADOR
                if (TwoPlayers == 0){       //Si TwoPlayers esta en 0, solo hay 1 jugador
                      String textM = "M HP: 3";
                      LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                      uint16_t anim_speed = Mario_posX/7;
                      uint16_t anim_frame = anim_speed%3;
                      LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                      selector = 11;
                }
                
                //FILTRO PARA 2 JUGADORES
                if (TwoPlayers == 1){       //Si TwoPlayers esta en 1, se renderizan 2 jugadores
                      String textM = "M HP: 3";
                      LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                      String textL = "L HP: 3";
                      LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                      uint16_t anim_speed = Mario_posX/7;
                      uint16_t anim_frame = anim_speed%3;
                      LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 1, 0);
                      LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                      selector = 12;
                }
  }
  //*************************************************************************************************************************************
  // NIVEL 2 | Selector = 10 | 
  //*************************************************************************************************************************************   
  else if (selector==10){
                nivel2();//Dibujamos el Nivel
                Mario_posX=20;
               //*************************************************************************************************************************************
                // FILTRO DE NO. de JUGADORES
                //*************************************************************************************************************************************   
                //FILTRO PARA 1 JUGADOR
                if (TwoPlayers == 0){       //Si TwoPlayers esta en 0, solo hay 1 jugador
                      String textM = "M HP: 3";
                      LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                      uint16_t anim_speed = Mario_posX/7;
                      uint16_t anim_frame = anim_speed%3;
                      LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                      selector = 14;
                }
                
                //FILTRO PARA 2 JUGADORES
                if (TwoPlayers == 1){       //Si TwoPlayers esta en 1, se renderizan 2 jugadores
                      String textM = "M HP: 3";
                      LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                      String textL = "L HP: 3";
                      LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                      uint16_t anim_speed = Mario_posX/7;
                      uint16_t anim_frame = anim_speed%3;
                      LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 1, 0);
                      LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                      selector = 15;
                }
  }
  //*************************************************************************************************************************************
  // NIVEL 1 | JUGABILIDAD 1 JUGADOR
  //************************************************************************************************************************************* 
        else if (selector==11){
                //Lanzando el primer barril
                    timer_enemy1++;     //temporizador del barril
                    if (timer_enemy1 == 2 && activeBarrel1){    //si el temporizador llego a 2 y el barril esta activo
                      barrel1--;                                //el barril se desplaza una posicion a la izquierda
                      uint16_t anim_frame = (barrel1/7)%3;      //velocidad de animacion
                      LCD_Sprite(barrel1, 215, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                      V_line(barrel1+12, 215, 10, 0x0000);                              //se cubre su huella
                      timer_enemy1 = 0;                                                 // se resetea el tmr
                      }
                    if ((barrel1 <5 || Mario_posY != 215) && indicadorB1 == 0){         //Si el barrel llego a una posicion en x menor a 5 o Mario escalo
                      activeBarrel1 = 0;                        //se desactiva
                      indicadorB1 = 1;                          //se desactiva el indicador
                      FillRect(barrel1,215, 20, 15, 0x0);             //se tapa
                      barrel1 = 0;                              //se le da una posicion en 0
                      FillRect(0,215, 20, 15, 0x0);             //se tapa
                      }

                     //Lanzando el segundo barril
                     if (Mario_posY ==173){//Si mario está  en la segunda plataforma
                      timer_enemy2++;                             //Aumenta el temporizador del enemigo2
                      if (timer_enemy2 == 2 && activeBarrel2){    //si el temporizador llego a 2 y el barril esta activo
                        barrel2++;                                //el barril se desplaza una unidad hacia la derecha
                        uint16_t anim_frame = (barrel2/7)%3;      //velocidad de animacion
                        LCD_Sprite(barrel2, 173, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel2-1, 173, 10, 0x0);          //se tapa
                        timer_enemy2 = 0;                         //se reinicia el tmr
                        }
                      }
                      if ((barrel2 > 307 || Mario_posY < 173) && indicadorB2 == 0){   //si el barril llego a una posicion mayor de 307 o mario escalo
                        activeBarrel2 = 0;                      //se desactiva el barril
                        indicadorB2 = 1;                        //se desactiva el indicador
                        FillRect(barrel2, 173, 20, 15, 0x0);    //se tapa
                        barrel2 = 307;                          //se le da una nueva posicion
                        FillRect(barrel2,173,20,15,0x0);        //se tapa
                        }
                    //Lanzando el tercer barril
                     if (Mario_posY ==132){//Si mario está  en la tercera plataforma
                      timer_enemy3++;
                      if (timer_enemy3 == 2 && activeBarrel3){
                        barrel3--;
                        uint16_t anim_frame = (barrel3/7)%3;
                        LCD_Sprite(barrel3, 132, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel3+12, 132, 10, 0x0);
                        timer_enemy3 = 0;
                        }
                      }
                      if ((barrel3 < 10 || Mario_posY < 132) && indicadorB3 == 0){    //si el barril llego a una posicion menor a 10 o mario escalo
                        activeBarrel3 = 0;                      //se desactiva el barril
                        indicadorB3 = 1;                        //se desactiva el indicador
                        FillRect(barrel3, 132, 20, 15, 0x0);    //se tapa  
                        barrel2 = 10;                           //se le da una nueva posicion
                        FillRect(barrel3,132,20,15,0x0);        //se tapa
                        }
                     //Lanzando el cuarto barril
                     if (Mario_posY ==91){//Si mario está  en la cuarta plataforma
                      timer_enemy4++;
                      if (timer_enemy4 == 2 && activeBarrel4){
                        barrel4++;
                        uint16_t anim_frame = (barrel4/7)%3;
                        LCD_Sprite(barrel4, 91, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel4-1, 91, 10, 0x0);
                        timer_enemy4 = 0;
                        }
                      }
                      if ((barrel4 > 307 || Mario_posY < 91) && indicadorB4 == 0){    //si el barril llego a una posicion mayor a 307 o mario escalo
                        activeBarrel4 = 0;                  //se desactiva el barril
                        indicadorB4 = 1;                    //se desactiva el indicador
                        FillRect(barrel4, 91,20,15,0x0);    //se tapa
                        barrel4 = 307;                      //se le da una nueva posicion
                        FillRect(barrel4,91,20,15,0x0);     //se tapa
                        }
                      
                    //Activando el martillo de mario
                    if (Mario_posX == 280 && Mario_posY == 173){
                      m_hammer = 1;
                      }

                    //Quitando vidas a mario
                    Serial.print("Vidas: ");
                    Serial.println(m_lives);
                    Serial.print("B1:");
                    Serial.println(activeBarrel1);
                    Serial.print("B2:");
                    Serial.println(activeBarrel2);
                    Serial.print("B3:");
                    Serial.println(activeBarrel3);
                    if (Mario_posX == barrel1 && Mario_posY == 215 && activeBarrel1==1){
                      activeBarrel1 = 0;                  //se desactiva el barril
                      FillRect(barrel1,215, 20, 15, 0x0);             //se tapa
                      if (m_hammer == 0){
                        String textM = "M HP: 2";
                        LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                        m_lives = m_lives-1;
                        }
                      }
                    else if (Mario_posX == barrel2 && Mario_posY == 173 && activeBarrel2==1){
                       activeBarrel2 = 0;                      //se desactiva el barril
                       FillRect(barrel2, 173, 20, 15, 0x0);    //se tapa
                       if (m_hammer == 0){
                        String textM = "M HP: 1";
                        LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                        m_lives = m_lives-1;
                        }
                      }
                    else if (Mario_posX == barrel3 && Mario_posY == 132 && activeBarrel3==1){
                      activeBarrel3 = 0;                      //se desactiva el barril
                      FillRect(barrel3, 132, 20, 15, 0x0);    //se tapa
                      if (m_hammer == 0)
                      {m_lives = m_lives-1;}
                      }
                     else if (Mario_posX == barrel4 && Mario_posY == 91 && activeBarrel4==1){
                      activeBarrel4 = 0;                  //se desactiva el barril
                      FillRect(barrel4, 91,20,15,0x0);    //se tapa
                      if (m_hammer == 0)
                      {m_lives = m_lives-1;}
                      }

                    //reiniciando el juego
                    if (m_lives == 0){
                      for (int x = Mario_posX; x < Mario_posX+3; x++){
                        delay(15);
                        LCD_Sprite(x, Mario_posY, 16, 15, mario_dies, 3, 3, 1, 0);
                        }
                      delay(900);
                      selector = 13;   
                      m_lives = 3;
                      }


                     if (btnState8==HIGH){
                        Mario_posX++;
                        if(Mario_posX!=Mario_posX2){
                          if(Mario_posX>320-16){
                              Mario_posX=(320-16);
                            }
                        if (m_hammer == 0){
                         uint16_t anim_speed = Mario_posX/7;
                         uint16_t anim_frame = anim_speed%3;
                         LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                         V_line(Mario_posX - 1, Mario_posY,16, 0x0000);
                        }else if (m_hammer == 1){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY-10, 31, 26, mario_hammer, 5, anim_frame, 1, 0);
                           V_line(Mario_posX - 1, Mario_posY-10,16, 0x0000);
                          }
                       }
                      }
                    if (btnState6==HIGH){
                        Mario_posX--;
                        if(Mario_posX!=Mario_posX2){
                          if(Mario_posX<16){
                              Mario_posX=16;
                            }
                        if (m_hammer == 0){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 0, 0);
                           V_line(Mario_posX + 15, Mario_posY, 16, 0x0000);
                        }else if (m_hammer == 1){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY-10, 31, 26, mario_hammer, 5, anim_frame, 0, 0);
                           V_line(Mario_posX+30, Mario_posY-10, 26, 0x0000);
                          }
                       }
                      }
                      
                    if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 159){
                      upstairs1(215, 174, 155, 16, 15, mario_climbs, 1, 1);
                      Mario_posY=173; 
                      }
                    if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 90 ){
                      upstairs1(173, 132, 90, 16, 15, mario_climbs, 2, 2);
                      Mario_posY=132;
                      }
                    if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 152){
                      upstairs1(132, 91, 155, 16, 15, mario_climbs, 3, 3);
                      Mario_posY=91;
                      }
                    if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 102){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(91, 50, 102, 16, 15, mario_climbs, 4, 4);
                      Mario_posY=50;
                      }
                    if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 222){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(50, 12, 222, 16, 15, mario_climbs, 4, 4);
                      Mario_posY=12;
                      }

                      //Logica para que no se pase de las plataformas (que quede flotando)
                    if (Mario_posY == 215 && Mario_posX > 275){ 
                      Mario_posX = 265;
                      }
                    if (Mario_posY == 173 && Mario_posX < 27){ 
                      Mario_posX = 35;
                      }
                    if (Mario_posY == 132 && Mario_posX > 240){ 
                      Mario_posX = 235;
                      }
                    if (Mario_posY == 91 && Mario_posX < 50){ 
                      Mario_posX = 55;
                      }
                      
                      if (Mario_posY == 12){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
                        delay(500);
                        selector = 16;
                      }
  }
  //*************************************************************************************************************************************
  // NIVEL 1 | JUGABILIDAD 2 JUGADORES
  //*************************************************************************************************************************************
  else if (selector==12){
            //Lanzando el primer barril
                    timer_enemy1++;     //temporizador del barril
                    if (timer_enemy1 == 2 && activeBarrel1){    //si el temporizador llego a 2 y el barril esta activo
                      barrel1--;                                //el barril se desplaza una posicion a la izquierda
                      uint16_t anim_frame = (barrel1/7)%3;      //velocidad de animacion
                      LCD_Sprite(barrel1, 215, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                      V_line(barrel1+12, 215, 10, 0x0000);                              //se cubre su huella
                      timer_enemy1 = 0;                                                 // se resetea el tmr
                      }
                    if ((barrel1 <5 || Mario_posY != 215 || Luigi_posY != 215) && indicadorB1 == 0){         //Si el barrel llego a una posicion en x menor a 5 o Mario escalo
                      activeBarrel1 = 0;                        //se desactiva
                      indicadorB1 = 1;                          //se desactiva el indicador
                      FillRect(barrel1,215, 20, 15, 0x0);             //se tapa
                      barrel1 = 0;                              //se le da una posicion en 0
                      FillRect(0,215, 20, 15, 0x0);             //se tapa
                      }

                     //Lanzando el segundo barril
                     if (Mario_posY ==173 || Luigi_posY ==173){   //Si mario  o Luigi está  en la segunda plataforma
                      timer_enemy2++;                             //Aumenta el temporizador del enemigo2
                      if (timer_enemy2 == 2 && activeBarrel2){    //si el temporizador llego a 2 y el barril esta activo
                        barrel2++;                                //el barril se desplaza una unidad hacia la derecha
                        uint16_t anim_frame = (barrel2/7)%3;      //velocidad de animacion
                        LCD_Sprite(barrel2, 173, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel2-1, 173, 10, 0x0);          //se tapa
                        timer_enemy2 = 0;                         //se reinicia el tmr
                        }
                      }
                      if ((barrel2 > 307 || Mario_posY < 173 || Luigi_posY < 173) && indicadorB2 == 0){   //si el barril llego a una posicion mayor de 307 o mario escalo
                        activeBarrel2 = 0;                      //se desactiva el barril
                        indicadorB2 = 1;                        //se desactiva el indicador
                        FillRect(barrel2, 173, 20, 15, 0x0);    //se tapa
                        barrel2 = 307;                          //se le da una nueva posicion
                        FillRect(barrel2,173,20,15,0x0);        //se tapa
                        }
                    //Lanzando el tercer barril
                     if (Mario_posY ==132 || Luigi_posY ==132){//Si mario o Luigi está  en la tercera plataforma
                      timer_enemy3++;
                      if (timer_enemy3 == 2 && activeBarrel3){
                        barrel3--;
                        uint16_t anim_frame = (barrel3/7)%3;
                        LCD_Sprite(barrel3, 132, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel3+12, 132, 10, 0x0);
                        timer_enemy3 = 0;
                        }
                      }
                      if ((barrel3 < 10 || Mario_posY < 132 || Luigi_posY < 132) && indicadorB3 == 0){    //si el barril llego a una posicion menor a 10 o mario escalo
                        activeBarrel3 = 0;                      //se desactiva el barril
                        indicadorB3 = 1;                        //se desactiva el indicador
                        FillRect(barrel3, 132, 20, 15, 0x0);    //se tapa  
                        barrel2 = 10;                           //se le da una nueva posicion
                        FillRect(barrel3,132,20,15,0x0);        //se tapa
                        }
                     //Lanzando el cuarto barril
                     if (Mario_posY==91 || Luigi_posY==91){//Si mario está  en la cuarta plataforma
                      timer_enemy4++;
                      if (timer_enemy4 == 2 && activeBarrel4){
                        barrel4++;
                        uint16_t anim_frame = (barrel4/7)%3;
                        LCD_Sprite(barrel4, 91, 12, 10, barrel, 4, anim_frame, 1, 0);    //se renderiza el barril
                        V_line(barrel4-1, 91, 10, 0x0);
                        timer_enemy4 = 0;
                        }
                      }
                      if ((barrel4 > 307 || Mario_posY < 91 || Luigi_posY < 91) && indicadorB4 == 0){    //si el barril llego a una posicion mayor a 307 o mario escalo
                        activeBarrel4 = 0;                  //se desactiva el barril
                        indicadorB4 = 1;                    //se desactiva el indicador
                        FillRect(barrel4, 91,20,15,0x0);    //se tapa
                        barrel4 = 307;                      //se le da una nueva posicion
                        FillRect(barrel4,91,20,15,0x0);     //se tapa
                        }
                      
                    //Activando el martillo de mario
                    if ((Mario_posX == 280 && Mario_posY == 173) || (Luigi_posX == 280 && Luigi_posY == 173)){
                      m_hammer = 1;
                      l_hammer = 1;
                      }

                    //Quitando vidas a Mario
                    if (Mario_posX == barrel1 && Mario_posY == 215 && activeBarrel1==1){
                      activeBarrel1 = 0;                  //se desactiva el barril
                      FillRect(barrel1,215, 20, 15, 0x0);             //se tapa
                      if (m_hammer == 0){
                        String textM = "M HP: 2";
                        LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                        m_lives = m_lives-1;
                        }
                      }
                    else if (Mario_posX == barrel2 && Mario_posY == 173 && activeBarrel2==1){
                       activeBarrel2 = 0;                      //se desactiva el barril
                       FillRect(barrel2, 173, 20, 15, 0x0);    //se tapa
                       if (m_hammer == 0){
                        String textM = "M HP: 1";
                        LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                        m_lives = m_lives-1;
                        }
                      }
                    else if (Mario_posX == barrel3 && Mario_posY == 132 && activeBarrel3==1){
                      activeBarrel3 = 0;                      //se desactiva el barril
                      FillRect(barrel3, 132, 20, 15, 0x0);    //se tapa
                      if (m_hammer == 0)
                      {m_lives = m_lives-1;}
                      }
                     else if (Mario_posX == barrel4 && Mario_posY == 91 && activeBarrel4==1){
                      activeBarrel4 = 0;                  //se desactiva el barril
                      FillRect(barrel4, 91,20,15,0x0);    //se tapa
                      if (m_hammer == 0)
                      {m_lives = m_lives-1;}
                      }

                    //Quitando vidas a Luigi
                    if (Luigi_posX == barrel1 && Luigi_posY == 215 && activeBarrel1==1){
                      activeBarrel1 = 0;                  //se desactiva el barril
                      FillRect(barrel1,215, 20, 15, 0x0);             //se tapa
                      if (l_hammer == 0){
                        String textL = "L HP: 2";
                        LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                        l_lives = l_lives-1;
                        }
                      }
                    else if (Luigi_posX == barrel2 && Luigi_posY == 173 && activeBarrel2==1){
                       activeBarrel2 = 0;                      //se desactiva el barril
                       FillRect(barrel2, 173, 20, 15, 0x0);    //se tapa
                       if (l_hammer == 0){
                        String textL = "L HP: 1";
                        LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                        l_lives = l_lives-1;
                        }
                      }
                    else if (Luigi_posX == barrel3 && Luigi_posY == 132 && activeBarrel3==1){
                      activeBarrel3 = 0;                      //se desactiva el barril
                      FillRect(barrel3, 132, 20, 15, 0x0);    //se tapa
                      if (l_hammer == 0)
                      {l_lives = l_lives-1;}
                      }
                     else if (Luigi_posX == barrel4 && Luigi_posY == 91 && activeBarrel4==1){
                      activeBarrel4 = 0;                  //se desactiva el barril
                      FillRect(barrel4, 91,20,15,0x0);    //se tapa
                      if (l_hammer == 0)
                      {l_lives = l_lives-1;}
                      }

                    //reiniciando el juego
                    if (m_lives == 0 || l_lives == 0){
                      for (int x = Mario_posX; x < Mario_posX+3; x++){
                        delay(15);
                        LCD_Sprite(x, Mario_posY, 16, 15, mario_dies, 3, 3, 1, 0);
                        }
                      }
                    if (l_lives == 0 || m_lives ==0){
                      for (int x = Luigi_posX; x < Luigi_posX+3; x++){
                        delay(15);
                        LCD_Sprite(x, Luigi_posY, 16, 15, luigi_dies, 3, 3, 1, 0);
                        }
                      }
                     if (l_lives == 0 || m_lives == 0){
                      selector = 13;
                      m_lives = 3;
                      l_lives = 3;
                     }

                 //CONTROLES DE MARIO
                    if (btnState8==HIGH){
                        Mario_posX++;
                        if(Mario_posX!=Mario_posX2){
                          if(Mario_posX>320-16){
                              Mario_posX=(320-16);
                            }
                        if (m_hammer == 0){
                         uint16_t anim_speed = Mario_posX/7;
                         uint16_t anim_frame = anim_speed%3;
                         LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                         V_line(Mario_posX - 1, Mario_posY,16, 0x0000);
                        }else if (m_hammer == 1){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY-10, 31, 26, mario_hammer, 5, anim_frame, 1, 0);
                           V_line(Mario_posX - 1, Mario_posY-10,16, 0x0000);
                          }
                       }
                      }
                    if (btnState6==HIGH){
                        Mario_posX--;
                        if(Mario_posX!=Mario_posX2){
                          if(Mario_posX<16){
                              Mario_posX=16;
                            }
                        if (m_hammer == 0){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 0, 0);
                           V_line(Mario_posX + 15, Mario_posY, 16, 0x0000);
                        }else if (m_hammer == 1){
                           uint16_t anim_speed = Mario_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Mario_posX, Mario_posY-10, 31, 26, mario_hammer, 5, anim_frame, 0, 0);
                           V_line(Mario_posX+30, Mario_posY-10, 26, 0x0000);
                          }
                       }
                      }
                      
                    if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 159){
                      upstairs1(215, 174, 155, 16, 15, mario_climbs, 1, 1);
                      Mario_posY=173; 
                      }
                    if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 90 ){
                      upstairs1(173, 132, 90, 16, 15, mario_climbs, 2, 2);
                      Mario_posY=132;
                      }
                    if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 152){
                      upstairs1(132, 91, 155, 16, 15, mario_climbs, 3, 3);
                      Mario_posY=91;
                      }
                    if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 102){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(91, 50, 102, 16, 15, mario_climbs, 4, 4);
                      Mario_posY=50;
                      }
                    if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 222){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(50, 12, 222, 16, 15, mario_climbs, 4, 4);
                      Mario_posY=12;
                      }
                      //CONTROLES LUIGI
                    if (btnState5==HIGH){
                        Luigi_posX++;
                        if(Luigi_posX!=Luigi_posX2){
                          if(Luigi_posX>320-16){
                              Luigi_posX=(320-16);
                            }
                        if (l_hammer == 0){
                         uint16_t anim_speed = Luigi_posX/7;
                         uint16_t anim_frame = anim_speed%3;
                         LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 1, 0);
                         V_line(Luigi_posX - 1, Luigi_posY,16, 0x0000);
                        }else if (l_hammer == 1){
                           uint16_t anim_speed = Luigi_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Luigi_posX, Luigi_posY-10, 31, 26, luigi_hammer, 5, anim_frame, 1, 0);
                           V_line(Luigi_posX - 1, Luigi_posY-10,16, 0x0000);
                          }
                       }
                      }
                    if (btnState3==HIGH){
                        Luigi_posX--;
                        if(Luigi_posX!=Luigi_posX2){
                          if(Luigi_posX<16){
                              Luigi_posX=16;
                            }
                        if (l_hammer == 0){
                           uint16_t anim_speed = Luigi_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 0, 0);
                           V_line(Luigi_posX + 15, Luigi_posY, 16, 0x0000);
                        }else if (l_hammer == 1){
                           uint16_t anim_speed = Luigi_posX/7;
                           uint16_t anim_frame = anim_speed%3;
                           LCD_Sprite(Luigi_posX, Luigi_posY-10, 31, 26, luigi_hammer, 5, anim_frame, 0, 0);
                           V_line(Luigi_posX+30, Luigi_posY-10, 26, 0x0000);
                          }
                       }
                     }
                     if (btnState4==HIGH && Luigi_posY == 215 && Luigi_posX == 159){
                      upstairs1(215, 174, 155, 16, 15, luigi_climbs, 1, 1);
                      Luigi_posY=173; 
                      }
                    if (btnState4==HIGH && Luigi_posY == 173 && Luigi_posX == 90 ){
                      upstairs1(173, 132, 90, 16, 15, luigi_climbs, 2, 2);
                      Luigi_posY=132;
                      }
                    if (btnState4==HIGH && Luigi_posY == 132 && Luigi_posX == 152){
                      upstairs1(132, 91, 155, 16, 15, luigi_climbs, 3, 3);
                      Luigi_posY=91;
                      }
                    if (btnState4==HIGH && Luigi_posY == 91 && Luigi_posX == 102){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(91, 50, 102, 16, 15, luigi_climbs, 4, 4);
                      Luigi_posY=50;
                      }
                      if (btnState4==HIGH && Luigi_posY == 50 && Luigi_posX == 222){
                      //void upstairs(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
                      upstairs1(50, 12, 222, 16, 15, luigi_climbs, 4, 4);
                      Luigi_posY=12;
                      }
                      //Logica para que no se pase de las plataformas (que quede flotando)
                    if (Mario_posY == 215 && Mario_posX > 275){ 
                      Mario_posX = 265;
                      }
                    if (Mario_posY == 173 && Mario_posX < 27){ 
                      Mario_posX = 35;
                      }
                    if (Mario_posY == 132 && Mario_posX > 240){ 
                      Mario_posX = 235;
                      }
                    if (Mario_posY == 91 && Mario_posX < 50){ 
                      Mario_posX = 55;
                      }
                      //Logica para que no se pase de las plataformas (que quede flotando)
                    if (Luigi_posY == 215 && Luigi_posX > 275){ 
                      Luigi_posX = 265;
                      }
                    if (Luigi_posY == 173 && Luigi_posX < 27){ 
                      Luigi_posX = 35;
                      }
                    if (Luigi_posY == 132 && Luigi_posX > 240){ 
                      Luigi_posX = 235;
                      }
                    if (Luigi_posY == 91 && Luigi_posX < 50){ 
                      Luigi_posX = 55;
                      }

                      
                      if (Mario_posY == 12 || Luigi_posY == 12){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
                        delay(500);
                        selector = 16;
                      }

                          
  }
  //*************************************************************************************************************************************
  // GAME OVER - REINICIO DEL JUEGO
  //*************************************************************************************************************************************
  else if (selector==13){
            mapeo_SD("gameover.txt", 0, 240);
            delay(1500);
            Mario_posX=32;
            Mario_posY=215;
            Mario_posX2;
            Mario_posY2;
            m_hammer = 0;
            m_lives = 3;
            Luigi_posX=30;
            Luigi_posY=215;
            Luigi_posX2;
            Luigi_posY2;
            l_hammer = 0;
            l_lives = 3;
            //Barril 1
            timer_enemy1 = 0;
            barrel1 = 280;
            activeBarrel1 = 1;
            indicadorB1 = 0;
            //Barril 2
            timer_enemy2 = 0;
            barrel2 = 26;
            activeBarrel2 = 1;
            indicadorB2 = 0;
            //Barril 3
            timer_enemy3 = 0;
            barrel3 = 275;
            activeBarrel3 = 1;
            indicadorB3 = 0;
            //Barril 4
            timer_enemy4 = 0;
            barrel4 = 50;
            activeBarrel4 = 1;
            indicadorB4 = 0;
            bola_posX=160-16;
            bola_posX2=160-16;
            bola_posX3=20;
            bolaa_posX=160-16; //plat 2
            bolaa_posX2=160-16; //plat 2
            bolaa_posX3=160-16; //plat 3
            bolaa_posX4=160-16; //plat 3
            bolaa_posX5=20; //plat 4
            bolaLD=1;
            bolaLI=1;
            bolaLD2=1;
            bolaLI2=1;
            bolaLD3=1;
            bolaLI3=1;
            bolaLD4=1;
            bolaLI4=1;
            bolaLD5=1;
            bolaLI5=1;
            hpcont=3;
            hpcontM=3;
            hpcontL=3;
            selector = 1;
            arrow_PosY = 10;
            arrow_PosY2=10;
            
  }
  //*************************************************************************************************************************************
  // NIVEL 2 | JUGABILIDAD 1 JUGADOR
  //*************************************************************************************************************************************
  else if (selector == 14){
    
           //*************************MOVIMIENTO MARIO**********************
            if (btnState8==HIGH){ //Movimiento derecha
                Mario_posX++;
                if(Mario_posX!=Mario_posX2){
                     if(Mario_posX>320-16){
                           Mario_posX=(320-16);
                     }
                     uint16_t anim_speed = Mario_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                     V_line(Mario_posX - 1, Mario_posY,16, 0x0000);
                }
            }
            if (btnState6==HIGH){ //Movimiento izquierda
                Mario_posX--;
                if(Mario_posX!=Mario_posX2){
                     if(Mario_posX<16){
                            Mario_posX=16;
                     }
                     uint16_t anim_speed = Mario_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 0, 0);
                     V_line(Mario_posX + 15, Mario_posY, 16, 0x0000);
                 }
            }
          
         //***********************SUBIENDO ECALERAS*******************************
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 71){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 71, 16, 15, mario_climbs, 1, 1);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 127){ //Plataforma 2 | escalera 2
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 127, 16, 15, mario_climbs, 1, 2);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 35 ){ //Plataforma 3 | escalera 3
            upstairs2(173, 132, 35, 16, 15, mario_climbs, 2, 3);
            Mario_posY=132;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 71){ //Plataforma 4 | escalera 4
            upstairs2(132, 91, 71, 16, 15, mario_climbs, 3, 4);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 127){ //Plataforma 4 | escalera 5
            upstairs2(132, 91, 127, 16, 15, mario_climbs, 3, 5);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 63){  //Plataforma 5 | escalera 6
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 63, 16, 15, mario_climbs, 4, 6);
            Mario_posY=50;
          }
           if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 96){  //Plataforma 6 | escalera 7
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 96, 16, 15, mario_climbs, 5, 7);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 144){  //Plataforma 6 | escalera 8
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 144, 16, 15, mario_climbs, 5, 8);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 320-96+8){  //Plataforma 6 | escalera 15
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-96+8, 16, 15, mario_climbs, 5, 15);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 320-144){  //Plataforma 6 | escalera 16
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-144, 16, 15, mario_climbs, 5, 16);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 320-127){ //Plataforma 2 | escalera 9
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-127, 16, 15, mario_climbs, 1,9);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 320-71){ //Plataforma 2 | escalera 10
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-71, 16, 15, mario_climbs, 1, 10);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 320-35-8){ //Plataforma 3 | escalera 11
            upstairs2(173, 132, 320-35-8, 16, 15, mario_climbs, 2, 11);
            Mario_posY=132;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 320-71){ //Plataforma 4 | escalera 12
            upstairs2(132, 91, 320-71, 16, 15, mario_climbs, 3, 12);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 320-127){ //Plataforma 4 | escalera 13
            upstairs2(132, 91, 320-127, 16, 15, mario_climbs, 3, 13);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 320-63-4){  //Plataforma 5 | escalera 14
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 320-63-4, 16, 15, mario_climbs, 4, 14);
            Mario_posY=50;
          }

          //**********************************RESTRICCIONES*******************************************
          if (btnState8==HIGH && Mario_posY == 173 && (Mario_posX > 127 && Mario_posX < 129)){ //Restricción Barril 1 (de abajo - arriba)
            Mario_posX=127;
          }
          if (btnState6==HIGH && Mario_posY == 173 && (Mario_posX > 181 && Mario_posX < 183)){ //Restricción Barril 2 (de abajo - arriba)
            Mario_posX=183;
          }
          if (btnState8==HIGH && Mario_posY == 132 && (Mario_posX > 127+8 && Mario_posX < 129+8)){ //Restricción Barril 3 (de abajo - arriba)
            Mario_posX=127+8;
          }
          if (btnState6==HIGH && Mario_posY == 132 && (Mario_posX > 181-12 && Mario_posX < 183-12)){ //Restricción Barril 3 (de abajo - arriba)
            Mario_posX=183-12;
          }
          if (btnState6==HIGH && Mario_posY == 132 && Mario_posX < 24){ //Restricción llama 1 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=24;
          }
          if (btnState6==HIGH && Mario_posY == 91 && Mario_posX < 24){ //Restricción Barril 4 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=24;
          }
          if (btnState8==HIGH && Mario_posY == 132 && Mario_posX > 320-36){ //Restricción llama 2 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=320-36;
          }
          if (btnState8==HIGH && Mario_posY == 91 && Mario_posX > 320-36){ //Restricción Barril 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=320-36;
          }
          if (btnState6==HIGH && Mario_posY == 50 && Mario_posX < 55){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=55;
          }
          if (btnState8==HIGH && Mario_posY == 50 && Mario_posX > 270){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=270;
          }
          if (btnState6==HIGH && Mario_posY == 12 && Mario_posX < 104){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=104;
          }
          if (btnState8==HIGH && Mario_posY == 12 && Mario_posX > 223){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=223;
          }
          
          //PINTANDO ESCALERAS AL CORRER
          if (btnState8==HIGH && Mario_posY == 215 && Mario_posX == 79){ //escalera 1 >
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 71 , 1, 8, 8, ladder); //escalera 1    
          }
          if (btnState8==HIGH && Mario_posY == 215 && Mario_posX == 127+8){ //escalera 2 >
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 127, 1, 8, 8, ladder); //escalera 2  
          }
          if (btnState8==HIGH && Mario_posY == 215 && Mario_posX == 320-71+8){ //escalera 3 >
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 320-71 , 1, 8, 8, ladder); //escalera 9     
          }
          if (btnState8==HIGH && Mario_posY == 215 && Mario_posX == 320-127+8){ //escalera 4 >
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 320-127, 1, 8, 8, ladder); //escalera 10
          }
          if (btnState6==HIGH && Mario_posY == 215 && Mario_posX < 70 && Mario_posX > 68){ //escalera 1 <
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 71 , 1, 8, 8, ladder); //escalera 4
          }
          if (btnState6==HIGH && Mario_posY == 215 && Mario_posX == 126){ //escalera 2 <
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 127, 1, 8, 8, ladder); //escalera 2
          }
         /* if (btnState4==LOW && Mario_posY == 215 && Mario_posX == 127+8){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 127, 1, 8, 8, ladder); //escalera 2  
          }
          if (btnState4==LOW && Mario_posY == 215 && Mario_posX == 320-71+8){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 320-71 , 1, 8, 8, ladder); //escalera 9     
          }
          if (btnState4==LOW && Mario_posY == 215 && Mario_posX == 320-127+8){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            tile_maker(198, 232, 7, 320-127, 1, 8, 8, ladder); //escalera 10
          }*/
          //Movimiento Bolas de Fuego
          //Bola 1
          if (Mario_posY == 173 && Mario_posX < 160-16){
             if (Mario_posY != 173){
                bola_posX=160-16;
              }
               if (bolaLD==1){
                  bola_posX--;
                  int bolaAnim = (bola_posX/7)%4;
                  LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                  LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bola_posX + 16, 190-16, 16, 0x0000);
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX);
                  if (bola_posX < 24){
                      bolaLD=0;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              if (bolaLD==0){
                bola_posX++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX);
                int bolaAnim = (bola_posX/7)%4;
                LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                V_line( bola_posX -1, 190-16, 16,  0x0000);
                LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                if (bola_posX > 160-16){
                  bolaLD=1;
                }
             }
             // QUITANDO VIDA
              if (Mario_posX == bola_posX && hpcont>=3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX/7)%4;
                LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=2;
             }
         } 
         //Bola 2
         else if (Mario_posY == 173 && Mario_posX > 160-16){
           if (Mario_posY != 173){
                bola_posX=160-16;
              }
              if (bolaLI==1){
                bola_posX++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX);
                int bolaAnim = (bola_posX/7)%4;
                LCD_Bitmap(160+4, 190-31, 16, 31, oil); //Barril de fuego 5
                LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                V_line( bola_posX -1, 190-16, 16,  0x0000);
                if (bola_posX > 320-35-8){
                  bolaLI=0;
                }
             }
             if (bolaLI==0){
                  bola_posX--;
                  int bolaAnim = (bola_posX/7)%4;
                  LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bola_posX + 16, 190-16, 16, 0x0000);
                  LCD_Bitmap(160+4, 190-31, 16, 31, oil); //Barril de fuego 5
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX);
                  if (bola_posX < 160+16){
                      bolaLI=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA
             if (Mario_posX == bola_posX && hpcont>=3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX/7)%4;
                LCD_Sprite(bola_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=2;
             }
         }
         //Bola 3
          else if (Mario_posY == 132 && Mario_posX < 160-16){
               if (bolaLD==1){
                  bola_posX2--;
                  int bolaAnim = (bola_posX2/7)%4;
                  LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                  LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bola_posX2 + 16, 149-16, 16, 0x0000);
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX2);
                  if (bola_posX2 < 24){
                      bolaLD=0;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              if (bolaLD==0){
                bola_posX2++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX2);
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                V_line( bola_posX2 -1, 149-16, 16,  0x0000);
                LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                if (bola_posX2 > 160-16){
                  bolaLD=1;
                }
             }
             // QUITANDO VIDA
              if (Mario_posX == bola_posX2 && hpcont==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=1;
             } else if (Mario_posX == bola_posX2 && hpcont==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=2;
             }
         } 
         //Bola 4
         else if (Mario_posY == 132 && Mario_posX > 160-16){
              if (Mario_posY != 132){
                bola_posX2=160-16;
              }
              if (bolaLI==1){
                bola_posX2++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX2);
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                V_line( bola_posX2 -1, 149-16, 16,  0x0000);
                if (bola_posX2 > 320-35-8){
                  bolaLI=0;
                }
             }
             if (bolaLI==0){
                  bola_posX2--;
                  int bolaAnim = (bola_posX2/7)%4;
                  LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bola_posX2 + 16, 149-16, 16, 0x0000);
                  LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX2);
                  if (bola_posX2 < 160+16){
                      bolaLI=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA
              if (Mario_posX == bola_posX2 && hpcont==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=1;
             } else if (Mario_posX == bola_posX2 && hpcont==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX2/7)%4;
                LCD_Sprite(bola_posX2,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=2;
             }
         }
         //Bola 5
         else if (Mario_posY == 91){
              if (bolaLI==1){
                bola_posX3++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX3);
                int bolaAnim = (bola_posX3/7)%4;
                LCD_Sprite(bola_posX3,109-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(3, 109-31, 16, 31, oil); //Barril de fuego 1 (arriba para abajo)
                V_line( bola_posX3 -1, 109-16, 16,  0x0000);
                if (bola_posX3 > 320-35-8){
                  bolaLI=0;
                }
             }
             if (bolaLI==0){
                  bola_posX3--;
                  int bolaAnim = (bola_posX3/7)%4;
                  LCD_Sprite(bola_posX3,109-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bola_posX3 + 16, 109-16, 16, 0x0000);
                  LCD_Bitmap(3, 109-31, 16, 31, oil); //Barril de fuego 1 (arriba para abajo)
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX3);
                  if (bola_posX3 < 20){
                      bolaLI=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA
              if (Mario_posX == bola_posX3 && hpcont==1){
                String textM = "M HP: 0";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX3/7)%4;
                LCD_Sprite(bola_posX3,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=3;
             } else if (Mario_posX == bola_posX3 && hpcont==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX3/7)%4;
                LCD_Sprite(bola_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=1;
             } else if (Mario_posX == bola_posX3 && hpcont==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bola_posX3/7)%4;
                LCD_Sprite(bola_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcont=2;
             }
         }

         //Verificación de vidas
         if (m_lives == 0){
            for (int x = Mario_posX; x < Mario_posX+3; x++){
              delay(15);
              LCD_Sprite(x, Mario_posY, 16, 15, mario_dies, 3, 3, 1, 0);
            }
            delay(900);
            selector = 13;
            m_lives = 3;
         }

         if (Mario_posY == 12){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            delay(500);
            selector = 16;
          }
  
  }
  //*************************************************************************************************************************************
  // NIVEL 2 | JUGABILIDAD 2 JUGADORES
  //*************************************************************************************************************************************
  else if (selector == 15){

     //******************MOVIMIENTO MARIO**************************
            if (btnState8==HIGH){ //Movimiento derecha
                Mario_posX++;
                if(Mario_posX!=Mario_posX2){
                     if(Mario_posX>320-16){
                           Mario_posX=(320-16);
                     }
                     uint16_t anim_speed = Mario_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 1, 0);
                     V_line(Mario_posX - 1, Mario_posY,16, 0x0000);
                }
            }
            if (btnState6==HIGH){ //Movimiento izquierda
                Mario_posX--;
                if(Mario_posX!=Mario_posX2){
                     if(Mario_posX<16){
                            Mario_posX=16;
                     }
                     uint16_t anim_speed = Mario_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Mario_posX, Mario_posY, 16, 16, mario_runs, 3, anim_frame, 0, 0);
                     V_line(Mario_posX + 15, Mario_posY, 16, 0x0000);
                 }
            }

      //******************MOVIMIENTO LUIGI*******************
            if (btnState5==HIGH){ //Movimiento derecha
                Luigi_posX++;
                if(Luigi_posX!=Luigi_posX2){
                     if(Luigi_posX>320-16){
                           Luigi_posX=(320-16);
                     }
                     uint16_t anim_speed = Luigi_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 1, 0);
                     V_line(Luigi_posX - 1, Luigi_posY,16, 0x0000);
                }
            }
            if (btnState3==HIGH){ //Movimiento izquierda
                Luigi_posX--;
                if(Luigi_posX!=Luigi_posX2){
                     if(Luigi_posX<16){
                            Luigi_posX=16;
                     }
                     uint16_t anim_speed = Luigi_posX/7;
                     uint16_t anim_frame = anim_speed%3;
                     LCD_Sprite(Luigi_posX, Luigi_posY, 16, 16, luigi_runs, 3, anim_frame, 0, 0);
                     V_line(Luigi_posX + 15, Luigi_posY, 16, 0x0000);
                 }
            }

       //***********************SUBIENDO ECALERAS - MARIO****************************
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 71){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 71, 16, 15, mario_climbs, 1, 1);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 127){ //Plataforma 2 | escalera 2
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 127, 16, 15, mario_climbs, 1, 2);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 35 ){ //Plataforma 3 | escalera 3
            upstairs2(173, 132, 35, 16, 15, mario_climbs, 2, 3);
            Mario_posY=132;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 71){ //Plataforma 4 | escalera 4
            upstairs2(132, 91, 71, 16, 15, mario_climbs, 3, 4);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 127){ //Plataforma 4 | escalera 5
            upstairs2(132, 91, 127, 16, 15, mario_climbs, 3, 5);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 63){  //Plataforma 5 | escalera 6
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 63, 16, 15, mario_climbs, 4, 6);
            Mario_posY=50;
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 320-127){ //Plataforma 2 | escalera 9
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-127, 16, 15, mario_climbs, 1,9);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 215 && Mario_posX == 320-71){ //Plataforma 2 | escalera 10
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-71, 16, 15, mario_climbs, 1, 10);
            Mario_posY=173; 
          }
          if (btnState7==HIGH && Mario_posY == 173 && Mario_posX == 320-35-8){ //Plataforma 3 | escalera 11
            upstairs2(173, 132, 320-35-8, 16, 15, mario_climbs, 2, 11);
            Mario_posY=132;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 320-71){ //Plataforma 4 | escalera 12
            upstairs2(132, 91, 320-71, 16, 15, mario_climbs, 3, 12);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 132 && Mario_posX == 320-127){ //Plataforma 4 | escalera 13
            upstairs2(132, 91, 320-127, 16, 15, mario_climbs, 3, 13);
            Mario_posY=91;
          }
          if (btnState7==HIGH && Mario_posY == 91 && Mario_posX == 320-63-4){  //Plataforma 5 | escalera 14
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 320-63-4, 16, 15, mario_climbs, 4, 14);
            Mario_posY=50;
          }
           if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 96){  //Plataforma 6 | escalera 7
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 96, 16, 15, mario_climbs, 5, 7);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 144){  //Plataforma 6 | escalera 8
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 144, 16, 15, mario_climbs, 5, 8);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 320-96+8){  //Plataforma 6 | escalera 15
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-96+8, 16, 15, mario_climbs, 5, 15);
            Mario_posY=12;
          }
          if (btnState7==HIGH && Mario_posY == 50 && Mario_posX == 320-144){  //Plataforma 6 | escalera 16
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-144, 16, 15, mario_climbs, 5, 16);
            Mario_posY=12;
          }
          //*********************SUBIENDO ECALERAS - LUIGI*******************
          if (btnState4==HIGH && Luigi_posY == 215 && Luigi_posX == 71){ //Plataforma 2 | escalera 1
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 71, 16, 15, luigi_climbs, 1, 1);
            Luigi_posY=173; 
          }
          if (btnState4==HIGH && Luigi_posY == 215 && Luigi_posX == 127){ //Plataforma 2 | escalera 2
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 127, 16, 15, luigi_climbs, 1, 2);
            Luigi_posY=173; 
          }
          if (btnState4==HIGH && Luigi_posY == 173 && Luigi_posX == 35 ){ //Plataforma 3 | escalera 3
            upstairs2(173, 132, 35, 16, 15, luigi_climbs, 2, 3);
            Luigi_posY=132;
          }
          if (btnState4==HIGH && Luigi_posY == 132 && Luigi_posX == 71){ //Plataforma 4 | escalera 4
            upstairs2(132, 91, 71, 16, 15, luigi_climbs, 3, 4);
            Luigi_posY=91;
          }
          if (btnState4==HIGH && Luigi_posY == 132 && Luigi_posX == 127){ //Plataforma 4 | escalera 5
            upstairs2(132, 91, 127, 16, 15, luigi_climbs, 3, 5);
            Luigi_posY=91;
          }
          if (btnState4==HIGH && Luigi_posY == 91 && Luigi_posX == 63){  //Plataforma 5 | escalera 6
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 63, 16, 15, luigi_climbs, 4, 6);
            Luigi_posY=50;
          }
          if (btnState4==HIGH && Luigi_posY == 215 && Luigi_posX == 320-127){ //Plataforma 2 | escalera 9
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-127, 16, 15, luigi_climbs, 1,9);
            Luigi_posY=173; 
          }
          if (btnState4==HIGH && Luigi_posY == 215 && Luigi_posX == 320-71){ //Plataforma 2 | escalera 10
          //void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num)
            upstairs2(215, 174, 320-71, 16, 15, luigi_climbs, 1, 10);
            Luigi_posY=173; 
          }
          if (btnState4==HIGH && Luigi_posY == 173 && Luigi_posX == 320-35-8){ //Plataforma 3 | escalera 11
            upstairs2(173, 132, 320-35-8, 16, 15, luigi_climbs, 2, 11);
            Luigi_posY=132;
          }
          if (btnState4==HIGH && Luigi_posY == 132 && Luigi_posX == 320-71){ //Plataforma 4 | escalera 12
            upstairs2(132, 91, 320-71, 16, 15, luigi_climbs, 3, 12);
            Luigi_posY=91;
          }
          if (btnState4==HIGH && Luigi_posY == 132 && Luigi_posX == 320-127){ //Plataforma 4 | escalera 13
            upstairs2(132, 91, 320-127, 16, 15, luigi_climbs, 3, 13);
            Luigi_posY=91;
          }
          if (btnState4==HIGH && Luigi_posY == 91 && Luigi_posX == 320-63-4){  //Plataforma 5 | escalera 14
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(91, 50, 320-63-4, 16, 15, luigi_climbs, 4, 14);
            Luigi_posY=50;
          }
           if (btnState4==HIGH && Luigi_posY == 50 && Luigi_posX == 96){  //Plataforma 6 | escalera 7
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 96, 16, 15, luigi_climbs, 5, 7);
            Luigi_posY=12;
          }
          if (btnState4==HIGH && Luigi_posY == 50 && Luigi_posX == 144){  //Plataforma 6 | escalera 8
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 144, 16, 15, luigi_climbs, 5, 8);
            Luigi_posY=12;
          }
          if (btnState4==HIGH && Luigi_posY == 50 && Luigi_posX == 320-96+8){  //Plataforma 6 | escalera 15
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-96+8, 16, 15, luigi_climbs, 5, 15);
            Luigi_posY=12;
          }
          if (btnState4==HIGH && Luigi_posY == 50 && Luigi_posX == 320-144){  //Plataforma 6 | escalera 16
            //void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num);
            upstairs2(50, 12, 320-144, 16, 15, luigi_climbs, 5, 16);
            Luigi_posY=12;
          }

          //**********************************RESTRICCIONES MARIO*******************************************
          if (btnState8==HIGH && Mario_posY == 173 && (Mario_posX > 127 && Mario_posX < 129)){ //Restricción Barril 1 (de abajo - arriba)
            Mario_posX=127;
          }
          if (btnState6==HIGH && Mario_posY == 173 && (Mario_posX > 181 && Mario_posX < 183)){ //Restricción Barril 2 (de abajo - arriba)
            Mario_posX=183;
          }
          if (btnState8==HIGH && Mario_posY == 132 && (Mario_posX > 127+8 && Mario_posX < 129+8)){ //Restricción Barril 3 (de abajo - arriba)
            Mario_posX=127+8;
          }
          if (btnState6==HIGH && Mario_posY == 132 && (Mario_posX > 181-12 && Mario_posX < 183-12)){ //Restricción Barril 3 (de abajo - arriba)
            Mario_posX=183-12;
          }
          if (btnState6==HIGH && Mario_posY == 132 && Mario_posX < 24){ //Restricción llama 1 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=24;
          }
          if (btnState6==HIGH && Mario_posY == 91 && Mario_posX < 24){ //Restricción Barril 4 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=24;
          }
          if (btnState8==HIGH && Mario_posY == 132 && Mario_posX > 320-36){ //Restricción llama 2 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=320-36;
          }
          if (btnState8==HIGH && Mario_posY == 91 && Mario_posX > 320-36){ //Restricción Barril 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=320-36;
          }
          if (btnState6==HIGH && Mario_posY == 50 && Mario_posX < 55){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=55;
          }
          if (btnState8==HIGH && Mario_posY == 50 && Mario_posX > 270){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=270;
          }
           if (btnState6==HIGH && Mario_posY == 12 && Mario_posX < 104){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=104;
          }
          if (btnState8==HIGH && Mario_posY == 12 && Mario_posX > 223){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Mario_posX=223;
          }

          //**********************************RESTRICCIONES LUIGI*******************************************
          if (btnState5==HIGH && Luigi_posY == 173 && (Luigi_posX > 127 && Luigi_posX < 129)){ //Restricción Barril 1 (de abajo - arriba)
            Luigi_posX=127;
          }
          if (btnState3==HIGH && Luigi_posY == 173 && (Luigi_posX > 181 && Luigi_posX < 183)){ //Restricción Barril 2 (de abajo - arriba)
            Luigi_posX=183;
          }
          if (btnState5==HIGH && Luigi_posY == 132 && (Luigi_posX > 127+8 && Luigi_posX < 129+8)){ //Restricción Barril 3 (de abajo - arriba)
            Luigi_posX=127+8;
          }
          if (btnState3==HIGH && Luigi_posY == 132 && (Luigi_posX > 181-12 && Luigi_posX < 183-12)){ //Restricción Barril 3 (de abajo - arriba)
            Luigi_posX=183-12;
          }
          if (btnState3==HIGH && Luigi_posY == 132 && Luigi_posX < 24){ //Restricción llama 1 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=24;
          }
          if (btnState3==HIGH && Luigi_posY == 91 && Luigi_posX < 24){ //Restricción Barril 4 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=24;
          }
          if (btnState5==HIGH && Luigi_posY == 132 && Luigi_posX > 320-36){ //Restricción llama 2 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=320-36;
          }
          if (btnState5==HIGH && Luigi_posY == 91 && Luigi_posX > 320-36){ //Restricción Barril 5 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=320-36;
          }
          if (btnState3==HIGH && Luigi_posY == 50 && Luigi_posX < 55){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=55;
          }
          if (btnState5==HIGH && Luigi_posY == 50 && Luigi_posX > 270){ //Restricción plataforma 5 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=270;
          }
           if (btnState5==HIGH && Luigi_posY == 12 && Luigi_posX < 104){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=104;
          }
          if (btnState5==HIGH && Luigi_posY == 12 && Luigi_posX > 223){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            Luigi_posX=223;
          }

          //****************************Movimiento Bolas de Fuego*******************************
          //Bola 1
          if ((Mario_posY == 173 && Mario_posX < 160-16) || (Luigi_posY == 173 && Luigi_posX < 160-16)){
            // if (Mario_posY != 173 || Luigi_posY != 173){
              //  bola_posX=160-16;
             // }
               if (bolaLD==1){
                  bolaa_posX--;
                  int bolaAnim = (bolaa_posX/7)%4;
                  LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                  LCD_Sprite(bolaa_posX,190-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bolaa_posX + 16, 190-16, 16, 0x0000);
                  Serial.print("Posición bola: ");
                  Serial.println(bolaa_posX);
                  if (bolaa_posX < 24){
                      bolaLD=0;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              if (bolaLD==0){
                bolaa_posX++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX);
                int bolaAnim = (bolaa_posX/7)%4;
                LCD_Sprite(bolaa_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                V_line( bolaa_posX -1, 190-16, 16,  0x0000);
                LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                if (bolaa_posX > 160-16){
                  bolaLD=1;
                }
             }
             // QUITANDO VIDA - MARIO
              if (Mario_posX == bolaa_posX && hpcontM==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX/7)%4;
                LCD_Sprite(bolaa_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=2;
             }
             // QUITANDO VIDA - LUIGI
              if (Luigi_posX == bolaa_posX && hpcontL==3){
                String textL = "L HP: 2";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX/7)%4;
                LCD_Sprite(bolaa_posX,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=2;
             }
         } 
         //Bola 2
         else if ((Mario_posY == 173 && Mario_posX > 160-16) || (Luigi_posY == 173 && Luigi_posX > 160-16)){
              if (bolaLI2==1){
                bolaa_posX2++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX);
                int bolaAnim = (bolaa_posX2/7)%4;
                LCD_Bitmap(160+4, 190-31, 16, 31, oil); //Barril de fuego 5
                LCD_Sprite(bolaa_posX2,190-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
                V_line( bolaa_posX2 -1, 190-16, 16,  0x0000);
                if (bolaa_posX2 > 320-35-8){
                  bolaLI2=0;
                }
             }
             if (bolaLI2==0){
                  bolaa_posX2--;
                  int bolaAnim = (bolaa_posX2/7)%4;
                  LCD_Sprite(bolaa_posX2,190-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bolaa_posX2 + 16, 190-16, 16, 0x0000);
                  LCD_Bitmap(160+4, 190-31, 16, 31, oil); //Barril de fuego 5
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX);
                  if (bolaa_posX2 < 160+16){
                      bolaLI2=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA - MARIO
             if (Mario_posX == bolaa_posX2 && hpcontM==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX2/7)%4;
                LCD_Sprite(bolaa_posX2,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=2;
             }
             // QUITANDO VIDA - LUIGI
             if (Luigi_posX == bolaa_posX2 && hpcontL==3){
                String textL = "L HP: 2";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX2/7)%4;
                LCD_Sprite(bolaa_posX2,190-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=2;
             }
         }
         //Bola 3
          else if ((Mario_posY == 132 && Mario_posX < 160-16) || (Luigi_posY == 132 && Luigi_posX < 160-16)){
               if (bolaLD3==1){
                  bolaa_posX3--;
                  int bolaAnim = (bolaa_posX3/7)%4;
                  LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                  LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bolaa_posX3 + 16, 149-16, 16, 0x0000);
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX2);
                  if (bolaa_posX3 < 24){
                      bolaLD3=0;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              if (bolaLD3==0){
                bolaa_posX3++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX2);
                int bolaAnim = (bolaa_posX3/7)%4;
                LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                V_line( bolaa_posX3 -1, 149-16, 16,  0x0000);
                LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                if (bolaa_posX3 > 160-16){
                  bolaLD3=1;
                }
             }
             // QUITANDO VIDA - MARIO
              if (Mario_posX == bolaa_posX3 && hpcontM==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX3/7)%4;
                LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=1;
             } else if (Mario_posX == bolaa_posX3 && hpcontM==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX3/7)%4;
                LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=2;
             }
             // QUITANDO VIDA - LUIGI
              if (Luigi_posX == bolaa_posX3 && hpcontL==2){
                String textL = "L HP: 1";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX3/7)%4;
                LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=1;
             } else if (Luigi_posX == bolaa_posX3 && hpcontL==3){
                String textL = "L HP: 2";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX3/7)%4;
                LCD_Sprite(bolaa_posX3,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=2;
             }
         } 
         //Bola 4
         else if ((Mario_posY == 132 && Mario_posX > 160-16) || (Luigi_posY == 132 && Luigi_posX > 160-16)){
              if (bolaLI4==1){
                bolaa_posX4++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX2);
                int bolaAnim = (bolaa_posX4/7)%4;
                LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                V_line( bolaa_posX4 -1, 149-16, 16,  0x0000);
                if (bolaa_posX4 > 320-35-8){
                  bolaLI4=0;
                }
             }
             if (bolaLI4==0){
                  bolaa_posX4--;
                  int bolaAnim = (bolaa_posX4/7)%4;
                  LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bolaa_posX4 + 16, 149-16, 16, 0x0000);
                  LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
                  Serial.print("Posición bola: ");
                  Serial.println(bola_posX2);
                  if (bolaa_posX4 < 160+16){
                      bolaLI4=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA - MARIO
              if (Mario_posX == bolaa_posX4 && hpcontM==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX4/7)%4;
                LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=1;
             } else if (Mario_posX == bolaa_posX4 && hpcontM==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX4/7)%4;
                LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=2;
             }
              // QUITANDO VIDA - LUIGI
              if (Luigi_posX == bolaa_posX4 && hpcontL==2){
                String textL = "L HP: 1";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX4/7)%4;
                LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=1;
             } else if (Luigi_posX == bolaa_posX4 && hpcontL==3){
                String textL = "L HP: 2";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX4/7)%4;
                LCD_Sprite(bolaa_posX4,149-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=2;
             }
         }
         //Bola 5
         else if ((Mario_posY == 91) || (Luigi_posY == 91)){
              if (bolaLI5==1){
                bolaa_posX5++;
                Serial.print("Posición bola nueva: ");
                Serial.println(bola_posX3);
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                LCD_Bitmap(3, 109-31, 16, 31, oil); //Barril de fuego 1 (arriba para abajo)
                V_line( bolaa_posX5 -1, 109-16, 16,  0x0000);
                if (bolaa_posX5 > 320-35-8){
                  bolaLI5=0;
                }
             }
             if (bolaLI5==0){
                  bolaa_posX5--;
                  int bolaAnim = (bolaa_posX5/7)%4;
                  LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,0,0);
                  V_line( bolaa_posX5 + 16, 109-16, 16, 0x0000);
                  LCD_Bitmap(3, 109-31, 16, 31, oil); //Barril de fuego 1 (arriba para abajo)
                  Serial.print("Posición bola: ");
                  Serial.println(bolaa_posX5);
                  if (bolaa_posX5 < 20){
                      bolaLI5=1;
                      Serial.print("BolaLD: ");
                      Serial.println(bolaLD);
                  }
               }
              // QUITANDO VIDA - MARIO
              if (Mario_posX == bolaa_posX5 && hpcontM==1){
                String textM = "M HP: 0";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=3;
             } else if (Mario_posX == bolaa_posX5 && hpcontM==2){
                String textM = "M HP: 1";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=1;
             } else if (Mario_posX == bolaa_posX5 && hpcontM==3){
                String textM = "M HP: 2";
                LCD_Print(textM, 20, 0, 1, 0xffff, 0x0);
                m_lives = m_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontM=2;
             }
              // QUITANDO VIDA - LUIGI
              if (Luigi_posX == bolaa_posX5 && hpcontL==1){
                String textL = "L HP: 0";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=3;
             } else if (Luigi_posX == bolaa_posX5 && hpcontL==2){
                String textL = "L HP: 1";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=1;
             } else if (Luigi_posX == bolaa_posX5 && hpcontL==3){
                String textL = "L HP: 2";
                LCD_Print(textL, 260, 0, 1, 0xffff, 0x0);
                l_lives = l_lives-1;
                int bolaAnim = (bolaa_posX5/7)%4;
                LCD_Sprite(bolaa_posX5,109-16,16,16,bola,4,bolaAnim,1,0);
                hpcontL=2;
             }
         }

         //Verificación de vidas
         if ((m_lives == 0) || (l_lives == 0)){
            for (int x = Mario_posX; x < Mario_posX+3; x++){
              delay(15);
              LCD_Sprite(x, Mario_posY, 16, 15, mario_dies, 3, 3, 1, 0);
            }
            for (int x = Luigi_posX; x < Luigi_posX+3; x++){
              delay(15);
              LCD_Sprite(x, Luigi_posY, 16, 15, luigi_dies, 3, 3, 1, 0);
            }
            delay(900);
            selector = 13;
            m_lives = 3;
            l_lives = 3;
         }

         //GANADORES
          if (Mario_posY == 12 || Luigi_posY == 12){ //Restricción plataforma 6 (de abajo - arriba, de izquierda - derecha)
            delay(500);
            selector = 16;
          }

    
    

    
  }
  //*************************************************************************************************************************************
  // WINNER - REINICIO DEL JUEGO
  //*************************************************************************************************************************************
  else if (selector==16){
            mapeo_SD("victory.txt", 0, 240);
            delay(1500);
            Mario_posX=32;
            Mario_posY=215;
            Mario_posX2;
            Mario_posY2;
            m_hammer = 0;
            m_lives = 3;
            Luigi_posX=30;
            Luigi_posY=215;
            Luigi_posX2;
            Luigi_posY2;
            l_hammer = 0;
            l_lives = 3;
            //Barril 1
            timer_enemy1 = 0;
            barrel1 = 280;
            activeBarrel1 = 1;
            indicadorB1 = 0;
            //Barril 2
            timer_enemy2 = 0;
            barrel2 = 26;
            activeBarrel2 = 1;
            indicadorB2 = 0;
            //Barril 3
            timer_enemy3 = 0;
            barrel3 = 275;
            activeBarrel3 = 1;
            indicadorB3 = 0;
            //Barril 4
            timer_enemy4 = 0;
            barrel4 = 50;
            activeBarrel4 = 1;
            indicadorB4 = 0;
            bola_posX=160-16;
            bola_posX2=160-16;
            bola_posX3=20;
            bolaa_posX=160-16; //plat 2
            bolaa_posX2=160-16; //plat 2
            bolaa_posX3=160-16; //plat 3
            bolaa_posX4=160-16; //plat 3
            bolaa_posX5=20; //plat 4
            bolaLD=1;
            bolaLI=1;
            bolaLD2=1;
            bolaLI2=1;
            bolaLD3=1;
            bolaLI3=1;
            bolaLD4=1;
            bolaLI4=1;
            bolaLD5=1;
            bolaLI5=1;
            hpcont=3;
            hpcontM=3;
            hpcontL=3;
            selector = 1;
            arrow_PosY = 10;
            arrow_PosY2=10;
            
  }


}
     
  
  /*for(int x = 0; x <320-32; x++){
    delay(15);
    int anim2 = (x/35)%2;
    
    LCD_Sprite(x,100,16,24,planta,2,anim2,0,1);
    V_line( x -1, 100, 24, 0x421b);
    
    //LCD_Bitmap(x, 100, 32, 32, prueba);
    
    int anim = (x/11)%8;
    

    int anim3 = (x/11)%4;
    
    LCD_Sprite(x, 20, 16, 32, mario,8, anim,1, 0);
    V_line( x -1, 20, 32, 0x421b);
 
    //LCD_Sprite(x,100,32,32,bowser,4,anim3,0,1);
    //V_line( x -1, 100, 32, 0x421b);
 
 
    LCD_Sprite(x, 140, 16, 16, enemy,2, anim2,1, 0);
    V_line( x -1, 140, 16, 0x421b);
  
    LCD_Sprite(x, 175, 16, 32, luigi,8, anim,1, 0);
    V_line( x -1, 175, 32, 0x421b);
  }
  
  for(int x = 320-32; x >0; x--){
    delay(5);
    int anim = (x/11)%8;
    int anim2 = (x/11)%2;
    
    LCD_Sprite(x,100,16,24,planta,2,anim2,0,0);
    V_line( x + 16, 100, 24, 0x421b);
    
    //LCD_Bitmap(x, 100, 32, 32, prueba);
    
    LCD_Sprite(x, 140, 16, 16, enemy,2, anim2,0, 0);
    V_line( x + 16, 140, 16, 0x421b);
    
    LCD_Sprite(x, 175, 16, 32, luigi,8, anim,0, 0);
    V_line( x + 16, 175, 32, 0x421b);

    LCD_Sprite(x, 20, 16, 32, mario,8, anim,0, 0);
    V_line( x + 16, 20, 32, 0x421b);
  } */




//***************************************************************************************************************************************
//***************************************************************************************************************************************
//**************************************************** CÓDIGO DE FUNCIONES **************************************************************
//***************************************************************************************************************************************
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_DC, HIGH);
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
  digitalWrite(LCD_DC, LOW);
  SPI.transfer(cmd);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_DC, HIGH);
  SPI.transfer(data);
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
  digitalWrite(LCD_DC, HIGH);
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
  digitalWrite(LCD_DC, HIGH);
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
  digitalWrite(LCD_DC, HIGH);
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
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
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
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
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
  digitalWrite(LCD_DC, HIGH);
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
  digitalWrite(LCD_DC, HIGH);
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
// Función Para Mapear los valores Hex del Bitmap a Decimales
//***************************************************************************************************************************************
int ascii2hex(int a ){
  switch (a){
    case (48):    //caso 0
      return 0;
    case (49):    //caso 1
      return 1;
    case (50):    //caso 2
      return 2;
    case (51):    //caso 3
      return 3;
    case (52):    //caso 4
      return 4;
    case (53):    //caso 5
      return 5;
    case (54):    //caso 6
      return 6;
    case (55):    //caso 7
      return 7;
    case (56):    //caso 8
      return 8;
    case (57):    //caso 9
      return 9;
    case (97):    //caso A
      return 10;
    case (98):    //caso B
      return 11;
    case (99):    //caso C
      return 12;
    case (100):   //caso D
      return 13;
    case (101):   //caso E
      return 14;
    case (102):   //caso F
      return 15;
    }
}
//***************************************************************************************************************************************
// Función para mostrar las imagenes desde SD
//***************************************************************************************************************************************
void mapeo_SD(char doc[], int y, int h) {
  myFile = SD.open(doc, FILE_READ);   //se toma el archivo de la imagen
  int hex1 = 0;                       //declaracion de variable 1 para valor hex
  int val1 = 0;
  int val2 = 0;
  int mapear = 0;
  int vertical = 0;
  unsigned char maps[640];            //se crea arreglo vacio para almacenar el mapeo

  if (myFile) {
    while (myFile.available() ){      //se leen los datos mientras este disponible
      mapear = 0;
      while (mapear < 640){           //se limita el rango
        hex1 = myFile.read();         //se lee el archivo con la imagen
        if (hex1 == 120){
          val1 = myFile.read();       //se lee el primer valor hexadecimal del bitmap
          val2 = myFile.read();       //se lee el segundo valor hexadecimal del bitmap
          val1 = ascii2hex(val1);     //se mapea el primer valor hexadecimal
          val2 = ascii2hex(val2);     //se mapea el segundo valor hexadecimal
          maps[mapear] = val1*16+val2;  //se coloca en el arreglo nuevo
          mapear++;
        }
      }
      LCD_Bitmap(0, vertical, 320, 1, maps);
      vertical++; 
      if (vertical == y+h){
        return;
        } 
    }
    myFile.close();
  }
  else{
    Serial.println("No se pudo abrir la imagen, prueba nuevamente");
    myFile.close();
    }
}
//***************************************************************************************************************************************
// Función para crear tiles
//***************************************************************************************************************************************
void tile_maker(int t_ini, int t_fin, int t_paso, int posx, int posy, int width, int height, unsigned char bitmap[]){
  if (posx == 1){
    for(int x=t_ini; x<t_fin;x++){
      LCD_Bitmap(x, posy, width, height, bitmap);
      x += t_paso;
    }
   }
  if (posy == 1){
    for(int x=t_ini; x<t_fin;x++){
      LCD_Bitmap(posx, x, width, height, bitmap);
      x += t_paso;
    }
   }
 }
//***************************************************************************************************************************************
// Función para subir escaleras 1 (animacion)
//***************************************************************************************************************************************
void upstairs1(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num){
  for(int y = y_ini; y > y_fin; y--){
      delay(20);
      int anim = (y/17)%3;
      LCD_Sprite(posx, y, width, height, sprite, 3, anim, 0, 0);
      H_line(posx, y+16, 20, 0x0);
   }
   if (plat_num == 1) {
      tile_maker(16, 304, 15, 1, 192-2, 16, 8, platform); // Plataforma 2
    } else if (plat_num == 2) {
      tile_maker(32, 288, 15, 1, 152-3, 16, 8, platform); // Plataforma 3
    } else if (plat_num == 3) {
      tile_maker(48, 272, 15, 1, 112-3, 16, 8, platform); // Plataforma 4
    } else if (plat_num == 4) {
      tile_maker(64, 256, 15, 1, 72-3, 16, 8, platform); // Plataforma 5
    } else if (plat_num == 5) {
      tile_maker(80, 240, 15, 1, 32-3, 16, 8, platform); // Plataforma 6
    }
    
    if (lad_num == 1) {
     tile_maker(200, 232, 7, 155, 1, 8, 8, ladder);  // Escalera 1
  } else if (lad_num == 2) {
     tile_maker(158, 188, 7, 94, 1, 8, 8, ladder);   // Escalera 2
  } else if (lad_num == 3) {
     tile_maker(118, 146, 7, 155, 1, 8, 8, ladder);  // Escalera 3
  } else if (lad_num == 4) {
     tile_maker(80, 112, 7, 102, 1, 8, 8, ladder);   // Escalera 4
  } else if (lad_num == 5) {
     tile_maker(40, 72, 7, 222, 1, 8, 8, ladder);    // Escalera 5
  }
}
//***************************************************************************************************************************************
// Función para subir escaleras 2 (animacion)
//***************************************************************************************************************************************
void upstairs2(int y_ini, int y_fin, int posx, int width, int height, unsigned char sprite[], int plat_num, int lad_num){
  for(int y = y_ini; y > y_fin; y--){
      delay(20);
      int anim = (y/17)%3;
      LCD_Sprite(posx, y, width, height, sprite, 3, anim, 0, 0);
      H_line(posx, y+16, 20, 0x0);
   }
   //ESCALERAS 
   if (lad_num == 1) {
     tile_maker(198, 232, 7, 71 , 1, 8, 8, ladder); //escalera 1
   }
   else if (lad_num == 2) {
     tile_maker(198, 232, 7, 127, 1, 8, 8, ladder); //escalera 2
   }
   else if (lad_num == 3) {
     tile_maker(157, 190, 7, 35 , 1, 8, 8, ladder); //escalera 3
   } 
   else if (lad_num == 4) {
     tile_maker(117, 149, 7, 71 , 1, 8, 8, ladder); //escalera 4
   }
   else if (lad_num == 5) {
     tile_maker(117, 149, 7, 127, 1, 8, 8, ladder); //escalera 5
  }
  else if (lad_num == 6) {
     tile_maker(77 , 109, 7, 63 , 1, 8, 8, ladder); //escalera 6
   }
   else if (lad_num == 7) {
     tile_maker(29 , 69 , 7, 96 , 1, 8, 8, ladder); //escalera 7
   }
   else if (lad_num == 8) {
     tile_maker(37 , 71 , 7, 144, 1, 8, 8, ladder); //escalera 8
   } 
   else if (lad_num == 9) {
    tile_maker(198, 232, 7, 320-127 , 1, 8, 8, ladder); //escalera 9            
   }
   else if (lad_num == 10) {
     tile_maker(198, 232, 7, 320-71, 1, 8, 8, ladder); //escalera 10
  }
  else if (lad_num == 11) {
     tile_maker(157, 190, 7, 320-35-8 , 1, 8, 8, ladder); //escalera 11
   }
   else if (lad_num == 12) {
     tile_maker(117, 149, 7, 320-71 , 1, 8, 8, ladder); //escalera 12
   }
   else if (lad_num == 13) {
     tile_maker(117, 149, 7, 320-127, 1, 8, 8, ladder); //escalera 13
   } 
   else if (lad_num == 14) {
     tile_maker(77 , 109, 7, 320-63-4 , 1, 8, 8, ladder); //escalera 14           
   }
   else if (lad_num == 15) {
     tile_maker(29 , 69 , 7, 320-96+8, 1, 8, 8, ladder); //escalera 15
  }
     else if (lad_num == 16) {
     tile_maker(37 , 71 , 7, 320-144, 1, 8, 8, ladder); //escalera 16
  }
   //PLATAFORMAS
   if (plat_num == 1) {
      tile_maker(0, 319, 15, 1, 232, 16, 8, platform2);  //Plataforma 1 (de abajo en la pantalla para arriba)
      tile_maker(0, 319, 15, 1, 192-2, 16, 8, platform2); //Plataforma 2
   }
   else if (plat_num == 2) {
      tile_maker(0, 319, 15, 1, 152-3, 16, 8, platform3); //Plataforma 3
      tile_maker(0, 319, 15, 1, 192-2, 16, 8, platform2); //Plataforma 2
   }
   else if (plat_num == 3) {
      tile_maker(0, 319, 15, 1, 112-3, 16, 8, platform2); //Plataforma 4
   } 
   else if (plat_num == 4) {
      tile_maker(55, 270, 15, 1, 72-3, 16, 8, platform2);  //Plataforma 5
   } 
   else if (plat_num == 5) {
      tile_maker(104, 223, 15, 1, 32-3, 16, 8, platform3);  //Plataforma 6
      tile_maker(55, 270, 15, 1, 72-3, 16, 8, platform2);  //Plataforma 5
   }
   
   
}
//***************************************************************************************************************************************
// NIVEL 1
//***************************************************************************************************************************************
void nivel1(){
  //Creando el fondo previo a colocar a los jugadores
                FillRect(0, 0, 320, 240, 0x0000);   //Fondo de color negro
                //Colocando los tubos
                LCD_Bitmap(0, 80, 49, 32, tube);    
                LCD_Bitmap(0, 160, 24, 31, tube1);
                LCD_Bitmap(297, 200 , 24, 31, tube2);
                LCD_Bitmap(285, 120 , 35, 32, tube3);
                LCD_Bitmap(280, 170, 9, 10, hammer);
                //colocando las plataformas
                tile_maker(0, 319, 15, 1, 232, 16, 8, platform);  //Plataforma 1 (de abajo en la pantalla para arriba)
                tile_maker(16, 304, 15, 1, 192-2, 16, 8, platform); //Plataforma 2
                tile_maker(32, 288, 15, 1, 152-3, 16, 8, platform); //Plataforma 3
                tile_maker(48, 272, 15, 1, 112-3, 16, 8, platform); //Plataforma 4
                tile_maker(64, 256, 15, 1, 72-3, 16, 8, platform);  //Plataforma 5
                tile_maker(80, 240, 15, 1, 32-3, 16, 8, platform);  //Plataforma 6
                //colocando las escaleras
                tile_maker(200, 232, 7, 155, 1, 8, 8, ladder);  //escalera 1            
                tile_maker(158, 188, 7, 94, 1, 8, 8, ladder);   //escalera 2
                tile_maker(118, 146, 7, 155, 1, 8, 8, ladder);  //escalera 3
                tile_maker(80, 112, 7, 102, 1, 8, 8, ladder);   //escalera 4
                tile_maker(40, 72, 7, 222, 1, 8, 8, ladder);    //escalera 5
                LCD_Bitmap(160-8, 2, 17, 27, peach); //Peach
                LCD_Bitmap(80, 0, 39, 27, donkey); //Donkey
}
//***************************************************************************************************************************************
// NIVEL 2
//***************************************************************************************************************************************
void nivel2(){
          //Creando el fondo previo a colocar a los jugadores
        FillRect(0, 0, 320, 240, 0x0000);   //Fondo de color negro
        //colocando las escaleras
        tile_maker(198, 232, 7, 71 , 1, 8, 8, ladder); //escalera 1            
        tile_maker(198, 232, 7, 127, 1, 8, 8, ladder); //escalera 2
        tile_maker(157, 190, 7, 35 , 1, 8, 8, ladder); //escalera 3
        tile_maker(117, 149, 7, 71 , 1, 8, 8, ladder); //escalera 4
        tile_maker(117, 149, 7, 127, 1, 8, 8, ladder); //escalera 5
        tile_maker(77 , 109, 7, 63 , 1, 8, 8, ladder); //escalera 6            
        tile_maker(29 , 69 , 7, 96 , 1, 8, 8, ladder); //escalera 7
        tile_maker(37 , 71 , 7, 144, 1, 8, 8, ladder); //escalera 8
        tile_maker(198, 232, 7, 320-71 , 1, 8, 8, ladder); //escalera 9            
        tile_maker(198, 232, 7, 320-127, 1, 8, 8, ladder); //escalera 10
        tile_maker(157, 190, 7, 320-35-8 , 1, 8, 8, ladder); //escalera 11
        tile_maker(117, 149, 7, 320-71 , 1, 8, 8, ladder); //escalera 12
        tile_maker(117, 149, 7, 320-127, 1, 8, 8, ladder); //escalera 13
        tile_maker(77 , 109, 7, 320-63-4 , 1, 8, 8, ladder); //escalera 14           
        tile_maker(29 , 69 , 7, 320-96+8, 1, 8, 8, ladder); //escalera 15
        tile_maker(37 , 71 , 7, 320-144, 1, 8, 8, ladder); //escalera 16
        //colocando las plataformas
        tile_maker(0, 319, 15, 1, 232, 16, 8, platform2);  //Plataforma 1 (de abajo en la pantalla para arriba)
        tile_maker(0, 319, 15, 1, 192-2, 16, 8, platform2); //Plataforma 2
        tile_maker(0, 319, 15, 1, 152-3, 16, 8, platform3); //Plataforma 3
        tile_maker(0, 319, 15, 1, 112-3, 16, 8, platform2); //Plataforma 4
        tile_maker(55, 270, 15, 1, 72-3, 16, 8, platform2);  //Plataforma 5
        tile_maker(104, 223, 15, 1, 32-3, 16, 8, platform3);  //Plataforma 6
        LCD_Bitmap(3, 109-31, 16, 31, oil); //Barril de fuego 1 (arriba para abajo)
        LCD_Bitmap(317-16, 109-31, 16, 31, oil); //Barril de fuego 2
        LCD_Bitmap(160-8, 149-31, 16, 31, oil); //Barril de fuego 3
        LCD_Bitmap(160-16, 190-31, 16, 31, oil); //Barril de fuego 4
        LCD_Bitmap(160+4, 190-31, 16, 31, oil); //Barril de fuego 5
        LCD_Bitmap(4, 149-16, 16, 16, llama); //Llama 1
        LCD_Bitmap(320-20, 149-16, 16, 16, llama); //Llama 2
        LCD_Bitmap(160-8, 2, 17, 27, peach); //Peach
        LCD_Bitmap(104, 0, 39, 27, donkey); //Donkey
}
