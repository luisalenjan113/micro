//Servomotor con sensor de proximidad

#include <driverlib.h>
#include <LCD16x2fr16MHz.h>
unsigned int ciclo_activo = 0, distancia = 0, d=0, angulo=0, PWM = 0, ang=0, c=0;
unsigned char bandera = 0;
void f_rango();
int main(void){
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);
    DCO_16MHz_REFO();
    Ini_Lcd();

    TA0CTL = TASSEL__SMCLK + ID__8 + MC__UP;
    TA0EX0 = TAIDEX_1; // AQUI VOLVEMOS A DIVIDIR CON TAIDEX SE DIVIDE EN 2, AQUI=2 ENTRE 2

    // para flancos TA0.1 P1.1
    TA0CCTL1 = CM_3 + CAP + CCIE;
    // pulso de para el sensor TA0.2 P1.2
    TA0CCR2 = 0xFFFF; // 60ms
    TA0CCTL2 = OUTMOD_7;
    TA0CCR2 = 10;
    // PWM para servomotor (TA1.2)
    TA1CTL = TASSEL__SMCLK + ID__8 + MC__UP; // Aquí se dice que timer y como se estara usando
    TA1EX0 = TAIDEX_1;
    TA1CCR0 = 20000; // 20ms
    TA1CCTL1 = OUTMOD_7;
    TA1CCR1 = 500; 
    // Angulo del servo

        P1DIR &= ~BIT1;
        P1DIR |= BIT2 + BIT5;
        P1SEL0 &= ~(BIT1+BIT2+BIT5);
        P1SEL1 |= (BIT1+BIT2+BIT5);

        PMM_unlockLPM5();
        __bis_SR_register(GIE);
        while (1) {

            TA1CCR1 = PWM; // P1.5
            if (distancia > 400) f_rango();
            else{
                Cmd_Lcd(0x80);
                Dato_Lcd(0x44);
                Dato_Lcd(0x69);
                Dato_Lcd(0x73);
                Dato_Lcd(0x3A);
                           d= distancia;
     Dato_Lcd(((d/100)%10 + '0'));
     Dato_Lcd(((d/10)%10 + '0'));
     Dato_Lcd(((d%10) + '0'));
     Dato_Lcd(0x63);
     Dato_Lcd(0x6D);

 Cmd_Lcd(0xC0);// Segunda Linea
                Dato_Lcd(0x41);
                Dato_Lcd(0x6E);
                Dato_Lcd(0x67);
                Dato_Lcd(0x3A);
                ang = angulo;
     Dato_Lcd(((ang/100)%10 + '0'));
     Dato_Lcd(((ang/10)%10 + '0'));
     Dato_Lcd(((ang%10) + '0'));
                Dato_Lcd(0x20);
                Dato_Lcd(0x20);
                Dato_Lcd(0x43);
                Dato_Lcd(0x41);
                Dato_Lcd(0x3A);
                  c = ciclo_activo;
     Dato_Lcd(((c/1000)%10 + '0'));
     Dato_Lcd(((c/100)%10 + '0'));
     Dato_Lcd(((c/10)%10 + '0'));
     Dato_Lcd(((c%10) + '0'));
          
            }
        }
    }

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Captura(void)
{
    switch (TA0IV){
        case 2: if (bandera == 0){
            TA0R = 0;
            bandera = 1;
        }else{
            ciclo_activo = TA0CCR1;
            distancia = ciclo_activo/58; // se divide y ya tenemos en cm
            angulo = (distancia*0.45); // Angulo
            PWM = 5*distancia+500; // Pulso a servo 0° ---> 500us, 90° ---> 1500us y 180° ---> 2500us
            bandera = 0;
        }
        break;
        case 4: break;
        case 14: break;
    }
}

void f_rango()
{
    Cmd_Lcd(0x80);
     Dato_Lcd(0x65);
    Dato_Lcd(0x72);
    Dato_Lcd(072);
    Dato_Lcd(0x6F);
    Dato_Lcd(0x72);
     Dato_Lcd(0x20);
      Dato_Lcd(0x20);
   


    char w;
  Cmd_Lcd(0xC0);
    for(w=0; w<16; w++) Dato_Lcd(0x20);
}