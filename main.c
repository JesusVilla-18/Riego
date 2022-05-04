#include <msp430g2553.h>
/**
 * Código para un sistema de riego automático con protección de bomba
 *
 * Materiales:
 *  Micro
 *  Jumpers
 *  Sensor de humedad en suelo
 *  Bomba
 *  Buzzer
 *  Sensor ultrasónico
 */

//Sobrecargo funciones
void Sonido();

//Asignación de variables
unsigned int humedad;         //Lo que mida el sensor de humedad
unsigned int minhumedad=800; //El valor que diferencía la tierra seca de la húmeda
unsigned int distancia=20;   //Resultado del sensor de distancia (inicializado en 20 para la 1° vez del código)
unsigned int dmin=11,dmed=6; //Valores intermedios fijos de la distancia del agua
unsigned int tiempo; // resultado del sensor de distancia
unsigned char inicio = 0;

void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//Bomba
	P2DIR |= BIT0|BIT1;              //Activar la bomba en el puerto 2.0 y buzzer 2.1

	//Ultrasonico
	 P1DIR |= BIT1|BIT4;       //Salidas
	 P1SEL |= BIT1;            //Seleccionar función TA0CLK

	 //HABILITO INTERRUPCION
     P1REN |= BIT5;
     P1OUT = 0; //aseguro el pin5 en 0 (pull-down) y apago leds
     P1IE |= BIT5;
     P1IES &= ~BIT5; // interrupcion en flanco de subida, de 0 a 1
     _BIS_SR(GIE);

     TACCR0 = 50000 ; // base de periodo aprox 50ms (tempo de echo 38ms si no hay ningun objeto)
     TACCR1 = 25000 ; // no es necesaria una salida del TimerA, es para verificar funcionamiento
     TACCTL0 = OUTMOD_3; // set cada que match a TACCR1 y reset cada que match a TACCR0

	//ADC10
	ADC10CTL0 = ADC10SHT_2 + ADC10ON; // SHTime16xADC10CLKs, ADC10ON
	ADC10CTL1 = INCH_3;               // input A3, PIN 5
	ADC10AE0 |= 0x08;

	while(1)
	{
	    ADC10CTL0 |= ENC + ADC10SC;     // Habilitar conversion, iniciar conversion
	    humedad = ADC10MEM;             // Guardar conversión en humedad

	    //Trigger Ultrasónico
	    P1OUT &= ~BIT4;
        _delay_cycles(4); // 4 us en bajo para asegurar el envio del pulso de 10uS (TRIGGER)
        P1OUT |= BIT4;
        _delay_cycles(10); // 10 us en alto, valor del pulso
        P1OUT &= ~BIT4;

	    if (humedad > minhumedad)   //V = tierra seca -> Regar     F = tierra húmeda -> No regar
	    {
	        if (distancia <= dmed) //Contenedor (casi) lleno -> 5 segs
	        {
	            P2OUT |= BIT0;
	            _delay_cycles(5000000);
	            P2OUT &= ~BIT0;
	        }else if (distancia < dmin && distancia > dmed) //Contenedor (casi) lleno -> 2.5 segs
	        {
	            P2OUT |= BIT0;
	            _delay_cycles(2500000);
	            P2OUT &= ~BIT0;
	        }else //Contenedor (casi) vacío -> 0 segs
	        {
	            Sonido();  //Buzzer de aviso
	            _delay_cycles(50000);
	        }
	        P2OUT &= ~ BIT0|BIT1;
	    }
	}
}

void Sonido(){
    int i;
    for (i=0;i<2;i++){
        P2OUT |= BIT1;
        _delay_cycles(250000);
        P2OUT ^= BIT1;
        _delay_cycles(45000);
        P2OUT ^= BIT1;
        _delay_cycles(250000);
        P2OUT ^= BIT1;
        _delay_cycles(45000);
        P2OUT ^= BIT1;
        _delay_cycles(250000);
        P2OUT ^= BIT1;
        _delay_cycles(45000);
        P2OUT ^= BIT1;
        _delay_cycles(750000);
        P2OUT ^= BIT1;
        _delay_cycles(45000);
        P2OUT ^= BIT1;
        _delay_cycles(750000);
        P2OUT ^= BIT1;
        _delay_cycles(45000);
        P2OUT ^= BIT1;
    }
    P2OUT &= ~ BIT0|BIT1;
}


#pragma vector = PORT1_VECTOR
__interrupt void interrupcion_por_puerto1 (void)
{
        P1IFG &= ~BIT5;

        if(inicio==0){
        TACTL |= TASSEL_2 + MC_1 + TACLR; // fuente SMCLK, modo UP, clr TA
        inicio=1;
        P1IES |= BIT5; //flanco de bajada, interrupcion de 1 a 0
        }
        else{
        TACTL |= MC_0;
        tiempo=TA0R;
        distancia = tiempo*0.017123 ; // =tiempo/58
        P1IES &= ~BIT5; //flanco de subida, interrupcion de 0 a 1
        inicio=0;
        }
//_delay_cycles(100000);
}
