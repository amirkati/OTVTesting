
// Step 1: Collect all the code that allows you to talk to “peripherals” - other devices like sensors and motors.
// Step 2: Write each basic function (need more information about aruco and ESP8266 WiFi Communication Module for example to start writing) 
// Step 3: Once all the necessary functions are finished, we can write the main body of code that will have the OTV perform each task (we’ll use each function when a task calls for it) 
// Step 4: Testing and making adjustments as needed 
// Note: It’s a lot easier to write code when we have something to test, we won’t know if it works until we try it on the OTV (or some part of it) 

#include "Enes100.h" // Includes the ENES100 library (which includes the VisionSystemClient.cpp)

// Choose one: testboard or otv
// #define HARDWARE_TESTBOARD
#define HARDWARE_OTV

//  ****** PIN ASSIGNMENTS *******

// For controlling DC motors with PWM Arduino with L298N H-Bridge  
// #define is more compatible with newer Arduino IDE.
// Naming it DCM_ prefix is a good way to prevent collisions with other code
// These numbers are actual hardware connections
// SWAP IN1 with IN2 (or IN3 with IN4) to reverse motor direction
// If the thermal camera angle keeps spinning then SWAP pairs (IN1, IN2) with (IN3, IN4) to swap left and right motor
#define DCM_IN1 7
#define DCM_IN2 6
#define DCM_IN3 5
#define DCM_IN4 4
#define DCM_ENA 9
#define DCM_ENB 3

/* This factor needs to be adjusted so that it gives number of seconds for motors to run to travel 1 meter */
#define DCM_DISTANCE_TO_TIME_FACTOR       1.5
/* This factor needs to be adjusted so that it gives number of minutes for motors to run in opposite to rotate 60 degrees (or number of seconds to rotate 1 degree, but that is hard to measure)
   reduce value to prevent overshoot in rotate_absolute() 
   */
#define DCM_ANGLE_TO_TIME_FACTOR          0.5

// For lab-provided ESP8266 for Aruco
// Note for Mega only these values are recommended:
// https://enes100.umd.edu/documentation/arduino/
// Mega - 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69) 
// Students have had difficulty with the Mega. Some students have a certain set of pins work while other students will find those same pins broken.

// Change these based on where the ESP8266 is actually connected. Check that it is an allowed pin for Mega.
#define ESP8266_TX        10
#define ESP8266_RX        11
// Ask TA and change ESP8266_MARKER
#define ESP8266_MARKER    25
#define ESP8266_ROOM      1116

// change this if ARUCO 0 angle does not align with Y direction that points from landing zone to finish.
#define ARUCO_Y_ANGLE     0

// Ultrasonic sensors:

#define ULTRA1_TRIG     32
#define ULTRA1_ECHO     33
#define ULTRA2_TRIG     50
#define ULTRA2_ECHO     51
#define ULTRA3_TRIG     52
#define ULTRA3_ECHO     53

// *************** END PIN ASSIGNMENTS ***********************

// These numbers are symbolic constants (can be changed if preferred) that are passed to set_dc_motor() 
#define DCM_MOTOR1  1
#define DCM_MOTOR2  2
#define DCM_DIR_FORWARD   0
#define DCM_DIR_BACKWARD  1
#define DCM_DIR_OFF       255

// TODO is a #define - every time the compiler sees it, it substitutes the string to the right of TODO. It’s a convenient way to leave messages (from my point of view). 
#define TODO(msg) fprintf(stderr, "*** TODO %s %s:%d\n", msg, __FILE__, __LINE__);

// This function will make each motor move forward, backward, or off.  
// 6 registers that control 2 motors, 2 ens and 4 INs. Each motor is controlled by 1 en and 2 IN. 
// en is a variable that will set the motor on or off 
// in1 determines voltage applied to wire 1 of motor
// in2 determines voltage applied to wire 2 of motor
// Arduino defines LOW as 0 and HIGH as 1
// We may have to swap 'forward' and 'backward' labels depending on how the motors are wired to the H-bridge.  
void set_dc_motor(int motor, int dir)
{
int en, in1, in2;
if(dir==DCM_DIR_OFF) {
    en=0;
    in1=LOW;
    in2=LOW;
    } else 
if(dir==DCM_DIR_FORWARD) {
    en=255;
    in1=HIGH;
    in2=LOW;
    } else
if(dir==DCM_DIR_BACKWARD) {
    en=255;
    in1=LOW;
    in2=HIGH;
    }

// analogWrite and digitalWrite are Arduino functions that set register. 'register' is a memory location that directly controls the hardware. (For example, input or output pins.)  

if(motor==DCM_MOTOR1) {
  analogWrite(DCM_ENA,   en);
  digitalWrite(DCM_IN1,   in1);
  digitalWrite(DCM_IN2,  in2);
  }
if(motor==DCM_MOTOR1) {
  analogWrite(DCM_ENB, en); 
  digitalWrite(DCM_IN3, in1);
  digitalWrite(DCM_IN4, in2);
  }
}

