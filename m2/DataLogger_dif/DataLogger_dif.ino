/*  oppdr GM high speed capture
 *   
 *   Based on M2 example
 *   DataLogger - non-due-pins and data logger
 *
 *   tmkdev llc
 *  
 *  Using modified Arduino_Due_SD_HSCMI library (https://github.com/macchina/Arduino_Due_SD_HSMCI) from Github user JoaoDiogoFalcao (https://github.com/JoaoDiogoFalcao/Arduino_Due_SD_HSCMI)
 *  
 *  Developed against Arduino IDE 1.8.5
 */

// Including Arduino_Due_SD_HSCMI library also creates SD object (MassStorage class)
#include <Arduino_Due_SD_HSMCI.h> // This creates the object SD
#include "Arduino.h"

// Macchina M2 specific defines for your board
const int SW1 = Button1;     // Pushbutton SW1
const int SW2 = Button2;     // Pushbutton SW2
const int Red =  DS2;       // the number of the RED LED pin
const int Yellow =  DS3;       // the number of the Yellow LED pin
const int Green =  DS6;       // the number of the Green LED pin


FileStore FS;

// Variables
int startlog = 0;
unsigned int i=0;

char fname[10];
String buf = "";
int logging = 0;

#define Serial SerialUSB

void setup() {
  Serial.begin(115200);
  delay(1000); // 1s delay so you have enough time to open Serial Terminal
  // Check if there is card inserted
  SD.Init(); // Initialization of HSCMI protocol and SD socket switch GPIO (to adjust pin number go to library source file - check Getting Started Guide)
  FS.Init(); // Initialization of FileStore object for file manipulation

   
  pinMode(Red, OUTPUT);
  pinMode(Yellow, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  digitalWrite(Red, HIGH);
  digitalWrite(Green, HIGH);
  digitalWrite(Yellow, LOW);

  }


void loop() {
  if ( logging == 0 ) {
    startlog = digitalRead(SW2);
    if ( startlog == LOW ) {
      strcat(fname, "LOG_");
      FS.CreateNew("0:",fname);
      FS.Close();
      logging = 1;
      digitalWrite(Green, LOW);
      delay(50);
    }
  }  else {
    i++;
    buf += "i= " + String(i) + "\n";
    
    if (buf.length() > 2000) {
      Serial.print(buf);
      digitalWrite(Red, LOW); 
      char write_buffer[buf.length()+1]; 
      buf.toCharArray(write_buffer, buf.length()+1); 
            
      FS.Open("0:",fname,true);
      FS.GoToEnd();
      FS.Write(write_buffer); 
      FS.Close();   
      digitalWrite(Red, HIGH);  
      buf = "";
    }
    delay(10); // 0.25s delay so there is not too much data   
  }

  if ( logging == 1 && digitalRead(SW1) == LOW ) {
    logging = 0;
    digitalWrite(Green, HIGH);
    delay(250);
  }
   

}
