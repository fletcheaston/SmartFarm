/* -------------------- Controller Firmware Version 0.1.0 ---------------------
   This program is the controller firmware for smartfarm DAQ board V5.4
   Please DO NOT change this program, unless you know exactly what happens
   Bo Liu
   Ag. Mechtronics Lab, BRAE, Cal Poly
   01/01/2017 

   Controller MCU pinout

  Phy. pin  Port           Description            I/O type
  19 ------ A6          -- Analog input only
  22 ------ A7          -- Analog input only

  12 ------ PB0/D8      -- Pro. DAQ control pin -- Output
  13 ------ PB1/D9      -- MCUs communication   -- Input/Output
  14 ------ PB2/D10     -- Timer power          -- Output
  15 ------ PB3/D11     -- MOSI
  16 ------ PB4/D12     -- MISO
  17 ------ PB5/D13     -- SCK
  7 ------- PB6/D20     -- Backup sensor power  -- Output
  8 ------- PB7/D21     -- Decagon sensor power -- Output

  23 ------ PC0/A0/D14  -- Timer 8              -- Input
  24 ------ PC1/A1/D15  -- Timer 4              -- Input
  25 ------ PC2/A2/D16  -- Timer 2              -- Input
  26 ------ PC3/A3/D17  -- Timer 1              -- Input
  27 ------ PC4/A4/D18  -- Temp. power control  -- Output
  28 ------ PC5/A5/D19  -- DAQ power control    -- Output
  29 ------ PC6/reset   -- reset pin

  30 ------ PD0/RX/D0   -- Measurement LED      -- Output
  31 ------ PD1/TX/D1   -- Wireless pro. LED    -- Output
  32 ------ PD2/INT0/D2 -- Wireless pro. detect -- Input
  1 ------- PD3/INT1/D3 -- XBee sleep indictor  -- Input
  2 ------- PD4/D4      -- XBee RTS             -- Output
  9 ------- PD5/D5      -- XBee Association     -- Input
  10 ------ PD6/D5      -- XBee CTS             -- Input
  11 ------ PD7/D7      -- XBee Sleep-RQ        -- Output

  3 ------- GND
  4 ------- VCC
  5 ------- GND
  6 ------- VCC
  18 ------ AVCC
  20 ------ AREE
  21 ------ GND
*/
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>

const byte MLED = 0; //Measurement LED
const byte WLED = 1; //Wireless pro. LED
const byte proSigIn = 2; // Wireless programming signal detection 
const byte xbeeOnsleep = 3; // XBee sleep indictor detection
const byte xbeeRTS = 4; // XBee ready to send
const byte xbeeAssco = 5; // XBee associate
const byte xbeeCTS = 6; // XBee clear to send
const byte xbeePower = 7; // XBee sleep control pin; reversed
const byte proReset = 8; // Programming reset pull down pin
const byte comm = 9; // uc communication 
const byte timerPower = 10; // Timer power pin
const byte tempPower = 18; // Timer power pin
const byte DAQPower = 19; // Timer power pin
const byte backPower = 20; // Timer power pin
const byte decagonPower = 21; // Timer power pin

unsigned char Time = 0;
volatile boolean proDone = false; // programming done variable

unsigned char sleepMode = 0; // Time reading from the timer
int proWaitTime = 30; //Wireless programming waiting time(in seconds)
int measureTimeOut = 30; //Measurement waiting time(in seconds);



// the setup routine runs once when you press reset:
void setup()
{
  pinMode(MLED, OUTPUT);
  pinMode(WLED, OUTPUT);
  pinMode(xbeePower, OUTPUT);
  pinMode(timerPower, OUTPUT);
  pinMode(tempPower, OUTPUT);
  pinMode(DAQPower, OUTPUT);
  pinMode(backPower, OUTPUT);
  pinMode(decagonPower, OUTPUT);
    
  // initialize the timer pins
  pinMode(14, INPUT);//8
  pinMode(15, INPUT);//4
  pinMode(16, INPUT);//2
  pinMode(17, INPUT);//1

  pinMode(proSigIn, INPUT);
  pinMode(xbeeOnsleep, INPUT);
  pinMode(comm, INPUT);
}

// the loop routine runs over and over again forever:
void loop()
{
  digitalWrite(timerPower, HIGH); delayMilliseconds(10);// Turn on timer power
  sleepMode = digitalRead(17) + digitalRead(16) * 2 + digitalRead(15) * 4 + digitalRead(14) * 8; // Get the timer reading
  digitalWrite(timerPower, LOW); //PORTB &= ~(1 << 2);  // Turn off timer power
  
  if (sleepMode == 0b0000) { // use xbee synchronous cyclic sleep mode 
    startMeasurement();
    sleepNow();
  }
  else { // use the timer to schedule the sleep cycle
    digitalWrite(xbeePower, HIGH);//PORTD |= (1 << 7);  //turn on the transmitter XBee
    startMeasurement();
    digitalWrite(xbeePower, LOW);//PORTD &= ~(1 << 7); ;//turn off the transmitter XBee
    switch (sleepMode)
    {
      case 0b0001: sleepMin(5); break;
      case 0b0010: sleepMin(10); break;
      case 0b0011: sleepMin(30); break;
      case 0b0100: sleepHour(1); break;
      case 0b0101: sleepHour(3); break;
      case 0b0110: sleepHour(6); break;
      case 0b0111: sleepHour(12); break;
      case 0b1000: sleepHour(24); break;
      case 0b1001: sleepHour(48); break;
      case 0b1010: sleepHour(72); break;
      case 0b1011: sleepHour(96); break;
      case 0b1100: sleepHour(120); break;
      case 0b1101: sleepHour(144); break;
      case 0b1110: sleepHour(168); break;
      case 0b1111: sleepHour(336); break;

      default: sleepHour(6);
    }
  }
  
 }


