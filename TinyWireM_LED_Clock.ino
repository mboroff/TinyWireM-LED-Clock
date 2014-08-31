/* ATtiny85 as an I2C Master   Ex3        BroHogan                         1/22/11
 * I2C master reading DS1621 temperature sensor & DS1307 RTC. Display to I2C GPIO LED.
 * SETUP:
 * ATtiny Pin 1 = (RESET) N/U                      ATtiny Pin 2 = (D3) N/U
 * ATtiny Pin 3 = (D4) to LED1                     ATtiny Pin 4 = GND
 * ATtiny Pin 5 = SDA on all devices               ATtiny Pin 6 = (D1) to LED2
 * ATtiny Pin 7 = SCK on all devices               ATtiny Pin 8 = VCC (2.7-5.5V)
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 * DS1621 wired per data sheet. This ex assumes A0-A2 are set LOW for an addeess of 0x48
 * DS1307 wired per data sheet. This ex assumes A0-A2 are set LOW for an addeess of 0x68
 * PCA8574A GPIO was used wired per instructions in "info" folder in the LiquidCrystal_I2C lib.
 * This ex assumes A0-A2 are set HIGH for an addeess of 0x3F
 * LiquidCrystal_I2C lib was modified for ATtiny - on Playground with TinyWireM lib.
 * TinyWireM USAGE & CREDITS: - see TinyWireM.h
 */

/*-----( Import needed libraries )-----*/
//#include <Wire.h>  // Comes with Arduino IDE
#include <TinyWireM.h>
#include <Time.h>  
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();


#define DS1307_ADDRESS 0x68
byte zero = 0x00; //workaround for issue #527

//------------Button Variables -------------------
#define buttonPin1  1   // pin 6 on chip
#define buttonPin2  3   // pin 2 on chip
#define buttonPin3  4   // pin 3 on chip


boolean currentButtonPin;
boolean lastButtonPin1 = HIGH;
boolean currentButtonPin1 = HIGH;
boolean lastButtonPin2 = HIGH;
boolean currentButtonPin2 = HIGH;
boolean lastButtonPin3 = HIGH;
boolean currentButtonPin3 = HIGH;

boolean drawDots;               // led variable to turn on the dot next to number

//---------- Variable for the clock SainSmart Tiny RTC ------
byte mySecond;
byte myMinute;
byte myHour;
byte myWeekDay;
byte myMonthDay;
byte myMonth;
byte myYear;                // end of get time/set time variables
byte prevSecond = 99;
//------------------ general variables -------------------

int dmyYear;
int menuCtr = 0;
boolean reverSe;
boolean reverSeMin;
boolean reverSeYr;
boolean displaySecond = false;
boolean lastFlash = false;
byte error, address;
int nDevices;

//------------------- LED Variables -----------------

static int ledBrightness = 5;        // set the brightness of the LED
 
void setup()  {
  Serial.begin(115200);  // Set serial monitor speed
  pinMode(buttonPin1, INPUT);   //Set switch modes
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);

  getDateAndTime();
  matrix.begin(0x70);                          // start communication to the LED
  matrix.setBrightness(ledBrightness);
  matrix.writeDisplay();
  getDateAndTime();  
  
}

void loop(){    
  currentButtonPin1 = debouncePin(lastButtonPin1, buttonPin1);             // Check if user wants to go to change menu
  if (lastButtonPin1 == LOW && currentButtonPin1 == HIGH)    {       
      lastButtonPin1 != lastButtonPin1;
      menuCtr = 1;
      lastButtonPin1 = currentButtonPin1;
      processMenu();
      }
  lastButtonPin1 = currentButtonPin1;
  if (minute() == 0 && second() == 0) {      // update time on the hour
      getDateAndTime();                         // get the date
  }
  if (prevSecond != second()) {            // Next second ?
      prevSecond =  second();
       mySecond = second();
       myMinute = minute();
       myHour = hour();
       printSegHour();          //do seven segment time display
  }

}


