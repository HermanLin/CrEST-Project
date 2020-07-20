#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define GLEDpin 4
#define RLEDpin 3
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("TAP HERE");
  Serial.println();
  pinMode(GLEDpin, OUTPUT);
  pinMode(RLEDpin, OUTPUT);        
}               
void loop()
{
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
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "C0 4A 9E 32") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("GO!");
   
    Serial.println();
    digitalWrite(GLEDpin, HIGH);
    delay(2000);
    digitalWrite(GLEDpin, LOW);

   
  }
 
 else   {
    Serial.println("TRY AGAIN");
    digitalWrite(RLEDpin, HIGH);
    delay(2000);
    digitalWrite(RLEDpin, LOW);

  }
} 
