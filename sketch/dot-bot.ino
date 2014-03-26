#include <ht1632c.h>
#include <Bridge.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h>

// which motor is on which pin?
#define M1_PIN          (1)
#define M2_PIN          (2)
// NEMA17 are 200 steps (1.8 degrees) per turn.  If a spool is 0.8 diameter
// then it is 2.5132741228718345 circumference, and
// 2.5132741228718345 / 200 = 0.0125663706 thread moved each step.
// NEMA17 are rated up to 3000RPM.  Adafruit can handle >1000RPM.
// These numbers directly affect the maximum velocity.
#define STEPS_PER_TURN  (200.0)
// plotter limits
// all distances are relative to the calibration point of the plotter.
// (normally this is the center of the drawing area)

// 300cm x 300cm area
static int limit_top = 150;  // distance to top of drawing area.
static int limit_right = 150;  // Distance to right of drawing area.
static int limit_left = -150;  // Distance to left of drawing area.

// which way are the spools wound, relative to motor movement?
uint8_t M1_REEL_IN  = BACKWARD;
uint8_t M1_REEL_OUT = FORWARD;
uint8_t M2_REEL_IN  = BACKWARD;
uint8_t M2_REEL_OUT = FORWARD;

// calculate some numbers to help us find feed_rate
float SPOOL_DIAMETER1 = 1.6;
float THREADPERSTEP1;  // thread per step

float SPOOL_DIAMETER2 = 1.6;
float THREADPERSTEP2;  // thread per step

boolean MANUAL = true;

ht1632c ledMatrix = ht1632c(&PORTF,7,6,5,4,GEOM_32x16,2);
//A0, A1, A2, A3
//data,wr,clk,cs

Adafruit_MotorShield AFMS0 = Adafruit_MotorShield();
Adafruit_StepperMotor *m1;
Adafruit_StepperMotor *m2;

static uint8_t step_delay = 50;
char geo[180];
char page[180];
char x[5];
char y[5];
float oldxx = 0;
float oldyy = 0;
String temp;

// motor position
static long laststep1, laststep2;
float xx, yy;

void setup() {
  Bridge.begin();
  Console.begin();
  while (!Console){
    ; // wait for Console port to connect.
  }
  Console.println("==================\nWelcome to DotBot!\n==================\n\nH) to set Home (center of map)\nM) to start Manual mode\nA) to start Auto mode\nX:Y) to move in position (X,Y)\n");
  // start the motor shield
  AFMS0.begin();
  m1 = AFMS0.getStepper(STEPS_PER_TURN, M2_PIN);
  m2 = AFMS0.getStepper(STEPS_PER_TURN, M1_PIN);
    
  THREADPERSTEP1 = (SPOOL_DIAMETER1*PI)/STEPS_PER_TURN;  // thread per step
  THREADPERSTEP2 = (SPOOL_DIAMETER2*PI)/STEPS_PER_TURN;  // thread per step

  // initialize the plotter position.
  teleport(0,0);

  line(-5,5);
  // start the led matrix
  ledMatrix.clear();
  ledMatrix.pwm(15);

  ledMatrix.hscrolltext(4, "Please, set HOME", 2, 50,1);
  
}

