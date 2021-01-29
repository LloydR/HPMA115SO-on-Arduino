/*
* Arduino DS3231 Real Time Clock Module Tutorial
*
* Crated by Dejan Nedelkovski,
* www.HowToMechatronics.com
*
* DS3231 Library made by Henning Karlsen which can be found and downloaded from his website, www.rinkydinkelectronics.com.
*
*/
/*
 * Code for reading Honeywell HPMA115S0 comes from
 * https://electronza.com/arduino-measuring-pm25-pm10-honeywell-hpma115s0/2/
 * by Teodor Costachioiu
 * Revised for an Arduino UNO or actually a Sparkfun RedBoard and using a software serial port for the read of the HPMA
 * REVISED AGAIN 11/11/2020
 * Using an Arduino MEGA 2560 and a different LCD Library which actually simplified things.
 * Thought that the softserial might be messing up.  Don't know if that is the case.  Need to do more long range testing.
 * 11/21/20
 * still have problemshat to think.
 */
#include <DS3231.h>
#include <LiquidCrystal_I2C.h> // includes the LiquidCrystal Library 


// For Arduino Uno int size is 8 bit, that is -32,768 to 32,767
// Use long or float if working with an Uno or simmilar 8-bit board
long PM25;
long PM10;
bool my_status;
int stop_status = 0;

// It appears that running this without the cable hooked to the PC IDE causes problems
// Try a debug if statement and if hooked to 5V then hooked to PC else not
// The above 2 statements are not correct - the problem appears to be
// that if one does not stop the auto measure then there are problems.  
// Zero's are returned when requesting measurements and then occasionally
// a measurement is received.  Causing confusion
//
int debugPin = 7;     // debug to digital pin 7
int debugging = 0; 

int lcount = 0;  //loop counter
int showmystatus = 0; // show when we read alt * +

DS3231  rtc(SDA, SCL);
LiquidCrystal_I2C lcd(0x27,20,4);  // Set the LCD I2C address
void (*resetFunc)(void) = 0;  //essentially this jumps the program to address 0 which is sort of like resetting the Arduino

//
//SETUP
//
void setup() { 
 pinMode(debugPin, INPUT);        // sets the digital pin 7 as input
 
 rtc.begin(); // Initialize the rtc object

 // The following lines can be uncommented to set the date and time
 //rtc.setDOW(FRIDAY);        // Set Day-of-Week to SUNDAY, MONDAY, TUESDAY, etc
 //rtc.setTime(06, 47, 0);     // Set the time to 12:00:00 (24hr format)
 //rtc.setDate(31, 8, 2018);  // Set the date to August 31, 2014
 
 lcd.init(); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display  Helps to get it correct
 lcd.backlight();
 
 Serial3.begin(9600);

 debugging = digitalRead(debugPin);     // read the debugging pin
 
/* if (debugging == 1)
   {
   Serial.begin(9600);
   Serial.println("start of !Serial ");
   while (!Serial);
   Serial.println("End of !Serial ");
   }
*/   
 lcd.setCursor(15,2);
 lcd.print("WAIT");
 
 delay(2000); // I don't think HPMA likes to be talked to early in the morning!  Let it get some time to ramp up.
 lcd.setCursor(15,2);
 lcd.print("START");
 delay(2000);
 
  // Stop autosend
  // I think this is critical that this stop else things don't get setup
  // and you get a zero for values until it sends 
  while (stop_status != 1)
    {
      lcd.setCursor(15,2);
      lcd.print("STOPA");
      delay(1000);
      stop_status = stop_autosend();
      if (stop_status != 1)
        {
        // If you ever get in here you never get out and need to reset the arduino
        // 
        lcd.setCursor(15,2);
        lcd.print("BADAS");
        stop_measurement();
        delay(1000); 
        lcd.setCursor(15,2);
        lcd.print("AGAIN");
        delay(3000);
        resetFunc();  // this seems to work - However the resulting reading is sometimes off by a lot.  Don't know what is going on there.
                      // Never had that problem with the python version on the Raspberry Pi ???
        }
    }
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
/*  if (debugging == 1)
    {
    Serial.print("Stop autosend status is ");
    Serial.println(my_status, BIN);
    Serial.println(" ");
    }
*/
 
  delay(1000);
  lcd.setCursor(15,2);
  lcd.print("FAN  ");  
  // Start fan
  my_status = start_measurement(); 
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
/*  if (debugging == 1)
    {
    Serial.print("Start measurement status is ");
    Serial.println(my_status, BIN);
    Serial.println(" ");
    }
*/
  
  delay(5000);
  lcd.setCursor(15,2);
  lcd.print("     ");
}

