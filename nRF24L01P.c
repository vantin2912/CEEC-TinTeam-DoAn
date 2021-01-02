/* 
                              nRF24L01+ 
                  Single Chip 2.4GHz Transceiver Driver 

Arquivo     :  nRF24L01P.C 
Programador :  Eduardo Guilherme Brandt                    
Criação     :  17/11/2011 ( V 1.0 ) 
Modificado  :  02/02/2012 
Contato     :  Eduardogbrandt@yahoo.com.br 

Histórico   :  Versão 1.0 - 17/11/2011 
                  Revision 1.1 - 29/01/2013                              


                
Functions   : 
   See nRF24_commandList.c    //Do not include this file. It´s just for reference. 
      

Further information: 
   Ps:   *Define SPI pins(RF24_CS, SPI_MISO, SPI_MOSI, SPI_CLK) 
         and control pins(RF24_CE, RF24_IRQ) for nRF24L01+ 
         *Define SPI hardware controller stream RF24_SPI, too. 
         *SPI must be configured for more than 2Mbps. 
         *Software SPI can be too slow for. It can lead to a 
         poor system performance if slower than 4Mbps** 

   REQUIRES: 
            CCS PICC 4.124+, 
            uC with SPI 
            #device PASS_STRINGS=IN_RAM  directive 
            spi_xfer(RF24_SPI, data, bits)      //It has bugs in V4.124(not return read value). Some tricks was made. 
            
   Command-set:        
            see nRF24L01.h header file 
            ps: For more details, see nRF24L01+ datasheet. 

   Config example: See in RF24_default_config() function 


   Revision 1.1:  The line below is a way of putting things work with hardware SPI in some CCS C compiller versions with some uCs. For example, with PIC16F876A hardware SPI it was not necessary. With software SPI it is never necessary. **You must comment(disable) every line with the code spi_read2() as the line below** in case the driver not work. 

Code: 
   //rv= spi_read2();             //It´s necessary due to spi_xfer read bug






Eduardo Guilherme Brandt  17/11/2011   */ 
////////////////////////////////////////////////////////////////////////////////////////// 
// 
// 
#ifndef RF24_CS 
   #ERROR pino RF24_CS nao definido 
#endif 
#ifndef RF24_CE 
   #ERROR pino RF24_CE nao definido 
#endif 
#ifndef RF24_IRQ 
   #ERROR pino RF24_IRQ nao definido 
#endif 
#ifndef SPI_MISO        
   #ERROR pino SPI_MISO nao definido 
#endif 
#ifndef SPI_MOSI        
   #ERROR pino SPI_MOSI nao definido 
#endif 
#ifndef SPI_CLK        
   #ERROR pino SPI_CLK nao definido 
#endif 
#ifdef RF24_USE_DMA        //nRF24 uses SPI DMA for multiple byte transfers 
   #warning Please, check if "spi_dma.c" is included 
#endif 
/*#ifdef RF24_PERFORMANCE_MODE       //performance mode ON 
   #warning RF24 in performance mode. Not all call functions parameters checks are make. 
#endif*/ 
#ifndef RF24_SPI_DISABLE_WARNING 
   #warning Initialize RF24_SPI stream for SPI communication with nRF24L01, as below: 
   #warning #USE SPI(SPI1, MASTER, BAUD=1000000, MODE=0, BITS=8, MSB_FIRST, STREAM=RF24_SPI) //this will set SPI in 4MHz(maximum for 16MHz Xtal. #Use SPI is a little buggy) 
   #warning At V4.124 compiler version, SPI must be SPI2, due to CCS compiler bug 
   #warning #define RF24_SPI_DISABLE_WARNING  for disabling nRF24 SPI warning message. 
#endif 
#ifndef RF24_PWUPDELAY  
   #define  RF24_PWUPDELAY()  delay_ms(2)       //Delay power on nRF24L01+ 
#endif 
#ifndef RF24_CSDELAY    
   #define  RF24_CSDELAY()    delay_us(10)      //em us 
#endif 
// 
// 
// 
#include <nRF24L01P.h>     //HEADER FILE 
// 
// 
// 
/***************************************************************************************** 
****************************************************************************************** 
*************************SPI handshake and I/O commands*********************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
*************************SPI handshake and I/O commands*********************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
*************************SPI handshake and I/O commands*********************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
* DEFINED MACROS 
* ex:    
*     RF24_select();    //Controls bit Chipselect 
*/ 
#define RF24_xfer(xdata)   spi_xfer(RF24_SPI, xdata)  //Send/receive data through SPI(controls RF24_CS manualy)___DON´T USE RF24_xfer() ALIAS FOR MORE THAN 1 xdata PARAMETER 
#define RF24_select()      output_low(RF24_CS)        //Controls bit Chipselect 
#define RF24_unselect()    output_high(RF24_CS)       //Controls bit Chipselect 
#define RF24_enable()      output_high(RF24_CE)       //Controls bit Chipenable 
#define RF24_disable()     output_low(RF24_CE)        //Controls bit Chipenable 
#define RF24_IRQ_state()   !input(RF24_IRQ)           //IsOn when receive or transmit Data IRQ asserted 

unsigned char RF24_xfer_code(unsigned char val) 
{
unsigned int i;                           
FOR (i = 8; i > 0; i--)
{                          
output_bit(SPI_CLK,0); //CLK=0;   
delay_us(2);  
output_bit(SPI_MOSI,(val&0x80));
 
val<<=1;
output_bit(SPI_CLK,1); //CLK=1;                  
delay_us(2);                           

if(input(SPI_MISO))//(DATaO)                   
val|=1; // read                              
}
output_bit(SPI_CLK,0); //CLK=0;                                                                 
delay_us(2); 
return val;                         
}

