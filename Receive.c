#include <16F877A.h>
#device ADC=16
#DEVICE *=16 PASS_STRINGS=IN_RAM      //Admite ponteiros para constantes 

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(crystal=20000000)

#define LCD_ENABLE_PIN PIN_E2
#define LCD_RS_PIN PIN_E0
#define LCD_RW_PIN PIN_E1
#define LCD_DATA4 PIN_D4
#define LCD_DATA5 PIN_D5
#define LCD_DATA6 PIN_D6
#define LCD_DATA7 PIN_D7

#USE SPI(FORCE_SW , DI=PIN_C4, DO=PIN_C5, CLK=PIN_C3, MASTER, BAUD=10000000, MODE=0, BITS=8, MSB_FIRST, STREAM=STREAM_SPI)  



#include <lcd.c> 

#define RF24_SPI_DISABLE_WARNING             //for disabling nRF24 SPI warning message. 
#define  SPI_MISO       PIN_C4   //SPI(Usar por hardware quando possivel) 
#define  SPI_MOSI       PIN_C5   //SPI(Usar por hardware quando possivel)                      
#define  SPI_CLK        PIN_C3   //SPI(Usar por hardware quando possivel) 
//Driver nRF24L01P.C            
#define  RF24_IRQ       PIN_B0   //interrupcao nRF24L01+ 
#define  RF24_CS        PIN_B1   //chipselect nRF24L01+ 
#define  RF24_CE        PIN_B5   //chipEnable nRF24L01+ 
#define  RF24_PERFORMANCE_MODE   //performance mode ON                                  
#define  RF24_SPI       STREAM_SPI//Redirects SPI2 port to RS24_SPI stream 
#include <nRF24L01P.C>           //Driver nRF24L01+   Single Chip 2.4GHz Transceiver Driver 

void main(){
   lcd_init(); 
   delay_ms(500);
   printf(LCD_PUTC, "\f MuaLinhKien.Vn");  
   delay_ms(1000);
   int RXbuffer1[20]; 
   int RXdatasize, RXpipe;  
   
   RF24_initPorts(); 
   RF24_default_config(); 
     if(RF24_RX_SET()==1){
         lcd_gotoxy(1,1);
         lcd_putc("\f   RX set OK!");
     }
     else {
         lcd_gotoxy(1,1);
         lcd_putc("\f  RX set error!");
     }
     Delay_ms(1000);
     printf(lcd_putc, "\f Waiting receve\n      data");  
    
   while(TRUE){     
  
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer1)!=true ); 
         //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
       
      printf(lcd_putc, "\f%s",RXbuffer1);  
      delay_ms(500);
   }

}
