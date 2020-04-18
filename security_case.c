#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "Hardware.h"

#define INICIO 0
#define CERRADO 1
#define ABIERTO 2
#define ROBO 3
	/*--------------------VARIABLES GLOBALES----------------------*/
	//const unsigned char;
	char d;
	char e;
	char aux;
	char ok=0,x=0;
    void check (void);
	void ADC_Piezo(void);
	void ADC_Temp(void);
	void ADC_Peso(void);
	void Transmision(void);
	char enter=0;
	unsigned char memoria_n=0;
	unsigned int apertura=0;
	unsigned int grados;
	unsigned int kilos;
	unsigned int contador1s=0;
	unsigned char enviar=0;
	unsigned char i=0;
	unsigned char n=0;
	char n_actual=0;
	char ap=0;
	char n1=0;
	char n2=0;
	char n3=0;
	char n4=0;
	char peso_actual=0;
	unsigned char memoriapeso;
	char temp_actual;
	char t1,t2;
	char tempmax=70;
	unsigned char memoriatemp;
	char estado=INICIO;
	char estado_cerrado=0;
	char contador_teclado=0;
	unsigned char piezo_actual;
	unsigned char memoriapiezo;
	
	char tramatx[]="<00C0000KGA0US>";//TEMP ACTUAL//TEMP MAX//PESO ACTUAL//ESTADO ALARMA//U->USUARIO//S->STATUS
	int trama_display=0b00000000; 
	/*  Vectores de Trama:	1Ã‚ÂªVector -> Primer bit Cerradura, Segundo Alarma (El resto en 0)	
	 * 						2Ã‚Âª y 3Ã‚Âª Vector -> Letras (a,b,c,d,etc) del display alfanumerico
	 * 						4Ã‚Âª Vector -> Primeros 4 bits seleccion de Catodo, El resto no se usan	 
	 */    

	
	
	
	int numeroshead[]={0b11111111,0b00110000,0b11101110,0b11111100,0b00110001,0b11011101,0b11011111,0b11110000,0b11111111,0b11110001,0b11001111,0b01100000};//Primera parte de los numero 1,2,3,4,5,6,7,8,9,0
	int numerostail[]={0b00000000,0b00000000,0b00010001,0b00010000,0b00010001,0b00010001,0b00010001,0b00000000,0b00010001,0b00010001,0b00000000,0b01010000};//Segunda parte de los numero 1,2,3,4,5,6,7,8,9,0
	int catodos[]={0b10000000,0b01000000,0b00100000,0b00010000};
	/*------------------------------------------------------------*/

