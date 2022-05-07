
#include <AccelStepper.h>

byte Lswitch = 7;
int MAX_SPEED = 1000;
int SPEED = 600;
int ACCEL = 500;
float ANGEL_FACTOR = 1.8;
int NUM_OF_STEPS = 200;

AccelStepper motor(1, 2, 5); // pin 2 = step, pin 5 = direction
int min_step = 0;
int max_step = 970;
int home_step = 90;

//time delay
int time_delay = 1;
//serial paparms
const byte numChars = 32;
char receivedChars[numChars];
String command = "Start";
int received_step = 0;
boolean newData = false;

//print setup
bool print_once = true;
bool isStopped = false;
bool isCalibrated = false;

void setup() {
  Serial.begin(9600);
  pinMode(Lswitch, INPUT_PULLUP); 
  
  motor.setMaxSpeed(MAX_SPEED);
  motor.setSpeed(SPEED);
  motor.setAcceleration(ACCEL);
  motor.setCurrentPosition(0);
  motorCalibration();
  Serial.println("<Arduino is ready>");
  print_();
  delay(2000); 
}

void loop() {  
  recvWithStartEndMarkers();
  myStrtok();
  parseSerialBuffer();

  if(motor.distanceToGo()!=0)
    motor.run();
  print_();
  delay(time_delay);
}

void motorCalibration(){
  Serial.println("<Calibrating...>");
  while(isCalibrated==false){
    motor.moveTo(-1200);
    motor.run();
    if(digitalRead(Lswitch) == LOW){    
      motor.setCurrentPosition(0);
      motor.runToNewPosition(20);
      motor.setCurrentPosition(0);
      print_once = true;
      isStopped = true;
      isCalibrated = true;
    }
    delay(time_delay);
  }
}
void print_(){
  if(print_once){
    print_once = false;
    Serial.println("<-------------Print motor state-------------");
    if(isStopped){
      Serial.println("The limit switch: TOUCHED");
      isStopped = false;
    }else
      Serial.println("The limit switch: UNTOUCHED");
    
    Serial.print("speed:\t");
    Serial.println(motor.speed());
    Serial.print("position:\t");
    Serial.println(motor.currentPosition());
    Serial.print("target:\t");
    Serial.println(motor.targetPosition());
    Serial.print("distance:\t");
    Serial.println(motor.distanceToGo());
    Serial.print("time_delay: ");
    Serial.println(time_delay);
    Serial.print("command: ");
    Serial.println(command);
    Serial.print("received_step: ");
    Serial.println(received_step);
    Serial.print("isCalibrated: ");
    Serial.println(isCalibrated);
    Serial.println(">"); 
  }
}
void parseSerialBuffer(){
   if(command == "seta"){
      motor.setAcceleration(received_step);
      print_once = true;
   }
   else if(command == "sets"){
      motor.setSpeed(received_step);
      print_once = true;
   }
   else if(command =="zerop"){
      motor.setCurrentPosition(received_step);
      print_once = true;
   }
   else if(command == "runsp"){
      motor.runToNewPosition(checkPwmBoundry(received_step));
      print_once = true;
   }
   else if(command == "runrel"){
      motor.move(received_step);
      print_once = true;
   }
   else if (command == "run"){
      motor.moveTo(checkPwmBoundry(received_step));
      print_once = true;
   }
   else if (command == "stop"){
      motor.stop();
      print_once = true;
   }
   else if(command == "runh"){
    motor.moveTo(home_step);
    print_once=1;
   }
   else if(command == "runrnd"){
    motor.moveTo(random(min_step, max_step));
    print_once=1;
   }
   else if (command == "print"){
      print_once = true;
   }
   else if(command == "delay"){
    time_delay = received_step;
    print_once=1;
  }
}


int checkPwmBoundry(int step_num){
  if(step_num<min_step){
    Serial.print("<Warning, Step is out of min boundry: ");
    Serial.print(min_step);
    Serial.print(" | ");
    Serial.println(step_num);
    Serial.println(">");
    step_num = min_step;
  }
  if(step_num>max_step){
    Serial.print("<Warning, Step is out of max boundry: ");
    Serial.print(max_step);
    Serial.print(" | ");
    Serial.println(step_num); 
    step_num = max_step;  
    Serial.println(step_num);
    Serial.println(">");
  }
  return step_num;
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void myStrtok (){
  const char *delimiter = ":";
  int ind=0;
  int num[3];
  command = "";
  if (newData == true) {
    char* d = strtok(receivedChars, delimiter);
    command = String((char *)d);
    while (d != NULL) {
        num[ind] = atoi(d);
        ind++;
        d = strtok(NULL, delimiter);
    }
    newData = false;
    if(ind==2){received_step = num[1];}
    else if(ind==1){Serial.print("<Warning: Inserted command only: ");Serial.print(command);Serial.println(">");}
    else if(ind == 0){Serial.println("<Error: Invalid input, missing right delimiter>");}
  }
}
