/***************************
 * AXSimpleTest
 * This sketch sends positional commands to the AX servo 
 * attached to it - the servo must set to ID # 1
 * The sketch will send a value, i, to the servo.
 * 'For' loops are used to increment and decrement the value of 'i'
 ***************************/

//import ax12 library to send DYNAMIXEL commands
#include <ax12.h>
#include <I2C.h>
#include <Commander.h>


#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.
//
//#define PAN_MIN 424
//#define PAN_MAX 600
#define PAN_MIN 0
#define PAN_MAX 1023

#define MAPPED_PAN_MIN 341
#define MAPPED_PAN_MAX 3754

#define DEFAULT_SPEED 300
           

int i = 0;
int servoMoving = 0;
int distance;
int panGoalPositon = 0;
int panCurrentPosition;
int tempGoalPosition;

long previousMillis = 0;        // will store last time LED was updated
long previousMillisReport = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 50;           // interval at which to blink (milliseconds)

long intervalReport = 33;//30hz

int ledState = LOW;             // ledState used to set the LED

boolean sweeping = false;


Commander command = Commander();


void setup()
{
  
  ax12SetRegister2(1,AX_GOAL_SPEED_L,DEFAULT_SPEED);
  delay(5);
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
   delay(50);
  digitalWrite(2,LOW);
   delay(50);
  
  Serial.begin(115200);
  delay(500);
  SetPosition(1,PAN_MIN); //set the position of servo # 1 to '0'
  delay(5);
  SetPosition(2,512); //set the position of servo # 2 to '512'
  delay(5);

  SetPosition(2,512); //set the position of servo # 2 to '512'
  delay(5);

  I2c.begin(); // Opens & joins the irc bus as master
  delay(100); // Waits to make sure everything is powered up before sending or receiving data  
  I2c.timeOut(50); // Sets a timeout to ensure no locking up of sketch if I2C communication fails
  
  //SetPosition(1,1024); //set the position of servo # 1 to '0'
  pinMode(0, OUTPUT);
  digitalWrite(0,HIGH);
  
  
  
}

void loop()
{
  
  //read commander message
  
  if(command.ReadMsgs() > 0)
  {
    
    
    //start sweep
    if(command.ext == 1)
    {
      
      sweeping = true;
    } 
    //stop sweep
    else if(command.ext == 2)
    {
      sweeping = false;
    }    
    
    
    //set speed from pan bytes
    else if(command.ext == 3)
    {     
      delay(5);
      ax12SetRegister2(1,AX_GOAL_SPEED_L,command.pan);
      delay(5);
    }    
    
    
  }
 
  
  if(sweeping == true)
  {
    intervalReport = random(20, 40);
    intervalReport = 10;
    
      unsigned long currentMillis = millis();
   
    if(currentMillis - previousMillis > interval) {
      // save the last time you blinked the LED 
      previousMillis = currentMillis;   
  
      servoMoving =   ax12GetRegister(1, AX_MOVING, 1);
  
    
  
  
    }
    
    
    
      unsigned long currentMillisReport = millis();
   
    if(currentMillisReport - previousMillisReport > intervalReport) 
    {
      // save the last time you blinked the LED 
      previousMillisReport = currentMillisReport;   
  
      servoMoving =   ax12GetRegister(1, AX_MOVING, 1);
  
  
      panCurrentPosition =   ax12GetRegister(1, AX_PRESENT_POSITION_L, 2);  //get current position from servo
      //distance = llGetDistanceAverage(3);                                   //get distance from LIDAR lite
      distance = llGetDistance();
      //    panCurrentPosition =   (panCurrentPosition + 10 )%4096;
      //distance = 100;                                   //get distance from LIDAR lite
    
      if(panCurrentPosition > PAN_MIN && panCurrentPosition < PAN_MAX)
      {

           int mappedPanCurrentPosition = map(panCurrentPosition, PAN_MIN, PAN_MAX, MAPPED_PAN_MIN, MAPPED_PAN_MAX);           
           
           
           
            Serial.write(0xff);//header1
            Serial.write(0xff);//header2
            Serial.write(highByte(mappedPanCurrentPosition));//position high byte
            Serial.write(lowByte(mappedPanCurrentPosition));//position low byte
            Serial.write(highByte(distance));//distance high byte
            Serial.write(lowByte(distance));//distance low byte
            Serial.write(255-(highByte(mappedPanCurrentPosition) + lowByte(mappedPanCurrentPosition) + highByte(distance) + lowByte(distance))%256);     
      }
      
      
  
  
    }
    
    
      
      
      if(servoMoving == 0)
      {
  
        
        //
        
        
        
        if(panGoalPositon == PAN_MIN)
        {
          panGoalPositon = PAN_MAX;
          
        }
        else
        {
          panGoalPositon = PAN_MIN;
        }
        
        delay(5);
        SetPosition(1,panGoalPositon); //set the position of servo # 1 to '0'
        servoMoving = 1;
      }
     
      
  //    delay(5);
  //    
  //    EEPROM.write(2*(panCurrentPosition/4), highByte(distance));
  //    EEPROM.write(2*(panCurrentPosition/4) + 1, lowByte(distance));
  //    
  //  
  
  }
  
  
}



/* ==========================================================================================================================================
Basic read and write functions for LIDAR-Lite, waits for success message (0 or ACK) before proceeding
=============================================================================================================================================*/

// Write a register and wait until it responds with success
void llWriteAndWait(char myAddress, char myValue){
  
  uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
  while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
    nackack = I2c.write(LIDARLite_ADDRESS,myAddress, myValue); // Write to LIDAR-Lite Address with Value
    delay(2); // Wait 2 ms to prevent overpolling
    
    
  }
}

// Read 1-2 bytes from a register and wait until it responds with sucess
byte llReadAndWait(char myAddress, int numOfBytes, byte arrayToSave[2]){
  
  uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
  while (nackack != 0 ){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
    nackack = I2c.read(LIDARLite_ADDRESS,myAddress, numOfBytes, arrayToSave); // Read 1-2 Bytes from LIDAR-Lite Address and store in array
    delay(2); // Wait 2 ms to prevent overpolling
    
  }
  
  return arrayToSave[2]; // Return array for use in other functions
}



int llGetDistance(){
  Serial.println("get ready to write");
  llWriteAndWait(0x00,0x04); // Write 0x04 to register 0x00 to start getting distance readings
  byte myArray[2]; // array to store bytes from read function
  Serial.println("get ready to read");
  llReadAndWait(0x8f,2,myArray); // Read 2 bytes from 0x8f
  int distance = (myArray[0] << 8) + myArray[1];  // Shift high byte [0] 8 to the left and add low byte [1] to create 16-bit int
  Serial.println("done");
  return(distance);
}


int llGetDistanceAverage(int numberOfReadings){ 
  if(numberOfReadings < 2){
    numberOfReadings = 2; // If the number of readings to be taken is less than 2, default to 2 readings
  }
  int sum = 0; // Variable to store sum
  for(int i = 0; i < numberOfReadings; i++){ 
      sum = sum + llGetDistance(); // Add up all of the readings
  }
  sum = sum/numberOfReadings; // Divide the total by the number of readings to get the average
  return(sum);
}