void processMenu(){                                              // ------ Process the menu 1 - 2 options -----------

//  currentButtonPin1 = 1;     // Reset button 1 state
//  lastButtonPin1 = 1;
  menuCtr = 1;              // set the processing to begin with option 1
  reverSe = false;          // ensure the down arrows do not display
  reverSeMin = false;

  
   while (menuCtr != 0) {                                       //  Loop untill the menu has cycled all the way through
         printSegHour();

        currentButtonPin = digitalRead(buttonPin1); 
        currentButtonPin1 = debouncePin(lastButtonPin1, buttonPin1);
     if (lastButtonPin1 == LOW && currentButtonPin1 == HIGH) {  // If button was pressed bump menu ctr and set state
         lastButtonPin1 != lastButtonPin1;
         reverSe = false;
         reverSeMin = false;
         reverSeYr = false;
         menuCtr++;
         }
     lastButtonPin1 = currentButtonPin1;           
     switch(menuCtr){
            case 1:                                            //******** Change Hour routine **********
              currentButtonPin3 = debouncePin(lastButtonPin3, buttonPin3);          // If S3 was pressed set reverse boolean
              if (lastButtonPin3 == LOW && currentButtonPin3 == HIGH) {
                  lastButtonPin3 != lastButtonPin3;
                  if (reverSe ==  false) {
                      reverSe = true;
                      }
                      else {
                            reverSe = false;
                          }
                }
             lastButtonPin3 = currentButtonPin3;
             
              currentButtonPin2 = debouncePin(lastButtonPin2, buttonPin2);               // If S2 was pressed bump hour
              if (lastButtonPin2 == LOW && currentButtonPin2 == HIGH) {
                  lastButtonPin2 != lastButtonPin2;
                  bumpHour();                  // bump the hour
                  }
                  lastButtonPin2 = currentButtonPin2;
               break;
  
            case 2:
              currentButtonPin3 = debouncePin(lastButtonPin3, buttonPin3);          // If S3 was pressed bump hour
              if (lastButtonPin3 == LOW && currentButtonPin3 == HIGH) {
                  lastButtonPin3 != lastButtonPin3;
                  if (reverSeMin ==  false) {
                      reverSeMin = true;
                      }
                      else {
                            reverSeMin = false;
                            }
                     }
                lastButtonPin3 = currentButtonPin3;

               currentButtonPin2 = debouncePin(lastButtonPin2, buttonPin2);
               if (lastButtonPin2 == LOW && currentButtonPin2 == HIGH) {      // If S2 was pressed bump minute
                   lastButtonPin2 != lastButtonPin2;
                   bumpMinute();                                     // bump the minutes
                   }
               lastButtonPin2 = currentButtonPin2;
               break;

              case 3: 
                 menuCtr=0;                                      // do not allow menu to be processed until S1 is pressed
                 break; 
  
         }               // end of switch
   }                     // end of while

}                      // end of menu processing
  