void main(void)
{
	SOPT1 = SOPT1 & 0x3F; // deshabilito COP
	//registros adc
		ADCCFG = 0b01000000; 	// fadc clock: fbus / 4 = 2MHz/ registro del timer
		ADCSC2 = 0; 			//registro comparador
		APCTL1 = 0b01000000; // Entrada para ADC: AD6 (pata B2 del SH8)		AGREGAR TODAS LAS PATA ACA	2 REGISTROS
		// se configura SCI - 9600 8N1
		 SCIBD = 52; // divisor = 52 baudrate = 8MHz / 52 / 16 = 9615,38 ? 9600
		 SCIC1 = 0b00000000; // sin paridad
		 SCIC2 = 0b10001000; //  rx encendidos
		 SCIC3 = 0;
	/*------------------------VARIABLES---------------------------*/
			
		
		
	/*------------------------PUERTOS-----------------------------*/
		
		//ENTRADAS
		
		DIR_Entrada_Matricial1=0;
		DIR_Entrada_Matricial2=0;
		DIR_Entrada_Matricial3=0;
		DIR_Peso=0;
		DIR_Piezo=0;
		DIR_Temperatura=0;
		DIR_FinCarrera=0;
		
		//SALIDAS
		
		DIR_RClk=1;
		DIR_Txd=1;
		DIR_Salida_Matricial1=1;
		DIR_Salida_Matricial2=1;
		DIR_Salida_Matricial3=1;
		DIR_Salida_Matricial4=1;
		DIR_Input=1;
		DIR_Clock=1;
		DIR_Clear=1;
		
	/*--------------------------------------------------------*/
	
	
	
	//Mejora de precisiÃƒÂ³n del cristal interno del microcontrolador
	asm {
		LDA $FFAF
		STA ICSTRM
	}
	while(!ICSSC_IREFST);
	//ConfiguraciÃƒÂ³n Timer
	//Usando un XTAL interno de 8MHz (o sea, Fbus = 8MHz)
	//Configurado a 20ms
	//TMOD = 1250 y Prescaler = 16
	TPM1MOD = 1500;
	TPM1SC = 0b01001100; //ÃƒÅ¡ltimos 3 nÃƒÂºmeros --> Prescaler
	PTASC_PTAIE=0;
		PTAES=0b00000111;
		PTAPE=0;
		PTAPS_PTAPS0=1;
		PTAPS_PTAPS1=1;
		PTAPS_PTAPS2=1;
		//PTAPS=0b00000111;
		PTASC_PTAACK=1;
		PTASC_PTAIE=1;
		Clear=1;
	EnableInterrupts;
	
	
	
	
	
	
	
	for (;;)
	{
		
		switch(estado)
		{
		case INICIO:
			
			/* Estados dento de INICIO*/
		
			
			ADCSC1 = 0b00100011; // Entrada para ADC: AD3
				
				if (ADCSC1_COCO == 1)
				{
					memoriapeso=ADCRL;
					estado=CERRADO;
					estado_cerrado=0;
				}
			
			
			break;
			/*------------------------------*/
			
		case CERRADO:
			tramatx[13]='C';
			trama_display=0b01000000;
			/*Estados dentro de CERRADO*/
			switch(estado_cerrado)
			{
			case 0: 
				ADC_Piezo();
			break;
			case 1:
				ADC_Temp();
				break;
			case 2:
				ADC_Peso();
				break;
			case 3:
				Transmision();
			case 4:
				check();//check de contraseÃƒÂ±a
				break;
			
			}
			break;
			/*----------------------------*/
			
			
			case ROBO:
				tramatx[13]='R';
				trama_display=0b11000000;
				Transmision();
				
				break;
				/*--------------------ABIERTO--------------*/
			case ABIERTO:
				Transmision();
				tramatx[13]='A';
				trama_display=0b00000000;
				if(ap==1)
				{
					
				if(FinCarrera==1)
				{
					estado=INICIO;
				}
				trama_display=0b00000000;

				
					
				
				
				
				
				
				}
				/*-----------------------------------------*/
				break;
				}
				
				
		
		}
		
		
				
		
	}




void ADC_Piezo (void)
{
	
	ADCSC1 = 0b00100110; // Entrada para ADC: AD6 (pata B2 del SH8)			
	if (ADCSC1_COCO == 1) 
				{
					piezo_actual = ADCRL;
					if(piezo_actual>=60)
					{
						estado=ROBO;
					}
					
					estado_cerrado=1;
				}
			
	
}

void ADC_Temp (void)
{
	ADCSC1 = 0b00100111; // Entrada para ADC: AD6 (pata B2 del SH8)
	
	if (ADCSC1_COCO == 1)
	{
		temp_actual = ADCRL;
		if(grados>tempmax)
		{
			estado=ROBO;
		}
		estado_cerrado=2;
	}
			
}


void ADC_Peso (void)
{
	ADCSC1 = 0b00100011; // Entrada para ADC: AD3
	
	if (ADCSC1_COCO == 1)
	{
		peso_actual = ADCRL;
		/*
		if(memoriapeso>150)
		{
		if(((memoriapeso+70)< peso_actual) || ((memoriapeso-70)>peso_actual) )
		{
			estado=ROBO;
		}
		
		
		}
		
		if(memoriapeso<150)
		{
			if(memoriapeso<20)
			{
				
			}
			else
			{
			if(((memoriapeso+50)<peso_actual) || ((memoriapeso-50)>peso_actual) )
			{
				estado=ROBO;
			}
			}
		}
		//if(((memoriapeso-20)>peso_actual) || (memoriapeso+20<peso_actual))
		//{
			//estado=ROBO;
		//}
			*/	
		if(memoriapeso>25)
		{
		if(peso_actual<25)
		{
			estado=ROBO;
		}
		}
		
		
		
		estado_cerrado=3;
	}
			
}



