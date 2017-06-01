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
#define _string_length 100  
#define lengthofpass 6   
#define _blelength_ 20

#define up_config "up"
#define down_config "dn"
#define extraterm_config "ex"

//**********************************************************

#define bleserial mySerial    
#define pcserial Serial          //serial configs

//**********************************************************

#define _status_mode 1
#define _update_mode 2
//**********************************************************

SoftwareSerial mySerial(8,9); 

//**********************************************************

const char update_string[]="up";
const char status_string[]="st";

bool rst=HIGH;
String bleresp="";

String username;
String password;

char storedpass[7]={0};

String default_username="default";
String default_pass="000000";

const char at_username[]="AT+NAME";
const char at_pass[]="AT+PASS";

void(* resetFunc) (void) = 0;

void setup() {
  
  bleserial.begin(9600);
  pcserial.begin(9600);
  delay(1000);
  // char bac[6]={'0','0','0','0','0','0'};   //uncomment these lines to set eeprom 000000
  // writestringeeprom(bac);
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
  storedpass[i+1]='\0';
  pcserial.print( "stored pass: "); 
   
   for (int i=0;i<lengthofpass+1;i++)
  {
     pcserial.print( storedpass[i]); 
  } 
}

void loop() {
  if (digitalRead(rst_red)==LOW)
  {
    ledblink(5);
    ble_rst();}
  
  if(bleserial.available()>0){
  readfunction();}

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
{  
  bleresp="";
  bleresp += bleserial.readString();
  pcserial.println("              ");
  pcserial.println("received string: ");
  pcserial.println(bleresp);
    
  char bleresp_copy[_string_length]={'1'};
  strcpy(bleresp_copy,bleresp.c_str());

   pcserial.println("              ");
   pcserial.println("blerespcopy: ");
   pcserial.println(bleresp_copy);
  /*********************************************
  string format to be received:
  update:
  #update#username#current_pass#new_pass#
  eg: #up#default#000000#111111#

  status:
  #status#username#current_pass#up/dwn#extra
  eg: #st#default#000000#dn#ex# , #st#default#000000#dn#00#,#st#default#000000#00#ex#

  login:
  #lg#username#current_password#
  eg: #lg#default#000000#
  ******************************************/

  String received_task_string     = strtok(bleresp_copy,_parameter_key); //takes out "update" or "status"
  String received_username_string = strtok(NULL,_parameter_key);         //takes out "username"
  String received_pass_string     = strtok(NULL,_parameter_key);         //takes out current password
  String received_4_string        = strtok(NULL,_parameter_key);         //takes out 4th param | "new_pass" or"up/dwn"
  String received_5_string        = strtok(NULL,_parameter_key);         //takes out 5th param| "extra terminal" (optional param)

  const char* received_task     = received_task_string.c_str();
  const char* received_pass     = received_pass_string.c_str(); 
  const char* received_username = received_username_string.c_str();
  const char* received_4        =  received_4_string.c_str(); 
  const char* received_5        =  received_5_string.c_str();

 
  pcserial.println("task received: " +received_task_string);
  pcserial.println("username received: "+received_username_string);
  pcserial.println("pass received: " + received_pass_string);
  pcserial.println("4th param received: "+received_4_string); 
  pcserial.println("5th param received: "+received_5_string);
  pcserial.println("***********************"); 


  if (strcmp(received_pass,storedpass)==0) //checks password
 {  //bleserial.println("cp");

    if(strcmp(received_task,"lg")==0) //login
      { bleserial.println("cp");
        pcserial.println("login successful");
        return 0;}

    else if (strcmp(received_task,status_string )==0) //enters status/toggle mode
     {pcserial.println("toggle mode"); 
      bleserial.println("cp"); 
      toggleupdown(received_4,received_5);
      return 0;}
  
    else if (strcmp(received_task,update_string)==0) //enter update mode
      { pcserial.println("update mode");
        bleserial.println("cp");
        bleupdate(received_username_string,received_4_string);

        return 0;}
    else
    {
      pcserial.println("invalid query");
      bleserial.println("invalid query");
      bleserial.println("wp");
    }    
 }
  else 
  { 
    pcserial.println("WRONG PASS!!");
    bleserial.println("wp");
    return 0;  
  }
} 

void bleupdate(String username,String password) /////////////add code for response to ble
{ ledblink(2);

  digitalWrite(ble,LOW); //power of BLE module
  delay(10);
  digitalWrite(ble,HIGH);
  delay(_toggle_wait);

  pcserial.println("  ");
  pcserial.println(at_username + username);
  pcserial.println(at_pass + password); 

  bleserial.println(at_username + username);
  delay(1000);
  bleserial.println(at_pass + password);
  
  delay(1000);
  digitalWrite(ble,LOW); //power off BLE module
  delay(_toggle_wait);
  digitalWrite(ble,HIGH);
  pcserial.println("*************");
  const char* temppass = password.c_str(); 
  writestringeeprom(temppass); 
  ledblink(2);
    ////////////add code for response to ble
  resetFunc();
  bleserial.println("update success");  
}

void toggleupdown(const char* main_terminal, const char* extra_terminal)
{
    if (strcmp(main_terminal,up_config )==0)
     { 
      pcserial.println("this is up config");
      all_low();
      digitalWrite(up,HIGH);
      delay(_toggle_wait);
      digitalWrite(up,LOW);
      bleserial.println("up success"); 
      return;
    }

    if (strcmp(main_terminal,down_config )==0)  
      { 
        pcserial.println("this is down config");
        all_low();
        digitalWrite(down,HIGH);
        delay(_toggle_wait); 
        digitalWrite(down,LOW); 
        bleserial.println("down success");
        return;     
      }

   if (strcmp(extra_terminal,extraterm_config )==0)  
    { 
      pcserial.println("this is extra_term config");
      all_low();
      digitalWrite(extra_term,HIGH);
      delay(_toggle_wait); 
      digitalWrite(extra_term,LOW); 
      bleserial.println("extra success");     
      return;
    }

   else  all_low();
}

void ble_rst()
{   pcserial.println("ble reset");
    bleserial.println("ble reset"); 
    bleresp="";
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
    //resetFunc(); 
    pcserial.println("reset done");
}

void writestringeeprom(const char* passed)
{   
  int i=0;
  for (i=0;i<lengthofpass;i++)
  {
    EEPROM.write(i,passed[i]);
    storedpass[i] = EEPROM.read(i);
    pcserial.print(storedpass[i]);
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
  } */

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
  

