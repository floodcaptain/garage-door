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

  rst = digitalRead(rst_red);
  if (rst==LOW)
  {
    ledblink(5);
    ble_rst();}
   
  p=0;
  
  if(bleserial.available()>0){
  p=readfunction();}

  if(p==_status_mode)
  { toggleupdown();}

  if(p==_update_mode)    
  { bleupdate();} 

  all_low();

  delay(200);
} 

void all_low()
{
  digitalWrite(down,LOW);
  digitalWrite(up,LOW);
  digitalWrite(extra_term,LOW);
}

int readfunction()
{ for (int i = 0; i < _string_length; i++) //reads serial output from BLE
    {bleresp[i] = bleserial.read();}    

  for (int i =0;i<_string_length;i++)
  { pcserial.print(bleresp[i]);}
  pcserial.println("    ");     
  
  char bleresp_copy[_string_length];
  strcpy(bleresp_copy,bleresp);

  String current_pass_temp="";
  String current_task_temp="";

  current_pass_temp = strtok(bleresp_copy,_parameter_key);
  const char* current_pass = current_pass_temp.c_str(); 

  current_task_temp = strtok(NULL,_parameter_key);
  const char* current_task = current_task_temp.c_str(); 


 if (strcmp(current_pass,storedpass)==0)
 {  if (strstr((char*)bleresp,status_string )) //strstr(task_temp,status_string )
     { return _status_mode;}
  
    else if (strstr((char*)bleresp,update_string ))
      { return _update_mode;}
 }
  else 
  { pcserial.println("WRONG PASS!!");
    return 0;  
  }
} 

void bleupdate()
{ ledblink(2);
  strtok(bleresp,_parameter_key);         //seperates "previous password"
  strtok(NULL,_parameter_key);            //seperates "update"
  username = strtok(NULL,_parameter_key); //seperates "username"
  password = strtok(NULL,_parameter_key); //seperates "password"

  digitalWrite(ble,LOW); //power of BLE module
  delay(10);
  digitalWrite(ble,HIGH);
  delay(_toggle_wait);

  pcserial.println("  ");
  pcserial.println(at_username + username);
  pcserial.println(at_pass + password); 

  bleserial.print(at_username + username);
  delay(1000);
  bleserial.print(at_pass + password);
  
  delay(1000);
  digitalWrite(ble,LOW); //power off BLE module
  delay(_toggle_wait);
  digitalWrite(ble,HIGH);
  pcserial.println("*************");
  const char* temppass = password.c_str(); 
  writestringeeprom(temppass); 
  ledblink(2);
  resetFunc();  
}

void toggleupdown()
{
    if (strstr((char*)bleresp,up_config ))
     { 
      pcserial.println("this is up config");
      all_low();
      digitalWrite(up,HIGH);
      delay(_toggle_wait);
      digitalWrite(up,LOW);}

    if (strstr((char*)bleresp,down_config ))  
      { 
        pcserial.println("this is down config");
        all_low();
        digitalWrite(down,HIGH);
        delay(_toggle_wait); 
        digitalWrite(down,LOW);     
      }

   if (strstr((char*)bleresp,extraterm_config ))  
    { 
      pcserial.println("this is extra_term config");
      all_low();
      digitalWrite(extra_term,HIGH);
      delay(_toggle_wait); 
      digitalWrite(extra_term,LOW);     
    }

   else  all_low();
}

void ble_rst()
{    pcserial.println("ble reset");
    memset(bleresp,0,_string_length);
    digitalWrite(ble,LOW); //power off BLE module
    delay(100);
    digitalWrite(ble,HIGH);
    delay(600);
    
    bleserial.print(at_username + default_username);
    delay(1000);
    bleserial.print(at_pass + default_pass);
    writestringeeprom("000000");
      
    digitalWrite(ble,LOW); //power off BLE module
    delay(500);
    digitalWrite(ble,HIGH);   
       
    while(rst !=HIGH)
    {
      rst = digitalRead(rst_red);
    }
    bleserial.print("AT+RESET");
    delay(500);
    resetFunc(); 
}

  void writestringeeprom(char* passed)
  { int i=0;
    for (i=0;i<lengthofpass;i++)
    {
      EEPROM.write(i,passed[i]);
    }
         
    for (i=0;i<lengthofpass;i++)
    {
     char eread = EEPROM.read(i) ;
     pcserial.print(eread); 
    }

   for (int i=0;i<lengthofpass;i++)
  {
   storedpass[i] = EEPROM.read(i) ; 
  } 
  }

void ledblink(int a)
{
  for (int i=0;i<a;i++)
  {   digitalWrite(led_green,HIGH);
      delay(200);
      digitalWrite(led_green,LOW);
      delay(200);
  }
}

/* //TEST CODE
  digitalWrite(led_green,HIGH);
  digitalWrite(led_red,HIGH);
  digitalWrite(up,HIGH);
  digitalWrite(down,HIGH);
  digitalWrite(extra_button,HIGH);

  delay(600);
  
  digitalWrite(led_green,LOW);
  digitalWrite(led_red,LOW);
  digitalWrite(up,LOW);
  digitalWrite(down,LOW);
  digitalWrite(extra_button,LOW);

  delay(600); */

/********************code snippet of my custom string extractor****************/
   /*  char *p;
  p=strstr(bleresp,_parameter_key); //#
  pcserial.print("strstr compared to # done");
    int i=1;
    
    while(*(p+i)!='#')
    { 
      username[i]= *(p+i);
      ++i;
    }
    //username[i]='\0';
    pcserial.println("first while loop done");
    int length_username=i;
   
    i+=2;
    
    while(*(p+i)!='#')
    {
        password[i]= *(p+i);
        delay(100);
        ++i;
    } 
  
    int length_password=i-length_username;
    
    //password[i]='\0';
    pcserial.println("second while loop done"); 
    
    delay(500);
    
    
    digitalWrite(ble,LOW); //power of BLE module
    delay(10);
    digitalWrite(ble,HIGH);
    delay(500); 
    char* temp=NULL;
    temp=  malloc(2 * _string_length * sizeof(char));
    memcpy(temp,at, 7 * sizeof(char)); 
    memcpy(temp + 7, username, length_username * sizeof(char));
    temp[_string_length * sizeof(char)]='\0';
    
    bleserial.println(temp);
    pcserial.print(temp);
    free(*temp); 
  */
  

