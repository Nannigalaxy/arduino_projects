//Written on Fri Mar 05 by nannigalaxy
//Example for Gesture navigation with menu using IR sensor (Mask Glove Dispenser)

#include <Adafruit_SH1106.h>
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>

Adafruit_MLX90614 temp = Adafruit_MLX90614();

Adafruit_SH1106 display(-1);

// Constants
#define timeout 120
#define temp_time 40
#define temp_threshold 100 // Temp in Fahrenheit
#define len 3
#define mSpeed 150
#define stock_threshold 3

// symbol character hex
// #define aup  0x1e
// #define adown  0x1f
// #define aright  0x10
// #define aleft  0x11

//#### PINS #######

//ultrasonar
#define echoPin 10 // attach pin D5 Arduino to pin Echo of HC-SR04
#define trigPin 11 //attach pin D6 Arduino to pin Trig of HC-SR04
#define d_threshold 30 // distance detect in cm
int duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

//IR
#define IRA A0
#define IRB A2
#define IRC A1


// Motor A
#define enA  9
#define mA1  8
#define mA2  7
// Motor B
#define enB 4
#define mB1 3
#define mB2 2

//buzzer
#define Buzzer 12
#define T 6
#define R 5

//###### Global variables #######
SoftwareSerial SIM900A(T, R);
uint8_t motors_en[] = {enA,enB};
uint8_t motors[] = {mA1,mA2,mB1,mB2};
bool temp_flag;
bool low_stock_mask = 0;
bool low_stock_glove = 0;
uint8_t stock[] = {10, 10}; //mask, glove
int ind;
uint8_t page_count=0;
int count=0;
String menu_option[] = {"Mask","Glove","Mask-Glove"};
String disp_option[] = {" Mask "," Glove "," Stock "," Menu "};
uint8_t state;

void setup() {
  SIM900A.begin(9600);  
  Serial.begin(9600);
  temp.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  // pinMode(ir_pin, INPUT);
  pinMode (IRA, INPUT);
  pinMode (IRB, INPUT);
  pinMode (IRC, INPUT);  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  
  pinMode(Buzzer,OUTPUT);
    
   // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(mA1, OUTPUT);
  pinMode(mA2, OUTPUT);
  pinMode(mB1, OUTPUT);
  pinMode(mB2, OUTPUT);
}

void loop() {  
  // check for low stock
  LowStockMsg(); 
  // distance sensor
  int d = ultra_distance();  
  // Serial.println(d);
  
  if(d <= d_threshold){
    select_tone();    
    uint8_t currentMillis = 0;
    
    while(currentMillis<=temp_time){
      currentMillis++;
      temp_flag = check_temp();
    } 
  if(temp_flag==1){
    disp_any("Select\nOption",2);
    select_tone();     
    
//  Loop in the menu        
   while(d != -1){
      // d = ultra_distance(); 
        
//  A : Dispense Mask
  if(digitalRead(IRA) == 0){ 
    select_tone();
    default_disp(3,menu_option[0]);
    if(stock[0]>0){
      fin_page(0,"Dispensing "+String(1));
      stock_info();     
      LowStockMsg(); 
    }
    else{
      out_of_stock(0);
    }    
        d=-1;
  }
  
//  B : Dispense Glove
  if(digitalRead(IRB) == 0) { 
    select_tone();    
    default_disp(3,menu_option[1]);
    if(stock[1]>0){    
      fin_page(1,"Dispensing "+String(1)); 
      stock_info();     
      LowStockMsg(); 
    }
        
    else{
        out_of_stock(1);
        }   
        d=-1;
  }
    
//  C : Dispense Both Mask & Glove
  if(digitalRead(IRC) == 0) { 
    select_tone();    
    default_disp(3,menu_option[2]);
    if(stock[0]<1){out_of_stock(0);}
    else if(stock[1]<1){out_of_stock(1);}   
    else {  
      fin_page(0,"Dispensing "+String(1)); 
      fin_page(1,"Dispensing "+String(1));
      stock_info();     
      LowStockMsg();
      LowStockMsg();     
    }
        d=-1;
  }  

    // nav_select(1,String(count),state); 
       
  }
// out of menu

   disp_any("Mask-Glove\nDispenser",2);
   }
   else{
        // high temp logic
        disp_any("Alert\nHigh Temp",2);  
        SendMsg(4);
        error_sound(1000);        
        delay(1000);
  }  
select_tone();   
  }    
disp_any("Mask-Glove\nDispenser",2);
}


// ######## Custom functions ######

//ultrasonic
int ultra_distance(){
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.0343 / 2; // Speed of sound wave divided by 2 (go and back)
if(distance<0)
    distance=distance*(-1);  
  return distance;
  }

//temperature check
byte check_temp(){
  byte t = temp.readObjectTempC();
  disp_any("Temp:\n"+ String(((t*1.8)+32)) + " F",2);
  if(((t*1.8)+32)>=temp_threshold){
    return 0;
  }
  else{
    return 1;
  }
}  
  

//####### Display #########//

void disp_any(String txt, uint8_t font){
  display.clearDisplay();
  display.setTextSize(font);           
  display.setTextColor(WHITE);       
  display.setCursor(0,0);             
  display.print(txt);
  display.display();
  delay(50);   
}
void default_disp(uint8_t opt, String str) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);   
  display.setTextColor(BLACK,WHITE);  
  display.println(disp_option[opt]);
  display.setTextSize(2); 
  display.setTextColor(WHITE); 
  if(str.length()<3){
    display.setCursor(48,8);
  }      
  else{
    display.setCursor(60-(str.length()*13/2),8); 
  }             
  display.println(" "+str+" ");

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.display();
  delay(1000);
}

