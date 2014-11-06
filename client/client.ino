/* Description: this side is the client that collects the data from two piezos and a touchpad, and
 *              sends the data to the server side over wireless connection. The data is encapsulated
 *              in a 6-byte buffer.
 *
 * Author: Liang He (Andrew ID: lianghe)
 * Date: 04/15/2014
 */
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <CapacitiveSensor.h>

// Global variables
int touchState = 0;         // flag for on/off
long touchNum = 0;          // touch sensing data
const int touchPin = 2;     // the number of the touch pin
const int piezo1Pin = A1;  // the number of one piezo pin
const int piezo2Pin = A2;  // the number of the other piezo pin
const int ledPin = 9;
boolean is_start = false;   // flag for starting gesture data
int piezo_data1;
int piezo_data2;
const int threshold = 25;   // if the piezo's data is bigger than this value
                            // it starts to send the data; otherwise, it does
                            // not send the data.
const int touch_thre = 999; // threshold for the touchpad

CapacitiveSensor  cs_3_2 = CapacitiveSensor(3,2); // 2 is sensing pin 

void setup(){
  cs_3_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  Serial.begin(9600);
  
  // set Pin mode
  pinMode(piezo1Pin, INPUT);
  pinMode(piezo2Pin, INPUT);
  pinMode(ledPin, OUTPUT);  
  digitalWrite(ledPin, LOW);
 
  // Set the SPI Driver
  Mirf.spi = &MirfHardwareSpi;
  // Initialize the nRFL24L01 module
  Mirf.init();
  
  // Configure transmitting address. 
  Mirf.setTADDR((byte *)"serv1");
   
  /*
   * Set the payload length to 6 bytes
   * ---on/off flag (1 byte) | piezo1 data (2 bytes) | piezo2 data (2 bytes) | end flag (1 byte)---: data
   * or --- (     ---: header of the data set
   * or --- )     ---: end of the data set
   * Get load the packet into the buffer.
   * On/off flag: @ - on
   *              # - end
   * return type of millis()
   * NB: payload on client and server must be the same.
   */
  Mirf.payload = 6;
  
  // Write channel and payload config then power up reciver.
  Mirf.config();
  // For testing with a console window
  Serial.println("Beginning ... "); 
}

void loop(){
  // A buffer to store the data.
  byte data[Mirf.payload];
  // Read Touch Pad
  touchNum = cs_3_2.capacitiveSensor(30);
  
  if (touchState == 0) {
    if (touchNum >= touch_thre) {
      // state: off -> on
      touchState = 1;
      digitalWrite(ledPin, HIGH);          // light up the led
    }
    else {
      // do nothing 
    }
  }
  else {
     if (touchNum >= touch_thre) {
       // state: on -> off
       touchState = 0;
       digitalWrite(ledPin, LOW);          // turn off the led
     } 
     else {
       // continue sending data
       piezo_data1 = analogRead(piezo1Pin);
       piezo_data2 = analogRead(piezo2Pin);
     
       // print both piezo data
       Serial.print("piezo 1 = ");
       Serial.print(piezo_data1);
       Serial.print("  ");
       Serial.print("piezo 2 = ");
       Serial.print(piezo_data2);
       Serial.print(" \n");
    
       if (piezo_data1 >= threshold || piezo_data2 >= threshold) {
         byte p1_high = highByte(piezo_data1);
         byte p1_low = lowByte(piezo_data1);
         byte p2_high = highByte(piezo_data2);
         byte p2_low = lowByte(piezo_data2);
        
         data[0] = '@';
         data[1] = p1_high;
         data[2] = p1_low;
         data[3] = p2_high;
         data[4] = p2_low;
         data[5] = '#';
         
         Mirf.send(data);
         while(Mirf.isSending()){}
       }
     }
  }
  delay(5);
} 
  
  
  
