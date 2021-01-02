/* 
                              nRF24L01+ 
                  Single Chip 2.4GHz Transceiver Driver 

HEADER FILE                                                                                                                                      

ps: (R)=Read Only register 
    (W)=Write Only register 
    " "=Read and Write register 
    (?)=Initial Register State 
    *=Automated functions contemplated register(it has specific functions for it in driver, already done). Another registers/functions must be read/written by hand by the programmer. see example of implementation in driver. 
    **=Frequently used tables 
    data&=~PRIM_RX;   //Reset PRIM_RX bit(for example) 
    data|=PRIM_RX;    //Set PRIM_RX bit(for example) 
/////////////////////////////////////////////////////////////////////////////////////////*/ 

enum  {     //**Command List: MOST OF TIMES, COMMAND AND ADDRESSES ARE USED TOGETHER. !!!!!NOT USE ADDRESS ALONE!!!!! 
   R_REGISTER  =0b00000000,   //*bits from 0 to 4 are the addresses to be read(0 a 31)//Read command and status registers. 
   W_REGISTER  =0b00100000,   //*bits from 0 to 4 are the addresses to be write(0 a 31)//Write command and status registers. Executable in power down or standby modes only 
   R_RX_PAYLOAD=0b01100001,   //*Read RX-payload: 1 – 32 bytes. A read operation always starts at byte 0. Payload is deleted from FIFO after it is read. Used in RX mode. 
   W_TX_PAYLOAD=0b10100000,   //*Write TX-payload: 1 – 32 bytes. A write operation always starts at byte 0 used in TX payload. 
   FLUSH_TX    =0b11100001,   //*Flush TX FIFO, used in TX mode 
   FLUSH_RX    =0b11100010,   //*Flush RX FIFO, used in RX mode. Should not be executed during transmission of acknowledge, that is, acknowledge package will not be completed. 
   REUSE_TX_PL =0b11100011,   //*Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission. 
   R_RX_PL_WID =0b01100000,   //*Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO. Note: Flush RX FIFO if the read value is larger than 32 bytes. 
   W_ACK_PAYLOAD=0b10101000,  //bits 0 to 2(value 0 a 5) Used in RX mode. Write Payload to be transmitted together with ACK packet on choosen Pipenumber(0 to 5). (Pipenumber valid in the range from 000 to 101[0d0 a 0d5]). Maximum three ACK packet payloads can be pending. Payloads with same pipenumber are handled using first in - first out principle. Write payload: 1– 32 bytes. A write operation always starts at byte 0. 
   W_TX_PAYLOAD_NOACK=0b10110000,//*Used in TX mode. Disables AUTOACK on this specific packet. 
   NOP         =0b11111111};  //No Operation 


enum RF24_addr {  //**Addresses list 
   CONFIGURATION=0x00,         //see RF24_CONFIGURATION 
   EN_AUTOACK,                  //bits 0 to 5 enables in pipe0 to pipe5(EN_AA) 
   EN_RXPIPES,                  //bits 0 to 5 enables pipe0 to pipe5 
   SETUP_ADDRESSWIDTH,         //set 1 to 3(3 to 5 bytes) 
   SETUP_AUTORETRANSMISSION,   //7:4(250 to 4000us) autoretry delay, 3:0(0 to 15)Auto retry times 
   RF_CHANNEL=0x05,            //0 to 126. F0= 2400 + RF_CHANNEL [MHz]. Use 1 channel of space for 250kbps to 1Mbps radios and 2 channels of space between 2Mbps radios. 
   RF_SETUP,                  //see RF24_SETUP 
   STATUS,                     //see RF24_STATUS 
   OBSERVE_TX,                  //(R)7:4 lost packets, 3:0 retry TX packets 
   RX_POWER_DETECTOR,         //(R)only bit0(1=true) 
   RX_ADDR_P0=0x0A,            //Receive address data pipe 0. 5 Bytes maximumlength. (LSByte first) 
   RX_ADDR_P1,RX_ADDR_P2,RX_ADDR_P3,RX_ADDR_P4,RX_ADDR_P5,   //same for pipe 1. For pipes 2 to 5 is sent 1 Byte only. You can change only the Least Significant Byte. 
   TX_ADDR=0x10,               //(3 to 5 bytes)Transmit address. Used for a PTX device only.(LSByte first). Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if Enhanced ShockBurst enabled 
   RX_PW_P0=0x11,             //(set 1 to 32)RX payload size pipe0 
   RX_PW_P1,RX_PW_P2,RX_PW_P3,RX_PW_P4,RX_PW_P5,   //same for pipes 1 to 5 
   FIFO_STATUS=0x17,            //(R)see RF24_FIFO_STATUS 
   EN_DYNAMIC_PAYLOAD=0x1C,   //bits 0 to 5 enables in pipe0 to pipe5 
   DYN_PAYLOAD_CONFIG=0x1D};   //see RF24_DYN_PAYLOAD_CONFIG 


//************ 
//Memory addresses subitems 


