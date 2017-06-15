
#define DisplaySerial Serial
#define printDelay 12
#define SIZE_OF_DATA 21

#include <Picaso_Const4D.h>
#include <Picaso_Serial_4DLib.h>
#include <SoftwareSerial.h>


SoftwareSerial rfid(7, 8);
Picaso_Serial_4DLib Display(&DisplaySerial); 


//Prototypes
void halt(void);
void parse1(void);
void print_serial(void);
void read_serial(void);
void seek1(void);
void set_flag(void);
void read_Block(int block, int Data[21]);
void write_Block(int block,int writeData[21]);
void Authenticate(void);
void printData(void);
void parse2(void);
void parse3(void);
void printAuth(void);
void read_data(void);
void set_flag3(void);


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
String displayData;

int TIMEOUT_COUNTER_LIMIT = 500;

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
  displayData.reserve(SIZE_OF_DATA);
  
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
//  Authenticate();                                       //MAKES SURE THE DATA IS CORRECT/ AUTHORIZATION IS THERE (NEEDED FOR READ/WRITE BLOCK
      delay(10);
//    write_Block(5, writeData);                                        //WRITES MESSAGE INTO TAG
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
void read_data()                                  //reads data stored in a block
{
  flushSoftSerial();                              //if anything is in buffer, it gets rid of it
    if (tagFound == 1) {
      Authenticate();                             //authenticates tag **** if using write function, COMMENT THIS OUT  
      read_Block (5, Data);                       //calls read function
  if (rfid.available() > 0) {                     //if something is in buffer
    for(int i=0; i<21;i++){                       
          Data[i] = rfid.read();                  //stores information from buffer into data
          delay(1);
        }
      printData();                                  //prints the data from specified block
    }
  }
}
void printData()                                  //prints data from blocks in tag
{
  
//    //print to serial port
//    Serial.print("Card Data:  ");
//    Serial.print(Data[0], HEX);
//    Serial.print(Data[1], HEX);
//    Serial.print(Data[2], HEX);
//    Serial.print(Data[3], HEX);
//    Serial.print(Data[4], HEX);
//   
//    for(int i=5;i<=21;i++){
//      Serial.write(Data[i]);
//    }
//    Serial.print("\n");

   Display.print("Card Data:  ");
    
    for(int i=5;i<=21;i++){
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

void Authenticate()                                     //authenticates the tag so information of tag can be changed
{
//  Serial.println("in authenticate");
  if (tagFound == 0) {
    Serial.println("tag not found");
  }
  if (tagFound == 1) {
    Serial.println("tag found");
    rfid.write((uint8_t)0xff);                            //header
    rfid.write((uint8_t)0x00);                            //reserve
    rfid.write((uint8_t)0x03);                            //length of command and response
    rfid.write((uint8_t)0x85);                            //command
    rfid.write((uint8_t)0x05);                            //block number
    rfid.write((uint8_t)0xff);                            //key type
    rfid.write((uint8_t)0x18c);                           //check sum
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

  
  
  if (tagFound == 1) {                                                        //if a tag is found, it writes and prints this once
    rfid.write((uint8_t)0xff);                                                //header
    rfid.write((uint8_t)0x00);                                                //reserve
    rfid.write((uint8_t)0x02);                                                //length
    rfid.write((uint8_t)0x86);                                                //command
    rfid.write((uint8_t)block);                                                //block
    rfid.write((uint8_t)0x8d);                                                //checksum
    delay(10);
  }
}

void write_Block(int block, int * writeData)                                      //command to write data into specified block
{
  flushSoftSerial();  
  if (tagFound == 1)
  {
    Authenticate();
    flushSoftSerial();
    rfid.write((uint8_t)0xff); //header
    rfid.write((uint8_t)0); //reserve
    rfid.write((uint8_t)0x12); //length
    rfid.write((uint8_t)0x89); //command (1)
    rfid.write((uint8_t)0x05); // block 5(2)
    rfid.write((uint8_t)'h'); //written (3)
    rfid.write((uint8_t)'o'); //written (4)
    rfid.write((uint8_t)'w'); //written (5)
    rfid.write((uint8_t)'s'); //written (6)
    rfid.write((uint8_t)' '); //written (7)
    rfid.write((uint8_t)'i'); //written (8)
    rfid.write((uint8_t)'t'); //written (9)
    rfid.write((uint8_t)' '); //written (a)
    rfid.write((uint8_t)'g'); //written (b)
    rfid.write((uint8_t)'o'); //written (c)
    rfid.write((uint8_t)'i'); //written (d)
    rfid.write((uint8_t)'n'); //written (e)
    rfid.write((uint8_t)'g'); //written (f)
    rfid.write((uint8_t)'?'); //written (10)
    rfid.write((uint8_t)':'); //written (11)
    rfid.write((uint8_t)')'); //written (12)    
    rfid.write((uint8_t)1588); //checksum
    delay(100);
//    Serial.println ("Data written");
    if(rfid.available()>0) {                //if something is in the buffer
      for(int i =0; i<21; i++){
        writeData[i] = rfid.read();         //store each char in writeData
      }
    }
//    printwriteData();
    if(writeData[4] ==0x46) {               //error code
      Serial.println("Write Failed");
    }
    if(writeData[4] == 0x4E) {              //error code
      Serial.println("No tag Present");
    }
    if(writeData[4] == 0x58){               //error code
      Serial.println("Unable to read after write");
    }
    if(writeData[4] == 0x55){               //error code
      Serial.println("Read after write failed");
    }
  }
}

void printwriteData()                                  //prints data from blocks in tag
{
  
    //print to serial port
    Serial.print("Card write Data:  ");
//    Serial.print(writeData[0], HEX);
//    Serial.print(writeData[1], HEX);
//    Serial.print(writeData[2], HEX);
//    Serial.print(writeData[3], HEX);
//    Serial.print(writeData[4], HEX);
    Serial.write(writeData[5]);
    Serial.write(writeData[6]);
    Serial.write(writeData[7]);
    Serial.write(writeData[8]);
    Serial.write(writeData[9]);
    Serial.write(writeData[10]);
    Serial.write(writeData[11]);
    Serial.write(writeData[12]);
    Serial.write(writeData[13]);
    Serial.write(writeData[14]);
    Serial.write(writeData[15]);
    Serial.write(writeData[16]);
    Serial.write(writeData[17]);
    Serial.write(writeData[18]);
    Serial.write(writeData[19]);
    Serial.write(writeData[20]);
    Serial.write(writeData[21]);
    Serial.println();
  }