//********** This function listens to wireless programming signals for a certain time period  **********
void watchProSig(int t) {
  digitalWrite(DAQPower,LOW);
  attachInterrupt(digitalPinToInterrupt(proSigIn), triggerPro, LOW);//start listening to wireless programming signals
  for (int x = 0; x < proWaitTime; x++) { // Wait for wirless programming signals before shutting down
    digitalWrite(WLED, HIGH);
    delayMilliseconds(50);
    digitalWrite(WLED, LOW);
    delayMilliseconds(950);
    
  }
  detachInterrupt(digitalPinToInterrupt(proSigIn));//stop listening to wireless programming signals
}



//********** This function triggers wireless programming process  **********
void triggerPro()  //triggered at the start of an upload first low byte of an upload
{
  digitalWrite(DAQPower,HIGH);
  detachInterrupt(digitalPinToInterrupt(proSigIn));  //stop monitoring wireless programming signal
  uploadProgram(); //start uploading
}


//********** This function uploads a program via XBee wirelessly  **********
void uploadProgram() {

  digitalWrite(proReset, LOW); //start uploading program wirelessly
  delayMicroseconds(30000); //delay 499500us, this works for atmega168 bootloader, S1B, S2B
  digitalWrite(proReset, HIGH); //bring reset pin to high to avoid resetting the mcu

  for (unsigned int i = 0; i < 70000; i = i + 50) //delay 40s till "proDone" is true
  {
    digitalWrite(WLED, HIGH); // Fast blinking for wireless programming
    delayMilliseconds(20);
    digitalWrite(WLED, LOW);
    delayMilliseconds(20);

    if (digitalRead(comm)==1){
      i = 70000;
      break;
    }
    
  }
}


//********** This function delays the program for x Seconds  **********
void delaySeconds(int x)   {
  for (int i = 0; i < x; i++)
  {
    for (int j = 0; j < 1000; j++) // 1 sec
    {
      delayMicroseconds(1000);// 1 millsec
    }
  }
}


//**********  This function delays the program for x Milliseconds  **********
void delayMilliseconds(int x)   {
  for (int i = 0; i < x; i++)
  {
    delayMicroseconds(1000);// 1 millsec
  }
}


//********** This function puts the controller into sleep mode  **********
void myWatchdogEnable(const byte interval)
{
  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay

  wdt_reset();
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  sleep_mode();            // now goes to Sleep and waits for the interrupt
}


//********** This function is the ISR vector executed when watchdog timed out  **********
ISR(WDT_vect)
{
  wdt_disable();
}


//********** Sleep in mins  **********
void sleepMin(int m)
{
  //* 1 second:  0b000110
  //* 2 seconds: 0b000111
  //* 4 seconds: 0b100000
  //* 8 seconds: 0b100001

  for (int i = 0; i < m * 60; i++)
  {
    myWatchdogEnable (0b000110);  // 1 sec
  }
}

//********** Sleep in mins  **********
void sleepHour(int h)
{
  for (int i = 0; i < h; i++)
  {
    sleepMin(60); // 1 hour
  }
}


//********** This function controls the DAQ measuremenmt process  **********
void startMeasurement() 
{
  //1. Power the DAQ system
    digitalWrite(backPower, HIGH);
    digitalWrite(decagonPower, HIGH);
    digitalWrite(tempPower, HIGH);
    delaySeconds(1);
    digitalWrite(DAQPower, HIGH);
    delaySeconds(1);
    
  for (int x = 0; x < measureTimeOut; x++) { // Slow blinking to show DAQ meansurements in progress
    digitalWrite(MLED, HIGH);
    delayMilliseconds(50);
    digitalWrite(MLED, LOW);
    delayMilliseconds(950);
  }

  //2. Shut down everything

    digitalWrite(backPower, LOW);
    digitalWrite(decagonPower, LOW);
    digitalWrite(tempPower, LOW);
    digitalWrite(DAQPower, LOW);

  //3. Watch for wirelessly programming signals for a while before shutting everything down
  watchProSig(proWaitTime);// Wait for wireless programming signals
}


//********** Sleep now  **********
void sleepNow()
{
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  // will be called when pin D3 goes HIGH
  attachInterrupt (digitalPinToInterrupt(xbeeOnsleep), wakeUpNow, RISING);
  sleep_mode(); // here the device is actually put to sleep!!

  /* The program will continue from here. */
  sleep_disable(); // first thing after waking from sleep
  detachInterrupt(digitalPinToInterrupt(xbeeOnsleep)); // disables interrupt 1 on pin 3 
}

void wakeUpNow()        // here the interrupt is handled after wakeup
{
    digitalWrite (MLED, HIGH);
    delay (200);
    digitalWrite (MLED, LOW);
    delay (200);
}
