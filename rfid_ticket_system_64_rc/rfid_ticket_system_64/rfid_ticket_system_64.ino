//Written on Fri Feb 07 by nannigalaxy
//A Simple Contactless Ticketing System using RFID module

#include <Adafruit_SH1106.h>
//#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 temp = Adafruit_MLX90614();

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SH1106 display(OLED_RESET);


#define R1_DATA 8
#define R2_DATA 7
#define NR_OF_READERS   2
#define RST_PIN 9
#define Buzzer 2 
#define GREEN_LED A0
#define RED_LED 4
#define STOP_PRICE 10
#define timeout 120
#define temp_time 40

#define echoPin 5 // attach pin D5 Arduino to pin Echo of HC-SR04
#define trigPin 6 //attach pin D6 Arduino to pin Trig of HC-SR04
#define d_threshold 10 // distance detect in cm

#define servoA 10
#define servoB 3
#define front 0
#define back 1
#define degree 180

byte servo[2] = {3, 10};

int duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

bool temp_flag;

// init RFID
byte ssPins[] = {R1_DATA, R2_DATA};
MFRC522 rfid[NR_OF_READERS];

// change the following to your needs
int BALANCE[] = {20,10,0};
int counter = 0;
// RFID tag as persons
const String PERSON_CARD[] = {"3359953D", "470A85C6", "BA50592F"};

// in and out status flag
bool status_in[] = {0,0,0};
byte len = sizeof(status_in)/sizeof(status_in[0]);

void setup() {
//  Serial.begin(9600);
  SPI.begin();
  temp.begin();
//  servo[0].attach(servoA);
//  servo[1].attach(servoB); 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  
for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    rfid[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
//    Serial.print(F("Reader "));
//    Serial.print(reader);
//    Serial.print(F(": "));
//    rfid[reader].PCD_DumpVersionToSerial();
  }
  pinMode(GREEN_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(Buzzer,OUTPUT);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
//    splash();
  display.clearDisplay();
//  Serial.println("[INFO] Please place your card...");
}
//################### setup


void loop() {
  default_disp(counter);
  // distance sensor
  int d = ultra_distance();
  if(d<d_threshold){
  // check RFID tag status
  uint8_t currentMillis = 0;
  
  while(currentMillis<=temp_time){
    currentMillis++;
//    Serial.println(currentMillis);
  temp_flag = check_temp();
//  Serial.println(temp_flag);
  }
  if(temp_flag==1){
    digitalWrite(GREEN_LED,HIGH);
    // Look for new cards
    String card_id = get_info(0);
    currentMillis = 0;
      display_any("Please\n","place the card",0);
    while(card_id=="" && currentMillis<timeout){
      card_id = get_info(0);
      currentMillis++;
    }
    digitalWrite(GREEN_LED,LOW);
    if(currentMillis<timeout){
      rfid_main(card_id, 0);
      }
     else{
      display_any("Timeout","",0);
      red_led(500);
      }
    }
    else {
      display_any("Temperatue Exceeded!","",0);
  error_indication(1000);
      }
  }

String id = get_info(1);
if(id!=""){
//  Serial.println(id);
  rfid_main(id, 1);
  }
}
// loop end #############################################
String get_info(uint8_t r){
  
  if (rfid[r].PICC_IsNewCardPresent() && rfid[r].PICC_ReadCardSerial()) {
  tone(Buzzer,600);
  delay(200);
  noTone(Buzzer);
  String card_id = "";
  card_id = dump_byte_array(rfid[r].uid.uidByte, rfid[r].uid.size);
//  Serial.print(card_id);
  return card_id;
  }
//  Serial.print("No");
  return "";
}

String dump_byte_array(byte *card, byte c_size) {
  String R1 = "";
for(byte i = 0; i < c_size; i++){
    R1.concat(String(card[i]<0x10?"0":""));
    R1.concat(String(card[i],HEX));
  }
  R1.toUpperCase();
return R1.substring(0);
  }

// ********************************************
// custom functions

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

byte check_temp(){
  byte t = temp.readObjectTempC();
  display_any("Temp: ",String(t) + " C",20);
  if(t>=38){
    return 0;
    }
  else{
    return 1;
    }
  }

void rfid_main(String CARD, byte reader){
  
// getting inside bus
if(reader==0){ 
  int8_t index = -1;
  for(byte i=0; i<len; i++){
    if(CARD.equals(PERSON_CARD[i])){
      index = i;
      break;
      }
    }
  if(index>=0 && status_in[index] == 0)
{
  if(BALANCE[index]>0){
    digitalWrite(GREEN_LED,HIGH);
    status_in[index]=1;
    counter++;
    // Display on OLED
    status_in_disp(PERSON_CARD[index], BALANCE[index]);
    digitalWrite(GREEN_LED,LOW);
    servo_door(front);
 }
  else{
    out_of_bal(PERSON_CARD[index]);
    
  }
}
else if(status_in[index] == 1){
  display_any("Already\nboarded","",0);
  red_led(1500);
}
else{
  not_found();
}
  }


// going out of bus
else if(reader==1){ 
  int8_t index = -1;
  for(byte i=0; i<len; i++){
    if(CARD.equals(PERSON_CARD[i])){
      index = i;
      
      break;
      }
    }
  if(index>=0 && status_in[index] == 1)
{
  
  status_in[index]=0;
  counter--;
  BALANCE[index] = BALANCE[index] - STOP_PRICE;
  status_out_disp(PERSON_CARD[index], BALANCE[index], STOP_PRICE);
  servo_door(back);
}


else{
  display_any("Please\nboard in\nfirst","",0);
  red_led(1500);
  }
  }
}
void splash(){
  display.clearDisplay();
  String msg = "Contactless\nTicketing\nSystem";
  display_any(msg,"",3000);
  }
  
void default_disp(int8_t count) {  
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);   
  display.println(F("Contactless TicketingSystem\n"));
  
  display.setTextSize(2);             // Normal 1:1 pixel scale 
  display.print(F("Count: ")); display.print(count);
  display.display();
  display.clearDisplay();
  }
  
