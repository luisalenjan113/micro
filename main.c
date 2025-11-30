#include <driverlib.h>
#include <LCD16x2fr16MHz.h>

// ============================
//   CONSTANTES PARA PWM
// ============================
#define PWM_RANGE 9999              // TOP del timer (TAxCCR0)
#define MAX_VAL_SERIAL 255
#define PWM_FACTOR (PWM_RANGE / MAX_VAL_SERIAL)

unsigned char i=0;
unsigned int L = 0, DATO = 0, numero=0;
unsigned int CI=0, D=0, U=0;

// ===== Variables del LCD =====
unsigned int RCI=48, RD=48, RU=48;
unsigned int GCI=48, GD=48, GU=48;
unsigned int BCI=48, BD=48, BU=48;

extern void DCO_16MHz_REFO(void);

// ============================
//   ACTUALIZA LCD
// ============================
void actualizarLCD(void)
{
    Cmd_Lcd(0x80);

    Dato_Lcd('R'); Dato_Lcd(RCI); Dato_Lcd(RD); Dato_Lcd(RU); Dato_Lcd(' ');
    Dato_Lcd('G'); Dato_Lcd(GCI); Dato_Lcd(GD); Dato_Lcd(GU); Dato_Lcd(' ');
    Dato_Lcd('B'); Dato_Lcd(BCI); Dato_Lcd(BD); Dato_Lcd(BU); Dato_Lcd(' ');
}

// ============================
//   PROGRAMA PRINCIPAL
// ============================
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    DCO_16MHz_REFO();

    Ini_Lcd();
    actualizarLCD();

    // ============================
    //      CONFIG UART (COMUICACION SERIAL)
    // ============================
    UCA0CTLW0 = UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0BRW = 104;
    UCA0MCTLW = UCOS16 | UCBRF_2 | 0xD600;

    P1SEL1 &= ~BIT5;
    P1SEL0 |= BIT5;

    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE;

    // ============================
    //    LIMPIAR RESISTENCIAS
    // ============================
    P1REN &= ~(BIT1 | BIT2 | BIT4);
    P1OUT &= ~(BIT1 | BIT2 | BIT4);
 // ====================================
    //    PWM – TIMERS A0 y A1
    // ============================
    //    PWM – TIMER A0 (G/B)
    // ============================
    TA0CTL = TASSEL__SMCLK | ID__8 | MC__UP;//TEMPORIZAODR PRINCIPAL 16MHZ / div 8=2Mhz /
    TA0EX0 = TAIDEX_1;                      //se  div poe 2 mas
    TA0CCR0 = PWM_RANGE;//TEMPORIZADOR REAL

    TA0CCTL1 = OUTMOD_7;    // Azul: P1.1
    TA0CCR1  = PWM_RANGE; //apaga el led al iniciar

    TA0CCTL2 = OUTMOD_7;    // Verde: P1.2
    TA0CCR2  = PWM_RANGE; 

    P1DIR   |= BIT1 | BIT2;
    P1SEL0 &= ~(BIT1 | BIT2);
    P1SEL1 |=  (BIT1 | BIT2);

    // ============================
    //    PWM – TIMER A1 (R)
    // ============================
    TA1CTL = TASSEL__SMCLK | ID__8 | MC__UP;//TEMPORIZAODR SECUNDARIO
    TA1EX0 = TAIDEX_1;
    TA1CCR0 = PWM_RANGE;//TEMPORIZADOR REAL

    TA1CCTL2 = OUTMOD_7;    // Rojo: P1.4
    TA1CCR2  = PWM_RANGE; 

    P1DIR |= BIT4;
    P1SEL0 &= ~BIT4;
    P1SEL1 |= BIT4;

    // ============================
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(1) { }
}

// ==========================================================================
//   UART RX – LÓGICA INVERTIDA ÁNODO COMÚN (LLEGADA DE LA INFO POR SERIAL)
// =========================================================================
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    DATO = UCA0RXBUF;//Aquí se lee cada byte que llega por la consola serial

    if(DATO == '\r' || DATO == '\n')
        return;
    i++;

    if(i == 1)
        L = DATO;//R,G o B
    else if(i == 2)
        CI = DATO;//RX
    else if(i == 3)
        D = DATO;//RXX
    else if(i == 4)
    {
        U = DATO;//RXXX

        // ------ Conversión ASCII → número (0–255) ------
        numero = (CI - '0')*100 + (D - '0')*10 + (U - '0');
        if (numero > 255) numero = 255;

        // ===========================================
        //      PWM INVERTIDO + APAGADO REAL 
        // ===========================================
//CALCULO DE PWM
        unsigned int pwm_val;

        if (numero == 0) {
            pwm_val = PWM_RANGE; //=9999=ccr0
        } 
        else if (numero >= 255) {
            // Máximo brillo (CCR = 0)
            pwm_val = 0;           
        } 
        else {
            // Cálculo para valores intermedios (1 a 254)
            // Se usa (PWM_RANGE - 1) para el rango intermedio para evitar desbordes
            pwm_val = (PWM_RANGE - 1) - (PWM_FACTOR * numero);
        }

        // ===== ACTUALIZAR CANAL =====
        if(L == 'B' || L == 'b')
        {
            TA0CCR1 = pwm_val; //AZUL TIMER A0
            BCI = CI; BD = D; BU = U;
        }
        else if(L == 'G' || L == 'g')
        {
            TA0CCR2 = pwm_val;//VERDE TIMER A0 
            GCI = CI; GD = D; GU = U;
        }
        else if(L == 'R' || L == 'r')
        {
            TA1CCR2 = pwm_val;//ROJO TIMER A1
            RCI = CI; RD = D; RU = U;
        }

        actualizarLCD();
        i = 0;
    }
}