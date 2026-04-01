


#define TODO(msg) fprintf(stderr, "*** TODO %s %s:%d\n", msg, __FILE__, __LINE__);

double current_x, current_y, current_angle;

void move_forward(double distance)
{
TODO("have motors move OTV forward a given distance")
}

void move_backward(double distance)
{
TODO("have motors move OTV backward a given distance")
}

void rotate_relative(double angle)
{
TODO("use drive motors in opposition")
}

void rotate_absolute(double angle)
{
TODO("use either angle to north from IMU or from aruco")
}

void extend_arm(int extend)
{
TODO("extend arm when extend is 1, retract when extend is 0")
}

double angle_to_gravity()
{
TODO("angle to gravity from IMU")
}

double angle_to_north()
{
TODO("angle to north from IMU")
}

double aruco_x()
{
TODO("aruco x coordinate")
}

double aruco_y()
{
TODO("aruco y coordinate")
}

double aruco_angle()
{
TODO("aruco angle")
}

int flame_detected()
{
TODO("this just tells if it sees flame")
}

double angle_to_flame()
{
if(!flame_detected())return(-1e20);
TODO("determine angle to flame from IR array")
}

double forward_obstacle_distance()
{
TODO("determine forward_obstacle distance from ultrasound or other sensor")
}

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
TODO("use aruco x,y and angle as well as map to drive to dest_x and dest_y. Update map as obstacles found")
}

void traverse_to_y(double dest_y)
{
TODO("use aruco x,y and angle as well as map to drive to dest_y and some dest_x as happens. Update map as obstacles found")
}

void main_program(void)
{
current_x=aruco_x();
current_y=aruco_y();
current_angle=aruco_angle();

traverse_to(current_x, 150);

TODO("find fire and face it")

extend_arm(1);
delay(10000); /* 10sec */
extend_arm(0);

traverse_to_y(3700);

}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
