
// Step 1: Collect all the code that allows you to talk to “peripherals” - other devices like sensors and motors.
// Step 2: Write each basic function (need more information about aruco and ESP8266 WiFi Communication Module for example to start writing) 
// Step 3: Once all the necessary functions are finished, we can write the main body of code that will have the OTV perform each task (we’ll use each function when a task calls for it) 
// Step 4: Testing and making adjustments as needed 
// Note: It’s a lot easier to write code when we have something to test it on, we could make a complete “skeleton” code but we won’t know if it works until we can try it on the OTV (or some part of it) 

#include "Enes100.h" // Includes the ENES100 library (which includes the VisionsystemClient.cpp)

// Choose one: testboard or otv
// #define HARDWARE_TESTBOARD
#define HARDWARE_OTV

// For controlling DC motors with PWM Arduino with L298N H-Bridge  
// #define is more compatible with newer Arduino IDE.
// Naming it DCM_ prefix is a good way to prevent collisions with other code
// These numbers are actual hardware connections
#define DCM_IN1 7
#define DCM_IN2 6
#define DCM_IN3 5
#define DCM_IN4 4
#define DCM_ENA 9
#define DCM_ENB 3

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

#define DCM_DISTANCE_TO_TIME_FACTOR       1.5
#define DCM_ANGLE_TO_TIME_FACTOR          0.5

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
return(Enes100.getTheta()*180/3.1415);
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
  while(fabs(angle-aruco_angle())>tolerance)
  {
    rotate_relative(angle-aruco_angle());
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

#define SQUARE_WIDTH_MM  50
#define NX (5000/SQUARE_WIDTH_MM)
#define NY (5000/SQUARE_WIDTH_MM)

#define MARK_CLEAR   0
#define MARK_OBSTACLE   1
#define MARK_FIRE 2

int topo_map[NX][NY];
// topo_map - an array of squares that holds marks of where the fire is and where obstacles are.

// This function will initialize the map. 
void init_map(void)
{
for(int i=0;i<NX;i++)
 for(int j=0;j<NY;j++) topo_map[i][j]=0;
}

void mark_map(double x, double y, int mark)
{
int x0=round(x/SQUARE_WIDTH_MM);
int y0=round(y/SQUARE_WIDTH_MM);

if((x0>=0) && (y0>=0) && (x0<NX) && (y0<NY))topo_map[x0][y0]=mark;
}

void traverse_to(double dest_x, double dest_y)
{
TODO("Use aruco x,y and angle as well as map to drive to dest_x and dest_y. Update map as obstacles are found")
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

void setup()
{
  Serial.begin(115200); // Arduino has a built-in Serial port, allows you to see TODO messages on the SerialMonitor (for convenience) 
  // 115200 is the transmission speed, adjust SerialMonitor's speed to the same value
  Serial.println();

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
  
  // Enes100.begin(const char* teamName, byte teamType, int markerId, int roomNumber, int wifiModuleTX, int wifiModuleRX);
  // TX and RX are lines on the ESP8266 used for communicating, TX is to transmit and RX is to receive
  // Mega has only certain pins for communicating over WiFi (stated on the Enes100 library), needs to be tested 
  #ifdef HARDWARE_OTV 
  Enes100.begin("Fabulous Firefighters", FIRE, 25, 1116, 10, 11);
  #endif 
  
  pinMode (DCM_IN1, OUTPUT);
  pinMode (DCM_IN2, OUTPUT);
  pinMode (DCM_IN3,   OUTPUT);
  pinMode (DCM_IN4, OUTPUT);
  pinMode (DCM_ENA, OUTPUT);
  pinMode (DCM_ENB,   OUTPUT);

  #ifdef HARDWARE_OTV
  Enes100.println("Setup done"); 
  #endif 
}

// In meters
#define ZONE_OBSTACLE_START 0.8
#define ZONE_OPEN_START     2.8
#define ZONE_GOAL_START     3.4

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