void loop() {
  
  if (MANUAL){
    if (Console.available() > 0) {
      char c = Console.read();
      if (c == '\n'){
        temp.substring(0, temp.indexOf(':')).toCharArray(x,4);
        temp.substring(temp.indexOf(':')+1).toCharArray(y,4);
        xx = atof(x);
        yy = atof(y);
        Console.print("Console X: ");
        Console.println(xx); //get latest parsed data
        Console.print("Console Y: ");
        Console.println(yy); //get latest parsed data
        if (oldxx != xx || oldyy != yy){
          drawLogo();
          line(xx,yy);
        }
        oldxx = xx;
        oldyy = yy;
        temp = "";
      }
      else if (c == 'H'){
        teleport(0,0);
        oldxx=0;
        oldyy=0;
        Console.println("This is my home!");
        ledMatrix.hscrolltext(4, "This is my home!", 2, 50,1);
        drawLogo();
      }else if (c == 'A'){
        MANUAL = false;
        Console.println("Starting AUTO mode!");
        ledMatrix.hscrolltext(4, "Starting AUTO mode!", 2, 50,1);
        drawLogo();
      }else{
        temp += c;
      }
    }
  }else{
    Bridge.get("page", page, 200);

    if (String("...") == page){
      ledMatrix.hscrolltext(4, "dotdotdot...", 1, 50);
      drawLogo();

      if (oldxx != 0 && oldyy != 0)
        line(0,0);

      oldxx = 0;
      oldyy = 0;
      Bridge.get("geo", geo, 200);
      ledMatrix.hscrolltext(4, geo, 1, 40);
    } else {
      Bridge.get("geo", geo, 200);
      Bridge.get("x", x, 5);
      Bridge.get("y", y, 5);
      xx=atof(x);
      yy=atof(y);
      if (oldxx != xx || oldyy != yy){
        drawLogo();
        line(xx,yy);
      }
      ledMatrix.hscrolltexts(0, geo, 1, 50, 8, page, 1);
      oldxx = xx;
      oldyy = yy;
    }
  }
}
//------------------------------------------------------------------------------
// instantly move the virtual plotter position
// does not validate if the move is valid
static void teleport(float x,float y) {
  long L1,L2;
  IK(x,y,L1,L2);
  laststep1=L1;
  laststep2=L2;
}
///------------------------------------------------------------------------------
static void line(float x,float y) {
  long l1,l2;
  IK(x,y,l1,l2);
  long d1 = l1 - laststep1;
  long d2 = l2 - laststep2;
   
  long ad1=abs(d1);
  long ad2=abs(d2);
  int dir1=d1>0?M1_REEL_IN:M1_REEL_OUT;
  int dir2=d2>0?M2_REEL_IN:M2_REEL_OUT;
  long over=0;
  long i;
  
  // bresenham's line algorithm.
  if(ad1>ad2) {
    for(i=0;i<ad1;++i) {
      m1->onestep(dir1,DOUBLE);
      over+=ad2;
      if(over>=ad1) {
        over-=ad1;
        m2->onestep(dir2,DOUBLE);
      }
      delay(step_delay/1000);
    }
  } else {
    for(i=0;i<ad2;++i) {
      m2->onestep(dir2,DOUBLE);
      over+=ad1;
      if(over>=ad2) {
        over-=ad2;
        m1->onestep(dir1,DOUBLE);
      }
      delay(step_delay/1000);
    }
  }
  laststep1=l1;
  laststep2=l2;
}

//------------------------------------------------------------------------------
// Inverse Kinematics - turns XY coordinates into lengths L1,L2
static void IK(float x, float y, long &l1, long &l2) {
  // find length to M1
  float dy = y - limit_top;
  float dx = x - limit_left;
  l1 = floor( sqrt(dx*dx+dy*dy) / THREADPERSTEP1 );
  // find length to M2
  dx = limit_right - x;
  l2 = floor( sqrt(dx*dx+dy*dy) / THREADPERSTEP2 );
}

void drawLogo(){
   //       ------
  ledMatrix.line(12,0,19,0,1);
  ledMatrix.line(12,15,19,15,1);
  
  //     |        |
  ledMatrix.line(8,4,8,12,1);
  ledMatrix.line(23,4,23,12,1);
  
  //       //-
  ledMatrix.line(10,0,8,2,1);
  ledMatrix.line(11,0,8,3,1);
  
  //       -\\
  ledMatrix.line(21,0,23,2,1);
  ledMatrix.line(20,0,23,3,1);
  
  //       ...
  ledMatrix.plot(11,12,1);
  ledMatrix.plot(13,12,1);
  ledMatrix.plot(15,12,1);

  //       \\_
  ledMatrix.line(8,12,11,15,1);
  ledMatrix.line(8,13,10,15,1);
 
  //         _//
  ledMatrix.line(23,13,21,15,1);
  ledMatrix.line(22,13,20,15,1);
  ledMatrix.sendframe();
}