double current_x, current_y, current_angle;


// This function will set the motors to rotate forward or backward and then wait. 
// Time to wait can be changed. 
// The negative distance means it moves backward.
void move(double distance)
{
if(distance>0) {
  set_dc_motor(DCM_MOTOR1, DCM_DIR_FORWARD);
  set_dc_motor(DCM_MOTOR2, DCM_DIR_FORWARD);
  } else 
if(distance<0) {
  set_dc_motor(DCM_MOTOR1, DCM_DIR_BACKWARD);
  set_dc_motor(DCM_MOTOR2, DCM_DIR_BACKWARD);
  }
// The DC motors do not provide any feedback, so the distance it goes will have to be determined by running time. 
// DCM_DISTANCE_TO_TIME_FACTOR is a constant that converts desired distance in meters to the time we'll need to run the motors for. 
// fabs() is absolute value for floating point numbers (real numbers that the computer stores) (like double or float) 
delay(fabs(distance)*DCM_DISTANCE_TO_TIME_FACTOR);
set_dc_motor(DCM_MOTOR1, DCM_DIR_OFF);
set_dc_motor(DCM_MOTOR2, DCM_DIR_OFF);
TODO("set DCM_DISTANCE_TO_TIME_FACTOR to values measured on real robot")
}

// This function will set the motors to rotate in opposite direction, which allows the OTV robot to rotate. It tells the robot to rotate some number of 
// degrees left or right from the direction it is currently facing in.
// It has its own ANGLE_TO_TIME constant (measured in degrees) but otherwise remains the same as the previous function. 
void rotate_relative(double angle)
{
if(angle>0) {
  set_dc_motor(DCM_MOTOR1, DCM_DIR_FORWARD);
  set_dc_motor(DCM_MOTOR2, DCM_DIR_BACKWARD);
  } else
if(angle<0)
  {
  set_dc_motor(DCM_MOTOR1, DCM_DIR_BACKWARD);
  set_dc_motor(DCM_MOTOR2, DCM_DIR_FORWARD);
  }
// DC motors do not provide any feedback, go by running time
delay(fabs(angle)*DCM_ANGLE_TO_TIME_FACTOR);
set_dc_motor(DCM_MOTOR1, DCM_DIR_OFF);
set_dc_motor(DCM_MOTOR2, DCM_DIR_OFF);
TODO("set DCM_ANGLE_TO_TIME_FACTOR to value measured on real robot")
}

int ultra_trig[3]={ULTRA1_TRIG, ULTRA2_TRIG, ULTRA3_TRIG};
int ultra_echo[3]={ULTRA1_ECHO, ULTRA2_ECHO, ULTRA3_ECHO};

#define ULTRA_SENSOR1     0
#define ULTRA_SENSOR2     1
#define ULTRA_SENSOR3     2

// Returns distance in meters
double read_ultrasonic_sensor(int sensor)
{
if(sensor<0)return(-1);
if(sensor>2)return(-1);

digitalWrite(ultra_trig[sensor], LOW);
delayMicroseconds(2);
digitalWrite(ultra_trig[sensor], HIGH);
delayMicroseconds(10);
digitalWrite(ultra_trig[sensor], LOW);
double duration=pulseIn(ultra_echo[sensor], HIGH);
return(duration*0.034*0.5);
}

void extend_arm(int extend)
{
TODO("extend arm when extend is 1, retract when extend is 0")
}

double angle_to_gravity()
{
TODO("angle to gravity from IMU")
return(0);
}

double angle_to_north()
{
TODO("angle to north from IMU")
return(0);
}

double aruco_x()
{
#ifdef HARDWARE_OTV
return(Enes100.getX());
#endif
return(0);
}

double aruco_y()
{
#ifdef HARDWARE_OTV
return(Enes100.getY());
#endif
return(0);
}

