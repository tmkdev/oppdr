/*  oppdr GM low speed capture

     Based on M2 example
     DataLogger - non-due-pins and data logger

     tmkdev llc

    Using modified Arduino_Due_SD_HSCMI library (https://github.com/macchina/Arduino_Due_SD_HSMCI) from Github user JoaoDiogoFalcao (https://github.com/JoaoDiogoFalcao/Arduino_Due_SD_HSCMI)

    Developed against Arduino IDE 1.8.5
*/

// Including Arduino_Due_SD_HSCMI library also creates SD object (MassStorage class)
#include <Arduino_Due_SD_HSMCI.h> // This creates the object SD
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
String buf = "";
int logging = 0;

#define Serial SerialUSB

// Create CAN object with pins as defined
SWcan CAN(CS_PIN, INT_PIN);

void CANHandler() {
  CAN.intHandler();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // 1s delay so you have enough time to open Serial Terminal
  // Check if there is card inserted
  SD.Init();
  FS.Init();

  // Set up SPI Communication
  // dataMode can be SPI_MODE0 or SPI_MODE3 only for MCP2515
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();
  
  // Initialize MCP2515 CAN controller at the specified speed and clock frequency
  // (Note:  This is the oscillator attached to the MCP2515, not the Arduino oscillator)
  //speed in KHz, clock in MHz
  CAN.setupSW(33333); //GMLAN
  CAN.mode(3); // Normal on the SWCAN bus. 
  // 0 = Sleep
  // 1 = High Speed
  // 2 = High Voltage
  // 3 = Normal

  attachInterrupt(SWC_INT, CANHandler, FALLING);

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

void printFrame(Frame &frame) {
  digitalWrite(CanActivity, LOW);
  String line = String(millis());
  if ( frame.extended ) {
    char hexval[9];
    sprintf(hexval, "%08x", frame.id);
    line += "," + String(hexval);
  } else {
    char hexval[4];
    sprintf(hexval, "%03x", frame.id);
    line += "," + String(hexval);
  }

  for (int count = 0; count < frame.length; count++) {
    char hexval[3];
    sprintf(hexval, "%02x", frame.data.bytes[count]);
    line += "," + String(hexval);
  }
  line += "\r\n";
  buf += line;
  digitalWrite(CanActivity, HIGH);
 
}

void writeToSD() {
  digitalWrite(Red, LOW);
  char write_buffer[buf.length() + 1];
  buf.toCharArray(write_buffer, buf.length() + 1);

  FS.Open("0:", fname, true);
  FS.GoToEnd();
  FS.Write(write_buffer);
  FS.Close();
  buf = "";
  digitalWrite(Red, HIGH);
}


void loop() {
  Frame message;

  if ( logging == 0 ) {
    startlog = digitalRead(SW2);
    if ( startlog == LOW ) {
      sprintf(fname, "LS_%04d", millis() % 10000);
      Serial.println(fname);
      FS.CreateNew("0:", fname);
      FS.Close();
      logging = 1;
      digitalWrite(Green, LOW);
      delay(50);
    }

  }  else {

    
    if (CAN.GetRXFrame(message)) {
      printFrame(message);
    }

    if (buf.length() > 8192) {
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
