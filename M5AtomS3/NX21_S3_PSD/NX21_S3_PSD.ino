#include <M5AtomS3.h>
#include <DabbleESP32.h>

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

const uint8_t Srv0 = 6; //GPIO Right Front
const uint8_t Srv1 = 7; //GPIO Right Back
const uint8_t Srv2 = 8; //GPIO Left Front
const uint8_t Srv3 = 38; //GPIO Left Back
const uint8_t Adc0 = 5; //ADC port

const uint8_t srv_CH0 = 0, srv_CH1 = 1, srv_CH2 = 2, srv_CH3 = 3; //チャンネル
const double PWM_Hz = 50;   //PWM周波数
const uint8_t PWM_level = 14; //PWM 14bit(0～16384)

int pulseMIN = 410;  //0deg 500μsec 50Hz 14bit : PWM周波数(Hz) x 2^14(bit) x PWM時間(μs) / 10^6
int pulseMAX = 2048;  //180deg 2500μsec 50Hz 14bit : PWM周波数(Hz) x 2^14(bit) x PWM時間(μs) / 10^6

int cont_min = 0;
int cont_max = 180;

int angZero[] = {87,102,99,86}; //Trimming
int ang0[4];
int ang1[4];
int ang_b[4];
char ang_c[4];
float ts=120;  //120msごとに次のステップに移る
float td=10;   //20回で分割

int position_status = 0;

// Forward Step
int f_s[4][6]={
  {0,-15,15,0},
  {-15,15,-15,15},
  {-15,0,0,15},
  {15,-15,15,-15}};

// Back Step
int b_s[4][6]={
  {0,15,-15,0},
  {15,-15,15,-15},
  {15,0,0,-15},
  {-15,15,-15,15}};
  
// Left Turn_Step
int l_s[3][6]={
  {-15,15,15,-15},
  {-15,0,0,-15},
  {0,0,0,0}};
  
// Right Turn Step
int r_s[3][6]={
  {15,-15,-15,15},
  {15,0,0,15},
  {0,0,0,0}};
  
// Home position
int h_p[6]={0,0,0,0};

// Up Step
int u_s[6]={-15,-60,15,60};

// Down_Step
int d_s[6]={60,15,-60,-15};

// Right_Down_Step
int rd_s[6]={70,30,-30,0};

// Left_Down_Step
int ld_s[6]={30,0,-70,-30};

int angry_state = 0;

void Initial_Value(){  //initial servo angle
  int cn = 50;
  for (int j=0; j <=3; j++){
    Srv_drive(j, angZero[j]);
    ang0[j] = angZero[j];
    ang1[j] = angZero[j];
    delay(cn);
  }
}

void face_clear(){
  M5.Lcd.fillScreen(M5.Lcd.color565(255, 150, 0)); //r, g, b   Yellow
}

void face_center_eye(void *pvParameters){
  while(1)
  {
    face_clear();
    M5.Lcd.fillCircle(39,49,10,0x001F); //Blue
    M5.Lcd.fillCircle(89,49,10,0x001F); //Blue
    M5.Lcd.fillRect(39,83,50,10,0x7800); //Red
    delay(1500);
    face_clear();
    M5.Lcd.fillRect(34,44,10,7,0x001F); //Blue
    M5.Lcd.fillRect(84,44,10,7,0x001F); //Blue
    M5.Lcd.fillRect(39,83,50,10,0x7800); //Red
    delay(200);
  }
}

void face_center(){
  face_clear();
  M5.Lcd.fillCircle(39,49,10,0x001F); //Blue
  M5.Lcd.fillCircle(89,49,10,0x001F); //Blue
  M5.Lcd.fillRect(39,83,50,10,0x7800); //Red
}

void face_angry(){
  M5.Lcd.fillScreen(M5.Lcd.color565(255, 0, 0)); //r, g, b   Red
  M5.Lcd.fillTriangle(14,24,54,54,14,44,0x001F); //Blue
  M5.Lcd.fillTriangle(74,54,114,24,114,44,0x001F); //Blue
  M5.Lcd.fillRect(39,73,50,40,0x0000); //Black
}

void face_right(){
  face_clear();
  M5.Lcd.fillCircle(19,49,10,0x001F); //Blue
  M5.Lcd.fillCircle(69,49,10,0x001F); //Blue
  M5.Lcd.fillRect(19,78,50,20,0x7800); //Red
}

void face_left(){
  face_clear();
  M5.Lcd.fillCircle(59,49,10,0x001F); //Blue
  M5.Lcd.fillCircle(109,49,10,0x001F); //Blue
  M5.Lcd.fillRect(59,78,50,20,0x7800); //Red
}

void Srv_drive(int srv_CH,int SrvAng){
  SrvAng = map(SrvAng, cont_min, cont_max, pulseMIN, pulseMAX);
  ledcWrite(srv_CH, SrvAng);
}


void servo_set(){
  int a[4],b[4];
  
  for (int j=0; j <=3 ; j++){
      a[j] = ang1[j] - ang0[j];
      b[j] = ang0[j];
      ang0[j] = ang1[j];
  }
#include <M5AtomS3.h>
  for (int k=0; k <=td ; k++){

      Srv_drive(srv_CH0, a[0]*float(k)/td+b[0]);
      Srv_drive(srv_CH1, a[1]*float(k)/td+b[1]);
      Srv_drive(srv_CH2, a[2]*float(k)/td+b[2]);
      Srv_drive(srv_CH3, a[3]*float(k)/td+b[3]);

      delay(ts/td);
  }
}