// void left_arrow(){
// display.setCursor(5,25); 
//   display.print(F(" BACK "));
//   display.setCursor(0,25); 
//   display.write(aleft);  
// }
// void right_arrow(){
//   display.setCursor(94,25); 
//   display.print(F(" OK "));
//   display.setCursor(115,25); 
//   display.write(aright);  
// }

// void up_arrow(){
//   display.setCursor(62,0); 
//   display.write(aup);
// }
// void down_arrow(){
//   display.setCursor(62,27); 
//   display.write(adown);
// }

// void nav_select(int nav, String txt, uint8_t menu){
//   display.setTextSize(1); 
//   display.setTextColor(BLACK,WHITE);

//   switch(nav){
//     case 0: display.setCursor(62,0); 
//             display.write(aup);
//             break;

//     case 1: display.setCursor(96,25); 
//             display.print(F(" OK "));
//             display.setCursor(115,25); 
//             display.write(aright);
//             break;

//     case 2: display.setCursor(62,27); 
//             display.write(adown);
//             break;

//     case 3: display.setCursor(5,25); 
//             display.print(F(" BACK "));
//             display.setCursor(0,25); 
//             display.write(aleft);
//             break;
//     }

//   display.display();
//   delay(200);  
//   default_disp(menu,txt);
//   delay(600); 
// }

void fin_page(uint8_t opt, String str){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);   
  display.setTextColor(BLACK, WHITE);    
  display.println(disp_option[opt]);
  display.setTextColor(WHITE);       
  display.setCursor(0,10);            
  display.println(str);

  display.setTextColor(WHITE);

  // left_arrow();

  display.display();
  dispense_item(opt, 1);
  
}

void stock_info(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);   
  display.setTextColor(BLACK, WHITE);    
  display.println(disp_option[2]);
  display.setTextSize(1); 
  display.setTextColor(WHITE);       
  display.setCursor(0,10);            
  display.print("Masks: "+String(stock[0])+"\nGloves: "+String(stock[1]));

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // left_arrow();

  display.display();
  delay(3000);
  disp_any("Thank\nYou",2); 
  delay(2000);
}

void out_of_stock(uint8_t opt){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);   
  display.setTextColor(BLACK, WHITE);    
  display.println(disp_option[opt]);
  display.setTextSize(1); 
  display.setTextColor(WHITE);       
  display.setCursor(0,8);            
  display.print(F("Out of stock"));

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // left_arrow();

  display.display();
  error_sound(1000);   
  SendMsg(opt+2);  
  delay(1000); 
  stock_info();
}

//####### Dispense and Motor ##########//

void dispense_item(uint8_t item, uint8_t nos){
    stock[item] = stock[item] - nos;
    success_sound(0);    
    // motor control 
    motor_control(item); 
}

void motor_control(uint8_t state){
  // Turn on motor m
  uint8_t m = state*2;
  digitalWrite(motors[m], HIGH);
  digitalWrite(motors[m+1], LOW);

  // Set speed to possible range 0~255
  analogWrite(motors_en[state], mSpeed);

  delay(3000);  
  digitalWrite(motors[m], LOW);
  digitalWrite(motors[m+1], LOW);
}

//buzzer
void error_sound(int dlay){
  tone(Buzzer,800);delay(100);
  tone(Buzzer,700);delay(100);
  tone(Buzzer,600);delay(100);
  noTone(Buzzer);
  delay(dlay);
  }
  
void success_sound(int dlay){
  tone(Buzzer,600);delay(100);
  tone(Buzzer,700);delay(100);
  tone(Buzzer,800);delay(100);
  noTone(Buzzer);
  delay(dlay);
  }
void select_tone(){
tone(Buzzer,400);delay(100);
    noTone(Buzzer);delay(200); 
}
//###### GSM SIM900A #######//

void SendMsg(uint8_t no) {
  // Serial.println ("Sending Message");
  SIM900A.println(F("AT+CMGF=1")); // Sets the GSM Module in Text Mode
  delay(500);
  // Serial.println ("Set SMS Number");
  SIM900A.println(F(
      "AT+CMGS=\"+919123456789\"\r")); // Mobile phone number to send message
  delay(500);
  // Serial.println ("Set SMS Content");
  switch (no) {
  case 0:
    SIM900A.println(F("Stock Low\nMask only 3 left."));
    break;
  case 1:
    SIM900A.println(F("Stock Low\nGlove only 3 left."));
    break;
  case 2:
    SIM900A.println(F("Mask No Stock."));
    break;
  case 3:
    SIM900A.println(F("Glove No Stock."));
    break;
case 4:
    SIM900A.println(F("High Temperature Alert"));
    break;    
  }  
  delay(200);
  // Serial.println ("Finish");
  SIM900A.println((char)26); // ASCII code of CTRL+Z
  delay(100);
  // Serial.println ("Message has been sent");
}

void LowStockMsg(){
  if (low_stock_glove == 0 && stock[1] <= stock_threshold) {
    low_stock_glove = 1;
    SendMsg(1);
  } 
  if (low_stock_mask == 0 && stock[0] <= stock_threshold) {
    low_stock_mask = 1;
    SendMsg(0);
  }
    
}
