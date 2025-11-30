//******************************************************************************
// Programa para controlar un LCD de 16 caracteres por 2 lineas, utilizando tres
// terminales del microcontrolador
//
//                MSP430FR2433 
//        3.3V -----------------       SN74HC164            LCD 16 x 2
//        /|\ |                 |     ___________            _______
//         |--| Vcc         P1.6|--->|CLOCK    Qa|---DB4--> |  ___  |-->5V
//            |             P1.7|-+->|A        Qb|---DB5--> | |   | |
//            |                 | |->|B        Qc|---DB6--> | |   | |
//            |                 |    |         Qd|---DB7--> | |   | |
//            |                 |    |         Qe|---RS --> | | L | |
//            |                 |    |           |          | | C | |
//            |                 |    |        CLR|-->5Vcc   | | D | |
//	          |                 |    |___________|          | |   | |
//            |                 |                           | |   | |
//            |             P2.4|-------------------- E --> | |___| |RW -->GND
//            |                 |                           |_______|----->GND
//            |             Vss |-->GND                                 
//            |                 |                                      
// MARZO 2024 E.S.I.M.E. Zacatenco  IPN  
// Ingenieria en Comunicaciones y Electronica
// Academia de Electronica
// EDGAR R CALDERON DIAZ
//****************************************************************************** 
#ifndef LCD16x2fr16MHz_H_
#define LCD16x2fr16MHz_H_

char a=0;

void Ini_Lcd(void);
void Cmd_Lcd(unsigned char CMD);
void Dato_Lcd(unsigned char DATO);
void Send_10b(unsigned int Dat_Ser);

void DCO_16MHz_REFO(void);

#define CLK  BIT6
#define AB   BIT7
#define E    BIT4

//Funcion de inicializacion LCD para un Bus de datos de 4Bits
void Ini_Lcd()
{
  P1DIR |= CLK+AB;
  P1SEL0 &= ~(AB+CLK);           //Configuracion de las terminales para controlar el
  P1SEL1 &= ~(AB+CLK);          //74HC164 y el LCD
  P2DIR |= E;
  P2SEL0 &= ~E;
  P2SEL1 &= ~E;
  P1OUT &= ~(AB+CLK);
  P2OUT &= ~E;
  
  PM5CTL0 &= ~LOCKLPM5;

  
  __delay_cycles(240000);        //Since VDD reaches more than 4.5V wait more than 15ms
  
                                //Function Set(Interface data length is 8-bit)
  Send_10b(0x063);              //|RS|R/W|DB7|DB6|DB5|DB4|
  __delay_cycles(640);           //| 0| 0 | 0 | 1 | 1 | 0 |
                                //|RS|R/W|DB7|DB6|DB5|DB4|
                                //| 0| 0 | 0 | 0 | 1 | 1 |
                                //Wait more than 40 us
  
  Send_10b(0x062);              //|RS|R/W|DB7|DB6|DB5|DB4|
  __delay_cycles(640);           //| 0| 0 | 0 | 1 | 1 | 0 |
                                //|RS|R/W|DB7|DB6|DB5|DB4|
                                //| 0| 0 | 0 | 0 | 1 | 0 |
                                //Wait more than 40 us
  
  Send_10b(0x048);             //|RS|R/W|DB7|DB6|DB5|DB4|
  __delay_cycles(640);           //| 0| 0 | 0 | 1 | 0 | 0 |
                                //|RS|R/W|DB7|DB6|DB5|DB4|
                                //| 0| 0 | 1 | 0 | 0 | 0 |
                                //Wait more than 40 us
  
  Send_10b(0x00F);             //|RS|R/W|DB7|DB6|DB5|DB4|
  __delay_cycles(640);           //| 0| 0 | 0 | 0 | 0 | 0 |
                                //|RS|R/W|DB7|DB6|DB5|DB4|
                                //| 0| 0 | 1 | 1 | 1 | 1 |
                                //Wait more than 40 us
  Cmd_Lcd(0x01);
}


void Cmd_Lcd(unsigned char CMD)
{
  unsigned int Dat10b;
  if ((CMD & 0xFC) <= 0x02)
  {
    Send_10b(CMD);
    __delay_cycles(26240);
  }
  else
  {
    Dat10b = CMD;
    Dat10b = (Dat10b & 0x00F0) << 1;
    CMD = (CMD & 0x0F);
    Dat10b |= CMD;                                                                                                                                                                                                                                                             
    Send_10b(Dat10b);
    __delay_cycles(640);
  }
}

void Dato_Lcd(unsigned char DATO)
{
  unsigned int Dat10b;
  Dat10b = DATO;
  Dat10b = (Dat10b & 0x00F0) << 1;
  DATO = (DATO & 0x0F);
  Dat10b |= DATO+BIT9+BIT4;
  Send_10b(Dat10b);
  __delay_cycles(640);
}

void Send_10b(unsigned int Dat_Ser)
{
  for(a=0; a<=9; a++)
  {
    if((Dat_Ser & BIT9) != 0)
    {
      P1OUT |= AB;
      P1OUT |= CLK;                 
      P1OUT &= ~CLK; 
      Dat_Ser <<= 1;
      if(a == 4)
      {
        P2OUT |= E; 
        __delay_cycles(640);
        P2OUT &= ~E; 
        __delay_cycles(640);
      }
    }
    else
    {
      P1OUT &= ~AB;
      P1OUT |= CLK;                
      P1OUT &= ~CLK; 
      Dat_Ser <<= 1;
      if(a == 4)
      {
        P2OUT |= E;  
        __delay_cycles(640);
        P2OUT &= ~E; 
        __delay_cycles(640);
      }
    }
  }
  P2OUT |= E; 
  __delay_cycles(640);
  P2OUT &= ~E;
  __delay_cycles(640);
}

void DCO_16MHz_REFO()
{
//  MSP430FR243x Demo - Configure MCLK for 16MHz operation, and REFO sourcing
//                                     FLLREF and ACLK. 
//
//  Description: Configure MCLK for 16MHz. FLL reference clock is REFO. At this 
//                    speed, the FRAM requires wait states. 
//                    ACLK = default REFO ~32768Hz, SMCLK = MCLK = 16MHz.

    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    __bis_SR_register(SCG0);                           // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source
    CSCTL0 = 0;                                        // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);                            // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_5;                               // Set DCO = 16MHz
    CSCTL2 = FLLD_0 + 487;                             // DCOCLKDIV = 16MHz
    __delay_cycles(3);  
    __bic_SR_register(SCG0);                           // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked
    
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;         // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                                       // default DCOCLKDIV as MCLK and SMCLK source
}
#endif

