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
const int SW = Button1;     // Pushbutton SW1
const int SW2 = Button2;     // Pushbutton SW2
const int Red =  DS2;       // the number of the RED LED pin
const int Yellow =  DS3;       // the number of the Yellow LED pin
const int Green =  DS6;       // the number of the Green LED pin


FileStore FS;

// Variables
int sw_state = 0;
unsigned int i=0;

void setup() {
  delay(1000); // 1s delay so you have enough time to open Serial Terminal
  // Check if there is card inserted
  SD.Init(); // Initialization of HSCMI protocol and SD socket switch GPIO (to adjust pin number go to library source file - check Getting Started Guide)
  FS.Init(); // Initialization of FileStore object for file manipulation
//------------------------------------------------------------------------------------------------------
/* Following code creates file "data" in dir "0:" for logging raw data. Every time pushbutton is pushed, counter increment for 1 and number is logged into "data".
   Check Getting Started Guide for Debug options (Debug is enabled by default). 

   CAUTION!
   Code only works if there is no directory or file with the same name. Code first checks if name is alredy used when is needed. */

  char message[] = "\nCounter\n"; // Message at the beginnign of the file
  
  // Create/Open file "data"
  FS.CreateNew("0:","data"); // Create new file, if alredy exists it will be overwritten
  //FS.GoToEnd(); // Do not need when creating file because new file is opened and position 0
  FS.Write(message); // writing message
  FS.Close(); // we need to close file to store all the data that was written to it
   
  // Configure pushbuttons and LEDs
  pinMode(Red, OUTPUT);
  pinMode(SW,INPUT);
  digitalWrite(Red, LOW);
  }

void loop() {
  // This code is for logging data when button in pushed
  // Check if pushbutton is pushed
  sw_state = digitalRead(SW);
  if (sw_state == HIGH) {
    digitalWrite(Red, HIGH);     // turn LED OFF:
  }
  else {
    // LED on while pushbutton is pushed
    digitalWrite(Red, LOW);     // turn LED ON:
    // Increment counter
    i++;
    // Preapre buffer, write() method is accepting only array of chars, it is easier for us to create a string and then transform it into array of chars
    String str = "i= " + String(i) + "\n"; // creates a string
    char write_buffer[sizeof(str)]; // Creating array of char in length of our string
    str.toCharArray(write_buffer,sizeof(str)); // transform string to array of chars of strings's size
   // SerialUSB.print(write_buffer); // We can check what was created, SerialUSB.print(.) uses serial buffer so we can print string or array of chars 
    // Write data to file.
    FS.Open("0:","data",true); // openning file
    FS.GoToEnd(); // Search for the end of file and write to it
    FS.Write(write_buffer); // write data to file
    FS.Close(); // to save data in file, we must close the file
  }
   
  delay(250); // 0.25s delay so there is not too much data   
}
