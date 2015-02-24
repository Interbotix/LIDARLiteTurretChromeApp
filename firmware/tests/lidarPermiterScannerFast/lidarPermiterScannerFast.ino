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

//TODO: Auto sense max bounds based on DYNAMIXEL model (but still have overrides)
//Uncomment one of the following lines - #define AX for AX servos and #define MX for MX servos
//#define AX
#define MX

//position definition for AX servos
//TODO: autocalculate mapped limits
#ifdef AX
  #define PAN_MIN  0      //Minimum Servo Position
  #define PAN_MAX  1023   //Maximum Servo Position
  #define MAPPED_PAN_MIN 341  //Minimum Physical Position on a scale from 0-4096. mapped value = 4095 * Physical limit/360 
  #define MAPPED_PAN_MAX 3754 //Maximum Physical Position on a scale from 0-4096. mapped value = 4095 * Physical limit/360 
  #define TILT_DEFAULT 512    //default tilt position / servo 2
#endif

//position definition for MX12 servos
#ifdef MX
  #define PAN_MIN  0      //Minimum Servo Position
  #define PAN_MAX  4095   //Maximum Servo Position
  #define MAPPED_PAN_MIN 0  //Minimum Physical Position on a scale from 0-4096. Mapped value is the same as the PAN value for MX servos
  #define MAPPED_PAN_MAX 4095 //Maximum Physical Position on a scale from 0-4096.  Mapped value is the same as the PAN value for MX servos
  #define TILT_DEFAULT 2048  //default tilt position / servo 2
#endif


//defualt startup speed. Here are some reccomendations
//AX12A AX18A     300
//MX28 MX64 MX106 300 
//AX12W MX 12w    64 PAN


//.25rps = 131
//.5rps = 263
//1 rps = 526

#define DEFAULT_SPEED 526
#define RPS .5 //rotations per second
#define CPS 4.5

//LIDAR lite definitions
#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.
           

int servoMoving = 0;            //whether or not the servo is moving, 0 = not moving, 1 = moving
int distance;                   //current distance reading from LIDAR Lite
int panGoalPositon = PAN_MIN;   // Goal Position, where we want the servo to go
int panPreviousPosition;         //Actual position of the servo
long panPreviousInterval;
int panCurrentPosition;         //Actual position of the servo
int panPredictedPosition;         //predicted position of the servo

long previousMillisMoving = 0;  // last time we requested movement data from the servo
long previousMillisReport = 0;  //  last time distance/angle was reported out to the serial port
long previousMillisPosition = 0;  //  last time distance/angle was reported out to the serial port
int intervalMoving = 50;        // interval at which to request moving data from the servo
int intervalReport = 33;        // interval at which to report the distance/angle data 30hz
int intervalPosition = 150;        // interval at which to report the distance/angle data 30hz

int servoDirection = 0; //-1 ccw, 1 cw
float rotationsPerSecond = RPS;
boolean scanActive = false;     //false if the servo is not activley scanning, true if it is activley scanning

Commander command = Commander();  //create commander object to accept serial commands


void setup()
{
  Serial.begin(115200); //start serial port communication
  delay(500);           //delay to let DYNAMIXEL services start  
  
  ax12SetRegister2(1,AX_GOAL_SPEED_L,DEFAULT_SPEED);    //set the speed on the pan servo
  delay(33);                                             //delay before next DYNAMIXEL command
  
  SetPosition(1,PAN_MIN);     //set the position of servo # 1 to its starting/min position
  delay(33);                   //delay before next DYNAMIXEL command
  
  SetPosition(2,TILT_DEFAULT); //set the position of servo # 2 to  its defualt position. If the servo is not present, nothing will happen
  delay(33);                    //delay before next DYNAMIXEL command
  
  I2c.begin();     // Opens & joins the i2c bus as master
  delay(100);      // Waits to make sure everything is powered up before sending or receiving data  
  I2c.timeOut(50); // Sets a timeout to ensure no locking up of sketch if I2C communication fails
  
  //program is ready, turn on the LED
  pinMode(0, OUTPUT);   //set pin mode 
  digitalWrite(0,HIGH); //turn LED on
}// end setup