// Assuming angle 0 is NORTH towards increasing y 
double aruco_angle()
{
#ifdef HARDWARE_OTV
double a=Enes100.getTheta()*180/3.1415-ARUCO_Y_ANGLE;
if(a> 180)a-=360.0;
if(a< -180)a+=360.0;
return(a);
#endif
return(0);
}

// This function will tell the motors to rotate with respect to absolute direction
// Choose one direction, like NORTH for example. NORTH is 0 degrees, and rotate_absolute(45) would be 45 EAST of NORTH and rotate_absolute(-45) would be 45 WEST of NORTH.
// Hardware will determine absolute angle and we'll pick direction from that. (If AruCo does that then AruCo will tell us.)  
// Will also depend on motor wiring, we may need to swap EAST and WEST 
void rotate_absolute(double angle)
{
  double tolerance=5; // Tolerance in degrees 
  // If it keeps spinning then the rotate needs a sign change because the motors were swapped
  // If it instead keeps moving back and forth, increase tolerance instead
  // Assuming angle 0 is NORTH towards increasing y 
  while(1)
  {
    float angle_error=angle-aruco_angle();
    if(angle_error< -180)angle_error+=360.0;
    if(angle_error>   180)angle_error-=360.0;
    if(fabs(angle_error)<tolerance)break;
    rotate_relative(angle_error);
  }
}

int flame_detected()
{
TODO("This tells if it sees the flame")
return(0);
}

double angle_to_flame()
{
if(!flame_detected())return(-1e20);
TODO("determine angle to flame from IR array")
return(0);
}

double forward_obstacle_distance()
{
TODO("Determine forward_obstacle distance from ultrasound or another sensor")
return(0);
}

// This function will give the estimated distance to flame (in meters). 
double distance_to_flame()
{
if(!flame_detected())return(-1e20);
return(forward_obstacle_distance());
}

// Change 50 to 100 or 150 if issues arise (higher number means less resolution and therefore less memory used)
#define SQUARE_WIDTH_MM  50
#define NX (5000/SQUARE_WIDTH_MM)
#define NY (5000/SQUARE_WIDTH_MM)

#define MARK_CLEAR   0
#define MARK_OBSTACLE   1
#define MARK_FIRE 2

int topo_map[NX][NY];
// topo_map - an array of squares (a grid) that holds marks of where the fire is and where obstacles are.
// topo_map[NX][NY]=(CONSTANT NAME);

// This function will initialize the map. 
void init_map(void)
{
for(int i=0;i<NX;i++)
 for(int j=0;j<NY;j++) topo_map[i][j]=0;
}

// mark_map(20, 15, MARK_FIRE); An example
void mark_map(double x, double y, int mark)
{
int x0=round(x/SQUARE_WIDTH_MM);
int y0=round(y/SQUARE_WIDTH_MM);

if((x0>=0) && (y0>=0) && (x0<NX) && (y0<NY))topo_map[x0][y0]=mark;
}

void traverse_to(double dest_x, double dest_y)
{
double dx, dy, angle;
double tolerance=0.1;
rotate_absolute(0);

 TODO("Use aruco x,y and angle as well as map to drive to dest_x and dest_y. Update map as obstacles are found")

while(1) {
  dx=dest_x-aruco_x();
  dy=dest_y-aruco_y();
  if(fabs(dx)<tolerance && fabs(dy)<tolerance)break;
  angle=atan2(dy, dx)*180/3.1415;
  rotate_absolute(angle);
  move(0.05);
  }
rotate_absolute(0);
}

void traverse_to_y(double dest_y)
{
TODO("Use aruco x,y and angle as well as map to drive to dest_y and some dest_x as happens. Update map as obstacles are found")
}

void main_program(void)
{
current_x=aruco_x();
current_y=aruco_y();
current_angle=aruco_angle();

traverse_to(current_x, 150);

TODO("Find fire and face it")

extend_arm(1);
delay(10000); /* 10sec */
extend_arm(0);

traverse_to_y(3700);

}


#ifdef HARDWARE_OTV
// For connecting to the ESP8266 WiFi communication module, a placeholder for when the actual hardware is there to program. 

// #include <ESP8266WiFi.h>

#endif

#ifdef HARDWARE_TESTBOARD

#include <WiFiS3.h>

#endif

// In meters
#define ZONE_OBSTACLE_START 0.8
#define ZONE_OPEN_START     2.8
#define ZONE_GOAL_START     3.4

