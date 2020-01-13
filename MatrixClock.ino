#include <SPI.h>
#include <MaTrix.h>
#include <Time.h>
#include <Wire.h>
#include <DS1307RTC.h>
//#include <SoftwareSerial.h>
#include <IRremote.h>

// Матрица
MaTrix mymatrix;
//SoftwareSerial xBee(17, 16);

extern unsigned char font5x8[];
extern unsigned char font6x8[];
extern unsigned char digit6x8bold[];
extern unsigned char digit6x8future[];
extern byte array[8][8];
extern byte shadow[8][8];

int brightLmax;
int brightLcur;
byte brightL;

// ИК-приемник
int RECV_PIN = 5; // ИК-приемник в Shield MaTrix подключен к 5 цифровому пину
// не забудьте поправить в файле /IRremote/IRremoteInt.h конфигурацию
//   #define IR_USE_TIMER3   // tx = pin 5
// иначе ИК-команды не будут обрабатываться

IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;
int codeType = -1; 
unsigned long codeValue; 
unsigned int rawCodes[RAWBUF]; 
int codeLen; 
int toggle = 0; 


// дни недели, месяцы
static char wDay[7][13] =
{
  "Niedziela","Poniedzialek","Wtorek","Sroda","Czwartek","Piatek","Sobota"
};
static char wMonth[12][4] =
{
  "Sty","Lut","Mar","Kwi","Maj","Cze","Lip","Sie","Wrz","Paz","Lis","Gru"
};

unsigned long ready;
byte color=GREEN;
byte count=0;
byte effect=3;
//unsigned int pause;
time_t t;

void setup(){
  Serial.begin(38400);
//  xBee.begin(38400);
  Serial.println("Hello, world!");
//  xBee.println("Hello, world!");
  delay(500);
  
  // RTC
  setSyncProvider(RTC.get);
  Serial.println("Waiting for sync message");
  
  // Матрица
  // инициализация 
  mymatrix.init();
  // очистим матрицу
  mymatrix.clearLed();
  mymatrix.brightness(10);
  
  // ИК-приемник
//  irrecv.enableIRIn();
  
  // Пищалка 
  tone(45, 2000, 100); // подключена к 45 пину
}

void loop(){
//  xBee.println("Waiting for sync message");
  String wD;
  char buff[60];
  char tbuf[6];
  char pbuf[6];
    switch(count) {
      case 0:
        sprintf(buff, "%02d%s%02d", hour(),(second()%2)?":":":",minute());
        mymatrix.printString(buff, 4, RED, digit6x8future, int(random(0,5)), VFAST);
        ready=millis()+30000;
        while(millis()<ready){
          sprintf(buff, "%02d%s%02d", hour(),(second()%2)?":":" ",minute());
          mymatrix.printString(buff, 4, RED, digit6x8future);
          delay(10);
        }
        break;

      case 1:
        sprintf(buff, "%s", wDay[weekday()-1]);
        wD = " "+String(buff);
        mymatrix.printRunningString(wD, GREEN, font6x8, FAST);
        break;

       case 2:
        sprintf(buff, "%2d %s %4d", day(), wMonth[month()-1], year());
        wD = String(buff);
        mymatrix.printRunningString(wD, YELLOW, font6x8, FAST);
        break;
            
       default:
         break;
  }
  if(count>=2) count = 0; else count++;
 
code();
}

#define TIME_HEADER  "T"
unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;
    if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}

void code(){
  // автоматическая регулировка яркости в зависимости от освещенности
  brightLcur = analogRead(LightSENS);
  if(brightLcur > brightLmax) {
    brightLmax = brightLcur;
  }
  brightL = map(brightLcur, 0, brightLmax, 20, 255);
  mymatrix.brightness(brightL);  
  
  // обработка ИК-команд
  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume(); 
  }

}
