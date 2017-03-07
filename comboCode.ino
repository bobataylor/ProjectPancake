#include <SPI.h>
#include <SD.h>

// Run a A4998 Stepstick from an Arduino UNO.
int x; 
int y;
#define BAUD (9600)

//open the coordinate file on teh sd card
File myfile;
String coords = "";
boolean repeat = true;

//current positions of x and y
int currentX = 0;
int currentY = 0;

//size of area
int tSize = 100;
int numSteps = tSize * 5;

//arrays for holding coordinates
int xCoords[0];
int yCoords[0];
int lengthR;

//setup the motor pins
int enablePin1 = 7;
int stepPin1 = 8;
int directionPin1 = 9;
int enablePin2 = 6;
int stepPin2 = 10;
int directionPin2 = 11;

void setup() 
{
  Serial.begin(BAUD);
  
  pinMode(enablePin1,OUTPUT); // Enable
  pinMode(stepPin1,OUTPUT); // Step
  pinMode(directionPin1,OUTPUT); // Dir
  digitalWrite(enablePin1,LOW); // Set Enable low

  pinMode(enablePin2,OUTPUT);
  pinMode(stepPin2,OUTPUT); // Step
  pinMode(directionPin2,OUTPUT); // Dir
  pinMode(enablePin2, LOW);

//--------SD-CARD-CODE--------------------------------------------------------
  Serial.print("Initializing card...");
  
  // declare default CS pin as OUTPUT
   pinMode(53, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization of the SD card failed!");
    return;
  }
  Serial.println("initialization of the SDcard is done.");
  
  // re-open the text file for reading:
  myfile = SD.open("coords.txt");

  int count = 0;
  while(myfile.available()){
      char comma = myfile.read();
      if(comma == ','){
        count++;
      }  
  }
  myfile.close();


  myfile = SD.open("coords.txt"); 
  int xCoords[count];
  int yCoords[count];
  int pos = 0;
  
  if (myfile)
  {
    Serial.println("coords.txt:");
    
    // read all the text written on the file
    while (myfile.available()) 
    {
      char line = myfile.read();
      
      if(repeat && line == ','){
        xCoords[pos] = coords.toInt();
        coords = "";
        repeat = false;
        
      }
      else if(!repeat && line == '\n'){
        yCoords[pos] = coords.toInt();
        coords = "";
        repeat = true;
        pos++;
      }
      else{
        coords.concat(line);
      }
    }
    // close the file:
    myfile.close();
    for(int i = 0; i < count; i++){
      Serial.print(xCoords[i]);
      Serial.print(",");
      Serial.print(yCoords[i]);  
      Serial.print("\n");   
    }
  } else 
  {
    // if the file didn't open, report an error:
    Serial.println("error opening the text file!");
  }
  lengthR = count;
  
//-----MOTOR-CODE--------------------------------------------------------------

  Serial.print("\nPrinting X: ");
  Serial.print(xCoords[0]);
  Serial.print("  ||  Printing Y: ");
  Serial.print(yCoords[0]);
  
  for (int i = 0; i < lengthR; i++){
    double ratio;

    //difference between current coordiinate and new coordinates
    int difX = currentX - xCoords[i];
    int difY = currentY - yCoords[i];
    int numStepsX = abs(numSteps * ((double)difX/500.0));
    int numStepsY = abs(numSteps * ((double)difY/500.0));

    //checks which coordinate is larger
    bool xLarger = numStepsX >= numStepsY;

    //ratio between x and y steps
    if (xLarger){
      if (numStepsY == 0){
        numStepsY++;
      }
      ratio = numStepsX / numStepsY;      
    } else {
      if (numStepsX == 0){
        numStepsX++;
      }
      ratio = numStepsY/ numStepsX;
    }
  
    bool positiveX = difX > 0;
    bool positiveY = difY > 0;

    //sets direction of x and y based on positive or negative distance
    if (positiveX){
        digitalWrite(directionPin1,HIGH); // Set Dir high
    } else {
        digitalWrite(directionPin1,LOW); // Set Dir high 
    }
    
    if (positiveY){
      digitalWrite(directionPin2,HIGH); // Set Dir high
    }else {
      digitalWrite(directionPin2,LOW); // Set Dir high
    }
    
    digitalWrite(enablePin1,LOW); // Set Enable low   

    //counter for slower stepper motor
    double counter = 1.2;

    if (xLarger){
      for(x = 0; x < numStepsX; x++) // Loop numStepsX times
      {
        //turns on other stepper motor when counter is greater than ratio
        if (counter > ratio){
           digitalWrite(stepPin2,HIGH); // Output high
        }
        digitalWrite(stepPin1,HIGH); // Output high
        delay(10); // Wait
  
        if(counter > ratio){
            digitalWrite(stepPin2,LOW); // Output high

            counter = 1.2;
        }
        digitalWrite(stepPin1,LOW); // Output low
        delay(10); // Wait
        counter++;
      }
      
    }else {
      for(x = 0; x < numStepsY; x++) // Loop numStepsX times
      {
        //turns on other stepper motor when counter is greater than ratio
        if (counter > ratio){
           digitalWrite(stepPin1,HIGH); // Output high
        }
        digitalWrite(stepPin2,HIGH); // Output high
        delay(10); // Wait
  
        if(counter > ratio){
            digitalWrite(stepPin1,LOW); // Output high

            counter = 1.2;
        }
        digitalWrite(stepPin2,LOW); // Output low
        delay(10); // Wait
        counter++;
      }
    }
    delay(1000); // pause one second       
     currentX = xCoords[i];
     currentY = yCoords[i];
     Serial.print("\nPrinting X: ");
     Serial.print(xCoords[i+1]);
     Serial.print("  ||  Printing Y: ");
     Serial.print(yCoords[i+1]);
   }
   currentX = 0;
   currentY = 0;  
}

void loop() {}