enum RF24_CONFIGURATION {  //RF Configuration Register 
   MASK_RX_DR  =0b01000000,   //(0)Disable RX_DR interrupt. 
   MASK_TX_DR  =0b00100000,   //(0)Disable TX_DR Interrupt. 
   MASK_MAX_RT =0b00010000,   //(0)Disable Max_retransmit interrupt 
   EN_CRC      =0b00001000,   //(1)Enable CRC. Forced high if one of the bits in the EN_AA is high 
   CRC8        =0b00000000,   //(0)Enable 1byte  CRC(8 bits) instead of two bytes(16 bits) 
   CRC16       =0b00000100,   //(0)Enable 2bytes CRC(16 bits) instead of one byte(8 bits) 
   PWR_UP      =0b00000010,   //(0)Bit seteed if in power UP. cleared if in power Down. 
   PRIM_RX     =0b00000001    //(0)RX/TX control. Put receiver in RX mode.  1: RX, 0: TX 
   }; 
    
enum RF24_SETUP { //RF Setup Register 
   CONT_WAVE    =0b10000000,   //(0)Enables continuous carrier transmit when high(TX is always on transmitting sinc signal) 
   PLL_LOCK     =0b00010000,   //(0)Force PLL lock signal. Only used in test 
   RF_DR_250kbps=0b00100000,   //(0)Select between the low speed data rates. 
   RF_DR_1Mbps  =0b00000000,   //(0)Select between the medium speed data rates. 
   RF_DR_2Mbps  =0b00001000,   //(1)(default_mode)Select between the high speed data rates. This bitis don’t care if RF_DR_250kbps is set. 
   RF_PWR_n18dBm=0b00000000,   //-18dBm. Set RF output power 
   RF_PWR_n12dBm=0b00000010,   //-12dBm. Set RF output power 
   RF_PWR_n6dBm =0b00000100,   //-6dBm. Set RF output power 
   RF_PWR_0dBm  =0b00000110    //0dBm(default). Set RF output power 
   }; 
    
//Automated functions contemplated register    
enum RF24_STATUS {   //**Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on the MISO pin) 
   IRQ_RX_dataready  =0b01000000,   //(0)Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFOc.Write 1 to clear bit. 
   IRQ_TX_datasent   =0b00100000,   //(0)Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,this bit is set high only when ACK isreceived.Write 1 to clear bit. 
   IRQ_MAX_retransmit=0b00010000,   //(0)Maximum number of TX retransmits interrupt Write 1 to clear bit. If asserted it must be cleared to enable further communication. 
   IRQ_ALL           =0b01110000,   //Allows clearing all IRQs at the same time 
   RX_PIPE0          =0b00000000,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_PIPE1          =0b00000010,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_PIPE2          =0b00000100,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_PIPE3          =0b00000110,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_PIPE4          =0b00001000,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_PIPE5          =0b00001010,   //(R)(bits 3:1)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   RX_BUFFER_EMPTY   =0b00001110,   //(R)Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 
   TX_BUFFER_FULLY    =0b00000001    //(R)(0)TX FIFO full flag. 1: TX FIFO full. 0: Available locations in TX FIFO. 
   }; 

//Automated functions contemplated register    
enum RF24_FIFO_STATUS { //**FIFO(I/O buffer) Status Register 
   TX_REUSE =0b01000000,   //(R)Pulse the rfce high for at least 10µs to Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX_REUSE is set by the SPI command REUSE_TX_PL, and is reset by the SPI commands W_TX_PAYLOAD or FLUSH TX 
   TX_FULL  =0b00100000,   //(R)TX FIFO buffer full flag 
   TX_EMPTY =0b00010000,   //(R)TX FIFO buffer empty 
   RX_FULL  =0b00000010,   //(R)RX FIFO buffer full flag 
   RX_EMPTY =0b00000001    //(R)RX FIFO buffer empty 
   }; 

enum RF24_DYN_PAYLOAD_CONFIG {   //(DPL)Dynamic payload length feature configuration register. Enable to be able to transmit variable data lenght packets(from 1 to 32 data bytes) 
   EN_DPL      =0b00000100,   //Enables Dynamic Payload Length(General DPL enable) 
   EN_ACK_PAY  =0b00000010,   //Enables Payload with ACK 
   EN_DYN_ACK  =0b00000001,   //Enables the W_TX_PAYLOAD_NOACK command 
   EN_DPL_ALL  =0b00000111    //All DPL functions enabled 
   }; 

/* 
ACK_PAYLOAD   //(W)(till 32 BYTES)ackPayload. Written by separate SPI command. ACK packet payload to data pipe number n given in SPI command. Used in RX mode only. Maximum three ACK packet payloads can be pending. 
TX_PAYLOAD   //(W)(till 32 BYTES)dada to send. Written by separate SPI command. TX data payload register 1 - 32 bytes. This register is implemented as a FIFO with three levels. 
rx_PAYLOAD   //(R)(till 32 BYTES)dada received. Read by separate SPI command. RX data payload register. 1 - 32 bytes. This register is implemented as a FIFO with three levels. All RX channels share the same FIFO. 
*/ 