void status_in_disp(String id, int8_t balance) {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("ID: ")); display.println(id);
  display.print(F("\nBalance: ")); display.println(balance);
  display.println();
  display.setTextSize(2);             // Draw 2X-scale text
  display.println(F("Welcome"));

  display.display();
  delay(3000);
  display.clearDisplay();
}


void status_out_disp(String id, int8_t balance, int8_t debit) {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("ID: ")); display.println(id);
  display.print(F("\nBalance: ")); display.println(balance);
  display.print(F("\nDebited: ")); display.println(debit);
  display.println();
  display.setTextSize(2);             // Draw 2X-scale text
  display.println(F("Thank you"));

  display.display();
  delay(3000);
  display.clearDisplay();
}


void out_of_bal(String id) {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("ID: ")); display.println(id);
  display.println();
  display.setTextSize(2);             // Draw 2X-scale text
  display.println(F("No Balance"));
  display.display();
  error_indication(2000);
  display.clearDisplay();
}

void not_found() {
    digitalWrite(RED_LED,HIGH);
  display_any(F("Sorry, not \nRegistered"),"",0);
  error_indication(2000);
  display.clearDisplay();
}

void error_indication(int dlay){
  digitalWrite(RED_LED,HIGH);
  tone(Buzzer,800);delay(100);
  tone(Buzzer,700);delay(100);
  tone(Buzzer,600);delay(100);
  noTone(Buzzer);
  delay(500);
  digitalWrite(RED_LED,LOW);
  delay(dlay);
  }

void red_led(int dlay){
  digitalWrite(RED_LED,HIGH);
  delay(dlay);
  digitalWrite(RED_LED,LOW);
  }

void display_any(String txt1, String txt2, int dlay) {
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(txt1);
  display.println(txt2);
  display.display();
  delay(dlay);
}

void servo_door(bool s){
  for (byte i = 0; i < degree; i++) { 
    servo_run(i,servo[s]);              
  }
  delay(500);
  for (byte i = degree; i > 0; i--) { 
    servo_run(i,servo[s]);               
  }
  }

void servo_run(int x, int pin){
  int val = (x*10.25)+500;
  digitalWrite(pin,HIGH);
  delayMicroseconds(val);
  digitalWrite(pin,LOW);
  delay(10);
  }
