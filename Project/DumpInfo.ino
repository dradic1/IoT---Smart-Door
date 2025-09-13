#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define PIR_PIN 2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

Servo myservo;
int pos = 0;
int pir_state = LOW;
int pir_val = 0;
bool servoFlag = false;

int trigPin = 5;    // Trigger
int echoPin = 3;    // Echo
long duration, cm, inches;
 
#define LED_PIN 8

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  myservo.attach(6);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("Approximate your card to the reader...");
  Serial.println();

}
bool unlocked = 1;
void loop() 
{
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  String x = Serial.readString();
  x.trim();
  if(x == "DOOR LOCKED"){
    unlocked = false;
  }
  else if(x == "DOOR UNLOCKED"){
    unlocked = true;
  }
  else if (x == "REMOTE DOOR OPEN" && unlocked){
    digitalWrite(LED_PIN, HIGH);
    open();    
    delay(1000);
  }
  else if(x == "REMOTE DOOR CLOSED" && unlocked){
    digitalWrite(LED_PIN, LOW);
    close();    
    delay(1000);
  }

  if(unlocked){
    
  pir_val = digitalRead(PIR_PIN);   // read sensor value
  if (pir_val == HIGH) {           // check if the sensor is HIGH
    digitalWrite(LED_PIN, HIGH);   // turn LED ON
    delay(100);                // delay 100 milliseconds 
    
    if (pir_state == LOW) {
      Serial.println("Motion detected!");
      open();
      close();
      pir_state = HIGH;       // update variable state to HIGH
    }
  } 
  else {
      //digitalWrite(LED_PIN, LOW); // turn LED OFF
      delay(200);             // delay 200 milliseconds 
      
      if (pir_state == HIGH){
        Serial.println("Motion stopped!");
        pir_state = LOW;       // update variable state to LOW
    }
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  //Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     //Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  if (content.substring(1) == "3C 5B C5 39") //change here the UID of the card/cards that you want to give access
  {
    myservo.attach(6);
    Serial.println("Authorized access");
    //Serial.println();
    digitalWrite(LED_PIN, HIGH);
    open();
    close();
    content.substring(1) = "";
    delay(3000);
  }
 
 else   {
    Serial.println("Access denied");
    digitalWrite(LED_PIN, LOW);
    delay(3000);
  }}
} 

void close(){
  myservo.attach(6);
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    do{
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);  
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH); // Get the pulse duration from the ultrasonic sensor
      cm = (duration / 2) / 29.1; // Convert the duration to centimeters
      if(cm < 10){
        myservo.detach();
        Serial.println("DOOR UNABLE TO CLOSE!");
        delay(1000);
      }
      //delay(2000);
    }while(cm < 10);
    myservo.attach(6);
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);
    
  }
  myservo.detach();
  digitalWrite(LED_PIN, LOW);
  Serial.println("DOOR CLOSED");
}

void open(){
  myservo.attach(6);
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);
  }
  myservo.detach();
  Serial.println("DOOR OPEN");
  delay(3000);
}