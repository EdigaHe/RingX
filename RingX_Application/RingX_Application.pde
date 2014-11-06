/* Description - application to recognize three gestures: tap, wipe and knock
 * 
 * Author: Liang He (Andrew ID: lianghe)
 * Date: 04/19/2014
 */

import processing.serial.*;
Serial myPort;                      // Create Serial object

int lf = 10;                        // ASCII: '\n'
byte[] inBuffer = new byte[35];
int pdata1;                         // data of piezo 1
int pdata2;                         // data of piezo 2
float ave1 = 0.0;                  // mean value of piezo1
float sd1 = 0.0;                   // standard deviation of piezo1
float ave2 = 0.0;                  // mean value of piezo2
float sd2 = 0.0;                   // standard deviation of piezo2
ArrayList<data> ges_data;           // array for all data of one gesture
int gesture = -1;                   // recognized gesture: 1'tap' 2'wipe' 3'knock'

PFont font;
final int VIEW_SIZE_X = 1024, VIEW_SIZE_Y = 768; // window size

void setup() { 
  size(VIEW_SIZE_X, VIEW_SIZE_Y, P2D);
  background(#000000);
  
  ges_data = new ArrayList<data>();  // Create an empty ArrayList
  
  // Initial the Serial object, get ready for data reading
  myPort = new Serial(this, "/dev/tty.usbserial-A400APTD", 9600);
  font = createFont("Helvetica",32);
}

/* 
 * WekaClassifier - using J48 trees algorithm in Weka and building the decision tree
 *                  for the dataset. 
 * Input - ave1 is the average value of piezo1's data, ave2 is the average value of piezo2's
 *         data, sd1 is the standard deviation of piezo1's data, and sd2 is the standard 
 *         deviation of piezo2's data.
 * Return value - 1 is 'tap' gesture, 2 is 'wipe' gesture, 3 is 'knock' gesture
 */
int WekaClassifier(float ave1, float ave2, float sd1, float sd2) {
  int result;
  if (ave1 <= 25.032875) {
    result = 1;  
  }
  else {
    if (ave2 <= 54.9501) {
      if (sd2 <= 14.806105) result = 1;
      else result = 3; 
    }
    else {
      if (ave2 <= 122.46707) {
        if (sd2 <= 57.68879) {
          if (sd1 <= 45.751892) result = 2;
          else result = 3; 
        }
        else {
          if (sd1 <= 27.658525) result = 3;
          else {
            if (ave1 <= 51.90803) {
              if (ave2 <= 72.031456) result = 2;
              else {
                if (ave2 <= 98.74232) {
                  if (ave1 <= 49.05764) result = 3;
                  else result = 1; 
                }
                else result = 1;
              } 
            }
            else {
              if (ave1 <= 60.207104) {
                if (ave1 <= 57.95359) {
                  if (ave1 <= 56.957207) result = 3;
                  else result = 2;
                }
                else result = 3;
              }
              else result = 2;
            }
          } 
        }
      }
      else {
        result = 3; 
      }
    } 
  }
  return result;
}

/*
 * readSensors - read the data from the Serial, which contains the piezo data
 */
void readSensors() {
  if(myPort.available() > 0) {
    myPort.readBytesUntil(lf, inBuffer);
    if (inBuffer != null) {
      String inputString = new String(inBuffer);
      String [] inputStringArr = split(inputString, ',');
     
      if (inputString.substring(0, 4).equals("@#,#")){  
        // the gesture is over
        // calculate the mean value of all data
        for (int i=0; i<ges_data.size(); i++) {
          ave1 += ges_data.get(i).p1;
          ave2 += ges_data.get(i).p2;
        }
        ave1 = ave1/ges_data.size();
        ave2 = ave2/ges_data.size();
        
        // calculate the standard deviation of all data
        for (int j = 0; j<ges_data.size(); j++) {
          sd1 += (ges_data.get(j).p1 - ave1) * (ges_data.get(j).p1 - ave1);
          sd2 += (ges_data.get(j).p2 - ave2) * (ges_data.get(j).p2 - ave2);
        }
        sd1 = sqrt(sd1/ges_data.size());
        sd2 = sqrt(sd2/ges_data.size());
       
        ges_data.clear();  // clear the gesture array
        
        gesture = WekaClassifier(ave1, ave2, sd1, sd2);  // detect the gesture
      }
      if (inputStringArr.length == 2) {
        pdata1 = int(inputStringArr[0].substring(1));
        pdata2 = int(inputStringArr[1].substring(0, inputStringArr[1].indexOf('\n',0)));
        ges_data.add(new data(pdata1, pdata2));  // adding one data in the gesture array
        drawdots(float(pdata1), float(pdata2)); 
      }
    }
  } 
}

/*
 * drawdots - Visualize the data on dots with random colors and sizes.
 *            Their positions are based on the value.
 * Input - p1 is the value of pizeo1, p2 is the value of piezo2
 */
void drawdots(float p1, float p2) {
  float size = random(50,200);
  float col_r = random(0,255);
  float col_g = random(0,255);
  float col_b = random(0,255);
  float col_a = random(0,255);

  // set the color of the dot
  color c1 = color(col_r, col_g, col_b, col_a);
  noStroke();
  fill(c1);
  // set the position and the size of the dot
  float center_x = map(p1, 0, 350, 0, width);
  float center_y = map(p2, 0, 350, 0, height);
  
  // draw the dot
  ellipse(center_x, center_y, size, size);  
}

void draw() {
  textFont(font);
  textAlign(LEFT, LEFT);
  
  readSensors();
  
  color c0 = color(0,0,0,0);
  noStroke();
  fill(c0);
  ellipse(512,384,250,250);
  fill(255,255,255);
  text("Gesture:", 410,395);
  
  if (gesture == 1) {
    text("Tap", 545,395);
  }
  else if (gesture == 2) {
    text("Wipe", 545,395);
  }
  else if (gesture == 3) {
    text("Knock", 545,395);
  }
  
  delay(1); 
}

/* 
 * data - class for storing two piezos' data
 */
class data{
  int p1;  // piezo1's data
  int p2;  // piezo2's data
  data() {}
  data(int i1, int i2) {
    p1 = i1;
    p2 = i2;
  }
}
