#define DisplaySerial Serial
#define printDelay 12
#define SIZE_OF_DATA 30

#define READ_TAG  1
#define WRITE_TAG 2
#define WRITE_FAIL 1000
#define UNABLE_READ_AFT_WRITE 1001
#define NO_TAG 1002
#define READ_AFT_WRITE_FAIL 1003
#define SUCCESS 999

#include <stdio.h>
#include <string.h>
#include <Picaso_Const4D.h>
#include <Picaso_Serial_4DLib.h>
#include <SoftwareSerial.h>

SoftwareSerial rfid(7, 8);
Picaso_Serial_4DLib Display(&DisplaySerial);

struct Cart
{
  String ID;
  String product;
  String ePoint;
  String sPoint;
  double timer;
  int Amount;
};

Cart currentRead;
Cart lastRead;

//Global var
int flag = 0;
int flag2 = 0;
int flag3 = 0;
int Str1[10];

String entry1 = "ASHLEYFURNITURE";
int Data[SIZE_OF_DATA];
char charData[SIZE_OF_DATA];
int auth[6];
int var[10];
int tagFound = 0;
int pickBlock = 0x05;
String Temp;
String output1;
int TIMEOUT_COUNTER_LIMIT = 500;
int sizeOfEntry = 0;
//#####################################SETUP#############################################
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
  pinMode(4, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(4, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(4, 0);  // unReset the Display via D4
  //----------------------------------------------------------------------------------------------
  delay (5000); //let the display start up
  

  Display.gfx_ScreenMode(PORTRAIT);
  Display.gfx_Cls();            //clear the screen
  //-------------------------------------ENDof4DSetup-----------------------------------------------
  currentRead.product.reserve(30);
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
  while (rfid.available() > 0)
  {
    for (int i = 0; i < 10; i++)
    {
      if (rfid.available() > 0) 
      {
        var[i] = rfid.read();
      }
    }
  }
}
//
//##################################################MAIN LOOP###################################################################
void loop()
{
  



    //errorCheck(write_Block(pickBlock,entry1));

    read_serial();
   
    read_data(pickBlock,currentRead.product);

  //Display.println(Temp);
  //                                 //WRITES MESSAGE INTO TAG
  //delay(10);
  

  
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
//****************************************************************************************************************************
void read_serial()                                //reads serial number and then uses print_serial to print it
{
  flushSoftSerial();                              //gets rid of anything in the buffer
  seek1();                                        //seeks tags. when tag found, selects it and returns the serial number
  delay(10);
  lookForSeekResponse();                          //checks if there is a tag there
  delay(10);
  print_serial();                                 //prints serial number if tag present
}
/***********************AUTHENTICATE AND PRINT AUTH****************************************************************************************************/

void Authenticate(int block)                                     //authenticates the tag so information of tag can be changed
{  
//Serial.println("in authenticate");
 
  int chkSum = (0x00 + 0x03  + 0x85 + block + 0xff); //finds check sum
  if (tagFound == 0) {
    Display.println("tag not found");
  }
 
  if (tagFound == 1) {
    Display.println("tag found");
    rfid.write((uint8_t)0xff);                            //header
    rfid.write((uint8_t)0x00);                            //reserve
    rfid.write((uint8_t)0x03);                            //length of command and response
    rfid.write((uint8_t)0x85);                            //command
    rfid.write((uint8_t)block);                            //block number
    rfid.write((uint8_t)0xff);                            //key type
    rfid.write((uint8_t)chkSum);                          //check sum
    delay(10);
     if(rfid.available() >0) {
    for (int i = 0; i < 6; i++)
    {      
      auth[i] = rfid.read();        
      }
     }
  }
}
//****************************************************************************************************************
void errorCheck(int input)
{
  switch (input)
  {
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
//***********SERIAL NUMBER**********************************************************************************************************/

void print_serial()                                 //PRINTS SERIAL NUMBER BY PRINTING STR1
{
  if (tagFound == 1) 
  {                               //print to serial port
    Serial.print("Serial number: ");
    Serial.print(Str1[8], HEX);
    Serial.print(Str1[7], HEX);
    Serial.print(Str1[6], HEX);
    Serial.print(Str1[5], HEX);
    Serial.println();

    delay(printDelay);
  }
}
/**********************READ AND PRINT DATA***********************************************************************************************/
void read_data(int pickBlock, String &data)                                  //reads data stored in a block
{
  
  int i = 0;
  int rawData[30];
  char ray[30];
  
  flushSoftSerial();
  if (tagFound == 1) 
  {
    Authenticate(pickBlock);
    read_Block (pickBlock);
    while(rfid.available() > 0)
    {   
       rawData[i] = rfid.read();
       i++;
       delay(12);
    }  
    for(int y = 5;y<i;y++)
    {
     ray[y-5] = rawData[y];
      delay(12);
    }
    ray[i+1] = 0;
    String abe(ray);
    Temp = abe;
    
    delay(30);
  Display.println(Temp);
  }
}
 /************READ AND WRITE INTO BLOCK 5*****************************************************************************************************************/
void read_Block(int block)                                       //command for reading a specified block
{

  int chkSum = (0x00 + 0x02 + 0x86 + block); //finds check sum

  if (tagFound == 1) 
  {                                                        //if a tag is found, it writes and prints this once
    rfid.write((uint8_t)0xff);                                                //header
    rfid.write((uint8_t)0x00);                                                //reserve
    rfid.write((uint8_t)0x02);                                                //length
    rfid.write((uint8_t)0x86);                                                //command
    rfid.write((uint8_t)block);                                                //block
    rfid.write((uint8_t)chkSum);                                             //checksum
    delay(10);
  }
}
//***************************************************************************************************************************************
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

    if(Data[4] == 0x46) return WRITE_FAIL;             //error code
    if(Data[4] == 0x4E) return NO_TAG;
    if(Data[4] == 0x58) return UNABLE_READ_AFT_WRITE;
    if(Data[4] == 0x55) return READ_AFT_WRITE_FAIL;
    return SUCCESS;
   }
}
