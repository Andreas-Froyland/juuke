/*
   Thanks to Original Author: ryand1011 (https://github.com/ryand1011)
   Edited by Ananords -
   See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data
   Uses MIFARE RFID card using RFID-RC522 reader
   Uses MFRC522 - Library
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
  PS: IF THE PLAYER FAILS TO START WHEN THE SERIAL MONITOR IS NOT OPEN, TRY TO COMMENT OUT Serial.begin(115200); 
*/

#include <SPI.h>
#include <MFRC522.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

const int playPauseButton = 4;
const int shuffleButton = 3;

boolean isPlaying = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

SoftwareSerial mySoftwareSerial(5, 6); // RX, TX

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);



//*****************************************************************************************//
void setup() {
  Serial.begin(115200);                                           // Initialize serial communications with the PC, COMMENT OUT IF IT FAILS TO PLAY WHEN DISCONNECTED FROM PC
  mySoftwareSerial.begin(9600);
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card



  pinMode(playPauseButton, INPUT_PULLUP);
  pinMode(shuffleButton, INPUT_PULLUP);




  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  Serial.println(F("DFPlayer Mini online. Place card on reader to play a spesific song"));

  myDFPlayer.volume(15);  //Set volume value. From 0 to 30


  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  //  myDFPlayer.EQ(DFPLAYER_EQ_POP);
  //  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
  //  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
  //  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
  //  myDFPlayer.EQ(DFPLAYER_EQ_BASS);

}



//*****************************************************************************************//
void loop() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;



  if (digitalRead(playPauseButton) == LOW) {
    if (isPlaying) {
      myDFPlayer.pause();
      isPlaying = false;
      Serial.println("Paused..");
    }
    else {
      isPlaying = true;
      myDFPlayer.start();
      Serial.println("Playing..");
    }
    delay(500);
  }


  if (digitalRead(shuffleButton) == LOW) {
    myDFPlayer.randomAll();
    Serial.println("Shuffle Play");
    isPlaying = true;
    delay(1000);
  }


  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( mfrc522.PICC_IsNewCardPresent()) {



    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    Serial.println(F("**Card Detected:**"));

    //-------------------------------------------

    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

    //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

    //-------------------------------------------

    Serial.print(F("Number: "));


    //---------------------------------------- GET NUMBER AND PLAY THE SONG

    byte buffer2[18];
    block = 1;
    len = 18;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    status = mfrc522.MIFARE_Read(block, buffer2, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }


    //PRINT NUMBER
    String number = "";

    for (uint8_t i = 0; i < 16; i++)
    {
      number += (char)buffer2[i];
    }
    number.trim();
    Serial.print(number);

    //PLAY SONG

    //if (isPlaying == false){
    // myDFPlayer.start();
    // }
    myDFPlayer.play(number.toInt());
    isPlaying = true;



    //----------------------------------------

    Serial.println(F("\n**End Reading**\n"));

    delay(1000); //change value if you want to read cards faster

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}



//*****************************************************************************************//
