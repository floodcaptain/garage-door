#include <SoftwareSerial.h>
#include <EEPROM.h>
/**********************************************************
            HARDWARE RELATED CONFIGS */
#define up 15                    //up terminal
#define down 14                  //down terminal
#define extra_term 13            //extra terminal

#define rst_red 7                //red led (also the rst button)
#define led_green 6              //green led
#define _toggle_wait 600         //terminal toggle delay
/**********************************************************
           BLUETOOTH RELATED CONFIGS */
#define ble A3                    
#define _parameter_key "#"    //#seperated strings
#define _string_length 50  
#define lengthofpass 6   

#define up_config "A0"
#define down_config "B0"
#define extraterm_config "0A"

//**********************************************************

#define bleserial mySerial    
#define pcserial Serial          //serial configs

//**********************************************************

#define _status_mode 1
#define _update_mode 2
//**********************************************************

SoftwareSerial mySerial(8,9); 

//**********************************************************

/*const char up_config[_string_length]  =up_string;
const char down_config[_string_length]=down_string;
const char extraterm_config[_string_length]=extraterm_string; */

String task;
const char update_string[]="up";
const char status_string[]="st";

bool rst=HIGH;
volatile char bleresp[_string_length]={0};

String username;
String password;

char storedpass[7]={0};

String default_username="default";
String default_pass="000000";
char ch;
int p=0;
const char at_username[]="AT+NAME";
const char at_pass[]="AT+PASS";

void(* resetFunc) (void) = 0;

void setup() {
  
  bleserial.begin(9600);
  pcserial.begin(9600);
  
  pinMode(up,OUTPUT);
  pinMode(down,OUTPUT);
  pinMode(extra_term,OUTPUT);
  
  pinMode(rst_red,INPUT_PULLUP);
  pinMode(led_green,OUTPUT);
  
  pinMode(ble,OUTPUT);

  digitalWrite(ble,HIGH);
  
  pcserial.println("              ");
  pcserial.print("module restarted!!");
  int i=0;  
   for (i=0;i<lengthofpass;i++)
  {
   storedpass[i] = EEPROM.read(i) ; 
  } 
  storedpass[i]='\0';

   for (int i=0;i<lengthofpass+1;i++)
  {
  pcserial.print( storedpass[i]); 
  } 

}

void loop() {
  digitalWrite(led_green,HIGH);
  digitalWrite(rst_red,HIGH);
  digitalWrite(up,HIGH);
  digitalWrite(down,HIGH);
  digitalWrite(extra_term,HIGH);

  delay(600);
  
  digitalWrite(led_green,LOW);
  digitalWrite(rst_red,LOW);
  digitalWrite(up,LOW);
  digitalWrite(down,LOW);
  digitalWrite(extra_term,LOW);

  delay(600); 
  

}
