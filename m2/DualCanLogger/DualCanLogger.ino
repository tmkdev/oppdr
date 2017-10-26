/*  oppdr GM high speed capture

     Based on M2 example
     DataLogger - non-due-pins and data logger

     tmkdev llc

    Using modified Arduino_Due_SD_HSCMI library (https://github.com/macchina/Arduino_Due_SD_HSMCI) from Github user JoaoDiogoFalcao (https://github.com/JoaoDiogoFalcao/Arduino_Due_SD_HSCMI)

    Developed against Arduino IDE 1.8.5
*/

// Including Arduino_Due_SD_HSCMI library also creates SD object (MassStorage class)
#include <Arduino_Due_SD_HSMCI.h> // This creates the object SD
#include "Arduino.h"
#include "variant.h"
#include <due_can.h>
#include <SPI.h>
#include <MCP2515_sw_can.h>

// Pin definitions specific to how the MCP2515 is wired up.
#define CS_PIN    SPI0_CS3
#define INT_PIN    SWC_INT

#define Serial SerialUSB


// Macchina M2 specific defines for your board
const int SW1 = Button1;     // Pushbutton SW1
const int SW2 = Button2;     // Pushbutton SW2
const int Red =  DS2;       // the number of the RED LED pin
const int Yellow =  DS3;       // the number of the Yellow LED pin
const int CanActivity =  DS4;       // the number of the Yellow LED pin
const int Green =  DS6;       // the number of the Green LED pin


FileStore FS;

// Variables
int startlog = 0;

char fname[10];
char buf[10000];
int logging = 0;

// Create CAN object with pins as defined
SWcan SWCAN(CS_PIN, INT_PIN);

void CANHandler() {
  SWCAN.intHandler();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // 1s delay so you have enough time to open Serial Terminal
  // Check if there is card inserted
  SD.Init();
  FS.Init();

  Can0.begin(CAN_BPS_500K);

  int filter;
  //extended
  for (filter = 0; filter < 2; filter++) {
    Can0.setRXFilter(filter, 0, 0, true);
  }
  //standard
  for (int filter = 2; filter < 7; filter++) {
    Can0.setRXFilter(filter, 0, 0, false);
  }


  // Set up SPI Communication
  // dataMode can be SPI_MODE0 or SPI_MODE3 only for MCP2515
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  // Initialize MCP2515 CAN controller at the specified speed and clock frequency
  // (Note:  This is the oscillator attached to the MCP2515, not the Arduino oscillator)
  //speed in KHz, clock in MHz
  SWCAN.setupSW(33333); //GMLAN
  SWCAN.mode(3); // Normal on the SWCAN bus.
  // 0 = Sleep
  // 1 = High Speed
  // 2 = High Voltage
  // 3 = Normal

  attachInterrupt(SWC_INT, CANHandler, FALLING);

  // Filter only the required packets from LS - Cause it's a slow write to SD..
  // 1000A0B0 - TPMS
  // 100A6097 - OnStar GPS
  // 100AA097 - OnStar GPS
  // 100AC097 - OnStar GPS
  SWCAN.InitFilters(false);
  SWCAN.SetRXMask(MASK0, 0x1FFF1FFF, 1); // ONSTAR
  SWCAN.SetRXFilter(FILTER0, 0x100A0097, 1);
  SWCAN.SetRXMask(MASK1, 0x1FFFFFFF, 1); // TPMS
  SWCAN.SetRXFilter(FILTER1, 0x1000A0B0, 1);

  pinMode(Red, OUTPUT);
  pinMode(Yellow, OUTPUT);
  pinMode(CanActivity, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  digitalWrite(Red, HIGH);
  digitalWrite(Green, HIGH);
  digitalWrite(Yellow, LOW);
  digitalWrite(CanActivity, HIGH);

}


//Timestamp,ID,Data0,Data1,...,
//412687,17F,00,00,00,00,00,00,00,00

void printHSFrame(CAN_FRAME &frame) {
  digitalWrite(CanActivity, LOW);
  char millistring[15];
  sprintf(millistring, "%Lu", millis());
  strcat(buf, millistring);
  strcat(buf, ",0,");

  if ( frame.extended ) {
    char hexval[9];
    sprintf(hexval, "%08x", frame.id);
    strcat(buf, hexval);    

  } else {
    char hexval[4];
    sprintf(hexval, "%03x", frame.id);
    strcat(buf, hexval);    
  }

  for (int count = 0; count < frame.length; count++) {
    char hexval[3];
    sprintf(hexval, "%02x", frame.data.bytes[count]);
    strcat(buf, ",");
    strcat(buf, hexval);
  }
  strcat(buf, "\n");
  digitalWrite(CanActivity, HIGH);
 
}

void printLSFrame(Frame &frame) {
  digitalWrite(CanActivity, LOW);
  char millistring[15];
  sprintf(millistring, "%Lu", millis());
  strcat(buf, millistring);
  strcat(buf, ",1,");
  
  if ( frame.extended ) {
    char hexval[9];
    sprintf(hexval, "%08x", frame.id);
    strcat(buf, hexval);   
  } else {
    char hexval[4];
    sprintf(hexval, "%03x", frame.id);
    strcat(buf, hexval); 
  }

  for (int count = 0; count < frame.length; count++) {
    char hexval[3];
    sprintf(hexval, "%02x", frame.data.bytes[count]);
    strcat(buf, ",");
    strcat(buf, hexval); 
  }
  strcat(buf, "\n");
  digitalWrite(CanActivity, HIGH);
 
}



void writeToSD() {
  digitalWrite(Red, LOW);
  
  FS.Open("0:", fname, true);
  FS.GoToEnd();
  FS.Write(buf);
  FS.Close();
  //memset(buf, 0, sizeof buf);
  buf[0] = 0x00;
  digitalWrite(Red, HIGH);
}


void loop() {
  CAN_FRAME incoming;
  Frame message; 
  
  if ( logging == 0 ) {
    startlog = digitalRead(SW2);
    if ( startlog == LOW ) {
      sprintf(fname, "HS_%04d", millis() % 10000);
      Serial.println(fname);
      FS.CreateNew("0:", fname);
      FS.Close();
      logging = 1;
      digitalWrite(Green, LOW);
      delay(50);
    }

  }  else {

    if (Can0.available() > 0) {
      Can0.read(incoming);
      printHSFrame(incoming);
    }

    if (SWCAN.GetRXFrame(message)) {
      printLSFrame(message);
    }

    if (strlen(buf) > 9250) {
      writeToSD();
    }
  }

  if ( logging == 1 && digitalRead(SW1) == LOW ) {
    writeToSD();
    logging = 0;
    digitalWrite(Green, HIGH);
    delay(250);
  }


}