void Transmision (void)//Ver si no de puede poner en el Timer
{
	//Conversion de los grados
	grados=(temp_actual*92)/255;
	if(grados!=0)
	{
	tramatx[1]=(grados/10) + 0x30;
	tramatx[2]=((grados%10)) + 0x30;
	tramatx[4]=(tempmax/10) + 0x30;
	tramatx[5]= (tempmax%10)+ 0x30;
	}
	else{
		tramatx[1]='E';
		tramatx[2]='R';
		
	}
	//Conversion del peso
	if(peso_actual!=0)//Si el peso no es 0 hace la conversion
	{
	        //pesov=((peso*2375)/255);
	   kilos=(memoriapeso*120)/10;
	}
	        else{
	            kilos=0;
	        }
	tramatx[6]=(kilos/1000) + 0x30;
	tramatx[7]=((kilos%1000)/100) + 0x30;
	//Insercion del estado de la alarma
	if(estado==3)
	{
		tramatx[11]= 1 + 0x30;
	}
	else
	{
	tramatx[11]= 0 + 0x30;
	}
	/*if(enviar==1)
	{
		if (SCIS1_TDRE == 1)
		{
			SCID = tramatx[i];
			i++;
		
		}
		if(i==15)
		{
			i=0;
			enviar=0;
			estado_cerrado=4;
		}
		
	}
	*/

}


void check (void)
{
	

	if(grados>tempmax)
	{
		estado=ROBO;
	}
	
	if(FinCarrera==0)
	{
		estado=ROBO;
	}
	
	
	
	estado_cerrado=0;
}

__interrupt 18 void tx_int()
{
	(void)SCIS1; // Limpia flag TDRE
	
	SCID = tramatx[i++]; // Transmite siguiente byte
	if (i == 15){ // Si se transmitieron todos los bytes
	i = 0;
	
	SCIC2_TIE = 0;
	
	
}
}	



__interrupt 11 void Timer_Overflow(void)
{
	if(x==1)
		{
		if(ok==0)
		{
		if(contador_teclado==0)
		{
			Salida_Matricial1=1;
			Salida_Matricial2=0;
			Salida_Matricial3=0;
			Salida_Matricial4=0;
		}
		if(contador_teclado==1)
		{
			Salida_Matricial1=0;
			Salida_Matricial2=1;
			Salida_Matricial3=0;
			Salida_Matricial4=0;
		}
		if(contador_teclado==2)
		{
			Salida_Matricial1=0;
			Salida_Matricial2=0;
			Salida_Matricial3=1;
			Salida_Matricial4=0;
		}
		if(contador_teclado==3)
		{
			Salida_Matricial1=0;
			Salida_Matricial2=0;
			Salida_Matricial3=0;
			Salida_Matricial4=1;
		}
		}
		}
	
	
	
		
			switch(contador_teclado)
			{
			case 0:
				aux=n1;
				break;
			case 1:
				aux=n2;
				break;
			case 2:
				aux=n3;
				if(estado==ABIERTO)
				{
					aux=11;
				}
				break;
				
			case 3:
				aux=n4;

				if(estado==ABIERTO)
				{
					aux=10;
				}
				break;
			}
			
			
			
			for(e=0;e<8;e++)
			{
				if((catodos[contador_teclado]&(1<<e))!=0)
				{
					Input=1;
				}
				else
				{
					Input=0;
				}
				Clock=1;
				Clock=0;
			}
			for(e=0;e<8;e++)
			{
				if((numerostail[aux]&(1<<e))!=0)
				{
					Input=1;
				}
				else
				{
					Input=0;
				}
					Clock=1;
					Clock=0;
			}
			for(e=0;e<8;e++)
			{
				if((numeroshead[aux]&(1<<e))!=0)
				{
					Input=1;
				}
				else
				{
					Input=0;
				}
					Clock=1;
					Clock=0;
			}
			for(e=0;e<8;e++)
			{
				if((trama_display&(1<<e))!=0)
				{
					Input=1;
				}
				else
				{
					Input=0;
				}
					Clock=1;
					Clock=0;
			}
			
		
			RClk=1;
			RClk=0;
			
	contador_teclado++;
	if(contador_teclado>=4)
		{
			x++;
			contador_teclado=0;
		}
		if(x>=2)
		{
			x=0;
		}
		if(ok==1 && Entrada_Matricial2==0 && Entrada_Matricial3==0 && Entrada_Matricial1==0  )
		{
			ok=0;
		}
	
		contador1s++;
		if(contador1s==334)	
		{
			SCIC2_TIE = 1;
				enviar=1;//Activo transmision a PC 
				contador1s=0;//reinicio contadro de 1 segundo
		}
			if(estado==ABIERTO)
			{
				apertura++;
				if(apertura>=1667)//CAMBIAR TIEMPO A 5 SEGUNDOS
				{
					
					ap=1;//Permite empezar a bloquear 
				}
		
		
			}
	TPM1SC_TOF=0; //ACK de la interrupciÃƒÂ³n del Timer
	}




