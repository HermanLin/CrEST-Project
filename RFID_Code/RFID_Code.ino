#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define redPin 6
#define bluePin 5
#define greenPin 3

float fare = 2.75;
bool foundCard = false;

byte cards[][4] = {
                    {0xC0, 0x43, 0x48, 0x32}, // Daniel's
                    {0x67, 0x3A, 0xB7, 0x60}, 
                    {0x22, 0x76, 0x48, 0x34}  // Herman's
                  };
int numCards = 3;
//int len =


float balance[] = {2.75, 0, 12.5};

byte custom_UID[16] = {"SmartScan"}; 

MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522::MIFARE_Key key;
 
void setup() {
  Serial.begin(9600);   
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  pinMode(redPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  for(byte i = 0; i < 6; i++) key.keyByte[i]= 0xFF;
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  /* DEV CODE */
//  writeBlock(4, custom_UID);
  /* END DEV CODE */

  byte readCustomUID[18];
  readBlock(4, readCustomUID);

  // check if the card scanned is a SmartScan card
  if (checkUID(readCustomUID,custom_UID,9) == false) {
    Serial.println("Not a SmartScan Card! >:(");
    // turn LED yellow
    blinkColor(255,255,0);
    stopAuth();
    return;
  }
  // check if the SmartScan card is in the system
  // - if the card IS in the system, we need to keep track of its index
  //   in order to manipulate the card's balance
 if (!foundCard){
      Serial.print(balance[2]) ;
 stopAuth();
 return;
  }
  else{
    Serial.println("Card is not in system");
    stopAuth();
    return;
    }

  // check if there is enough money on the card to satisfy the fare
  // - if not, turn LED red
  // - else, deduct the fare from the balance, turn LED green
  
  if(balance[2] >= fare){
  Serial.print("GO");
  blinkColor(0,255,0); //  LED turns green
  balance = balance[2] - fare ;                         //deduct fare from balance 
  Serial.print("New Balance: $ ");      
  Serial.print(balance);                               // read function           
  }
  else{
    //LED turns red
     Serial.print("insufficient balance");
    blinkColor(255,0,0);
      }
  stopAuth();
}


/*******************************************************************************
 ============================= OTHER FUNCTIONS =================================
 *******************************************************************************/

int writeBlock(int blockNumber, byte arrayAddress[]) {
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber+1)%4 == 0) { // block number is a trailer block (modulo 4); quit and send error code 2
    Serial.print(blockNumber);
    Serial.println(" is a trailer block:");
    return 2;
  } 
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
  //writing the block 
  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);
  if (status != MFRC522::STATUS_OK) {
           Serial.print("MIFARE_Write() failed: ");
           Serial.println(mfrc522.GetStatusCodeName(status));
           return 4;//return "4" as error message
  }
  Serial.println("block was written");
}

int readBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A  = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed (read): ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }
  //it appears the authentication needs to be made before every block read/write within a specific sector.
  //If a different sector is being authenticated access to the previous one is lost.

  /*****************************************reading a block***********************************************************/  
  byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size... 
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("block was read");
}

void blinkColor (int redValue, int greenValue, int blueValue){
  analogWrite(redPin,redValue);
  analogWrite(greenPin,greenValue);
  analogWrite(bluePin,blueValue);
  delay(750);
  analogWrite(redPin,0);
  analogWrite(greenPin,0);
  analogWrite(bluePin,0); 
}

boolean checkUID(byte arr_a[], byte arr_b[],int len) {
  for (int i = 0; i < len; i++) {
    if (arr_a[i] != arr_b[i]) return false;
  }
  return true;
}

void stopAuth() {
  delay(500);
  mfrc522.PCD_StopCrypto1();
}

void dumpByteArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