void forward_step()
{
  face_center();
  for (int i=0; i <=3 ; i++){
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + f_s[i][j];
    }
  servo_set();
  }
}

void back_step()
{
  face_center();
  for (int i=0; i <=3 ; i++){
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + b_s[i][j];
    }
  servo_set();
  }
}

void right_step()
{
  face_right();
  for (int i=0; i <=2 ; i++){
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + r_s[i][j];
    }
  servo_set();
  }
  face_center();
}

void left_step()
{
  face_left();
  for (int i=0; i <=2 ; i++){
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + l_s[i][j];
    }
  servo_set();
  }
  face_center();
}

void up_step()
{
  if(position_status == 0)
  {
    face_angry();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + u_s[j];
    }
    servo_set();
    position_status = 1;
  }
  else if(position_status == 1)
  {
    face_center();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + h_p[j];
    }
    servo_set();
    position_status = 0;
  }
}

void down_step()
{
  if(position_status == 0)
  {
    face_angry();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + d_s[j];
    }
    servo_set();
    position_status = 1;
  }
  else if(position_status == 1)
  {
    face_center();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + h_p[j];
    }
    servo_set();
    position_status = 0;
  }
}

void right_down_step()
{
  if(position_status == 0)
  {
    face_right();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + rd_s[j];
    }
    servo_set();
    position_status = 1;
  }
  else if(position_status == 1)
  {
    face_center();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + h_p[j];
    }
    servo_set();
    position_status = 0;
  }
}

void left_down_step()
{
  if(position_status == 0)
  {
    face_left();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + ld_s[j];
    }
    servo_set();
    position_status = 1;
  }
  else if(position_status == 1)
  {
    face_center();
    for (int j=0; j <=3 ; j++){
      ang1[j] = angZero[j] + h_p[j];
    }
    servo_set();
    position_status = 0;
  }
}

void home_position()
{
  for (int j=0; j <=3 ; j++){
    ang1[j] = angZero[j] + h_p[j];
  }
  servo_set();
}

float psd()
{
  float val0, val1, val2;
  val0 = analogRead(Adc0);
  val1 = val0/4095*5;
  val2 = 46.95/val1-3.624;
  return val2;
}

void setup() {
  Serial.begin(151200);
  M5.begin(true, true, false, false);  // Init M5AtomS3.  初始化 M5AtomS3 液晶,USBシリアル,I2C(38,39),LED
  M5.Lcd.setRotation(4);
  Dabble.begin("M5AtomS3");       //set bluetooth name of your device
  
  pinMode(Srv0, OUTPUT);
  pinMode(Srv1, OUTPUT);
  pinMode(Srv2, OUTPUT);
  pinMode(Srv3, OUTPUT);
  pinMode(Adc0, INPUT);
  
  //モータのPWMのチャンネル、周波数の設定
  ledcSetup(srv_CH0, PWM_Hz, PWM_level);
  ledcSetup(srv_CH1, PWM_Hz, PWM_level);
  ledcSetup(srv_CH2, PWM_Hz, PWM_level);
  ledcSetup(srv_CH3, PWM_Hz, PWM_level);

  //モータのピンとチャンネルの設定
  ledcAttachPin(Srv0, srv_CH0);
  ledcAttachPin(Srv1, srv_CH1);
  ledcAttachPin(Srv2, srv_CH2);
  ledcAttachPin(Srv3, srv_CH3);

  face_center();

  Initial_Value();

  xTaskCreatePinnedToCore(face_center_eye, "face_center_eye", 4096, NULL, 1, NULL, 1);
}

void loop() {
  M5.update();
  if ( M5.Btn.wasReleased() ) {
    Initial_Value();
  }
  
  Dabble.processInput();             //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.
  
  int a = GamePad.getAngle();
  int b = GamePad.getRadius();
  
  if (GamePad.isTrianglePressed())
  {
    up_step();
    //USBSerial.println("UP STEP");
  }

  if (GamePad.isCrossPressed())
  {
    down_step();
    //USBSerial.println("DOWN STEP");
  }

  if (GamePad.isCirclePressed())
  {
    right_down_step();
    //USBSerial.println("RIGHT DOWN STEP");
  }

  if (GamePad.isSquarePressed())
  {
    left_down_step();
    //USBSerial.println("LEFT DOWN STEP");
  }

  if ((position_status == 0)&&(b < 5))
  {
    home_position();
  }
  else if(b >=5)
  {
    if ((a >= 45)&&(a < 135))
    {
      forward_step();
    }
    if ((a >= 135)&&(a < 225))
    {
      left_step();  
    }
    if ((a >= 225)&&(a < 315))
    {
      back_step();
    }
    if ((a >= 315)||(a < 45))
    {
      right_step();
    }
  }
  
  if((psd()<8)&&(position_status == 0))
  {
    back_step();
    back_step();
  }
  
  if(((psd()<15)&&(psd()>10))&&(position_status == 0))
  {
    forward_step();
  }
}
