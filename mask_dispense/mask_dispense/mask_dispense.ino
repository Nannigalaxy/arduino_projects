#include "Adafruit_APDS9960.h"
#include <Adafruit_SH1106.h>
Adafruit_APDS9960 apds;
Adafruit_SH1106 display(-1);

// symbol character hex
#define aup  0x1e
#define adown  0x1f
#define aright  0x10
#define aleft  0x11
#define len 3

uint8_t fg;
uint8_t stock[] = {3,5}; //mask, glove
int ind;
uint8_t page_count=0;
int count=0;
String menu_option[] = {"Mask","Glove","Stock"};
String disp_option[] = {" Mask "," Glove "," Stock "," Menu "};
uint8_t state;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  if(!apds.begin()){
    Serial.println("failed to initialize Gesture sensor! Please check your wiring.");
  }
  else Serial.println("Gesture sensor initialized!");

  //gesture mode will be entered once proximity mode senses something close
  apds.enableProximity(true);
  apds.enableGesture(true);

  default_disp(3,menu_option[0]); 
  

}

void loop() {
  // put your main code here, to run repeatedly:
fg=0;
  uint8_t gesture = apds.readGesture();
  if(gesture == APDS9960_UP){ 
    if(page_count==0){
    count--;
    ind = count%len;
    ind = ind < 1 ? ind*(-1) : ind;
    state = uint8_t(ind);
    nav_select(0, menu_option[ind],3);
    Serial.print(String(ind)+"\t"+menu_option[ind]+"\t"+count+"|");
    Serial.print(String(page_count));    
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
    Serial.print(String(ind)+"\t"+menu_option[ind]+"\t"+count+"|");    
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
      if(page_count==0 && !(state==2) && fg==0){ 
        page_count++;
        count=1;      
        nav_select(1,"1",state); 
          if(page_count==1 && state!=2 && stock[state]==0){  
            out_of_stock(state);                  
        }
      }
      
      if(state==2){
        nav_select(1,String(count),state); 
        stock_info();       
      }
    }
  
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
    display.setCursor(50,8);
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
  if(page_count!=0)
  left_arrow();

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

void dispense_item(uint8_t item, uint8_t nos){
    stock[item] = stock[item] - nos;
    // motor control 
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

  left_arrow();

  display.display();
  delay(1500);
  count=0;
  page_count=0;
  default_disp(3,menu_option[0]);   
}