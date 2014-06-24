// iridiumppx.ino
#include <SoftwareSerial.h>
#include <String.h>
#include <Ultrasonic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <avr/wdt.h>
#include <avr/sleep.h> 
///////////////////////CONSTANTES///////////////////
#define PHMAX 3000
#define PLMAX 1000 
#define EQID  23
#define BEACON_INTERVAL 3600


int leer_entradas();
char mensaje[30]; //variavle para poner el mensaje global
int  MOStatus;
     int  MTStatus;
     char message[272];
     char MOTransferStatus;    
     char MTTransferStatus;
      
     
	  
     void WriteToMOB(char *text);
     void InitiateSBDS();
     void Initialisation();
     void Status();
     int  SignalQuality();
     void Read();
    
     // Functions
     void readAnswer();
     void REG();
     void ClearBuffer(int clearnumber);
 
  char temp_in[20];
char tempreturn;
char clearing   = 0;
char buffclear  = 0;
char readByte   = 0;                                                             
char read       = 0;
char reg        = 0;
int  zaehler    = 0;
char sbdcommand = 0;



Ultrasonic ultrasonic(11,12,6000); // (Trig PIN,Echo PIN) 103 cm
SoftwareSerial mySerial(6, 7);

void setup() {

	//mySerial.begin(19200); // the GPRS baud rate 
 Serial.begin(19200); // the GPRS baud rate 
 delay(500);
 

}

void loop() {

	Initialisation();
int SignalQuality = 0;
	unsigned long loopStartTime = millis();
	

	leer_entradas();
    
    WriteToMOB(mensaje);                         // Write a text message in the Mobile Originated Buffer
    
    InitiateSBDS();                                                         // Sending/Receiving
    
    if(MOTransferStatus == '0')                                             // 0 => No SBD message to send from the 9602
    {
        Serial.println("No message to send");                   
    }
      
    if(MOTransferStatus == '1')                                             // 1 => SBD message successfully sent from the 9602 to the GSS
    {
        Serial.println("Your Message sent successfully!");                   
    }
      
    if(MOTransferStatus == '2')                                             // 2 => An error occurred while attempting to send SBD message from 9602 to GSS
    {
        Serial.println("Your Message couldn't be transmited!");                   
    }
    Serial.println("IRIDIUM-Shield off!");
    delay(500);
    digitalWrite(13,LOW);                                                        // Turn off the Iridium Module
   

    int elapsedSeconds = (int)((millis() - loopStartTime) / 1000);
  //if (elapsedSeconds < BEACON_INTERVAL)
    //bigSleep(BEACON_INTERVAL - elapsedSeconds); // wait about an hour
	bigSleep(3600);
   

}




void Initialisation()
{
  char init_state = 0;
  int sq = 0;

  //pinMode(0, INPUT);                                                            // RX 
  //pinMode(1, OUTPUT);                                                           // TX
  //pinMode(2, INPUT);                                                            // RING
  //pinMode(3, INPUT);                                                            // CTS
  //pinMode(4, OUTPUT);                                                           // DTR
  //pinMode(5, OUTPUT);                                                           // RTS
  //pinMode(6, INPUT);                                                            // DCD
  pinMode(13, OUTPUT);                                                          // Shield_POWER_ON
  digitalWrite(13, HIGH);                                                       // Shield_POWER_ON
  
  mySerial.begin(19200);
  delay(500);
  Serial.println("IRIDIUM-Shield on!");
  delay(500);
  
  sbdcommand = 0;
  while(init_state < 2)
  {
    switch (init_state)
    {
      case 0: mySerial.print("AT\r");                                             // Enable Iridium Module
              readAnswer();
              init_state++;
              break; 
              
      case 1: mySerial.print("ATE0\r");                                           // Disable AT-Echo
              readAnswer();
              init_state++;
              break;
    }
  }

  while(sq < 2)
  { 
    sq = SignalQuality();
    if(sq < 2)
      Serial.print("signal quality not good enough! wait...\r");
  }

  REG();                                                                        // Register with Network
}
  