__interrupt 20 void Interrupt(void) //Interrupcion 20 -> Interrupt puerto A ***INTERRUPCION TERMINADA****
{
	if(ok==0)
			{
		
	
	if(Entrada_Matricial1==1)
	{
		ok=1;
		n_actual++;				
		if(Salida_Matricial1==1)
		{
			n=1;
		}
		if(Salida_Matricial2==1)
		{
			n=4;
		}
		if(Salida_Matricial3==1)
		{
			n=7;
		}
		if(Salida_Matricial4==1)
		{
			n=99;
		}
		
	}
	
	if(Entrada_Matricial2==1)
		{
		ok=1;
		n_actual++;
			if(Salida_Matricial1==1)
			{
				n=2;
			}
			if(Salida_Matricial2==1)
			{
				n=5;
			}
			if(Salida_Matricial3==1)
			{
				n=8;
			}
			if(Salida_Matricial4==1)
			{
				n=0;
			}
			
		}
		
	if(Entrada_Matricial3==1)
		{
		ok=1;
		n_actual++;
			if(Salida_Matricial1==1)
			{
				n=3;
			}
			if(Salida_Matricial2==1)
			{
				n=6;
			}
			if(Salida_Matricial3==1)
			{
				n=9;
			}
			if(Salida_Matricial4==1)
			{
				n=88;
			}
			
			
		}
		
			if(memoria_n==1)
			{
				n_actual=0;
			}
	
			if(n==88)
			{
				enter=1;
				memoria_n=1;
				n_actual=6;
						
			}
			else{
				memoria_n=0;
			}
			switch(estado)
			{
			case ROBO:
			/*-------CERRADO-----------*/
			case CERRADO:
				
			if(n_actual>=4)
				{
					n_actual=0;
				}
			break;
			/*-------ABIERTO-----------*/
			case ABIERTO:
				
				if(n_actual>=2)
				{
					n_actual=0;
				}
				
				break;
			}
		}
	if(n!=88)
	{
			
	if(estado==CERRADO || estado==ROBO)
		{
		switch(n_actual)
			{
				case 0:
					n1=n;
					break;
				case 1:
					n2=n;
					break;
				case 2:
					n3=n;
					break;
				case 3:
					n4=n;
					break;
			}
		}
	
	if(estado==ABIERTO)
	{
		switch(n_actual)//seleccion valores de temperatura
				{
				case 0:
					
					t1=n;
					n1=t1;
					break;
				case 1:
					
					t2=n;
					n2=t2;
					break;
				
				}
				
				
				
		
		
		}
			
	}
	if(enter==1)
	{
		if(estado==ABIERTO)
		{
		tempmax=((t1*10)+t2);
		if(tempmax>70 || tempmax<15)
		{
						
						tempmax=70;
		}
		}
		if(estado==CERRADO)
		{
			
					if(n1==0)
					{
						if (n2==4 && n3==0 && n4==7)
						{
						trama_display=0b00000000;
						estado=ABIERTO;
						n_actual=0;
						apertura=0;
									ap=0;
						tramatx[12]=2+ 0x30;
						}
						if(n2==6 && n3==0 && n4==3)
						{
							trama_display=0b00000000;
							estado=ABIERTO;
							n_actual=0;
							apertura=0;
										ap=0;
							tramatx[12]=4+ 0x30;	
						}
						if(n2==2 && n3==1 && n4==0)
						{
							trama_display=0b00000000;
							estado=ABIERTO;
							n_actual=0;
							apertura=0;
										ap=0;
							tramatx[12]=5+ 0x30;	
							
						}
					}
					if(n1==2)
					{
						if(n2==4 && n3==0 && n4==8)
						{

							trama_display=0b00000000;
							estado=ABIERTO;
							n_actual=0;
							apertura=0;
										ap=0;
							tramatx[12]=1+ 0x30;	
						
						}
						
						
					}
					if(n1==3)
					{
						if(n2==0 && n3==0 && n4==9)
						{

							trama_display=0b00000000;
							estado=ABIERTO;
							n_actual=0;
							apertura=0;
										ap=0;
							tramatx[12]=3+ 0x30;	
						
						}
					}
				
		}
		if(estado==ROBO)
		{
			if(n1==0)
									{
									if (n2==4 && n3==0 && n4==7)
									{

										tramatx[12]=2+ 0x30;
										ok=0;
										x=0;
										n=0;
										apertura=0;
													ap=0;
										memoria_n=0;
										if(FinCarrera==1)
										{
											estado=INICIO;
											estado_cerrado=0;
											
										}
										else
										{
										estado=ABIERTO;
										
										}
										n_actual=4;
										tempmax=70;
										
									}
									if(n2==6 && n3==0 && n4==3)
									{

										tramatx[12]=4+ 0x30;
										ok=0;
										x=0;
										n=0;
										apertura=0;
													ap=0;
										memoria_n=0;
										if(FinCarrera==1)
										{
											estado=CERRADO;
											estado_cerrado=0;
											
										}
										else
										{
										estado=ABIERTO;
										
										}
										n_actual=4;
										tempmax=70;
									}
									if(n2==2 && n3==1 && n4==0)
									{

										tramatx[12]=5+ 0x30;
										ok=0;
										x=0;
										n=0;
										apertura=0;
													ap=0;
										memoria_n=0;
										if(FinCarrera==1)
										{
											estado=CERRADO;
											estado_cerrado=0;
											
										}
										else
										{
										estado=ABIERTO;
										
										}
										n_actual=4;
										tempmax=70;
									}
								}
								if(n1==2)
								{
									if(n2==4 && n3==0 && n4==8)
									{

										tramatx[12]=1+ 0x30;
										ok=0;
										x=0;
										n=0;
										apertura=0;
													ap=0;
										memoria_n=0;
										if(FinCarrera==1)
										{
											estado=CERRADO;
											estado_cerrado=0;
											
										}
										else
										{
										estado=ABIERTO;
										
										}
										n_actual=4;
										tempmax=70;
									}
									
									
								}
								if(n1==3)
								{
									if(n2==0 && n3==0 && n4==9)
									{
										
										tramatx[12]=3+ 0x30;
										ok=0;
										x=0;
										n=0;
										apertura=0;
													ap=0;
										memoria_n=0;
										if(FinCarrera==1)
										{
											estado=CERRADO;
											estado_cerrado=0;
											
										}
										else
										{
										estado=ABIERTO;
										
										}
										n_actual=4;
										tempmax=70;
										
									}
								}
			
			
			
			///////////////////////////////
		/*if(n1==passwd[0] && n2==passwd[1])
		{
			if (n3==passwd[2] && n4==passwd[3])
			{
			ok=0;
			x=0;
			n=0;
			apertura=0;
						ap=0;
			memoria_n=0;
			if(FinCarrera==1)
			{
				estado=CERRADO;
				estado_cerrado=0;
				
			}
			else
			{
			estado=ABIERTO;
			
			}
			n_actual=4;
			}
		}*/
		}
	
		n1=0;
		n2=0;
		n3=0;
		n4=0;
		enter=0;
	}
	

		PTASC_PTAACK=1;
}

