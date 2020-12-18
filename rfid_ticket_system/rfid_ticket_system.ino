//A Simple Contactless Ticketing System using RFID module

#include<SPI.h>
#include<MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
#define Buzzer 2
#define GREEN_LED 3
#define RED_LED 4

// init RFID
MFRC522 rfid(SS_PIN,RST_PIN);

// change the following to your needs
int BALANCE = 100;
int STOP_PRICE = 10;

// RFID tag as persons
String PERSON_1_CARD = "3359953D";
String PERSON_2_CARD = "47A85C6";

// in and out status flag
byte status_in = 0;
byte status_out = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("\n########################################");
  Serial.println("      Contactless Ticketing System        ");
  Serial.println("########################################\n");
  Serial.println("[INFO] Please place your card...");
  SPI.begin();
  rfid.PCD_Init();
  pinMode(GREEN_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(Buzzer,OUTPUT);
}

void loop() {
// check RFID tag status
if(!rfid.PICC_IsNewCardPresent())
  return;
if(!rfid.PICC_ReadCardSerial())
  return;

tone(Buzzer,600);
delay(200);
noTone(Buzzer);

//get tag ID
//Serial.println("NUID tag is: ");
String ID = "";
for(byte i = 0; i < rfid.uid.size; i++){
    //Serial.print(rfid.uid.uidByte[i]<0*10?"0":"");
    //Serial.print(rfid.uid.uidByte[i],HEX);
    ID.concat(String(rfid.uid.uidByte[i]<0*10?"0":""));
    ID.concat(String(rfid.uid.uidByte[i],HEX));
    delay(10);
  }
Serial.println("\n");
ID.toUpperCase();

String current_card = ID.substring(0);

// check for tag ID in database and is not in.
if(current_card == PERSON_1_CARD && status_in == 0)
{
  if(BALANCE>0){
    digitalWrite(GREEN_LED,HIGH);
    status_in=1;
    
    Serial.println("[MESSAGE] Welcome");
    Serial.print("[PERSON ID] ");
    Serial.println(PERSON_1_CARD);
    Serial.print("[INFO] Balance: ");
    Serial.println(BALANCE);
    Serial.println("[STATUS] IN");
    Serial.println();
    delay(1000);
 }
  else{
    digitalWrite(RED_LED,HIGH);
    Serial.print("[PERSON ID] ");
    Serial.println(PERSON_1_CARD);
    Serial.println("[WARNING] No Balance");
    tone(Buzzer,800);delay(100);
    tone(Buzzer,600);delay(300);
    noTone(Buzzer);
    digitalWrite(RED_LED,LOW);
    delay(300);
  }
}
// deduct amount at end.
else if(current_card == PERSON_1_CARD && status_in == 1)
{
  digitalWrite(GREEN_LED,LOW);
  status_in=0;
  BALANCE = BALANCE - STOP_PRICE;

  Serial.println("[MESSAGE] Thank you for travelling with us.");
  Serial.print("[PERSON ID] ");
  Serial.println(PERSON_1_CARD);
  Serial.print("[INFO] Deducted amount: ");
  Serial.println(STOP_PRICE);
  Serial.print("[INFO] Balance: ");
  Serial.println(BALANCE);
  Serial.println("[STATUS] OUT\n");
  delay(300);
}


else{
Serial.println("[WARNING] Person not in Database\n");
Serial.println("[INFO] Please place correct card..");
tone(Buzzer,800);delay(100);
tone(Buzzer,600);delay(100);
tone(Buzzer,700);delay(100);
noTone(Buzzer);
delay(1000);
}
}