void loop()
{
  
  //check if commander messages are avaialble from the app
  if(command.ReadMsgs() > 0)
  {
    //extended instruction is 1, start scanning
    if(command.ext == 1)
    {  
      digitalWrite(0,LOW);
      scanActive = true;
    } 
    //extended instruction is 2, stop scanning
    else if(command.ext == 2)
    {
      scanActive = false;

      SetPosition(1,panCurrentPosition); //set the position of servo # 1 to '0'
      delay(33);

    }    
 
    //extended instruction is 3, set speed from pan bytes
    else if(command.ext == 3)
    {     
      
      rotationsPerSecond = (float)command.pan / 1024.0;
    }        
  }
  
  
  if(scanActive == true)
  {
    
    
     panPreviousInterval = millis() - previousMillisPosition;
    //compare the currrent number if milliseconds with the last time the servo was polled
    if(panPreviousInterval > intervalPosition) 
    {
//      Serial.print(panCurrentPosition);
//      Serial.print(" ");
//      Serial.println(panPredictedPosition);
      panCurrentPosition =   ax12GetRegister(1, AX_PRESENT_POSITION_L, 2);  //get current position from servo
      //check that the reported position is within the range (incase of bad return or no return data (-1) )
      
      
//      Serial.print((float)(abs(panCurrentPosition-panPreviousPosition))/panPreviousInterval);
//      Serial.print(" ");
//      
      previousMillisPosition = millis();     
      servoMoving =   ax12GetRegister(1, AX_MOVING, 1);
      
      
     
      if(panCurrentPosition >= 0)
      {
        
        panPredictedPosition = panCurrentPosition;
      }
      
      panPreviousPosition = panCurrentPosition;
    }
    
    
    
    
    //compare the currrent number if milliseconds with the last time the servo was polled
    if(millis() - previousMillisMoving > intervalMoving) 
    {
      // save the last time the servo was polled for moving status
      previousMillisMoving = millis();     
      servoMoving =   ax12GetRegister(1, AX_MOVING, 1);
    }
    
    
    //compare the currrent number if milliseconds with the last time the distance/position data was reported
    if(millis() - previousMillisReport > intervalReport) 
    {
      
        float rotationPercent = (CPS / (millis() - previousMillisPosition));
        panPredictedPosition = panPredictedPosition + (servoDirection * rotationPercent);
//        Serial.print(" ");
//        Serial.print(panPredictedPosition);
//        Serial.print(" ");
//        Serial.println(rotationPercent);
      
       //figure out current position from last position
       
      
        distance = llGetDistance();                                           //get distance from LIDAR lite
        //distance = llGetDistanceAverage(3);                                 //get distance from LIDAR lite, averaging version
    
      //check that the reported position is within the range (incase of bad return or no return data (-1) )
      if(panPredictedPosition > PAN_MIN && panPredictedPosition < PAN_MAX)
      {
        // save the last distance/position was reported
        previousMillisReport = millis();   
       
        //map the current position to a 0-4095 / 0-360 scale. This helps us work with AX servos with a limited 
        int mappedPanCurrentPosition = map(panPredictedPosition, PAN_MIN, PAN_MAX, MAPPED_PAN_MIN, MAPPED_PAN_MAX);           
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
  
        
        if(panGoalPositon == PAN_MIN)
        {
          servoDirection = -1;
          panGoalPositon = PAN_MAX;
          panPredictedPosition = PAN_MIN;     
          panCurrentPosition = PAN_MIN;
          
          previousMillisPosition = millis();     
          delay(33);
//      Serial.print("HIT MAX  ");
//      Serial.print(panCurrentPosition);
//      Serial.print(" ");
//      Serial.println(panPredictedPosition);
          
        }
        else
        {
          servoDirection = 1;
          panGoalPositon = PAN_MIN;
          panPredictedPosition = PAN_MAX;    
//          panCurrentPosition = PAN_MAX; 
//      Serial.print("HIT MIN");  
//      Serial.print(panCurrentPosition);
//      Serial.print(" ");
//      Serial.println(panPredictedPosition);
              
          previousMillisPosition = millis();     
          delay(33);
        }
        
        SetPosition(1,panGoalPositon); //set the position of servo # 1 to '0'
        servoMoving = 1;
      }
     
  
  }
  
 
  
  
}//end loop



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