byte decToBcd(byte val){                                   // conversion routine between clock board and code variables
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)  {                               // conversion routine between clock board and code variables
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

void getDateAndTime(){                                 // ------ Get the time and date from clock board

  TinyWireM.beginTransmission(DS1307_ADDRESS);       // Reset the register pointer
  TinyWireM.write(zero);                             // Stop the Oscilllator
  TinyWireM.endTransmission();                       // Send endof transmission
  TinyWireM.requestFrom(DS1307_ADDRESS, 7);          // Request to receive 3 bytes from the clock

  mySecond = bcdToDec(TinyWireM.read());
  myMinute = bcdToDec(TinyWireM.read());
  myHour = bcdToDec(TinyWireM.read() & 0b111111); //24 hour time
  myWeekDay = bcdToDec(TinyWireM.read()); //0-6 -> sunday - Saturday
  myMonthDay = bcdToDec(TinyWireM.read());
  myMonth = bcdToDec(TinyWireM.read());
  myYear = bcdToDec(TinyWireM.read());
  dmyYear = myYear + 2000;                    // Add 2000 for year display - don't worry about next century
  setTime(myHour, myMinute, mySecond, myMonthDay, myMonth, myYear);
  Serial.print(hour()); Serial.print(":"); Serial.print(minute()); Serial.print(":"); Serial.println(second());
}
void setDateTime(){
  TinyWireM.beginTransmission(DS1307_ADDRESS);              // Address the clock board for receipt of writes
  TinyWireM.write(zero);                                    //stop Oscillator

  TinyWireM.write(decToBcd(mySecond));                      // Write the time to buffer
  TinyWireM.write(decToBcd(myMinute));
  TinyWireM.write(decToBcd(myHour));
  TinyWireM.write(decToBcd(myWeekDay));
  TinyWireM.write(decToBcd(myMonthDay));
  TinyWireM.write(decToBcd(myMonth));
  TinyWireM.write(decToBcd(myYear));

  TinyWireM.write(zero);                                   //start the Oscillator

  TinyWireM.endTransmission();                             // Signal end of transmission

}

boolean debouncePin(boolean last, int buttonPin) {                    // Debounce Switch to ensure it was pressed - parameter is last button pin 1 state

  boolean currentButtonPin = digitalRead(buttonPin);     // Read switch 
  if (last != currentButtonPin) {                       // If last button pin state not equal the current state
     delay(50);                                          // Wait 50 milliseconds
     currentButtonPin = digitalRead(buttonPin);          // Read the switch again
     }
  return currentButtonPin;                           // Return the state of the pin
}
void printSegHour() {                // ----- Display the Hour on Seven Segment display ----
  boolean drawDots = false;
  getDateAndTime();                   // read time from chip to my values and set time
  int wrkHr;
  wrkHr = myHour;
  if (myHour < 23 && myHour > 12){ // If hour is between 10 and 20
      wrkHr = (myHour - 12) ; // subtract 12 hour and multiply the hour by 100 to print digits 2 - 4
      }
  if (myHour == 23 || myHour == 11){ // if hour is 11 or 23 display 11
      wrkHr = 11;
      }
  if (myHour == 22 || myHour == 10){ // if hour is 10 or 22 display 10
      wrkHr = 10;
      }
  if (myHour == 0 || myHour == 12){ // if hour is 12 or midnight display 12
      wrkHr = 12;
      }
  if (myHour > 9 && myHour < 12) {
      matrix.writeDigitNum(0, 1 , false); // print hour 1st digit
      }
  if (myHour < 12) {
      matrix.writeDigitNum(1, wrkHr % 10, false); // print hour 2nd digit
      }
  if (myHour == 0) { // routine to print 1 and either 2, 1 or 0
      matrix.writeDigitNum(0, 1, false);
      matrix.writeDigitNum(1, 2, false);
      }
  if (myHour == 12) {
      matrix.writeDigitNum(0, 1, false);
      matrix.writeDigitNum(1, 2, false);
      }
  if (myHour == 22) {
      matrix.writeDigitNum(0, 1, false);
      matrix.writeDigitNum(1, 0, false);
      }
  if (myHour == 23) {
      matrix.writeDigitNum(0, 1, false);
      matrix.writeDigitNum(1, 1, false);
      }
  if (wrkHr < 10) {
      matrix.displaybuffer[0] = B00000000; // leading 0
      matrix.writeDigitNum(1, wrkHr % 10, false);
      }
  if (myMinute < 10 || myMinute > 59) {
      matrix.writeDigitNum(3, 0, false);
      }
  if (myMinute > 9 && myMinute < 20) {
      matrix.writeDigitNum(3, 1, false);
      }
  if (myMinute > 19 && myMinute < 30) {
      matrix.writeDigitNum(3, 2, false);
      }
  if (myMinute > 29 && myMinute < 40) {
      matrix.writeDigitNum(3, 3, false);
      }
  if (myMinute > 39 && myMinute < 50) {
      matrix.writeDigitNum(3, 4, false);
      }
  if (myMinute > 49 && myMinute < 60) {
      matrix.writeDigitNum(3, 5, false);
      }
  if (myHour > 11) {
      matrix.writeDigitNum(4, myMinute % 10, false);
      }
  else {
        matrix.writeDigitNum(4, myMinute % 10, true);
        }
// ================= If menu is active the following routines will replace the time display
  if (menuCtr == 1){
      matrix.displaybuffer[0] = B01110110 ; // Hr ..
      matrix.displaybuffer[1] = B01010000 ;
      if (myHour > 9 && myHour < 12) {
          matrix.writeDigitNum(3, 1 , false); // print hour 1st digit
          }
      if (myHour < 12) {
          matrix.writeDigitNum(4, wrkHr % 10, false); // print hour 2nd digit
          }
      if (myHour == 0) { // routine to print 1 and either 2, 1 or 0
          matrix.writeDigitNum(3, 1, false);
          matrix.writeDigitNum(4, 2, false);
          }
      if (myHour == 12) {
          matrix.writeDigitNum(3, 1, false);
          matrix.writeDigitNum(4, 2, false);
          }
      if (myHour == 22) {
          matrix.writeDigitNum(3, 1, false);
          matrix.writeDigitNum(4, 0, false);
          }
      if (myHour == 23) {
          matrix.writeDigitNum(3, 1, false);
          matrix.writeDigitNum(4, 1, false);
          }
      if (wrkHr < 10) {
          matrix.displaybuffer[3] = B00000000; // leading 0
          matrix.writeDigitNum(4, wrkHr % 10, false);
          }
      }
  if (menuCtr == 2){
      matrix.displaybuffer[0] = B00110011 ;
      matrix.displaybuffer[1] = B00100111 ;
      if (myMinute < 10 || myMinute > 59) {
          matrix.writeDigitNum(3, 0, false);
          }
      if (myMinute > 9 && myMinute < 20) {
          matrix.writeDigitNum(3, 1, false);
          }
      if (myMinute > 19 && myMinute < 30) {
          matrix.writeDigitNum(3, 2, false);
          }
      if (myMinute > 29 && myMinute < 40) {
          matrix.writeDigitNum(3, 3, false);
          }
      if (myMinute > 39 && myMinute < 50) {
          matrix.writeDigitNum(3, 4, false);
          }
      if (myMinute > 49 && myMinute < 60){
          matrix.writeDigitNum(3, 5, false);
          }
      if (myHour > 12) {
          matrix.writeDigitNum(4, myMinute % 10, false);
          }
      else {
            matrix.writeDigitNum(4, myMinute % 10, true);
            }
  }
  flashColon(); // flash is only when menu is not being processed
  matrix.writeDisplay();
}
void flashColon(){ //---------- colon print routine for LED -----
  if (menuCtr == 0) {
      if (lastFlash == true) {
          lastFlash = false;
          matrix.drawColon(true) ;
          }
      else {
            lastFlash = true;
            matrix.drawColon(false);
            }
      }
}

void bumpHour(){                          // User pressed S2 in menu Case 1
 getDateAndTime();                       // get thetime from the board and reset system time
 if (reverSe == false) {
     myHour++;        // Add 1 to hour
     }
 if (reverSe == true) {
     myHour--;
     }
 if (myHour < 0 || myHour > 254) {
     myHour = 23;  
     }
 if (myHour > 23) {
     myHour = 0;            // If it is midnight rest to zero
     }
 setDateTime();                          // set the new time in the clock board
 getDateAndTime();                       // get thetime from the board and reset system time  
}
 
void bumpMinute(){                       // User press S2 in the menu Case 2
getDateAndTime();                       // get thetime from the board and reset system time
 if (reverSeMin == false)  {
     myMinute++;   // Add 1 to minutes
     }
 if (reverSeMin == true)  {
     myMinute--;     // Add 1 to minutes
     }
 if (myMinute <= 0 || myMinute == 255) {   // adjust for negative value
     myMinute = 59;  
     }
  if (myMinute > 59) {
      myMinute = 00;      // If greater than 59 minutes reset to zero
      }
  mySecond = 0;                        // rest the seconds to zero
  setDateTime();                       // set the new time in the clock board        
  getDateAndTime();                    // get thetime from the board and reset system time  
}

