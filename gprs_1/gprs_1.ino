////////////////////////CONSTANTES///////////////////
#define PHMAX 3000
#define PLMAX 1000 
#define EQID  2


///////////////////////////////////////////////////////7

#include <Ultrasonic.h>
#include <SoftwareSerial.h>

#define CHARCMD 128
#define ENTER 13

int leer_entradas();
/////menu///////////

////////////////////



char mensaje[30]; //variavle para poner el mensaje global

Ultrasonic ultrasonic(12,13,6000); // (Trig PIN,Echo PIN) 103 cm
SoftwareSerial mySerial(6,7);


//////////////////////////////////////////////////////// SETUP ////////////////////////////////////////////////////////////
void setup(){
 

mySerial.begin(19200); //puerto serie por software para comunicacion con modem
Serial.begin(19200);   // serie rx-tx 0,1 




}

//////////////////////////////////////////////////// LOOP ////////////////////////////////////////////////////////////////

void loop(){

  
//LEER ENTRADAS ANALOGICAS,DIGITALES y retornar mensaje 

delay(1000);

connectUDP();

leer_entradas();

sendUDP(mensaje);
    
 


powerUpOrDown();


}




///////////////////////////////////////////////////////PROTOTIPOS FUNCIONES //////////////////////////////////////////////////////
int leer_entradas(){
 
 ////////////////////////////////////////////////////DEFINIMOS PINES ANALOGICOS /////////////////////////////////////////////////
  int nivelPin   =0;
  int presureHPin=2;
  int presureLPin=3;
  //int D3Pin      =3;
  //int D4Pin      =4;
  //int D5Pin      =5;

 ////////////////////////////////////////////////////////SETEAMOS VARIABLES ANALOGICAS ///////////////////////////////////////// 
  int nivel_t=103;
  int nivelVal   =0;
  int presureHVal=0;
  int presureLVal=0;

/////////////////////////////////////////////////////////// DEFINIMOS PINES DIGITALES //////////////////////////////////////////
  int motorPin=2;

////////////////////////////////////////////////////////// SETEAMOS VARIABLES DIGITALES ///////////////////////////////////////
  int motorState=LOW;
  
 ///////////////////////////////////////////////////////// DEFINIMOS VARIABLE PARA MENSAJE ///////////////////////////////////
  float fph;
  int iph;

  float fpl;
  int ipl;
    

  pinMode(motorPin, INPUT);
  //////////////////////////////////////////////////////// leer valores ///////////////////////////////////////////////////// 
   motorState= digitalRead(motorPin);
   delay(10);
   nivelVal=ultrasonic.Ranging(CM);
   delay(10);
   presureHVal= analogRead(presureHPin);
   delay(10);
   presureLVal= analogRead(presureLPin);
   
  fph=(((float)presureHVal/1024.0f))*PHMAX;  // ADC calibration
  iph=( int)fph;
  
  fpl=(((float)presureLVal/1024.0f))*PLMAX;  // ADC calibration
  ipl=( int)fpl;  


  sprintf(mensaje,"%d,%d,%d,%d,%d",EQID,motorState,nivelVal,ipl,iph);
   //delay(10000);
   Serial.println(mensaje); 
   return 0;
   
}
//end leer entradas


/////////////////////////////////////////////////////////////////CONECXION//
int connectUDP()
{
 
 mySerial.println( "AT");
 delay(3000);
 ShowSerialData(); 
 
 mySerial.println( "ATE1");
 delay(3000);
 ShowSerialData(); 
 

 
 mySerial.println( "AT+CSQ");
 delay(3000);
 ShowSerialData(); 

 mySerial.println( "AT+CGATT=1");
 delay(3000);
 ShowSerialData(); 
  
 mySerial.println("AT+CGDCONT=1,\"IP\",\"internet.ctimovil.com.ar\"");
 delay(3000);
 ShowSerialData(); 
 
 mySerial.println("AT+CSTT=\"internet.ctimovil.com.ar\",\"cti\",\"cti\"");
 delay(3000);
 ShowSerialData();
 
 

 mySerial.println("AT+CIICR");
 delay(3000);
 ShowSerialData();
 


 mySerial.println("AT+CIFSR");
 delay(3000);
 ShowSerialData();
 

 mySerial.println("AT+CIPSTART=\"UDP\",\"xxxyyy.sytes.net\",\"17722\"");
 delay(3000);
 ShowSerialData();




}


//////////////////////////////////////////////////////////////////send udp packet to server ////////////////////////////// 
void sendUDP(char *mensaje)
{
 
   mySerial.println("AT+CIPSEND"); 
   delay(100);
   ShowSerialData();
   
   mySerial.println(mensaje);
   delay(100);
   ShowSerialData();
   mySerial.println((char)26);
   delay(1000);
   ShowSerialData();
    
}


void powerUpOrDown()
{
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}





void ShowSerialData()
{
  while(mySerial.available()!=0)
    Serial.write(mySerial.read());
}

