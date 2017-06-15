#define DisplaySerial Serial
#define printDelay 12
#define SIZE_OF_DATA 30

#define WRITE_FAIL 1000
#define UNABLE_READ_AFT_WRITE 1001
#define NO_TAG 1002
#define READ_AFT_WRITE_FAIL 1003
#define SUCCESS 999

#include <Picaso_Const4D.h>
#include <Picaso_Serial_4DLib.h>
#include <SoftwareSerial.h>


SoftwareSerial rfid(7, 8);
Picaso_Serial_4DLib Display(&DisplaySerial); 

//Global var
int flag = 0;
int flag2 = 0;
int flag3 = 0;
int Str1[10];
int writeData[21];
int Data[SIZE_OF_DATA];
char charData[SIZE_OF_DATA];
int auth[6];
int var[10];
int tagFound = 0;
int pickBlock = 0x05;
String entry1 = "ASHLEYFURNITURE";
String output1;
int TIMEOUT_COUNTER_LIMIT = 500;
int sizeOfEntry = 0;


//INIT
void setup()
{
  Serial.begin(9600);                             //Serial doesnt effact NFC
  Serial.println("Start");
//----------------------------------4D Systems---------------------------------------------
   //For handling errors
//  Display.Callback4D = mycallback ;
  //
  //5 second timeout on all commands  
  Display.TimeLimit4D   = 5000 ;

  
  //--------------------------------Optional reset routine-----------------------------------
  //Reset the Display using D4 of the Arduino (if using the new 4D Arduino Adaptor - Rev 2)
  //If using the old 4D Arduino Adaptor (Rev 1), change D4 to D2 below.
  //If using jumper wires, reverse the logic states below. 
  //Refer to the accompanying application note for important information.
  pinMode(4, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(4, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(4, 0);  // unReset the Display via D4
  //----------------------------------------------------------------------------------------------
  delay (5000); //let the display start up
  

  Display.gfx_ScreenMode(PORTRAIT);
  //Display.gfx_BGcolour(WHITE) ; //change background color to white
  Display.gfx_Cls();            //clear the screen
//-------------------------------------ENDof4DSetup-----------------------------------------------
  sizeOfEntry = entry1.length();
  output1.reserve(sizeOfEntry);
  for(int i = 0; i < sizeOfEntry; i++)
  {
    output1 += "0";
  }
  
  // set the data rate for the SoftwareSerial ports
  rfid.begin(19200);
  delay(10);

  for (int i = 0; i < 10; i++)
  {
    var[i] = 0x00;
  }

  halt();
  seek1();
  while (rfid.available() > 0) {
    for (int i = 0; i < 10; i++) {
      if (rfid.available() > 0) {
        var[i] = rfid.read();
      }
    }
  }
}

/***********MAIN LOOP***********************************************************************************************************/
//MAIN
void loop()
{


  read_serial();                                          //read serial number and print it
  delay(10);
//  Authenticate(pickBlock);                                       //MAKES SURE THE DATA IS CORRECT/ AUTHORIZATION IS THERE (NEEDED FOR READ/WRITE BLOCK
      delay(10);
  write_Block(5, entry1);                                        //WRITES MESSAGE INTO TAG
//    delay(10);
  read_data();                                           //READS DATA FROM TAG
//  delay(10);

}
/*****************************HALT******************************************************************************************/

void halt()                                             //executes mifare halt command on selected tag
{
  //Halt tag
  rfid.write((uint8_t)0xff);
  rfid.write((uint8_t)0x00);
  rfid.write((uint8_t)0x01);
  rfid.write((uint8_t)0x93);
  rfid.write((uint8_t)0x94);
}
/************************************************************************************************************************/
void flushSoftSerial () {               //clears buffer
  while (rfid.available() > 0) {
    rfid.read();
  }
}
/************************lookForSeekResponse***********************************************************************************************/
void lookForSeekResponse()                                    //looks for response from seek command. if there is a tag there, the "seeking" response comes back first, then it comes back with a "found" response
{
  int firstTimeoutCounter = 0;
  bool foundFirstResponse = false;
  while (!foundFirstResponse && firstTimeoutCounter < TIMEOUT_COUNTER_LIMIT) //while command was sent out and time hasnt run out
  {
    firstTimeoutCounter++;                                  //increment time
    if (rfid.available() > 0)                               //if something is in the buffer
      foundFirstResponse = (rfid.peek() == 0xFF);           //first response is true if header is correct
  }
  if (foundFirstResponse)                                   //if first response is true
  {
    for (int i = 0; i < 6; i++)                             //parse through data
    {
      Str1[i] = rfid.read();              
    }

    int secondTimeoutCounter = 0;
    bool foundSecondResponse = false;
    while (!foundSecondResponse && secondTimeoutCounter < TIMEOUT_COUNTER_LIMIT)    //while second response found and time not out
    {
      secondTimeoutCounter++;           //increment second timer (this is to make sure we dont get stuck)
      if (rfid.available() > 0)         //if theres something in the buffer
        foundSecondResponse = (rfid.peek() == 0xFF); //foundSecondResponse true if header is correct
    }
    if (!foundSecondResponse) {                 //if second response not true
      tagFound = 0;                             //tag not found
    }
    if (foundSecondResponse)                    //if it was found
    {
      tagFound++;                               //increment tagfound
      for (int i = 0; i < 10; i++)              //parse through data, storing it into Str1
      {
        Str1[i] = rfid.read();
      }
    }
  }
}
/***********SERIAL NUMBER**********************************************************************************************************/

void print_serial()                                 //PRINTS SERIAL NUMBER BY PRINTING STR1
{
  if (tagFound == 1) {                               //print to serial port
    Serial.print("Serial number: ");
    Serial.print(Str1[8], HEX);
    Serial.print(Str1[7], HEX);
    Serial.print(Str1[6], HEX);
    Serial.print(Str1[5], HEX);
    Serial.println();

    delay(printDelay);
    
    
  }
}

void read_serial()                                //reads serial number and then uses print_serial to print it
{
  flushSoftSerial();                              //gets rid of anything in the buffer
  seek1();                                        //seeks tags. when tag found, selects it and returns the serial number
  delay(10);
  lookForSeekResponse();                          //checks if there is a tag there
  delay(10);
  print_serial();                                 //prints serial number if tag present
}
/**********************READ AND PRINT DATA***********************************************************************************************/
void read_data( )                                  //reads data stored in a block
{
  int amount = sizeOfEntry + 6;
  
  flushSoftSerial();                              //if anything is in buffer, it gets rid of it
    if (tagFound == 1) {
      Authenticate(pickBlock);                             //authenticates tag **** if using write function, COMMENT THIS OUT  
      read_Block (5, Data);                       //calls read function
  if (rfid.available() > 0) {                     //if something is in buffer
    for(int i=0; i<amount;i++){                       
          Data[i] = rfid.read();         //stores information from buffer into data
       
          delay(12);
        }
      printData();                                  //prints the data from specified block
    }
  }
}
void printData()                                  //prints data from blocks in tag
{
   Display.print("Data:");
   delay(12);
   
   for(int i=5;i< (sizeOfEntry + 6);i++){
      charData[i] = Data[i];
      Display.print(charData[i]);
    }
   Display.println("");
 }

/**************************SEEK********************************************************************************************/
void seek1()                                     //looks for tags and selects them
{
  //search for RFID tag
  rfid.write((uint8_t)0xff);                    //header
  rfid.write((uint8_t)0x00);                    //reserve
  rfid.write((uint8_t)0x01);                    //length of command and response
  rfid.write((uint8_t)0x82);                    //command
  rfid.write((uint8_t)0x83);                    //check sum
  delay(10);
}

/***********************AUTHENTICATE AND PRINT AUTH****************************************************************************************************/

void Authenticate(int block)                                     //authenticates the tag so information of tag can be changed
{  
//Serial.println("in authenticate");
 
  int chkSum = (0x00 + 0x03  + 0x85 + block + 0xff); //finds check sum
  if (tagFound == 0) {
    Serial.println("tag not found");
  }
 
  if (tagFound == 1) {
    Serial.println("tag found");
    rfid.write((uint8_t)0xff);                            //header
    rfid.write((uint8_t)0x00);                            //reserve
    rfid.write((uint8_t)0x03);                            //length of command and response
    rfid.write((uint8_t)0x85);                            //command
    rfid.write((uint8_t)block);                            //block number
    rfid.write((uint8_t)0xff);                            //key type
    rfid.write((uint8_t)chkSum);                          //check sum
    delay(10);
     if(rfid.available() >0) {
    for (int i = 0; i < 6; i++) {      
      auth[i] = rfid.read();        
    }
    printAuth();   
  }
  }
}
void printAuth()                                        //prints authenticate response
{
    if (tagFound == 1) {                                //if a tag is found, prints to serial once
  Serial.print("auth code:  ");
  Serial.print(auth[0], HEX );
  Serial.print(auth[1], HEX );
  Serial.print(auth[2], HEX );
  Serial.print(auth[3], HEX );
  Serial.print(auth[4], HEX );
  Serial.print(auth[5], HEX);

  Serial.println();
      }
  //    else {
  //      Serial.println("flag3 wasnt 1");
  //      Serial.println(flag3);
}
//    }
//}

/************READ AND WRITE INTO BLOCK 5*****************************************************************************************************************/
void read_Block(int block, int * Data)                                       //command for reading a specified block
{

  int chkSum = (0x00 + 0x02 + 0x86 + block); //finds check sum

  if (tagFound == 1) {                                                        //if a tag is found, it writes and prints this once
    rfid.write((uint8_t)0xff);                                                //header
    rfid.write((uint8_t)0x00);                                                //reserve
    rfid.write((uint8_t)0x02);                                                //length
    rfid.write((uint8_t)0x86);                                                //command
    rfid.write((uint8_t)block);                                                //block
    rfid.write((uint8_t)chkSum);                                             //checksum
    delay(10);
  }
}

int write_Block(int block, String entry)                                      //command to write data into specified block
{
  int sSize = 0;
  int chkSum = 0;
  int byteLength = 2;
  
  sSize = entry.length();
  byteLength += sSize;
  
  chkSum = (byteLength + 0x89 + block);

  for(int i = 0; i < sSize; i++)chkSum += entry.charAt(i);
  
  flushSoftSerial();  
  if (tagFound == 1)
  {
    Authenticate(pickBlock);
    flushSoftSerial();
    rfid.write((uint8_t)0xff); //header
    rfid.write((uint8_t)0); //reserve
    rfid.write((uint8_t)byteLength); //length
    rfid.write((uint8_t)0x89); //command (1)
    rfid.write((uint8_t)block); // block 5(2)

    for(int i = 0; i < sSize; i++)rfid.write((uint8_t)entry.charAt(i));
    
    rfid.write((uint8_t)chkSum); //checksum
    delay(100);
    
    if(rfid.available()>0) 
    {                //if something is in the buffer
      for(int i =0; i<21; i++)
      {
        Data[i] = rfid.read();         //store each char in writeData
      }
    }

    if(Data[4] ==0x46)  return WRITE_FAIL;             //error code
    if(Data[4] == 0x4E) return NO_TAG;
    if(Data[4] == 0x58) return UNABLE_READ_AFT_WRITE;
    if(Data[4] == 0x55) return READ_AFT_WRITE_FAIL;
    return SUCCESS;
   }
}

void errorCheck(int input)
{
  switch (input) {
    case WRITE_FAIL:   
      Display.println("Write Failed");
      break;
    case NO_TAG:                                  // your hand is close to the sensor
      Display.println("No tag Present");
      break;
   case UNABLE_READ_AFT_WRITE:
      Display.println("Unable to read after write");
      break;
   return;
  }

}


