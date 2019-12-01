#include <ArduinoBLE.h>

byte midiData[] = {0x80, 0x80, 0x00, 0x00, 0x00};
BLEService midiService("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
BLECharacteristic midiCharacteristic("7772E5DB-3868-4112-A1A9-F2669D106BF3",
                                     BLEWrite | BLEWriteWithoutResponse |
                                     BLENotify | BLERead, sizeof(midiData));
                                     
#include <Wire.h>
#include "Adafruit_Trellis.h"

#define MOMENTARY 0
#define LATCHING 1
// set the mode here
#define MODE LATCHING 


Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);
#define NUMTRELLIS 1
#define numKeys (NUMTRELLIS * 16)

#define INTPIN A2

byte midiChannel = 0x90;
bool pRecordState1 = false;
bool pRecordState5 = false;
bool pRecordState9 = false;
bool pRecordState13 = false;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  
    // Initialize BLE:
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (true);
  }
  BLE.setLocalName("waaaaa");
  BLE.setAdvertisedService(midiService);
  midiService.addCharacteristic(midiCharacteristic);
  BLE.addService(midiService);
  BLE.advertise();

  // INT pin requires a pullup
  pinMode(INTPIN, INPUT);
  digitalWrite(INTPIN, HIGH);
  
  // begin() with the addresses of each panel in order
  // I find it easiest if the addresses are in order
  trellis.begin(0x70);  // only one

  // light up all the LEDs in order
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();    
    delay(50);
  }
  // then turn them off
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();    
    delay(50);
  }

  
}


void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();
  if (central) {
    digitalWrite(13, HIGH);
  }
  
  delay(30); // 30ms delay is required, dont remove me!



  if (MODE == LATCHING) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed...
        
        if (trellis.justPressed(i)) {
          Serial.print("v"); Serial.println(i);

          //button 1，5，9，13 controlls Arm Recording for dif tracks
          if (i == 1 ){
            
           sendMidiToArm(i,pRecordState1);
           pRecordState1 = !pRecordState1;
           
          } else if (  i == 5 ) {
            
            sendMidiToArm(i,pRecordState5);
            pRecordState5 = !pRecordState5;
            
          } else if ( i == 9 ) {
            
            sendMidiToArm(i,pRecordState9);
            pRecordState9 = !pRecordState9;
          } else if ( i == 13 ){
            
            sendMidiToArm(i,pRecordState13);
            pRecordState13 = !pRecordState13;
        
          } else {
            
            midiCommand(midiChannel, i+60, 99);

          }

          // Alternate the LED
          if (trellis.isLED(i))
            trellis.clrLED(i);
          else
            trellis.setLED(i);
              } 
              
            }
        // tell the trellis to set the LEDs we requested
        trellis.writeDisplay();
    }
  }
}

void sendMidiToArm (int index, bool pState){
    Serial.println("Arm recording midi!!");
    Serial.println(pState);

    //when press the button
    //if recording is off now, then turn it on
    if (pState == false){
      
       midiCommand(midiChannel, index+60, 127);
       Serial.println("sent 127");
       
      //if recording is on now, then turn it off
      //this part doesn't work
      } else {
        
        midiCommand(midiChannel, index+60, 127);
        Serial.println("sent 127");
      }
      
}


// send a 3-byte midi message
void midiCommand(byte cmd, byte data1, byte  data2) {
  // MIDI data goes in the last three bytes of the midiData array:
  midiData[2] = cmd;
  midiData[3] = data1;
  midiData[4] = data2;

  midiCharacteristic.setValue(midiData, sizeof(midiData));
}
