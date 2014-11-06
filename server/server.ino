/* Description: this side is the server that receives the data collected from the ring
 *              over wireless connection. After it receives the buffer it prints the 
 *              data to the serial, which the Processing application can read from.
 *
 * Author: Liang He (Andrew ID: lianghe)
 * Date: 04/15/2014
 */

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
int piezodata1;
int piezodata2;
int counter = 0;         // count for waiting data
boolean isdata = false;  // flag for data

void setup(){
  Serial.begin(9600);
  
  // Set the SPI Driver.
  Mirf.spi = &MirfHardwareSpi;
  // Initialize the nRF24L01 module
  Mirf.init();
  
  // Configure reciving address.
  Mirf.setRADDR((byte *)"serv1");
  
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
}

void loop(){
  // A buffer to store the data.
  byte data[Mirf.payload];
  /*
   * If a packet has been recived.
   *
   * isSending also restores listening mode when it 
   * transitions from true to false.
   */
  if(!Mirf.isSending() && Mirf.dataReady()){
    /* Get load the packet into the buffer.
     * On/off flag: @ - on
     *              # - end
     */
    Mirf.getData(data);
    
   
    if (data[0] == '@') { // Start reading data...
      counter = 0;        // there are data coming
      isdata = true;
      
      Serial.print('@');
      piezodata1 = int(data[1]<<8 | data[2]);
      piezodata2 = int(data[3]<<8 | data[4]);
      Serial.print(piezodata1);
      Serial.print(',');
      Serial.print(piezodata2);
      if (data[5] == '#') { // Normally ended
        Serial.print('\n');
      }
    }
  }
  
  if (isdata) {
    if (counter >= 800) {
      // longer than 0.8 s without any data, this gesture is over
      Serial.print("@#,#\n");
      isdata = false;
    }
    else {
      counter++;
    }
  } 
  delay(1);
}
