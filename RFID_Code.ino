#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define bzr 5
#define gled 4
#define rled 3
#define yled 2

int balance =50;
int block = 4;
byte content[9] = {"SmartScan"}; 

MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522 :: MIFARE_Key key ;
 
void setup() 
{
  Serial.begin(9600);   
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  pinMode(bzr,OUTPUT);
  pinMode(gled,OUTPUT);
  pinMode(rled,OUTPUT);
  pinMode(yled,OUTPUT);
  for(byte i = 0; i< 6; i++) key.keyByte[i]= 0xFF;
}

int writeBlock(int blockNumber, byte arrayAddress[]) 
{
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber+1)%4 == 0){Serial.print(blockNumber);Serial.println(" is a trailer block:");return 2;}//block number is a trailer block (modulo 4); quit and send error code 2
  Serial.print(blockNumber);
  Serial.println(" is a data block:");
  
  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A  = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed: ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }
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
    Serial.print("Balance");
    Serial.println(balance);
  }
  writeblock(block,content);
  
//  //Show UID on serial monitor
//  Serial.print("UID tag :");
//  String content= "";
//  byte letter;
//  for (byte i = 0; i < mfrc522.uid.size; i++) 
//  {
//     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//     Serial.print(mfrc522.uid.uidByte[i], HEX);
//     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
//     content.concat(String(mfrc522.uid.uidByte[i], HEX));
//  }
//  Serial.println();
//  Serial.print("Message : ");
//  content.toUpperCase();
//  if (content.substring(1) == "C0 43 48 32") 
//  {
//    Serial.println("Authorized access");
//    Serial.println("Balance: $");
//    tone(bzr, 1000);
//     digitalWrite(gled,HIGH);
//    delay(1000);    
//    noTone(bzr);     
//    digitalWrite(gled,LOW);
//  }
// else if(balance < fare) {
//    digitalWrite(rled,HIGH);
//    Serial.println("Insufficient Balance");
//    tone(bzr,200);
//    delay(500);
//    digitalWrite(rled,LOW);
//    noTone(bzr);
//    }
// else {
//     tone(bzr, 500); 
//    digitalWrite(yled,HIGH);
//    Serial.println("Insufficient Balance");
//    delay(1000);     
//    noTone(bzr);    
//    digitalWrite(rled,LOW);
//  }
} 
