#include <wiringPi.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>

#define ENC_ERROR 0 //valores de error como 0
#define PI 3.141592

int hall1_pin = 0; //pin lectura hall 1, cable azul
int hall2_pin = 1; //pin lectura hall 2, cable amarillo

int pulsador_pin = 2; //pin lectura pulsador de posición cero

int in1 = 3; //pines control giro
int in2 = 4;

int PWM_pin = 12 ; //pin control velocidad

float t_estabilizacion = 5000; //tiempo estabilizacion sonda en ms
float diam_bobina=0.06;

int inc, previoA, previoB, actualA,actualB, ind,pulsos, vueltas, pulsador, d;
float d_sonda;

int M_inc[16]={0,-1,1,0,
	1,ENC_ERROR,ENC_ERROR,-1,
	-1,ENC_ERROR,ENC_ERROR,1,
	0,1,-1,0}; //matriz de pulsos de encoder según salidas hall
 //+1 pulso sentido horario
 //-1 pulso sentido antihorario
 //0 no hay movimiento
 
 void fgiro(int giro){
	 if(giro==1){ //bajar
		 digitalWrite(in1,0);
		 digitalWrite(in2,0);
		 delay(100);
		 digitalWrite(in1,0);
		 digitalWrite(in2,1);
	 }
	 if(giro==-1){ //subir
		 digitalWrite(in1,0);
		 digitalWrite(in2,0);
		 delay(100);
		 digitalWrite(in1,1);
		 digitalWrite(in2,0);
	 }
	 if(giro==0){ //parar
		 digitalWrite(in1,0);
		 digitalWrite(in2,0);
	 }
 }
 
 PI_THREAD(contador_vueltas){
	 pulsador = digitalRead(pulsador_pin);
	 pulsos = 0; //inicializar a cero
	 vueltas = 0;
	 actualA = digitalRead(hall1_pin); //primera lectura
	 actualB = digitalRead(hall2_pin);
	 
	 for(;;){
		 previoA = actualA; //actualización
		 previoB = actualB;
		 
		 pulsador = digitalRead(pulsador_pin);
		 if(pulsador==1){ //sonda en posición cero
			 fgiro(0);
			 pulsos = 0; //inicializa a cero el encoder
			 vueltas = 0;
		 }
		 
		 actualA = digitalRead(hall1_pin); //lectura actual
		 actualB = digitalRead(hall2_pin);
		 ind = 8*previoA+4*actualA+2*previoB+actualB; //cálculo índice matriz
		 inc = M_inc[ind]; //incremento 
		 pulsos +=inc; //pulsos totales 
		 vueltas = pulsos/490; //cálculo vueltas
	 }
 }
 
float vec_vueltas[10]; //vueltas para llegar a cada posición
int vec_dist[10] = {1,2,3,4,5,6,7,8,9,10}; // diferentes distancias a elegir
int i;
 
 void rutina(float d_sonda){ //bajar hasta d (m) parando en intervalos de 1m y volver a cero
	 //primero comprobar que es posible bajar
	 if(d_sonda<=1){
		 d=0;
		 printf("No es posible medida.\n");
	 }
	 else{
		 d=floor(d_sonda-0.5); //cálculo profundidad de bajada
		 
		 //primero se va a colocar el sensor en posición cero
		digitalWrite(in1,0); //se garantiza que el motor esté parado
		while(pulsador==0){ //mientras no se pulse, sube
			digitalWrite(in1,1);
			digitalWrite(in2,0);
		} 
		digitalWrite(in1,0);
		digitalWrite(in2,0); //se para el motor
		printf("Sensor en posición cero.\n");
		delay(1000);
		
		i=0;
		while(i<d){ // va bajando metro a metro
			 
			while(vueltas<vec_vueltas[i]){ //baja 1m
				digitalWrite(in1,0);
				digitalWrite(in2,1);
			}
			digitalWrite(in1,0); //para
			digitalWrite(in2,0);
			printf("Ha llegado a %i m.\n",vec_dist[i]); //pone en pantalla a qué profundidad llegó
			delay(t_estabilizacion); //espera 5s
			 
			i++;
		 }
		 //vuelta a cero
		 while(pulsador==0){ //mientras no se pulse, sube
			 digitalWrite(in1,1);
			 digitalWrite(in2,0);
		 }
		 delay(2000); //espera 2s
		 printf("Sensor en posición cero.\n");
		 //para no gastar se baja el sensor una vuelta de cuerda
		 while(vueltas<1){
			 digitalWrite(in1,0);
			 digitalWrite(in2,1);
			 printf("Vueltas %i \n",vueltas);
		 }
		 digitalWrite(in1,0); //para
		 digitalWrite(in2,0);
		 printf("Fin de rutina.\n");
	 }
 }
 
 int j=0;
 
 void main(){
	 wiringPiSetup();
	 pinMode(hall1_pin,INPUT);
	 pinMode(hall2_pin,INPUT);
	 pinMode(pulsador_pin,INPUT);
	 pinMode(in1, OUTPUT);
	 pinMode(in2,OUTPUT);
	 pinMode(PWM_pin,PWM_OUTPUT);
	 
	 //Cálculo de parámetros necesarios
	 while(j<10){
		 vec_vueltas[j] = (j+1)/(PI*diam_bobina); //cada profundidad entre el perimetro
		 j++;
	 }

	 piThreadCreate(contador_vueltas);
	 rutina(1.5);
	 
 }
	 