/***************************************************************************************** 
****************************************************************************************** 
* 
* RF24_comm_in : Send command and receive input(variable lenght, or fixed only 1 input byte) 
* RF24_comm_out: Send command and send output(variable lenght) 
* 
* Dependencies: 
*     #device PASS_STRINGS=IN_RAM 
* 
* Parameters: 
* 
* comm:     command to send(before sending or receiving data) 
* datain:   pointer to data array for receiving 
* dataout:  pointer to data array for sending 
* dataSZ:   data size(number of bytes for receiving into "datain" pointer) 
* 
* ex:    
*     data=RF24_comm_in(R_REGISTER|CONFIGURATION);          //Read RF24 config register(receive only one byte into data variable) 
*     RF24_comm_in(R_REGISTER|RX_ADDR_P0, *address, 3);     //Send command R_REGISTER|RX_ADDR_P0. After that, receives 3 bytes to array(or pointer). You can send strings directly 
*     RF24_comm_out(W_REGISTER|TX_ADDR_P0,  *address, 5);   //Writes 5 bytes from pointer address. Please, set RX address too(for shockburst auto acknowledge) 
*     RF24_comm_out(W_REGISTER|TX_ADDR_P0, "test", 4);      //Writes 4 ascii bytes from pointer(string "test"). You can put strings directly 
*/ 
int RF24_comm_in(int comm) {     //Send command and receive input(receive only one byte) 
   int rv; 
   RF24_select();        
   RF24_xfer(comm);              //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
   rv = RF24_xfer(0); 
   //rv = spi_read();             //It´s necessary due to spi_xfer read bug 
   RF24_unselect();    
   return rv;                    //Returns read value 
} 
// 
void RF24_comm_in(int comm, char *datain, int dataSZ) {       //Send command and receive input(dataSZ=number of input bytes)(datain=pointer to data for receiving) 
   #ifndef RF24_USE_DMA    int i;   #endif 
   RF24_select();        
   RF24_xfer(comm);                 //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
   #ifdef RF24_USE_DMA 
      DMA_read(dataSZ, datain);     //Simple DMA Read and start transfer 
      DMA_start();                  //Start DMA transfer 
      while(DMA_is_busy());         //DMA is working 
   #else                            //Programed IO mode(Normal mode) 
      for(i=0;i<dataSZ;i++) { 
         *(datain+i)=RF24_xfer(0); 
        // *(datain+i)=spi_read();   //It´s necessary due to spi_xfer read bug 
      } 
   #endif      
   RF24_unselect();    
} 
// 
int RF24_comm_out(int comm, char *dataout, int dataSZ) {       //Send command and send output string(dataSZ=number of bytes to output)(dataout=pointer to data for sending) 
   #ifndef RF24_USE_DMA    int i;   #endif 
   int rv;                          //rv=return value(SPI read value) 
   RF24_select();        
   RF24_xfer(comm);                 //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
    
   #ifdef RF24_USE_DMA 
      DMA_write(dataSZ, dataout);   //Simple DMA Write and start transfer 
      DMA_start();                  //Start DMA transfer 
      while(DMA_is_busy());         //DMA is working 
   #else                            //Programed IO mode(Normal mode) 
      for(i=0;i<dataSZ;i++) { 
         RF24_xfer(*(dataout+i)); 
      }  
   #endif 
   //rv = RF24_xfer();              //just catch last spi_xfer received byte) 
   //rv = spi_read2();              //It´s necessary due to spi_xfer read bug 
   RF24_unselect();    
   return rv;                       //Return last read value 
} 
/***************************************************************************************** 
****************************************************************************************** 
* Send command (1 or 2 bytes), return status value of nRF24L01 
* 
* parameters: 
*     comm, comm1 : Commands to send 
*     commDS      : Comm1 datasize in bits(8 to 32 bits, from lsb to msb) 
* 
* returns  : It only returns nRF24 general STATUS register(see RF24_STATUS enum) 
* 
* ex:    
*     RF24_comm(W_REGISTER|CONFIGURATION, PWR_UP);       //RF_packet CRC and nRF24L01+ configuration   //1. Set PWR_UP = 1 and PRIM_RX = 0 in the CONFIG register. 
*     RF24_comm(W_REGISTER|RX_ADDR_P1, 0xA1A2A4, 24);    //Address of data pipe 1. 5 Bytes maximumlength. (LSByte first). //Address was set as 0xA1A2A4  24bits(3 bytes) 
* 
*/ 
int RF24_comm(int comm) { 
   int rv;                      //// //rv=return value(SPI nRF24 status read value) 
   RF24_select();        
   rv=RF24_xfer(comm);           //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
   //rv=spi_read();               //It´s necessary due to spi_xfer read bug 
   RF24_unselect();    
   return rv;                    //Return last read value 
} 
// 
int RF24_comm(int comm, int comm1) { 
   int rv;                       //rv=return value(SPI nRF24 status read value) 
   RF24_select();        
   RF24_xfer(comm);              //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
   rv=RF24_xfer(comm1);          //Write config.value or command 
  // rv=spi_read();               //It´s necessary due to spi_xfer read bug 
   RF24_unselect();    
   return rv;                    //Return last read value 
} 
// 
int RF24_comm(int comm, int32 comm1, int commDS) {   //commDS=datasize(1 to 32 bits)
   int i;
   int rv;                             //rv=return value(SPI nRF24 status read value) 
   RF24_select();        
   RF24_xfer(comm);                    //RF24 Write address/command(see RF24_addr addresses list tabble in .h file) 
   
//!   rv=spi_xfer(RF24_SPI, comm1, commDS);//Write config.value or command(1 to 32 bits of data)___I CANNOT USE RF24_xfer() ALIAS BECAUSE IT NOT ACCEPTS MORE THAN 1 PARAMETER
   for(i=0;i<commDS;i++)
   {
   rv=spi_xfer(RF24_SPI, comm1);
   //rv=RF24_xfer(comm);
   }
   //rv=spi_read();                     //It´s necessary due to spi_xfer read bug
   RF24_unselect();    
   return rv;                    //Return last read value 
} 
/***************************************************************************************** 
****************************************************************************************** 
*************************General Driver commands****************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
*************************General Driver commands****************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
*************************General Driver commands****************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
* DEFINED MACROS 
* 
* 
*/ 
//General command functions 
//ex:    RF24_FIFO_STATUS_TX_REUSE(RF24_FIFO_STATUS()); 
// 
#define  RF24_FLUSH_TX()      RF24_comm(FLUSH_TX)        //Flush TX FIFO, used in TX mode 
#define  RF24_FLUSH_RX()      RF24_comm(FLUSH_RX)        //Flush RX FIFO, used in RX mode. Should not be executed during transmission of acknowledge, that is, acknowledge package will not be completed. 
#define  RF24_REUSE_TX_PL()   RF24_comm(REUSE_TX_PL)     //(do not check for errors like TX_EMPTY)Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
#define  RF24_R_RX_PL_WID()   RF24_comm_in(R_RX_PL_WID)  //Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO. Note: Flush RX FIFO if the read value is larger than 32 bytes. 
#define  RF24_W_TX_PAYLOAD_NOACK()  RF24_comm(W_TX_PAYLOAD_NOACK)  //Disables AUTOACK on this specific packet 
//to be defined in function  #define  RF24_W_ACK_PAYLOAD(xpipe)  RF24_comm(W_ACK_PAYLOAD&(xpipe&0b00000111)) 
                                                         //bits 0 to 2(value 0 a 5) Used in RX mode. Write Payload to be transmitted together with ACK packet on choosen Pipenumber(0 to 5). (Pipenumber valid in the range from 000 to 101[0d0 a 0d5]). Maximum three ACK packet payloads can be pending. Payloads with same pipenumber are handled using first in - first out principle. Write payload: 1– 32 bytes. A write operation always starts at byte 0. 
