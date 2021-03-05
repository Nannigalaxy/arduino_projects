//Written on Fri Mar 05 by nannigalaxy
//Example for Gesture navigation with menu (Mask Glove Dispenser)

#include <Adafruit_APDS9960.h>
#include <Adafruit_SH1106.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 temp = Adafruit_MLX90614();

Adafruit_APDS9960 apds;
Adafruit_SH1106 display(-1);

#define timeout 120
#define temp_time 40
#define temp_threshold 38
#define len 3
#define mSpeed 150

// symbol character hex
#define aup  0x1e
#define adown  0x1f
#define aright  0x10
#define aleft  0x11

//#### PINS #######

//ultrasonar
#define echoPin 10 // attach pin D5 Arduino to pin Echo of HC-SR04
#define trigPin 11 //attach pin D6 Arduino to pin Trig of HC-SR04
#define d_threshold 10 // distance detect in cm
int duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

//ir
#define ir_pin 3

// Motor A
#define enA  9
#define mA1  8
#define mA2  7
// Motor B
#define enB  5
#define mB1  4
#define mB2  6

//buzzer
#define Buzzer 12

//###### Global variables #######

uint8_t motors_en[] = {enA,enB};
uint8_t motors[] = {mA1,mA2,mB1,mB2};
bool temp_flag;
uint8_t fg;
uint8_t stock[] = {3,5}; //mask, glove
int ind;
uint8_t page_count=0;
int count=0;
String menu_option[] = {"Mask","Glove","Stock"};
String disp_option[] = {" Mask "," Glove "," Stock "," Menu "};
uint8_t state;

void setup() {
  Serial.begin(9600);
  temp.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(ir_pin, INPUT);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  
  if(!apds.begin()){
    Serial.println("failed to initialize Gesture sensor! Please check your wiring.");
  }
  else Serial.println("Gesture sensor initialized!");

  //gesture mode will be entered once proximity mode senses something close
  apds.enableProximity(true);
  apds.enableGesture(true);

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
  fg=0;

  // distance sensor
  int d = digitalRead(ir_pin); //= ultra_distance();
  if(d==0){
  uint8_t currentMillis = 0;
  while(currentMillis<=temp_time){
    currentMillis++;
    temp_flag = check_temp();
  } 
  if(temp_flag==1){
    default_disp(3,menu_option[0]);        
   while(d==0){
      d = digitalRead(ir_pin);// = ultra_distance();    
      uint8_t gesture = apds.readGesture();

  if(gesture == APDS9960_UP){ 
    if(page_count==0){
      count--;
      ind = count%len;
      ind = ind < 1 ? ind*(-1) : ind;
      state = uint8_t(ind);
      nav_select(0, menu_option[ind],3);
    }
    if(page_count==1 && state!=2){    
      if(count>1){
        count--;        
        nav_select(0, String(count),state);
      }      
    }      
  }
  
  if(gesture == APDS9960_DOWN) { 
    if(page_count==0){    
    count++;
    ind = count%len;
    ind = ind < 1 ? ind*(-1) : ind;
    state = uint8_t(ind);
    nav_select(2, menu_option[ind],3);   
  }
    
      if(page_count==1 && state!=2){   
        if(count<stock[state]){
          count++;        
          nav_select(2, String(count),state);
        }      
    }
  }

  if(gesture == APDS9960_LEFT) {
    if(page_count>0){
      page_count--;
      count=1;
      nav_select(3,"1",state);
    }
    if(page_count==0){
      default_disp(3,menu_option[state]); 
    }
    
    
  }   
  if(gesture == APDS9960_RIGHT){
      if(page_count==1){
        nav_select(1,String(count),state); 
        fin_page(state,"Dispensing "+String(count));
        fg=1;       
      }
      else if(page_count==0 && !(state==2) && fg==0){ 
        page_count++;
        count=1;      
        nav_select(1,"1",state); 
          if(page_count==1 && state!=2 && stock[state]==0){  
            out_of_stock(state);                  
        }
      }
      if(state==2){
        // nav_select(1,"",state); 
        stock_info();       
      }
    }
  }  
  disp_any("Mask-Glove\nDispenser",2);
   }
   else{
        // high temp
        disp_any("High\nTemperature",2);  
        error_sound(1000);        
        delay(1000);
  }  
  }    
disp_any("Mask-Glove\nDispenser",2);
}


// ######## Custom functions

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
  return distance;
  }

//temperature check
byte check_temp(){
  byte t = temp.readObjectTempC();
  disp_any("Temp: "+ String(t) + " C",2);
  if(t>=temp_threshold){
    return 0;
    }
  else{
    return 1;
    }
  }  
  
//display
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
    display.setCursor(60-(str.length()*15/2),8); 
  }             
  display.println(" "+str+" ");

  display.setTextSize(1);
  display.setTextColor(WHITE);

  up_arrow();

  right_arrow();

  down_arrow();
  
  if(page_count!=0)  left_arrow();

  display.display();
  delay(100);
}

void left_arrow(){
display.setCursor(5,25); 
  display.print(F(" BACK "));
  display.setCursor(0,25); 
  display.write(aleft);  
}
void right_arrow(){
  display.setCursor(94,25); 
  display.print(F(" OK "));
  display.setCursor(115,25); 
  display.write(aright);  
}

void up_arrow(){
  display.setCursor(62,0); 
  display.write(aup);
}
void down_arrow(){
  display.setCursor(62,27); 
  display.write(adown);
}

void nav_select(int nav, String txt, uint8_t menu){
  display.setTextSize(1); 
  display.setTextColor(BLACK,WHITE);

  switch(nav){
    case 0: display.setCursor(62,0); 
            display.write(aup);
            break;

    case 1: display.setCursor(96,25); 
            display.print(F(" OK "));
            display.setCursor(115,25); 
            display.write(aright);
            break;

    case 2: display.setCursor(62,27); 
            display.write(adown);
            break;

    case 3: display.setCursor(5,25); 
            display.print(F(" BACK "));
            display.setCursor(0,25); 
            display.write(aleft);
            break;
    }

  display.display();
  delay(200);  
  default_disp(menu,txt);
  delay(600); 
}

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

  left_arrow();

  display.display();
  dispense_item(state, count);
  count=0;
  page_count=0;
  default_disp(3,menu_option[0]); 
  
}

void stock_info(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);   
  display.setTextColor(BLACK, WHITE);    
  display.println(disp_option[2]);
  display.setTextSize(1); 
  display.setTextColor(WHITE);       
  display.setCursor(0,8);            
  display.print("Mask: "+String(stock[0])+"\nGloves: "+String(stock[1]));

  display.setTextSize(1);
  display.setTextColor(WHITE);

  left_arrow();

  display.display();
  delay(100);
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

  left_arrow();

  display.display();
  delay(1500);
  count=0;
  page_count=0;
  default_disp(3,menu_option[0]);   
}

//dispense and motor
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

  delay(500);  
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