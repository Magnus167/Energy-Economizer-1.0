  #include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "EmonLib.h"                   // Include Emon Library


// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
// The analog pins that connect to the sensors
#define currentPin1 1           // analog 0
#define currentPin2 3           // analog 1
#define acvolts 220             // In the absence of a Mains Voltage sensor, we use a default 220 V value for power calculations

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
  uint32_t TimeBegin = 0;
  uint32_t TimeEnd = 0;

EnergyMonitor emon1,emon2;                 
// Create 2 instances of the energy monitor

// the logging file
File logfile;

void error(char *str) // Code to print an error message to the serial monitor
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println();

  
  emon1.current(currentPin1, 4.4);             // Current: input pin, calibration.
  emon2.current(currentPin2, 4.4);
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, ........chipselect error or not present");
  }
  Serial.println("card initialized.");
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
  // create a new file
  char filename[] = "LOG.CSV";
   if (SD.exists("LOG.CSV")) 
   {
      logfile = SD.open(filename, FILE_WRITE); 
      if (!logfile) 
      error("couldnt create file");
   }
   else
   {
     logfile = SD.open(filename, FILE_WRITE); 
      if (!logfile)
     { 
      error("couldnt create file"); 
     }
     logfile.println("Timestamp , Current 1 , Current 2");    
    #if ECHO_TO_SERIAL
      Serial.println("Timestamp , Current 1 , Current 2");
    #endif //ECHO_TO_SERIAL
   }
   
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------     
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

   Serial.print("Logging to: ");
   Serial.println(filename);
   #if ECHO_TO_SERIAL
      Serial.println("Timestamp, Current 1 , Current 2");
      #endif //ECHO_TO_SERIAL
  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }


 
}//Setup() Close/

void loop(void)
{
  DateTime now;
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(1480);


  
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting

  
//  logfile.print(m);        // milliseconds since start
//  logfile.print(", ");    
#if ECHO_TO_SERIAL
//  Serial.print(m);         // milliseconds since start
//  Serial.print(", ");  
#endif

// fetch the time
  now = RTC.now();
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
 // logfile.print('"');
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(",   ");
 #endif //ECHO_TO_SERIAL
    
  logfile.print(Irms);
  logfile.print(", ");
  logfile.print(Irms2);

  
#if ECHO_TO_SERIAL
  
  Serial.print(Irms);
  Serial.print(",      ");
  Serial.print(Irms2);

#endif //ECHO_TO_SERIAL
logfile.println();
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL
// Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
// which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  // blink LED to show we are syncing data to the card & updating FAT!
 logfile.flush();
}