void setup()
{
  Serial.begin(115200); // Arduino has a built-in Serial port, allows you to see TODO messages on the SerialMonitor (for convenience) 
  // 115200 is the transmission speed, adjust SerialMonitor's speed to the same value
  Serial.println();

  // An example of connecting to a WiFi network
  #if 0 
  WiFi.begin("network-name", "pass-to-network");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  #endif 
  
  
  pinMode (DCM_IN1, OUTPUT);
  pinMode (DCM_IN2, OUTPUT);
  pinMode (DCM_IN3,   OUTPUT);
  pinMode (DCM_IN4, OUTPUT);
  pinMode (DCM_ENA, OUTPUT);
  pinMode (DCM_ENB,   OUTPUT);
  pinMode (ULTRA1_TRIG, OUTPUT);
  pinMode (ULTRA1_ECHO , INPUT);
  pinMode (ULTRA2_TRIG, OUTPUT);
  pinMode (ULTRA2_ECHO , INPUT);
  pinMode (ULTRA3_TRIG, OUTPUT);
  pinMode (ULTRA3_ECHO , INPUT);

/* Change 0 to 1 to make motors constantly spin forward for FORWARD LOCOMOTION */
/* 0 comments out the code essentially */ 
#if 1
  set_dc_motor(DCM_MOTOR1, DCM_DIR_FORWARD);
  set_dc_motor(DCM_MOTOR2, DCM_DIR_FORWARD);
  while(1); /* endless loop */
#endif


#if 0
  /* 90 degree turns using rotate_relative(), without Aruco */
   // Rotate left first
  rotate_relative(90);
  delay(5000);
  // Rotate right
  rotate_relative(-90);
  delay(5000);
  // Rotate left again
  rotate_relative(90);
  while(1);

#endif


  // Enes100.begin(const char* teamName, byte teamType, int markerId, int roomNumber, int wifiModuleTX, int wifiModuleRX);
  // TX and RX are lines on the ESP8266 used for communicating, TX is to transmit and RX is to receive
  // Mega has only certain pins for communicating over WiFi (stated on the Enes100 library), needs to be tested 
  #ifdef HARDWARE_OTV 
  Enes100.begin("Fabulous Firefighters", FIRE, ESP8266_MARKER, ESP8266_ROOM, ESP8266_TX, ESP8266_RX);
  Enes100.println("Setup done"); 
  #endif 

#if 0
/* 90 degree rotation test */
  rotate_absolute(0);
  delay(5000);
  rotate_absolute(90);
  delay(5000);
  rotate_absolute(180);
  delay(5000);
  rotate_absolute(270);
  delay(5000);
  rotate_absolute(0);
  while(1); /* endless loop */
  
#endif


#if 0
  /* Random mission test */
 if(aruco_x()<1.0)traverse_to(1.5, 0.4);
     else traverse_to(0.5, 0.4);
  while(1); /* endless loop */

#endif
}


// A flowchart turns into a state machine if you assign a different number to each box.
// Then the arrows tell you how and when you go from one state to another.

#define STATE_START 0
#define STATE_FOUND_CANDLES 1
#define STATE_DRIVING_OBSTACLES 2 
#define STATE_REACHED_GOAL 3
 
#define STATE_DONE 255 

int state=STATE_START; 

// IF this compiles well with Mega and ESP8266 WiFi module, this code will tell the OTV to drive to the other end of the field (as wanted in the MS5 document)
void loop() 
{
  if(state==STATE_START) 
  {
    rotate_absolute(0); 
    move(0.05); // An example 
    rotate_absolute(90); // It points towards the candles 
    TODO("add candle seeking after IR array is sorted out")
    state=STATE_FOUND_CANDLES; 
    Enes100.mission(NUM_CANDLES, 5); // Found 5 candles 
    Enes100.mission(TOPOGRAPHY, TOP_A); // or TOP_B or TOP_C
  }
  
  if(state==STATE_FOUND_CANDLES) 
  {
    // Add any code necessary to start obstacle avoidance 
    // For example, moving OTV to the middle of the field
    // move(0.5);
    state=STATE_DRIVING_OBSTACLES; 
  }

  if(state==STATE_DRIVING_OBSTACLES) 
  {
    rotate_absolute(0);
    TODO("Add obstacle avoidance code when ultrasonic sensors are sorted out (pin locations and distance response)")  
    move(0.1); // An example
    if(aruco_y()>ZONE_GOAL_START) 
    {
      state=STATE_REACHED_GOAL; 
    }
  }

  if(state==STATE_REACHED_GOAL) 
  {
    state=STATE_DONE;
    Enes100.println("All done!"); 
  }
}
