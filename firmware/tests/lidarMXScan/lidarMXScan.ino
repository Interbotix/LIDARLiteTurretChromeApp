/***************************
 * AXSimpleTest
 * This sketch sends positional commands to the AX servo 
 * attached to it - the servo must set to ID # 1
 * The sketch will send a value, i, to the servo.
 * 'For' loops are used to increment and decrement the value of 'i'
 ***************************/

//import ax12 library to send DYNAMIXEL commands
#include <ax12.h>
#include <EEPROM.h>
#include <I2C.h>


#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.


int i = 0;
int servoMoving = 0;
int distance;
int panGoalPositon = 0;
int panCurrentPosition;
int tempGoalPosition;
void setup()
{
  Serial.begin(115200);
  delay(500);
  SetPosition(1,0); //set the position of servo # 1 to '0'
  delay(100);
  SetPosition(2,2048); //set the position of servo # 1 to '0'

  //ax12SetRegister2(1,AX_GOAL_SPEED_L,64);
  delay(5);

  Serial.println("LidarLite Scanner1");
  I2c.begin(); // Opens & joins the irc bus as master
  delay(100); // Waits to make sure everything is powered up before sending or receiving data  
  I2c.timeOut(50); // Sets a timeout to ensure no locking up of sketch if I2C communication fails
  
  //delay(5000);
  Serial.println("LidarLite Scanner2");
  


  
  
  //clear EEPROM
  for( int j = 0; j < 255; j++)
  {
  
//    
//  Serial.print(" EEPROM.write(");
//  Serial.print(j);
//  Serial.print(", ");
//  Serial.print(j);
//  Serial.println(");");
//  
    EEPROM.write(i, 0);
    //delay(500);
  }
  
  
  Serial.println("LidarLite Scanner3");
  SetPosition(1,1024); //set the position of servo # 1 to '0'
  pinMode(0, OUTPUT);
  digitalWrite(0,HIGH);
  
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
   delay(500);
  digitalWrite(2,LOW);
  
}

void loop()
{
    servoMoving = -1;
    while(servoMoving == -1)
    {
      
      servoMoving =   ax12GetRegister(1, AX_MOVING, 1);
      delay(5);
    }
    
    
    if(servoMoving == 0)
    {
      delay(100);
      
    //ax12SetRegister2(1,AX_GOAL_SPEED_L,64);
    delay(5);
    
      //dump EEPROM
      for( i = 0; i< 1024; i++)
      {  
        
        byte distanceHigh = EEPROM.read(i);
        byte distanceLow = EEPROM.read(i+1);
        Serial.write(0xff);//header1
        Serial.write(0xff);//header2
        Serial.write(highByte(i));//position high byte
        Serial.write(lowByte(i));//position low byte
        Serial.write(distanceHigh);//distance high byte
        Serial.write(distanceLow);//distance low byte
        Serial.write(255-(highByte(i) + lowByte(i) + highByte(distance) + lowByte(distance))%256);     
  
  
  
  
      }
      
      //
      
      
      
      if(panGoalPositon == 0)
      {
        panGoalPositon = 4095;
        
      }
      else
      {
        panGoalPositon = 0;
      }
      
      delay(5);
      SetPosition(1,panGoalPositon); //set the position of servo # 1 to '0'
      
    }
   
    
//    delay(5);
//    panCurrentPosition =   ax12GetRegister(1, AX_PRESENT_POSITION_L, 2);  //get current position from servo
//    distance = llGetDistanceAverage(3);                                   //get distance from LIDAR lite
//    
//    EEPROM.write(2*(panCurrentPosition/4), highByte(distance));
//    EEPROM.write(2*(panCurrentPosition/4) + 1, lowByte(distance));
//    
//  
  

  
  
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
  while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
    nackack = I2c.read(LIDARLite_ADDRESS,myAddress, numOfBytes, arrayToSave); // Read 1-2 Bytes from LIDAR-Lite Address and store in array
    delay(2); // Wait 2 ms to prevent overpolling
  }
  return arrayToSave[2]; // Return array for use in other functions
}



int llGetDistance(){
  llWriteAndWait(0x00,0x04); // Write 0x04 to register 0x00 to start getting distance readings
  byte myArray[2]; // array to store bytes from read function
  llReadAndWait(0x8f,2,myArray); // Read 2 bytes from 0x8f
  int distance = (myArray[0] << 8) + myArray[1];  // Shift high byte [0] 8 to the left and add low byte [1] to create 16-bit int
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

