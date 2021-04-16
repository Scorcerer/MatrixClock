#include <SPI.h>
#include <MaTrix.h>
#include <Time.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <Timezone.h>
#include <SoftwareSerial.h>
//#include <IRremote.h>

// Матрица
MaTrix mymatrix;

extern unsigned char font5x8[];
extern unsigned char font6x8[];
extern unsigned char digit6x8bold[];
extern unsigned char digit6x8future[];
extern byte array[8][8];
extern byte shadow[8][8];

int brightLmax;
int brightLcur;
byte brightL;

char unixString[11];
long unixTime;
boolean dataSync = false;

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

TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 3, 60};
Timezone myTZ(myDST, mySTD);

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200);
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
  
  // Пищалка 
  tone(45, 2000, 100); // подключена к 45 пину
}

void loop(){
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
        processSyncMessage();
        break;

      case 1:
        sprintf(buff, "%s", wDay[weekday()-1]);
        wD = " "+String(buff);
        mymatrix.printRunningString(wD, GREEN, font6x8, FAST);
        processSyncMessage();
        break;

       case 2:
        sprintf(buff, "%2d %s %4d", day(), wMonth[month()-1], year());
        wD = String(buff);
        mymatrix.printRunningString(wD, YELLOW, font6x8, FAST);
        processSyncMessage();
        break;
            
       default:
         break;
  }
  if(count>=2) count = 0; else count++;
}

unsigned long processSyncMessage() {
  char buffer[40];
  int i = 0;
  while (Serial2.available()) {
    buffer[i++] = Serial2.read();
    dataSync = true;
    Serial.print(buffer[i]);
  }
  while (Serial2.available()) {
    buffer[i++] = Serial2.read();
    dataSync = true;
  }

  // if data is available, parse it
  if (dataSync == true) {
    if ((buffer[0] == 'U') && (buffer[1] == 'N') && (buffer[2] == 'X')) {
      // if data sent is the UNX token, take it
      unixString[0] = buffer[3];
      unixString[1] = buffer[4];
      unixString[2] = buffer[5];
      unixString[3] = buffer[6];
      unixString[4] = buffer[7];
      unixString[5] = buffer[8];
      unixString[6] = buffer[9];
      unixString[7] = buffer[10];
      unixString[8] = buffer[11];
      unixString[9] = buffer[12];
      unixString[10] = '\0';

      // print the UNX time on the MEGA serial
      Serial.println();
      Serial.print("TIME FROM ESP: ");
      Serial.print(unixString[0]);
      Serial.print(unixString[1]);
      Serial.print(unixString[2]);
      Serial.print(unixString[3]);
      Serial.print(unixString[4]);
      Serial.print(unixString[5]);
      Serial.print(unixString[6]);
      Serial.print(unixString[7]);
      Serial.print(unixString[8]);
      Serial.print(unixString[9]);
      Serial.println();

      unixTime = atol(unixString);
      time_t localTime = myTZ.toLocal(unixTime);
      // Synchronize the time with the internal clock
      // for external use RTC.setTime();
      setTime(localTime);
      tmElements_t tm;
      breakTime(localTime,tm);
      RTC.write(tm);
    }
  }
}

void code(){
  // автоматическая регулировка яркости в зависимости от освещенности
  brightLcur = analogRead(LightSENS);
  if(brightLcur > brightLmax) {
    brightLmax = brightLcur;
  }
  brightL = map(brightLcur, 0, brightLmax, 20, 255);
  mymatrix.brightness(brightL);  
}