//Several defined return functions(returns directly into input variable) 
#define  RF24_lost_packets()     RF24_comm_in(R_REGISTER|OBSERVE_TX)>>4            //Know how many packets was lost during Transmition(with autoack feature enabled) 
#define  RF24_retry_packets()    RF24_comm_in(R_REGISTER|OBSERVE_TX)&0b00001111    //Know how many retrys for transmit data(TX)(with autoack feature enabled) 

#define  RF24_RX_power_detector()   RF24_comm_in(R_REGISTER|RX_POWER_DETECTOR)     //Returns 1 if power is over -65dbm 

#define  RF24_setup_autoretry(delay,retrys)  RF24_comm(W_REGISTER|SETUP_AUTORETRANSMISSION, (delay&0b11110000)|(retrys&0b00001111)) 
                                                                        //delay=0to15(250 to 4000us)(delay between retrys, see datasheet), retrys=0to15(How many TX retrys) 
//FIFO_STATUS defined functions(read fifo_status before check bits) 
//ex:    RF24_FIFO_STATUS_TX_REUSE(RF24_FIFO_STATUS()); 
// 
#define  RF24_FIFO_STATUS()                  RF24_comm_in(R_REGISTER|FIFO_STATUS) //Returns FIFO_STATUS Register 
#define  RF24_FIFO_STATUS_TX_REUSE(fstat)    bit_test(fstat, 6)         //(R)make RF24_enable for at least 10µs to Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX_REUSE is set by the SPI command REUSE_TX_PL, and is reset by the SPI commands W_TX_PAYLOAD or FLUSH TX 
#define  RF24_FIFO_STATUS_TX_FULL(fstat)     bit_test(fstat, 5)         //(R)TX FIFO buffer full flag 
#define  RF24_FIFO_STATUS_TX_EMPTY(fstat)    bit_test(fstat, 4)         //(R)TX FIFO buffer empty 
#define  RF24_FIFO_STATUS_RX_FULL(fstat)     bit_test(fstat, 1)         //(R)RX FIFO buffer full flag 
#define  RF24_FIFO_STATUS_RX_EMPTY(fstat)    bit_test(fstat, 0)         //(R)RX FIFO buffer empty 

//General STATUS register defined functions(read RF24 status[RF24_get_status()] before check bits) 
//ex:    RF24_STATUS_clr_IRQs(RF24_get_status()); 
// 
#define  RF24_get_status()    RF24_comm(NOP)    //Get nRF24 STATUS register in a fast way: It sends NO OPERATION command byte while receives STATUS register byte. 
// 
#define  RF24_STATUS_extract_pipe_number(xstat)       ((xstat&0b00001110)>>1)    //Return received pipe number(0 to 5) 
#define  RF24_STATUS_RX_buffer_empty(xstat)           (((xstat&RX_BUFFER_EMPTY)==RX_BUFFER_EMPTY)?true:false)  //return true if RX_buffer_empty 
#define  RF24_STATUS_TX_buffer_full(xstat)            bit_test(xstat, 0)         //return true if TX_buffer_full 
// 
//Before commands below, use RF24_IRQ_state() to check if some interrupt occurred 
#define  RF24_STATUS_RX_dataready_IRQ(xstat)          bit_test(xstat, 6)         //return true if buffer_empty 
#define  RF24_STATUS_TX_datasent_IRQ(xstat)           bit_test(xstat, 5)         //return true if buffer_empty 
#define  RF24_STATUS_MAX_retrys_reached_IRQ(xstat)    bit_test(xstat, 4)         //return true if buffer_empty 
// 
//IRQs must be cleared through this command. 
#define  RF24_STATUS_clr_IRQs(xstat)                  RF24_comm(W_REGISTER|STATUS, xstat)     //clear selected IRQs(in xstat) 
                        //RF24_STATUS_clr_IRQs(IRQ_RX_dataready|IRQ_TX_datasent|IRQ_MAX_retransmit);  //select IRQs to be cleared, or 
                        //RF24_STATUS_clr_IRQs(IRQ_ALL);       //clear all possible IRQs 
