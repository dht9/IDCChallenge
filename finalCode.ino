#include <Servo.h>
#include <SoftwareSerial.h> 
#include "pitches.h"

#define enablePin  51   
#define rxPin      53  
#define txPin      11  
#define BUFSIZE    11 
#define RFID_START  0x0A  
#define RFID_STOP   0x0D
#define RLED 9
#define GLED 3
#define YLED 2
#define Rx 10 //DOUT
#define Tx 11 //DIN
SoftwareSerial rfidSerial(rxPin, txPin);
SoftwareSerial Xbee (Rx, Tx);
                  
Servo servoLeft;                                
Servo servoRight;
long RCTime(int sensorIn);
void writeServo(float leftSpeed, float rightSpeed);

const int pingPin = 6;            
unsigned int duration, inches;
int song = 0;  

void setup() {
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT); 
    pinMode(5, OUTPUT);        
    pinMode(8,INPUT);         
    pinMode(9,OUTPUT);
    
    Serial.begin(9600);
    Xbee.begin(9600);
    
    pinMode(enablePin, OUTPUT);  
    pinMode(rxPin, INPUT);
    digitalWrite(enablePin, HIGH);
    rfidSerial.begin(2400);     
    Serial.flush();  
  
    servoLeft.attach(13);               
    servoRight.attach(12);
    
    LineFollow();             
    RFID();              
}

void loop() { }

void writeServo(float leftSpeed, float rightSpeed) {
    int left = 1500 + leftSpeed * 500.0;
    int right = 1500 - rightSpeed * 500.0;
    servoLeft.writeMicroseconds(left);
    servoRight.writeMicroseconds(right);
}

long RCTime(int sensorIn) {
    long duration = 0;
    pinMode(sensorIn, OUTPUT);     
    digitalWrite(sensorIn, HIGH);  
    delay(1);                      
    pinMode(sensorIn, INPUT);      
    digitalWrite(sensorIn, LOW);
    while(digitalRead(sensorIn)) {  
        duration++;
    }
    return duration;
}

void LineFollow() {
    while(1) {
        //Serial.println(RCTime(7));        //RCTime(7) LEFT sensor
        Serial.println(RCTime(4));        //RCTime(4) RIGHT sensor
        //Serial.println(RCTime(33));     //outside LEFT sensor
        //Serial.println(RCTime(31));     //outside RIGHT sensor
        //delay(5);
        int threshold = 40;              
        int left = RCTime(7) < threshold;
        int right = RCTime(4) < threshold;
        
        if(right && left)   writeServo(1,1);   //forward
        else if(right)      writeServo(0,1);   //turn right
        else if(left)       writeServo(1,0);   //turn left
        else if(RCTime(33) < threshold) writeServo(0,1);
        else if(RCTime(31) < threshold) writeServo(1,0);
        else {
            writeServo(1,1);     
            delay(240);
            writeServo(-1,1);
            delay(650);
            writeServo(0,0);
            delay(100);
            break;
        }
    } 
}

void RFID() {
    digitalWrite(enablePin, LOW);   
    digitalWrite(3,LOW);
    digitalWrite(2,LOW);
    digitalWrite(9,LOW);
    delay(50);
    int check = 0;
    while(1) {
        servoLeft.attach(13);
        servoRight.attach(12);
        writeServo(1,1); 
        if (check == 0) delay(275);
        else if (check == 1) delay(750);
        else if (check == 2) delay(650);
        writeServo(0,0);
        delay(500);
        servoLeft.detach();
        servoRight.detach();
        check++;
        //delay(200);
        if(rfidSerial.available()) {
            if (check == 1) {
                digitalWrite(3,HIGH);
                char ret = transmitUntilRecieved('L');
                Serial.println(ret);
                singDanceOrLight(ret);  
                break;
            }
            else if (check == 2) {
                digitalWrite(2,HIGH);   
                char ret = transmitUntilRecieved('K');
                singDanceOrLight(ret);
                break; 
            }
            else if (check == 3) {
                digitalWrite(9,HIGH); 
                char ret = transmitUntilRecieved('J');
                singDanceOrLight(ret);  
                break;  
            }                         
        }
        if (check == 4) break;
    }
}

char transmitUntilRecieved(char toTransmit) {
    servoLeft.detach();
    servoRight.detach();
    Xbee.begin(9600);
    while(1) {
        char charXbee = ' ';
        if(Xbee.available()) {
        charXbee = Xbee.read();
        }
        if(charXbee == 'X' || charXbee == 'Y' || charXbee == 'Z') {
            servoLeft.attach(13);
            servoRight.attach(12);
            return charXbee;
        }
        Xbee.print(toTransmit);
        //Serial.println(toTransmit);
        delay(100);
    }
}

void singDanceOrLight(char in) {
    if(in == 'X') {
        dance();
    }
    else if (in == 'Y') {
        servoLeft.detach();
        servoRight.detach();
        lightShow();
    }
    else if(in == 'Z') {
        sing();
    }
    servoLeft.detach();
    servoRight.detach();
}

void buzz(int targetPin, long frequency, long length) { 
    long delayValue = 1000000/frequency/2;
    long numCycles = frequency * length/ 1000;
    for (long i=0; i < numCycles; i++) { 
        digitalWrite(targetPin,HIGH); 
        delayMicroseconds(delayValue); 
        digitalWrite(targetPin,LOW); 
        delayMicroseconds(delayValue);
    }
}

void sing() {      
    Serial.println(" 'Mario Theme'");
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {  
        int noteDuration = 1000/tempo[thisNote];
        buzz(melodyPin, melody[thisNote],noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        buzz(melodyPin, 0,noteDuration);
    }
}

void lightShow() {
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(YLED, OUTPUT);
  digitalWrite(RLED, LOW);
  digitalWrite(GLED, LOW);
  digitalWrite(YLED, LOW);
  for(int k = 0; k < 5; k++) {
    for(int j = 0; j < 5; j++) {
      digitalWrite(RLED, HIGH);
      delay(100);
      digitalWrite(GLED, HIGH);
      delay(100);
      digitalWrite(RLED, LOW);
      digitalWrite(YLED, HIGH);
      delay(100);
      digitalWrite(GLED, LOW);
      delay(100);
      digitalWrite(YLED, LOW);
    }
    
    for(int j = 0; j < 5; j++) {
      digitalWrite(RLED, HIGH);
      digitalWrite(GLED, HIGH);
      digitalWrite(YLED, HIGH);
      delay(200);
      digitalWrite(RLED, LOW);
      digitalWrite(GLED, LOW);
      digitalWrite(YLED, LOW);
      delay(200);
    }
  }
}

void dance() {
    writeServo(1,-1);
    delay(1000);
    int count = 0;
    while(count<3) {
        count++;
        writeServo(.5,.5);
        delay(600);
        writeServo(-.5,-.5);
        delay(600);
    }
    count = 0;
    while(count<3) {
        count++; 
        writeServo(1,-1);
        delay(600);
        writeServo(-1,1);
        delay(600);
    }
    delay(925);
    writeServo(0,0);
}