//
//LOOP
//
void loop() { 
  
 lcd.setCursor(1,0);
 lcd.print("Time:  ");
 lcd.print(rtc.getTimeStr());
 
 lcd.setCursor(1,1);  
 lcd.print("Date: ");
 lcd.print(rtc.getDateStr());
 
 delay(1000); 
 lcount++;
 // Read the particle data every once in a while
 if (lcount >= 3)
   {
   my_status = read_measurement(); 
   lcount = 0;
   showmystatus++;
   if (showmystatus % 2) // if odd
     {
      lcd.setCursor(17,3);
      lcd.print("*");
     }
   else
     {
      lcd.setCursor(17,3);
      lcd.print("+");
      showmystatus = 0;
     }
   }
 // Serial print is used just for debugging
 // But one can design a more complex code if desired
 /*if (debugging == 1)
   {
   Serial.print("Read measurement status is ");
   Serial.println(my_status, BIN);
   Serial.print("PM2.5 value is ");
   Serial.println(PM25, DEC);
   Serial.print("PM10 value is ");
   Serial.println(PM10, DEC);
   Serial.println(" ");
   }
*/
 lcd.setCursor(1,2);
 lcd.print("PM2.5 =  ");
 lcd.print(PM25, DEC);
 
 lcd.setCursor(1,3);  
 lcd.print("PM10  =  ");
 lcd.print(PM10, DEC);

 
 if (my_status == 1)
   {
    lcd.setCursor(17,2);
    lcd.print("OK ");
   }
 else
   {
    lcd.setCursor(17,2);
    lcd.print("BAD");
   }

 
}

bool start_measurement(void)
{
  // First, we send the command
  byte start_measurement[] = {0x68, 0x01, 0x01, 0x96 };
  Serial3.write(start_measurement, sizeof(start_measurement));
  //Then we wait for the response
  while(Serial3.available() < 2);
  byte read1 = Serial3.read();
  byte read2 = Serial3.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
 
bool stop_measurement(void)
{
  // First, we send the command
  byte stop_measurement[] = {0x68, 0x01, 0x02, 0x95 };
  Serial3.write(stop_measurement, sizeof(stop_measurement));
  //Then we wait for the response
  while(Serial3.available() < 2);
  byte read1 = Serial3.read();
  byte read2 = Serial3.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
 
bool read_measurement (void)
{
  // Send the command 0x68 0x01 0x04 0x93
  byte read_particle[] = {0x68, 0x01, 0x04, 0x93 };
  Serial3.write(read_particle, sizeof(read_particle));
  // A measurement can return 0X9696 for NACK
  // Or can return eight bytes if successful
  // We wait for the first two bytes
  while(Serial3.available() < 1);
  byte HEAD = Serial3.read();
  while(Serial3.available() < 1);
  byte LEN = Serial3.read();
  // Test the response
  if ((HEAD == 0x96) && (LEN == 0x96)){
    // NACK
/*    if (debugging == 1)
      {
      Serial.println("NACK");
      }
*/      
    return 0;
  }
  else if ((HEAD == 0x40) && (LEN == 0x05))
  {
    // The measuremet is valid, read the rest of the data 
    // wait for the next byte
    while(Serial3.available() < 1);
    byte COMD = Serial3.read();
    while(Serial3.available() < 1);
    byte DF1 = Serial3.read(); 
    while(Serial3.available() < 1);
    byte DF2 = Serial3.read();     
    while(Serial3.available() < 1);
    byte DF3 = Serial3.read();   
    while(Serial3.available() < 1);
    byte DF4 = Serial3.read();     
    while(Serial3.available() < 1);
    byte CS = Serial3.read();      
    // Now we shall verify the checksum
    if (((0x10000 - HEAD - LEN - COMD - DF1 - DF2 - DF3 - DF4) % 0XFF) != CS){
/*      if (debugging == 1)
        {
        Serial.println("Checksum fail");
        }
*/        
      return 0;
    }
    else
    {
      // Checksum OK, we compute PM2.5 and PM10 values
      PM25 = DF1 * 256 + DF2;
      PM10 = DF3 * 256 + DF4;
      return 1;
    }
  }
}
 
int stop_autosend(void)
{
 // Stop auto send
  byte stop_autosend[] = {0x68, 0x01, 0x20, 0x77 };
  Serial3.write(stop_autosend, sizeof(stop_autosend));
  //Then we wait for the response
  while(Serial3.available() < 2);
  byte read1 = Serial3.read();
  byte read2 = Serial3.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 2;
}
 
bool start_autosend(void)
{
 // Start auto send
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  Serial3.write(start_autosend, sizeof(start_autosend));
  //Then we wait for the response
  while(Serial3.available() < 2);
  byte read1 = Serial3.read();
  byte read2 = Serial3.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