//NOT_WORKS  #define  RF24_STATUS_clr_IRQs(xstat)                  RF24_comm(W_REGISTER|STATUS, (xstat&=IRQ_ALL) )     //clear selected IRQs(in xstat) 
/***************************************************************************************** 
****************************************************************************************** 
* DEFINED FUNCTIONS 
* 
* 
*/ 
/***************************************************************************************** 
****************************************************************************************** 
* Inicialize tris ports of uC 
* ex:   RF24_initPorts(); 
*/ 
void RF24_initPorts() { 
   int1 a; 
   //Control 
   RF24_unselect();    
   RF24_enable();        
   a=input(RF24_IRQ);      //set tris register to input 
   //SPI initial states 
   a=input(SPI_MISO);      //set tris register to input 
   output_low (SPI_MOSI); 
   output_low (SPI_CLK); 
} 
/***************************************************************************************** 
****************************************************************************************** 
* Inicializa configuracao padrao nRF24L01+ (Ititializes default configuration for chip) 
* ex:   RF24_defconf(); 
*/ 
void RF24_default_config() {  //Ititializes default configuration for chip nRF24L01+ 

   #ifdef RF24_USE_DMA 
      DMA_default_config();   //Sets default DMA configuration 
   #endif      

   RF24_disable();      

   RF24_comm(W_REGISTER|CONFIGURATION, EN_CRC|CRC16|PWR_UP);   //RF_packet CRC and nRF24L01+ configuration and initialization 
   //RF24_comm(W_REGISTER|CONFIGURATION, CRC16|PWR_UP);   //NO_CRC, RF_packet CRC and nRF24L01+ configuration and initialization 
   RF24_PWUPDELAY(); 

   RF24_comm(W_REGISTER|EN_AUTOACK, 0b00011111);   //autoack in pipe 0,1,2,3,4 and 5 
   //RF24_comm(W_REGISTER|EN_AUTOACK, 0b00000000);   //autoack in pipe 0,1,2,3,4 and 5 

   RF24_comm(W_REGISTER|EN_RXPIPES, 0b00000011);   //****enable only pipes 0 and 1(pipes 2 to 5 are not used) 

   RF24_comm(W_REGISTER|SETUP_ADDRESSWIDTH, 1);    //addresswidth setting[1 to 3(3 to 5 bytes)] 
                                                   //ADDRESSWIDTH = 1 -> it means 3 bytes 

   RF24_comm(W_REGISTER|SETUP_AUTORETRANSMISSION,  //[7:4(250 to 4000us) autoretry delay, 3:0(0 to 15)Auto retry times] 
      0b00011111);                                 //waitretry 500us   , retry 15times  //ARD=500µs is long enough for any ACK payload length in 1 or 2Mbps mode 
   RF24_comm(W_REGISTER|RF_CHANNEL, 0);   //Set RF channel. F0= 2400 + RF_CHANNEL [MHz]. Use 1 channel of space for 250kbps to 1Mbps radios and 2 channels of space between 2Mbps radios. 
                                          //RF channel=0(2400MHz) 
   RF24_comm(W_REGISTER|RF_SETUP, 
      RF_DR_2Mbps|RF_PWR_0dBm);           //Datarate 2Mbps. PowerAmplifier in 0dBm. 
    
   RF24_comm(W_REGISTER|STATUS,           //Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on the MISO pin) 
      IRQ_RX_dataready|IRQ_TX_datasent|IRQ_MAX_retransmit);   //Clear these tree interrupts 

   //RF24_comm(W_REGISTER|RX_ADDR_P0,     //Receive address data pipe 0. 5 Bytes maximumlength. (LSByte first) 
   //   0xA1A2A300, 32);                  //Address 0xA1A2A3  24bits(3 bytes, the last fourth byte was written as 0x00) 
   RF24_comm_out(W_REGISTER|RX_ADDR_P0,   //Receive address data pipe 0. 5 Bytes maximumlength. (LSByte first) 
      "0rdda", 5 );                       //Address ascii "addr0"(5 bytes), LSB first. Because of that, it is writed on the contrary. 
   RF24_comm_out(W_REGISTER|RX_ADDR_P1,   //Receive address data pipe 1. 5 Bytes maximumlength. (LSByte first) 
      "1rdda", 5 );                       //Address ascii "addr1"(5 bytes), LSB first. Because of that, it is writed on the contrary. 
   RF24_comm_out(W_REGISTER|RX_ADDR_P2,   //Receive address data pipe 2. 5 Bytes maximumlength. (LSByte first) 
      '2', 1);                            //Address ascii "addr2"  (I can change only the LsByte) 
   RF24_comm_out(W_REGISTER|RX_ADDR_P3,   //Receive address data pipe 3. 5 Bytes maximumlength. (LSByte first) 
      '3', 1);                            //Address ascii "addr3"  (I can change only the LsByte) 
   RF24_comm_out(W_REGISTER|RX_ADDR_P4,   //Receive address data pipe 4. 5 Bytes maximumlength. (LSByte first) 
      '4', 1);                            //Address ascii "addr4"  (I can change only the LsByte) 
   RF24_comm_out(W_REGISTER|RX_ADDR_P5,   //Receive address data pipe 5. 5 Bytes maximumlength. (LSByte first) 
      '5', 1);                            //Address ascii "addr5"  (I can change only the LsByte) 

   RF24_comm_out(W_REGISTER|TX_ADDR,      //(3 to 5 bytes)Transmit address. Used for a PTX device only.(LSByte first). Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if Enhanced ShockBurst enabled 
      "0rdda", 5 );                       //Address ascii "addr0"(5 bytes), LSB first. Because of that, it is writed on the contrary. 

   RF24_comm(W_REGISTER|RX_PW_P0, 32);    //(set 1 to 32)RX payload size pipe0 set to 32 bytes 
   RF24_comm(W_REGISTER|RX_PW_P1, 32);    //(set 1 to 32)RX payload size pipe1 set to 32 bytes 
   RF24_comm(W_REGISTER|RX_PW_P2, 32);    //(set 1 to 32)RX payload size pipe2 set to 32 bytes 
   RF24_comm(W_REGISTER|RX_PW_P3, 32);    //(set 1 to 32)RX payload size pipe3 set to 32 bytes 
   RF24_comm(W_REGISTER|RX_PW_P4, 32);    //(set 1 to 32)RX payload size pipe4 set to 32 bytes 
   RF24_comm(W_REGISTER|RX_PW_P5, 32);    //(set 1 to 32)RX payload size pipe5 set to 32 bytes 
                                          //although other pipes(pipe 2 to 5) are not used(see EN_RXPIPES setting) 
   RF24_comm(W_REGISTER|EN_DYNAMIC_PAYLOAD,//bits 0 to 5 enables in pipe0 to pipe5 
      0b00111111);                        //Dynamic payload Enabled in all pipes(pipe 0 to 5) 

   RF24_comm(W_REGISTER|DYN_PAYLOAD_CONFIG,//Enables (DPL)Dynamic payload length feature. Enable to be able to transmit variable data lenght packets(from 1 to 32 data bytes) 
      EN_DPL|EN_ACK_PAY|EN_DYN_ACK);       //All DPL functions enabled 
    
   RF24_comm(W_REGISTER|FLUSH_RX);        //Delete RX buffer(already transfered to buffer) 
   RF24_comm(W_REGISTER|FLUSH_TX);        //Delete TX buffer(already transfered to buffer) 
   RF24_STATUS_clr_IRQs(IRQ_ALL);         //clear IRQs 

   RF24_disable();        
}// 
/***************************************************************************************** 
****************************************************************************************** 
* Constant carrier wave output for testing: 
*        (TX is always on transmitting sinc signal) 
* 
* The output power of a radio is a critical factor for achieving wanted range. Output 
* power is also the first test criteria needed to qualify for all telecommunication regulations. 
* Datasheet, see last page. 
*/ 
void RF24_waveout_testing(int channel) {    //TX is always on transmitting sinc signal 

   RF24_comm(W_REGISTER|CONFIGURATION, //RF_packet CRC and nRF24L01+ configuration 
               PWR_UP);                //1. Set PWR_UP = 1 and PRIM_RX = 0 in the CONFIG register. 
   RF24_PWUPDELAY();                   //2. Wait 1.5ms PWR_UP->standby. 
    
   RF24_comm(W_REGISTER|RF_SETUP,      //3. In the RF register set:  CONT_WAVE = 1. PLL_LOCK = 1. RF_PWR. 
               CONT_WAVE|PLL_LOCK|RF_DR_2Mbps|RF_PWR_0dBm);    //Datarate 2Mbps. PowerAmplifier in 0dBm. 

   RF24_comm(W_REGISTER|RF_CHANNEL, channel);//4. Set the wanted RF channel. //If RF channel=0(2400MHz)(increases 1MHz per channel) 

   RF24_enable();                      //5. Set CE high. 
   //                                  //6. Keep CE high as long as the carrier is needed. 
}// 
/************************************************************************************ 
* Enhanced ShockBurst transmitting payload: 
* Datasheet, see page 75. 
* 
* 
*/ 
int RF24_TX_SET() {    //Enhanced ShockBurst transmitting payload(return 1 if ok, return other number if error) 
int data; 
   RF24_disable();                      
   //    
   data=RF24_comm_in(R_REGISTER|CONFIGURATION); //Read RF24 config register 
   if ((data&PWR_UP)!=PWR_UP) return 0x02;      //Error: Turn PWR_UP on before transmit 
   data&=~PRIM_RX;                              //1. Set the configuration bit PRIM_RX low. 
   RF24_comm(W_REGISTER|CONFIGURATION, data);   //rewrite config data with PRIM_RX off(transmitter mode)//RF24_PWUPDELAY(); 
    
   RF24_enable();                               //3. A high pulse on CE starts the transmission. The minimum pulse width on CE is 10µs. 
   delay_us(10); 
   RF24_disable();                            
    

/* 
2. When the application MCU has data to transmit, clock the address for the receiving node 
(TX_ADDR) and payload data (TX_PLD) into nRF24L01+ through the SPI. The width of TX-payload 
is counted from the number of bytes written into the TX FIFO from the MCU. TX_PLD must 
be written continuously while holding CSN low. TX_ADDR does not have to be rewritten if it is 
unchanged from last transmit. If the PTX device shall receive acknowledge, configure data pipe 0 
to receive the ACK packet. The RX address for data pipe 0 (RX_ADDR_P0) must be equal to the 
TX address (TX_ADDR) in the PTX device. For the example in Figure 14. on page 41 perform the 
following address settings for the TX5 device and the RX device: 
TX5 device: TX_ADDR = 0xB3B4B5B605 
TX5 device: RX_ADDR_P0 = 0xB3B4B5B605 
RX device: RX_ADDR_P5 = 0xB3B4B5B605 
4. nRF24L01+ ShockBurst: 
   Radio is powered up. 
   16MHz internal clock is started. 
   RF packet is completed (see the packet description). 
   Data is transmitted at high speed (1Mbps or 2Mbps configured by MCU). 
5. If auto acknowledgement is activated (ENAA_P0=1) the radio goes into RX mode immediately, 
unless the NO_ACK bit is set in the received packet. If a valid packet is received in the valid 
acknowledgement time window, the transmission is considered a success. The TX_DS bit in the 
STATUS register is set high and the payload is removed from TX FIFO. If a valid ACK packet is 
not received in the specified time window, the payload is retransmitted (if auto retransmit is 
enabled). If the auto retransmit counter (ARC_CNT) exceeds the programmed maximum limit 
(ARC), the MAX_RT bit in the STATUS register is set high. The payload in TX FIFO is NOT 
removed. The IRQ pin is active when MAX_RT or TX_DS is high. To turn off the IRQ pin, reset the 
interrupt source by writing to the STATUS register (see Interrupt chapter). If no ACK packet is 
received for a packet after the maximum number of retransmits, no further packets can be transmitted 
before the MAX_RT interrupt is cleared. The packet loss counter (PLOS_CNT) is incremented 
at each MAX_RT interrupt. That is, ARC_CNT counts the number of retransmits that were 
required to get a single packet through. PLOS_CNT counts the number of packets that did not get 
through after the maximum number of retransmits. 
6. nRF24L01+ goes into standby-I mode if CE is low. Otherwise, next payload in TX FIFO is transmitted. 
If TX FIFO is empty and CE is still high, nRF24L01+ enters standby-II mode. 
7. If nRF24L01+ is in standby-II mode, it goes to standby-I mode immediately if CE is set low. 
*/ 
   return 1;   //Operation success 
}// 
/************************************************************************************ 
* Enhanced ShockBurst receiving payload: 
* Datasheet, see page 76. 
* 
* 
*/ 
int RF24_RX_SET() {    //Enhanced ShockBurst receiving payload(return 1 if ok, return other number if error) 
int i,data; 

   i = RF24_comm_in(R_REGISTER|FIFO_STATUS); 
   if ((i&RX_FULL)==RX_FULL) return 0;                 //Error: RX FIFO buffer full flag, must be read before receive new packets(REVISION 1.1) 
   //    
   data = RF24_comm_in(R_REGISTER|CONFIGURATION); 
   if ((data&PWR_UP)!=PWR_UP) return 0x02;   //Error: Turn PWR_UP on before transmit 
   data|=PRIM_RX;                            //1. Select RX by setting the PRIM_RX bit in the CONFIG register to high. 
   RF24_comm(W_REGISTER|CONFIGURATION, data); 
   /* 
   All data pipes that receive 
   data must be enabled (EN_RXADDR register), enable auto acknowledgement for all pipes running 
   Enhanced ShockBurst (EN_AA register), and set the correct payload widths (RX_PW_Px registers). 
   Set up addresses as described in item 2 in the Enhanced ShockBurst transmitting payload 
   example above. 
   */ 
   RF24_enable();             //2. Start Active RX mode by setting CE high. 
   //                         //3. After 130µs nRF24L01+ monitors the air for incoming communication.    
   /*4. When a valid packet is received (matching address and correct CRC), the payload is stored in the 
   RX-FIFO, and the RX_DR bit in STATUS register is set high. The IRQ pin is active when RX_DR is 
   high. RX_P_NO in STATUS register indicates what data pipe the payload has been received in. 
   5. If auto acknowledgement is enabled, an ACK packet is transmitted back, unless the NO_ACK bit 
   is set in the received packet. If there is a payload in the TX_PLD FIFO, this payload is added to 
   the ACK packet.*/ 
   //RF24_disable();          //6. MCU sets the CE pin low to enter standby-I mode (low current mode). 
   //                         //7. MCU can clock out the payload data at a suitable rate through the SPI. 
   //                         //8. nRF24L01+ is now ready for entering TX or RX mode or power down mode. 
   return true;               //Success 
}// 
/************************************************************************************ 
* Change address for transmiting data: 
* 
* Parameters: 
* 
* address   : transmit address 
* addrsize  : address size in bits(3 to 5 bytes). (can be int32, array or pointer) 
* ShockBurst: set true if shockburst is active(because it have to change TX and RX receive address) 
* returns   //True if ok. 0 if error 
* 
* Ex: 
*     RF24_TX_setaddress(*addr, 5, true)  or    //5 bytes address lenght read from pointer *addr, set. Shockburst on mode(Sets Pipe0 receive address as the same). 
*     RF24_TX_setaddress("6aA7G", 5, true)      //5 bytes address lenght(ascii) read and set as "6aA7G". Shockburst on mode(Sets Pipe0 receive address as the same). 
*/ 
short int RF24_TX_setaddress(char *address, int addrsize, short int ShockBurst) {   //Change address for transmiting data(with or without shockburst) 
   if ((addrsize<3)||(addrsize>5))  return 0;   //Error: Wrong address size 
   // 
   RF24_comm_out(W_REGISTER|TX_ADDR, address, addrsize);    //Set TX address 

   if (ShockBurst) {                            //Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if Enhanced ShockBurst enabled 
      RF24_comm_out(W_REGISTER|RX_ADDR_P0, address, addrsize); //Set RX address(for shockburst auto acknowledge) 
   } 
   return true;                                 //Success              
}// 
/************************************************************************************ 
* Transmit data(1 to 32 bytes) to actual address: 
* 
* Parameters: 
* 
* datasize  : number of bytes in TX pointer_buffer(1 to 32) for transmiting 
* burst     : This function disable packet acknowledge receiving. It makes the link less reliable, but tree times faster. 
*                 true: No returning acknowledge(triplicates datarate in 2Mbps). You can reach max. transfer speed(200KByte/s).    
*                 false: Without burst, the max speed is 70KByte/s(It´s good when in low power, long range or noise is present). 
* buffer    : buffer where data for transmiting is 
* returns   : Returns true(1) if data put into buffer. Other if not(error) 
* 
*/ 
int RF24_TX_putbuffer(short int burst, int datasize, char *buffer) {//Transmit data(1 to 32 bytes) to actual address 
   int i; 
   //    
   if ((datasize==0)||(datasize>32)) return 2;     //datasize must be 1 to 32 
   // 
   i = RF24_get_status();                          //receive status data in i. 
   if (RF24_STATUS_TX_buffer_full(i)) return 3;    //error: buffer full(Flush TX buffer) 
   if (RF24_IRQ_state()) {                         //Check what interrupt was set 
      if (RF24_STATUS_TX_datasent_IRQ(i)) return 4; //Error: TX interrupt not cleared 
      if (RF24_STATUS_MAX_retrys_reached_IRQ(i)) return 5;//Error: TX interrupt MAX_RETRYS not cleared 
   } 
   // 
   //Load data to nRF24L01+ 
   if (burst==true)              //No returning acknowledge(triplicates datarate in 2Mbps) 
      RF24_comm_out(W_TX_PAYLOAD_NOACK, buffer, datasize);  //Send command and send output string(dataSZ=number of bytes to output)(dataout=pointer to data for sending) 
   else                          //Normal(waits for aknowledge). It´s good when in low power, long range or noise is present. 
      RF24_comm_out(W_TX_PAYLOAD, buffer, datasize);  //Send command and send output string(dataSZ=number of bytes to output)(dataout=pointer to data for sending) 
   // 
   RF24_enable();                            
   return true;                                    //Data transmited 
}// 
/************************************************************************************ 
* Ckeck TX acknowledge register: 
* 
* returns   //Returns true(1) if data transmited(with ACK received). Other if not(error) 
* 
*/ 
//NOT IMPLEMENTED 
// 
/************************************************************************************ 
* Retransmit last TX data present in TX buffer. 
* 
* returns : Returns true(1) if data transmited(with ACK received). Other if not(error) 
* ps      : FLUX_TX if do not want to retransmit infinitely 
*/ 
int RF24_TX_reusebuffer() {   //Retransmit last TX data present in TX buffer 
int i; 
   i = RF24_comm_in(R_REGISTER|FIFO_STATUS); 
   if (i==TX_EMPTY) return 0; //Error: TX_Flushed 
   // 
   //Load data to nRF24L01+ 
   RF24_comm(REUSE_TX_PL);    //Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
    
   RF24_enable();                            

   return true;               //Success 
}// 
/************************************************************************************ 
* Receive data(1 to 32 bytes) into buffer: 
* 
* Parameters: 
* 
* pipe      //return number of received pipe(0 to 5) 
* datasize  //return number of received bytes in RX buffer(1 to 32) 
* buffer    //32bytes buffer for received data(can be a pointer or an array) 
* returns   //Returns true(1) if data received. False(0) if not 
* 
*/ 
int RF24_RX_getbuffer(int *pipe, int *datasize, char *buffer) {   //Receive data(1 to 32 bytes) into buffer 
   int i; 
   //    
   RF24_enable();                                  //Enables receiver 
   if (!RF24_IRQ_state()) return 0;                //No data received(no IRQ set) 
   // 
   i = RF24_get_status();                          //receive status data in i. 
   if (!RF24_STATUS_RX_dataready_IRQ(i)) return 2; //No data received(another IRQ was set) 
   // 
   *pipe = RF24_STATUS_extract_pipe_number(i);     //Extract received pipe number(0 to 5) 
   // 
   *datasize = RF24_comm_in(R_RX_PL_WID);          //Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO. 
   if (*datasize>32 || *datasize==0) {             //Note: Flush RX FIFO if the read value is larger than 32 bytes. 
      RF24_comm(FLUSH_RX);                         //Delete RX buffer 
      RF24_STATUS_clr_IRQs(IRQ_RX_dataready);      //clear RX IRQ 
      return 3;                                    //RX error 
   } 
   // 
   //hehe 
   RF24_STATUS_clr_IRQs(IRQ_RX_dataready);         //clear RX IRQ 
   //RF24_STATUS_clr_IRQs(IRQ_ALL);                //clear All irqs 
   //Load data from nRF24L01+ 
   RF24_comm_in(R_RX_PAYLOAD, buffer, *datasize);  //Read RX-payload: 1 – 32 bytes. A read operation always starts at byte 0. Payload is deleted from FIFO after it is read. Used in RX mode. 
   return true;                                    //Data received 
}// 
/************************************************************************************ 
* Beep if there is RFsignal stronger that -64dB on choosen channel 
* 
* returns   //Returns true(1) if data received. False(0) if not 
*      
*/ 
#define  buzzer_on()    set_pwm1_duty(61)      //Liga Buzzer 
#define  buzzer_off()   set_pwm1_duty(0)        //Desliga Buzzer 
int RF24_check_rfsignal() {     //Beep if there is RFsignal stronger that -64dB on choosen channel 
   int rfstat=0; 
   while(true) { 
      RF24_comm_in(R_REGISTER|RX_POWER_DETECTOR, &rfstat, 1); 
      if (rfstat==0) { 
         buzzer_off(); delay_ms(1); 
         return false; } 
      else { 
         buzzer_on();  delay_ms(1);  
         return true; 
      } 
   } 
}// 
/************************************************************************************ 
* Check actual configuration 
* (only for debugging purposes, stop in each delay) 
* 
* 
*/ 
void RF24_check_config() {      //Check actual configuration(only for debugging purposes, stop in each delay) 
   int bbuf[40]; 
    
   RF24_comm_in(R_REGISTER|CONFIGURATION, bbuf, 1);      //EN_CRC|CRC16|PWR_UP);   //RF_packet CRC and nRF24L01+ configuration and initialization 
   delay_ms(10); 
      
   RF24_comm_in(R_REGISTER|EN_AUTOACK, bbuf, 1);         //0b00011111);   //autoack in pipe 0,1,2,3,4 and 5 
   delay_ms(10); 

   RF24_comm_in(R_REGISTER|EN_RXPIPES, bbuf, 1);         //0b00000011);   //****enable only pipes 0 and 1(pipes 2 to 5 are not used) 
   delay_ms(10); 

   RF24_comm_in(R_REGISTER|SETUP_ADDRESSWIDTH, bbuf, 1);          //, 1);  //addresswidth setting[1 to 3(3 to 5 bytes)]//ADDRESSWIDTH = 1 -> it means 3 bytes 
   delay_ms(10); 
   RF24_comm_in(R_REGISTER|SETUP_AUTORETRANSMISSION, bbuf, 1);    //[7:4(250 to 4000us) autoretry delay, 3:0(0 to 15)Auto retry times] 
                                                                     //0b00011111);    //waitretry 500us   , retry 15times  //ARD=500µs is long enough for any ACK payload length in 1 or 2Mbps mode 
   delay_ms(10); 

   RF24_comm_in(R_REGISTER|RF_CHANNEL, bbuf, 1);         //, 0);   //Set RF channel. F0= 2400 + RF_CHANNEL [MHz]. Use 1 channel of space for 250kbps to 1Mbps radios and 2 channels of space between 2Mbps radios.//RF channel=0(2400MHz) 
   delay_ms(10); 
   RF24_comm_in(R_REGISTER|RF_SETUP, bbuf, 1);           //, RF_DR_2Mbps|RF_PWR_0dBm);           //Datarate 2Mbps. PowerAmplifier in 0dBm. 
   delay_ms(10); 
    
   RF24_comm_in(R_REGISTER|STATUS, bbuf, 1);             //,           //Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on the MISO pin) 
                                                            //IRQ_RX_dataready|IRQ_TX_datasent|IRQ_MAX_retransmit);   //Clear these tree interrupts 
   delay_ms(10); 
    
   RF24_comm_in(R_REGISTER|RX_ADDR_P0, bbuf, 5);//PROBLEMA( bbuf=0x00A1A2 )         //,       //Receive address data pipe 0. 5 Bytes maximumlength. (LSByte first) 
                                                            //0xA1A2A3, 24);                      //Address 0xA1A2A3  24bits(3 bytes) 
   delay_ms(10); 
   RF24_comm_in(R_REGISTER|RX_ADDR_P1, bbuf, 5);//PROBLEMA( bbuf=0x00A1A2 )         //,       //Receive address data pipe 1. 5 Bytes maximumlength. (LSByte first) 
                                                            //0xA1A2A4, 24);                      //Address 0xA1A2A4  24bits(3 bytes) 
   delay_ms(10); 

   RF24_comm_in(R_REGISTER|TX_ADDR, bbuf, 5);            //,          //(3 to 5 bytes)Transmit address. Used for a PTX device only.(LSByte first). Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if Enhanced ShockBurst enabled 
                                                            //0xA1A2A3, 24); //Address 0xA1A2A3  24bits(3 bytes) 
   delay_ms(10); 
    
   RF24_comm_in(R_REGISTER|RX_PW_P0, bbuf, 1);     //, 32);    //(set 1 to 32)RX payload size pipe0 set to 32 bytes 
   delay_ms(10); 
   RF24_comm_in(R_REGISTER|RX_PW_P1, bbuf, 1);     //, 32);    //(set 1 to 32)RX payload size pipe1 set to 32 bytes 
   delay_ms(10); 

   RF24_comm_in(R_REGISTER|EN_DYNAMIC_PAYLOAD, bbuf, 1);       //,//bits 0 to 5 enables in pipe0 to pipe5 
                                                                  //0b00111111);  //Dynamic payload Enabled in all pipes(pipe 0 to 5) 
   delay_ms(10); 
}// 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/************************************************************************************ 
* Driver use example 1: Receiving data 
* 
* ps: At first, set nRF24L01 SPI and control PINS 
*     Then, include driver file. 
*     Now, that´s all right 
*/ 
void RF24_driver_use_example_RXdata() {    //Example of using this driver 
   int RXbuffer1[32], RXbuffer2[32], RXbuffer3[32]; 
   int RXbuffer4[32], RXbuffer5[32]; 
   int RXdatasize, RXpipe; 
   //int TXbuffer[40], TXdatasize;        //not necessary 
   //int stat=0,fstat=0,retrys,lost,ret;  // 
   //int i=false; 
    
   RF24_initPorts(); 
   RF24_default_config(); 
    
   //RF24_check_config();    //check configuration  
                                
   RF24_RX_SET();       //Receiver on 
   while(true) { 
      
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer1)!=true );    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer2)!=true );    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer3)!=true );    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer4)!=true );    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
      while ( RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer5)!=true );    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
      
      
      /*while (!i) { 
         i=RF24_RX_getbuffer(&RXpipe, &RXdatasize, RXbuffer1);    //Wait till receive data(1 to 32 bytes) into buffer(from default_config address of pipe0) 
         if (!i) buzzer_off(); 
         else { buzzer_on(); delay_ms(10); buzzer_off(); } 
      } 
      i=false;*/ 
    
      //RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      
      //fstat=RF24_FIFO_STATUS(); 
      //stat=RF24_get_status(); 
      //buzzer_on(); delay_ms(1); buzzer_off(); //Beepa Buzzer 
      delay_cycles(1); 
   } 
    

}// 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/************************************************************************************ 
* Driver use example 2: Transmiting data(Using 3 level FIFO buffering, for achieve high trasfer speed) 
* 
* ps: At first, set nRF24L01 SPI and control PINS 
*     Then, include driver file. 
*     Now, that´s all right 
* 
*     put RF24_TX_putbuffer(true,32, "...        for enabling burst and reach 2Mbps
* 
*/ 
void RF24_driver_use_example_TXdata() {    //Example of using this driver 
   //int TXbuffer[40], TXdatasize;     //not necessary 
   //int stat=0,fstat=0,retrys,lost,ret; 
   int stat=0,retrys,lost,ret;  
    
   RF24_initPorts(); 
   RF24_default_config(); 
    
   RF24_TX_SET();       //Transmitter on    

   //USE the 3 nRF24 FIFO buffer stacks 
   //This procedure increases performance 
   ret=RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack1"); //Transmit 32bytes data(text string) to actual address(default_config address) 
   //now, you can write FIFO stack2 while nRF is transmiting 
   ret=RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack2"); //Transmit 32bytes data(text string) to actual address(default_config address) 
   while(true) {        //Enter transmit mode infinitely 
      /*//now, you can write FIFO stack3, filling the last nRF24 FIFO space 
      ret=RF24_TX_putbuffer(true, 32, "La cucaracha!!, Fill FIFO Stack3"); //Transmit 32bytes data(text string) to actual address(default_config address) 

      if (ret!=1) {     //if error occurred(if you passed wrong parameters or something else...) 
         //RF24_TX_reusebuffer();            //Retransmit last TX data present in TX buffer    
         RF24_comm(REUSE_TX_PL);             //Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
         //RF24_FLUSH_TX(); 
         RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
         lost=RF24_lost_packets();           //How how many packets was lost during Transmition 
         retrys=RF24_retry_packets();        //How how many retrys for transmit data(TX) 
         continue;      //return begining of while 
      }*/ 
      // 
      // 
      //now, you can write FIFO stack3, filling the last nRF24 FIFO space 
      ret=RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack3"); //Transmit 32bytes data(text string) to actual address(default_config address) 
      //Do this to read each of 3 filled FIFO buffer stacks      
      while(true) { 
         while(RF24_IRQ_state()==false);        //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
         stat=RF24_get_status(); 
         if (RF24_STATUS_TX_datasent_IRQ(stat)) break;   //Fifo 1 transfered 
         else {                                 //if (RF24_STATUS_MAX_retrys_reached_IRQ(stat)) { 
            RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
            //RF24_comm(REUSE_TX_PL);             //Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
         } 
      } 
      //You can check statistic regiters if you want to 
      //lost=RF24_lost_packets();           //How how many packets was lost during Transmition 
      //retrys=RF24_retry_packets();        //How how many retrys for transmit data(TX) 
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      // 
      // 
      // 
      // 
      //now, you can write FIFO stack3, filling the last nRF24 FIFO space 
      ret=RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack1"); //Transmit 32bytes data(text string) to actual address(default_config address) 
      //Do this to read each of 3 filled FIFO buffer stacks      
      while(true) { 
         while(RF24_IRQ_state()==false);        //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
         stat=RF24_get_status(); 
         if (RF24_STATUS_TX_datasent_IRQ(stat)) break;   //Fifo 2 transfered 
         else {                                 //if (RF24_STATUS_MAX_retrys_reached_IRQ(stat)) { 
            RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
            //RF24_comm(REUSE_TX_PL);             //Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
         } 
      } 
      //You can check statistic regiters if you want to 
      //lost=RF24_lost_packets();           //How how many packets was lost during Transmition 
      //retrys=RF24_retry_packets();        //How how many retrys for transmit data(TX) 
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      // 
      // 
      // 
      // 
      //now, you can write FIFO stack3, filling the last nRF24 FIFO space 
      ret=RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack2"); //Transmit 32bytes data(text string) to actual address(default_config address) 
      //Do this to read each of 3 filled FIFO buffer stacks      
      while(true) { 
         while(RF24_IRQ_state()==false);        //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
         stat=RF24_get_status(); 
         if (RF24_STATUS_TX_datasent_IRQ(stat)) break;   //Fifo 3 transfered 
         else {                                 //if (RF24_STATUS_MAX_retrys_reached_IRQ(stat)) { 
            RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
            //RF24_comm(REUSE_TX_PL);             //Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
         } 
      } 
      //You can check statistic regiters if you want to 
      lost=RF24_lost_packets();           //How how many packets was lost during Transmition 
      retrys=RF24_retry_packets();        //How how many retrys for transmit data(TX) 
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      // 
      // 
      //You must check and clear IRQ to assert that two last packets was sent(They remain in FIFO stack) 
      /*while(RF24_IRQ_state()==false);     //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      while(RF24_IRQ_state()==false);     //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time*/ 

      /*while(true) {    
         RF24_TX_reusebuffer();            //Retransmit last TX data present in TX buffer    
         while(RF24_IRQ_state()==false);     //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
         RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      }*/ 
      /*stat=RF24_get_status(); 
      if (RF24_STATUS_TX_datasent_IRQ(stat))   //return true if buffer_empty 
         delay_ms(1);                     //ACK Received OK 
      if (RF24_STATUS_MAX_retrys_reached_IRQ(stat))   //return true if trnasmit error 
         delay_ms(1);                     //ACK not Received 
      
      lost=RF24_lost_packets();         //How how many packets was lost during Transmition 
      retrys=RF24_retry_packets();      //How how many retrys for transmit data(TX) 
      */ 
      
      //RF24_STATUS_clr_IRQs(IRQ_ALL);    //Allows clearing all IRQs at the same time 
      //RF24_TX_reusebuffer();            //Retransmit last TX data present in TX buffer 
      //RF24_REUSE_TX_PL();               //(do not check for errors like TX_EMPTY)Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission.    
      //RF24_FLUSH_TX(); 
      //RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
      
      //output_toggle(INDICACAO_LED);       //Show something is happening 
   } 

}// 
/***************************************************************************************** 
****************************************************************************************** 
****************************************************************************************** 
*****************************************************************************************/ 
/************************************************************************************ 
* Driver use example 3: Transmiting data(simple withou 3 level fifo) 
* 
* ps: At first, set nRF24L01 SPI and control PINS 
*     Then, include driver file. 
*     Now, that´s all right 
* 
*     put RF24_TX_putbuffer(true,32, "...        for enabling burst and reach 2Mbps 
* 
*/ 
void RF24_driver_use_example_TXdata_simple() {    //Example of using this driver 
    
   RF24_initPorts(); 
   RF24_default_config();        
    
   RF24_TX_SET();       //Transmitter on    

   while(true) {        //Enter transmit mode infinitely 
      //now, you can write FIFO stack1 
      RF24_TX_putbuffer(false,32, "La cucaracha!!, Fill FIFO Stack1"); //Transmit 32bytes data(text string) to actual address(default_config address) 

      //Do this to check each data transfer 
      while(RF24_IRQ_state()==false);        //Waits for any interrupt.  Same as "while(!RF24_IRQ_state());"      
      RF24_STATUS_clr_IRQs(IRQ_ALL);      //Allows clearing all IRQs at the same time 
   } 
}// 
/***************************************************************************************** 
****************************************************************************************** 
******************************************************************************************/