void readAnswer()
{
  int comma_count = 0;

  read = 0;

  while(read == 0)                                                              // While answer not read
  {
    readByte = mySerial.read();
    
    if(readByte > 0x20)
    {
      //FIFO Iridium-Answer
      temp_in[0]  = temp_in[1];
      temp_in[1]  = temp_in[2];
      temp_in[2]  = temp_in[3];
      temp_in[3]  = temp_in[4];
      temp_in[4]  = temp_in[5];
      temp_in[5]  = temp_in[6];
      temp_in[6]  = temp_in[7];
      temp_in[7]  = temp_in[8];
      temp_in[8]  = temp_in[9];
      temp_in[9]  = temp_in[10];
      temp_in[10] = temp_in[11];
      temp_in[11] = temp_in[12];
      temp_in[12] = temp_in[13];
      temp_in[13] = temp_in[14];
      temp_in[14] = temp_in[15];
      temp_in[15] = temp_in[16];
      temp_in[16] = temp_in[17];
      temp_in[17] = temp_in[18];
      temp_in[18] = temp_in[19];
      temp_in[19] = char(readByte); 
      
    
      // SDBREG
      if((temp_in[12] == 'D') && (temp_in[13] == 'R') && (temp_in[14] == 'E') && 
         (temp_in[15] == 'G'))
      {
        if((temp_in[17] == '2')) 
          reg = 1;                                                              // Registered
        else if((temp_in[17] == '3') && (temp_in[18] == '6'))
          reg = -1;
        else
          reg = 0;                                                              // Detached, Not registered or Registration denied
                                                                                
        read = 1;                                                               // Answer read
      }
      
      // OK
      else if((temp_in[18] == 'O') && (temp_in[19] == 'K'))                     // Answer is 'OK'
      {
        
        if(!sbdcommand)
          read = 1;
      }
      
      // ERROR
      else if((temp_in[16] == 'R') && (temp_in[17] == 'R') &&                   // Answer is 'Error' 
              (temp_in[18] == 'O') && (temp_in[19] == 'R'))                 
      {
        read = 1;
        Serial.print("ERROR\r");
      } 
    
      // SBDI
      else if((temp_in[5] == 'S') && (temp_in[6] == 'B') && 
              (temp_in[7] == 'D') && (temp_in[8] == 'I') && (temp_in[9] == ':'))
      {
        // Example: SBDI:0,14,1,34   
	  	  //               ^    ^                      
        comma_count = 0;
        for(int i = 0; i < 20; i++)
        {
          if(temp_in[i] == ',')
            comma_count++;
  
          if((comma_count == 0) && (temp_in[i] != ','))
          {
            MOTransferStatus = temp_in[i];                                      // MO status
          }
  
          if((comma_count == 2) && (temp_in[i] != ','))
          {       
            MTTransferStatus = temp_in[i];                                      // MT status
          }
        }                                                                          
          
        read = 1;                                                               
      }        
  
      //SBDS
      else if((temp_in[0] == 'S') && (temp_in[1] == 'B') && 
              (temp_in[2] == 'D') && (temp_in[3] == 'S') && (temp_in[4] == ':'))
      {
        // Example: SBDS:0,8,0,21
        //               ^   ^
        comma_count = 0;
        for(int i = 5; i < strlen(temp_in); i++)
        {
          if(temp_in[i] == ',')
            comma_count++;
  
          if((comma_count <= 0)&& (temp_in[i] != ','))
          {
            MOTransferStatus = temp_in[i];                                      // MO status
          }
  
          if((comma_count == 2) && (temp_in[i] != ','))
          {       
            MTTransferStatus = temp_in[i];                                      // MT status
          }
        }    
          
        read = 1;                                                               
      }      
  
      // CSQ:
      else if((temp_in[15] == 'C') && (temp_in[16] == 'S') && 
              (temp_in[17] == 'Q') && (temp_in[18] == ':'))
      {  
        tempreturn = temp_in[19];                                               // Signal Quality     
        read = 1;                                                               
      }                   
  
      // SBSDRT:          
      if(((temp_in[15] == 'D') && (temp_in[16] == 'R') &&                       // Read message from module
          (temp_in[17] == 'T') && (temp_in[18] == ':')) || 
         (zaehler > 0))
      {
        message[zaehler] = temp_in[19];
        zaehler++ ;
        read = 1;
      }
      
      // Clearing Buffer          
      if(clearing)
      {        
        if(temp_in[19] == '0')                                                  // MOB cleared succesfully
        {
          read = 1;
          buffclear = 1;
        }            
        
        if(temp_in[19] == '1')                                                  // Error while clearing
        {
          read = 1;
          buffclear = 0;
        }            
                 
      }
		
		// Display-Delay
		if(read > 0)
		{
          delay(1000);
		} 
    }     
  }                                                                             // End of 'while(readed=0)'
  read=0;
}

void REG()
{
  sbdcommand = 1;
  reg = 0;
  do
  { 
    mySerial.print("AT+SBDREG\r");                                                // Try to register
    readAnswer();  
    if (reg == 0)
    {
      Serial.print("Not connected to the network. retry...\r"); 
      delay(5000);
    }
    else if (reg < 0)
    {
      Serial.print("Not connected to the network. retry in 3 Minutes...\r"); 
      delay(180000);
    }
    else
      Serial.print("Connected\r");
  } while(reg <= 0);                                                            // While not registered
}


