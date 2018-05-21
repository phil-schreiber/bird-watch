/*************************************************** 
  This is a sketch to test the camera module with the CC3000 WiFi chip
  
  Written by Marco Schwartz for Open Home Automation
  Code inspired by the work done on the Adafruit_VC0706 & CC3000 libraries
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Include camera  
#include <Adafruit_VC0706.h>

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>

#include <SoftwareSerial.h>  
#include <string.h>
#include "utility/debug.h"
#include <stdlib.h>


SoftwareSerial cameraconnection = SoftwareSerial(2, 4);

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);



// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(10, 3, 5,
                                         SPI_CLOCK_DIV2);
                                         
// Local server IP, port, and repository (change with your settings !)
uint32_t ip = 0L;


        const char
  host[]          = "XXX";                              
void setup() {
 
 Serial.begin(115200);


  
  
  Serial.println("Camera test");  
  
     
 
 

    
  

  
  // Initialise the module
  
  if (!cc3000.begin())
  {
    Serial.println("!wifi");
    return;
    
  }

  // Connect to  WiFi network
  cc3000.connectToAP("LOOKWHOSTHERE", "ROT42INX2006HELL", WLAN_SEC_WPA2);
    
  // Display connection details
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
   
  

  
  
// Try to locate the camera
  if (!cam.begin()) {    
    Serial.println("C !found");
    return;
  }
  delay(2000);
  cam.setImageSize(VC0706_640x480); 
   delay(2000);
    uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
     //  Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true);           // turn it on
  
 delay(2000);
  // You can also verify whether motion detection is active!
  Serial.print("Motion detection is ");
  if (cam.getMotionDetect()) 
    Serial.println("ON");
  else 
    Serial.println("OFF");
  
  cam.setCompression(70);

}

void loop() {
if (cam.motionDetected()) {
   Serial.println("Motion!");   
   cam.setMotionDetect(false);
   delay(3000);
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  if (! cam.takePicture()) 
    Serial.println("nope");
  else 
    Serial.println("oki");
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  
  Serial.print(jpglen, DEC);
     uint16_t len = jpglen+149;
  cc3000.getHostByName((char *)host, &ip);
  
  //cc3000.printIPdotsRev(ip);
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  // Connect to the server, please change your IP address !
  if (client.connected()) {
      Serial.println(F("Connected !"));
      client.println(F("POST /index.php HTTP/1.1"));
client.println(F("Host: tweetproxy.ephemeroid.net"));
client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
client.print(F("Content-Length: "));
client.println(len);
      client.print(F("\n--AaB03x\nContent-Disposition: form-data; name=\"picture\"; filename=\"cam.jpg\"\n"));
      client.print(F("Content-Type: image/jpeg\n"));
      client.print(F("Content-Transfer-Encoding: binary\n\n"));
      
  
      // Read all the data up to # bytes!
      byte wCount = 0; // For counting # of writes
      while (jpglen > 0) {
         
        uint8_t *buffer;
        uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!

        
        buffer = cam.readPicture(bytesToRead);
        /*int i=0;
         if (buffer[i] != 0) {                                                
            while(i < bytesToRead) {                                            
                client.print(buffer[i]);                                                                            
                i++;                                                             
            } 
         }*/
         client.write(buffer,bytesToRead);
        delay(100);
        buffer=0;

    
        if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
          Serial.print('.');
          wCount = 0;
        }
        jpglen -= bytesToRead; 
      }
       
      client.print(F("\n--AaB03x--\n"));
      client.println();
      Serial.println("Transmission over");
  } 
  
  else {
      Serial.println(F("Connection failed"));    
    }
    
     while (client.connected()) {

      while (client.available()) {

    // Read answer
        char c = client.read();
        Serial.print(c);

      }
    }
    client.close();
  cam.resumeVideo();
  cam.setMotionDetect(true);
 }
  
}

