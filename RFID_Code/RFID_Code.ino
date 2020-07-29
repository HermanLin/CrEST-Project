#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define redPin 6
#define bluePin 5
#define greenPin 3
#define dev_button 2

float fare = 2.75;
bool devPressed = false;
bool foundCard = false;

byte cards[][4] = {
                    {0xC0, 0x43, 0x48, 0x32}, // Daniel's
                    {0x67, 0x3A, 0xB7, 0x60}, 
                    {0x22, 0x76, 0x48, 0x34}  // Herman's
                  };
int numCards = 3;

float balance[] = {2.75, 0, 12.5};

byte custom_UID[16] = {"SmartScan"}; 

MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522::MIFARE_Key key;
 
void setup() {
  Serial.begin(9600);   
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  
  pinMode(redPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  pinMode(dev_button, INPUT);
  for(byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop() {
  if (digitalRead(dev_button) == HIGH) {
    delay(100);
    if (digitalRead(dev_button) == HIGH) {
      Serial.println("Dev Button Pressed");
      devPressed = true;
    }
  }
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // uncomment below in order to write the custom UID to a new card
//  if (devPressed) {
//    Serial.println("W r i t i n g C u s t o m U I D");
//    writeBlock(4, custom_UID);
//    blinkColor(0, 0, 50);
//    stopAuth();
//    return;
//  }

  byte readCustomUID[18];
  readBlock(4, readCustomUID);

  // check if the card scanned is a SmartScan card
  if (checkUID(readCustomUID, custom_UID, 9) == false) {
    Serial.println("Not a SmartScan Card! >:(");
    // turn LED yellow
    blinkColor(255,255,0);
    stopAuth();
    return;
  }
  
  // check if the SmartScan card is in the system
  // - if the card IS in the system, we need to keep track of its index
  //   in order to manipulate the card's balance
  int i;
  for (i = 0; i < numCards; i++) {
    if (checkUID(mfrc522.uid.uidByte, cards[i], 4)) {
      foundCard = true;
      break;
    }
  }
  if (foundCard == false){
    Serial.print("Invalid SmartScan card!");
    // turn LED yellow
    
    stopAuth();
    return;
  }

  // check if there is enough money on the card to satisfy the fare
  // - if not, turn LED red
  // - else, deduct the fare from the balance, turn LED green
  if (devPressed) {
    balance[i] += fare;
    Serial.print("Adding funds. New Balance: ");
    Serial.println(balance[i]);
    blinkColor(0, 0, 50); // blink LED blue
  }
  else if (balance[i] < fare) {
    Serial.print("Insufficient Funds. Current balance: ");
    Serial.println(balance[i]);
    // turn LED red
  
  }
  else if (balance[i] >= fare) {
    Serial.println("GO");
    balance[i] -= fare;
    Serial.print("New Balance: $");      
    Serial.print(balance[i]);
    // turn LED green
    
  }

  stopAuth();
}


/*******************************************************************************
 ============================= OTHER FUNCTIONS =================================
 *******************************************************************************/

int writeBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber + 1) % 4 == 0) return 2; // block number is a trailer block (modulo 4); quit and send error code 2

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) return 3;//return "3" as error message

  //writing the block
  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
  if (status != MFRC522::STATUS_OK) return 4;//return "4" as error message
}

int readBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber + 1) % 4 == 0) return 2; // block number is a trailer block (modulo 4); quit and send error code 2

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) return 3;//return "3" as error message

  /*****************************************reading a block***********************************************************/
  byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size...
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) return 4;//return "4" as error message
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
  devPressed = false;
  foundCard = false;
  delay(500);
  mfrc522.PCD_StopCrypto1();
}