void WriteToMOB(char *text)
{
  sbdcommand = 0;
  mySerial.print("AT+SBDWT=");
  mySerial.print(text);                                                    		  // Write Text in MOB
  mySerial.print("\r");                                                    		  
  readAnswer();
}


void InitiateSBDS()                                                        // Initiate session to send and receive
{
  int sq = 0;
  // make sure that the signal quality is good
  while(sq < 2)
  { 
    sq = SignalQuality();
    if(sq < 2)
      Serial.print("signal quality not good enough! wait...\r");
  }
  
  delay(200);
  sbdcommand = 1;
  mySerial.print("AT+SBDI\r");
  readAnswer(); 
  /*   
  if(MOTransferStatus == '1')
    ClearBuffer(0);
   //mySerial.print("AT+SBDD0");                 
*/
}


void Status()
{
  sbdcommand = 1;
  delay(500);
  mySerial.print("AT+SBDS\r");                                                    // SBD Status
  readAnswer();
  MOStatus = (atoi((char*)&temp_in[6]));                                        // Mobile Originated Status (if a message is in the buffer ready to send to the GSS)
  MTStatus = (atoi((char*)&tempreturn));                                        // Mobile Terminated Status (if a message is in the buffer ready to read from the module)
}


int SignalQuality()
{   
  sbdcommand = 1;
  delay(1000);
  mySerial.print("AT+CSQ\r");
  readAnswer();
  
  return atoi((char*)&tempreturn);                                              // Return Signal Quality
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
void Read()
{
  sbdcommand = 1;
  delay(500);
  mySerial.print("AT+SBDRT\r");                                                   // Read message from Mobile Terminated Buffer
  
  zaehler = 0;
  while(zaehler < 273)
  {
    readAnswer();
    if((message[zaehler - 2] == 'O') && (message[zaehler - 1] == 'K'))          // Message end is 'OK'
      zaehler = 273;
  }
  message[zaehler - 1] == ' ';
  message[zaehler - 2] == '\0';
  //ClearBuffer(1);
}

void ClearBuffer(int buffernumber)
{
  buffclear = 0;
  sbdcommand = 1;
  while(!buffclear)
  {
    clearing = 1;
    mySerial.print("AT+SBDD");   
    mySerial.print(buffernumber);                                                 // 0 => Clear Mobile Originated buffer, 1 => Clear Mobile Terminated buffer, 2 => Clear both buffers
    mySerial.print("\r");                                                    		  
    readAnswer(); 

    if(buffclear <= 0)
      Serial.print("buffer clearing failed. retry...\r");
    else
    {
      if(buffernumber < 1)
        Serial.print("mo buffer cleared!\r");
      else
        Serial.print("mt buffer cleared!\r");  
    }
  }
  clearing = 0;
}

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


  sprintf(mensaje,",%d,%d,%d,%d,%d",EQID,motorState,nivelVal,ipl,iph);
   //delay(10000);
   Serial.println(mensaje); 
   return 0;
   
}
//end leer entradas
/*void ShowSerialData()
{
 while(mySerial.available()!=0)
 Serial.write(mySerial.read());
}

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
*/
// Sleep stuff
SIGNAL(WDT_vect) {
  wdt_disable();
  wdt_reset();
  WDTCSR &= ~_BV(WDIE);
}

void babySleep(uint8_t wdt_period) 
{
  wdt_enable(wdt_period);
  wdt_reset();
  WDTCSR |= _BV(WDIE);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
  wdt_disable();
  WDTCSR &= ~_BV(WDIE);
}

void smallSleep(int milliseconds) {
  while (milliseconds >= 8000) { babySleep(WDTO_8S); milliseconds -= 8000; }
  if (milliseconds >= 4000)    { babySleep(WDTO_4S); milliseconds -= 4000; }
  if (milliseconds >= 2000)    { babySleep(WDTO_2S); milliseconds -= 2000; }
  if (milliseconds >= 1000)    { babySleep(WDTO_1S); milliseconds -= 1000; }
  if (milliseconds >= 500)     { babySleep(WDTO_500MS); milliseconds -= 500; }
  if (milliseconds >= 250)     { babySleep(WDTO_250MS); milliseconds -= 250; }
  if (milliseconds >= 125)     { babySleep(WDTO_120MS); milliseconds -= 120; }
  if (milliseconds >= 64)      { babySleep(WDTO_60MS); milliseconds -= 60; }
  if (milliseconds >= 32)      { babySleep(WDTO_30MS); milliseconds -= 30; }
  if (milliseconds >= 16)      { babySleep(WDTO_15MS); milliseconds -= 15; }
}

void bigSleep(int seconds)
{
   while (seconds > 8) { smallSleep(8000); seconds -= 8;	}
   smallSleep(1000 * seconds);
}