//Simple Demo that demonstrates 'print' and 'println' new functionality.

#define DisplaySerial Serial

//-------Picaso DISPLAYS-------

#include <Picaso_Const4D.h>
#include <Picaso_Serial_4DLib.h>

//use Serial0 to communicate with the display.
Picaso_Serial_4DLib Display(&DisplaySerial); 

//---------END-----------------

void setup() {
  //For handling errors
 
  //
  //5 second timeout on all commands  
  Display.TimeLimit4D   = 5000 ;
  Serial.begin(9600) ;

  //--------------------------------Optional reset routine-----------------------------------
  //Reset the Display using D4 of the Arduino (if using the new 4D Arduino Adaptor - Rev 2)
  //If using the old 4D Arduino Adaptor (Rev 1), change D4 to D2 below.
  //If using jumper wires, reverse the logic states below. 
  //Refer to the accompanying application note for important information.
  pinMode(4, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(4, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(4, 0);  // unReset the Display via D4
  //-----------------------------------------END---------------------------------------------
  
  delay (5000); //let the display start up  

  // put your main code here, to run repeatedly:

Display.gfx_ScreenMode(PORTRAIT);
  //Display.gfx_BGcolour(WHITE) ; //change background color to white
  Display.gfx_Cls();            //clear the screen
  Display.println("Never gonna give you up");
  delay(1000);
  Display.println("Never gonna let you down");
  delay(1000);
  Display.println("Never gonna go around");
  delay(1000);
  Display.println("And hurt you");
  delay(1000);

  
  
  

}


void loop() {
  
}













