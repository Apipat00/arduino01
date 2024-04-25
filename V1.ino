#include <EEPROM.h>
#include <Wire.h>                 // เรียกใช้งานไฟล์ Wire.h เพื่อเปิดใช้งาน I2C
#include <ESP32Servo.h>           // เรียกใช้งานไฟล์ ESP32Servo.h เพื่อใช้คำสั่งควบคุม Servo
#include <Keypad_I2C.h>           // เรีียกใช้งานไฟล์ Keypad_I2C.h เพื่อใช้ติดต่อกับ Keypad
#include <LiquidCrystal_I2C.h>    // เรียกใช้งานไฟล์ LiquidCrystal_I2C.h เพื่อใช้คำสั่งติดต่อกับจอ LCD
#include <WiFi.h>                 // เรียกใช้งานไฟล์ WiFi.h
#include <WiFiClient.h>           // เรียกใช้งานไฟล์ WiFiClient.h
#include <WebServer.h>            // เรียกใช้งานไฟล์ WebServer.h
#include <WiFiClientSecure.h>     // เรียกใช้งานไฟล์ WiFiClientSecure.h
#include <ArduinoJson.h>
#include <HTTPClient.h>

//**********************************************************************************
// จอ LCD กำหนดแอดเดรส 0x27 กำหนดขนาดจอ 16 อักษร 2 บรรทัด
//**********************************************************************************
LiquidCrystal_I2C lcd(0x27,16,2);


//**********************************************************************************
// Keypad 4x4
//**********************************************************************************
#define I2CADDR 0x21 // กำหนดแอดเดรสติดต่อกับ Keypad 

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {7, 6, 5, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {3, 2, 1, 0};    //connect to the column pinouts of the keypad

Keypad_I2C customKeypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR); 


//**********************************************************************************
// Sevo Motor
//**********************************************************************************
#define POSITION_CLOSE  105
#define POSITION_OPEN    10

Servo MyServo;
const byte SERVO_PIN = 32;


//**********************************************************************************
// Inductive Sensor
//**********************************************************************************
const byte INDUCTIVE_SENSOR = 33;


//**********************************************************************************
// Infrared Sensor
//**********************************************************************************
const byte INFRARED_SENSOR = 25;


WebServer server(80);

/*****************************************************************************/
// แอดเดรส EEPROM                      
/*****************************************************************************/
#define EEPROM_SSID_ADDR        1   // กำหนดแอดเดรสเริ่มต้นในการเก็บ Wifi name
#define EEPROM_PASS_ADDR        34  // กำหนดแอดเดรสเริ่มต้นในการเก็บ Password

#define SSID_LEN                33   // จอง EEPROM ไว้ 33 ไบต์ สำหรับเก็บ WIFI Name
#define PASS_LEN                33   // จอง EEPROM ไว้ 33 ไบต์ สำหรับเก็บ Password

char Ssid[SSID_LEN];                        // ตัวแปรสำหรับเก็บ Name ของ Wifi
char Password[PASS_LEN];                    // ตัวแปรสำหรับเก็บ Password ของ Wifi
char Pass[6] = {'1','2','3','4','5','6'};

/***********************************************************************************/
/* FUNCTION : setup()                                                              */
/***********************************************************************************/
void setup() {
  
  unsigned long Timeout = millis();


  pinMode(INDUCTIVE_SENSOR,INPUT_PULLUP);
  pinMode(INFRARED_SENSOR ,INPUT_PULLUP);


  Serial.begin(115200);
  
  customKeypad.begin( );  

  lcd.begin();

  
  MyServo.attach(SERVO_PIN);
  MyServo.write(POSITION_CLOSE);  

    // เปิดใช้งาน EEPROM
  EEPROM.begin(1000);

  // กำหนดค่าเริ่มต้นใน EEPROM
  if(EEPROM.read(0)!=0x02){
    EEPROM.write(0,0x02);
    
    // กำหนดชื่อ WIFI
    String str;
    str = "Toplewtina2G";  
    str.toCharArray(Ssid,str.length()+1);
    EEPROM.put(EEPROM_SSID_ADDR,Ssid);

    // กำหนดรหัสผ่าน
    str = "0838224023";  
    str.toCharArray(Password,str.length()+1);
    EEPROM.put(EEPROM_PASS_ADDR,Password);

    for(byte i=0; i<6; i++){
      EEPROM.write(200+i,Pass[i]);
    }

    EEPROM.commit();  
  }


  //############################################################
  // อ่านค่าจาก EEPROM
  //############################################################
  EEPROM.get(EEPROM_SSID_ADDR,Ssid);
  EEPROM.get(EEPROM_PASS_ADDR,Password);
  for(byte i=0; i<6; i++){
    Pass[i] = EEPROM.read(200+i);
  }

  Serial.print("Ssid=");      
  Serial.println(Ssid);
  
  Serial.print("Password=");  
  Serial.println(Password);

  Serial.print("Admin Password:");
  for(byte i=0; i<6; i++){
    Serial.print(Pass[i]);
  }
  Serial.println();


  lcd.setCursor(0,0);
  lcd.print(" Connecting...  ");
  lcd.setCursor(0,1);
  lcd.print(Ssid);

  WiFi.softAPdisconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(Ssid,Password);           //เชื่อมต่อกับ AP  

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(Ssid);


  Timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if((millis()-Timeout)>60000){
      goto Exit;    
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  Exit:;


}


/***********************************************************************************/
/* FUNCTION : loop()                                                               */
/***********************************************************************************/
void loop() {
  static byte Count = 0;
  static unsigned long InfraredTime = millis();
  static unsigned long InductiveOnTime = millis();
  static unsigned long InductiveOffTime = millis();
  static unsigned long ServoTime = millis();
  static bool Display = true;
  static char Key;
  static byte State = 0;
  String ID;
  int Score;

  // ถ้า State เท่ากับ 0 ให้ทำงานในเงื่อนไขนี้
  if(State == 0){

    // ถ้าเซนเซอร์ Infrared ตรวจจับขยะได้ ให้ทำงานในเงื่อนไข
    if(digitalRead(INFRARED_SENSOR)==0){

      // ถ้าเวลาในการตรวจจับ มากกว่า 10 ms ให้ทำงานในเงื่อนไขนี้
      if((millis()-InfraredTime)>1000){
        InductiveOnTime = millis();   // เก็บค่าเวลาเริ่มต้น
        InductiveOffTime = millis();  // เก็บค่าเวลาเริ่มต้น
        State = 1;                    // เลื่อนลำดับการทำงาน ไปทำงานที่ State เท่ากับ 1
      }
    }else{  // ถ้าไม่ใช่
      InfraredTime = millis();  // เก็บค่าเวลาเริ่มต้นใหม่
    }
  
  // ถ้า State เท่ากับ 1 ให้ทำงานในเงื่อนไขนี้
  }else if(State == 1){

    // ถ้าเซนเซอร์ Iductive ตรวจจับโลหะ ได้ ให้ทำงานในเงื่อนไขนี้
    if(digitalRead(INDUCTIVE_SENSOR)==0){

      // ถ้าเวลาในการตรวจจับ มากกว่า 100 ms ให้ทำงานในเงื่อนไขนี้
      if((millis()-InductiveOnTime)>2000){
        State = 3;          // เลื่อนลำดับการทำงาน  ไปทำงานที่ State เท่ากับ 2
      }
      InductiveOffTime = millis(); // เก็บค่าเวลาเริ่มต้นของ Inductive ในขณะที่ไม่สามารถตรวจจับโลหะได้
    }else{

      // ถ้าเวลาในการตรวจจับ มากกว่า 100 ms ให้ทำงานในเงื่อนไขนี้
      if((millis()-InductiveOffTime)>2000){
        MyServo.write(POSITION_OPEN); // สั่งให้ Servo ปล่อยขยะลงในถัง
        Count++;            // นับจำนวนขยะ
        State = 2;          // เลื่อนลำดับการทำงาน ไปทำงานที่ State เท่ากับ 2
        ServoTime = millis(); // เก็บค่าเวลาเริ่มต้นของ Servo
        Display = true;
      }
      InductiveOnTime = millis(); // เก็บค่าเวลาเริ่มต้นของ Inductive ในขณะที่ตรวจจับโลหะได้
    }
  
  // ถ้า State เท่ากับ 2 ให้ทำงานในเงื่อนไขนี้
  }else if(State == 2){
    // ถ้าเวลาปัจจุบัน ลบ เวลาเริ่มต้น มีค่ามากกว่า 1000 ms ให้ทำงานในเงื่อนไขนี้
    if((millis()-ServoTime)>3000){
      State = 0;  // เลื่อนลำดับการทำงาน ไปทำงานที่ State เท่ากับ 0
      MyServo.write(POSITION_CLOSE);  // สั่งให้ Servo กลับไปยังตำแหน่งเริ่มต้น
    }
  }else if(State == 3){
    if(digitalRead(INDUCTIVE_SENSOR)==1){
      if((millis()-InductiveOffTime)>2000){
        State = 0;
      }
    }else{
      InductiveOffTime = millis();
    }
  }


  if(Display==true){
    Display = false;  
    lcd.setCursor(0,0);
    lcd.print("    Auto Bin    ");
    lcd.setCursor(0,1);
    lcd.print("Plastic: ");
    lcd.print(Count);
    if(Count<100) {lcd.write(' ');}
    if(Count<10)  {lcd.write(' ');}
    lcd.print("    ");
  }

  
  // อ่านค่าจาก Keypad
  Key = customKeypad.getKey();  

  if(Key!=NO_KEY){
    if(Key == '*'){       // กด * เพื่อเข้าตั้งค่า สำหรับ Admin
      if(Login()==1){
        SelectMenu();
      }
      Display = true; 
    }else if(Key == 'A'){ // กด A สำหรับส่ง Score
      lcd.setCursor(0,0);
      lcd.print("   Send Score   ");
      ID = GetID();
      if(SendScore(ID,Count) == true){
        Count = 0;  
        lcd.setCursor(0,1);
        lcd.print("    Complete    ");
      }else{
        lcd.setCursor(0,1);
        lcd.print("      Fail!     ");
      }
      delay(2000);
      Display = true;
    }else if(Key == 'B'){ // กด B สำหรับอ่าน Score
      lcd.setCursor(0,0);
      lcd.print("   Read Score   ");
      ID = GetID();
      if(GetCore(ID,&Score)==true){
        lcd.setCursor(0,1);
        lcd.print(" Score:");
        lcd.print(Score);
        if(Score<100) {lcd.write(' ');}
        if(Score<10)  {lcd.write(' ');}
        lcd.print("      ");
      }else{
        lcd.setCursor(0,1);
        lcd.print("     Fail!      ");  
      }
      
      while(customKeypad.getKey()==NO_KEY);
      Display = true;
    }

  }

}



/***********************************************************************************/
/* FUNCTION : GetCore (String ID)                                                  */
/***********************************************************************************/
String GetID (void)
{
  bool Exit = false;
  bool Display = true;
  char Key;
  byte Index = 0;
  char Buffer[9] = {'-','-','-','-','-','-','-','-','-'};
  unsigned long Timeout = millis();

  lcd.blink();

  while(Exit == false){
    
    if(Display == true){
      Display = false;
      lcd.setCursor(0,1);
      lcd.print("ID:");
      lcd.print(Buffer[0]);
      lcd.print(Buffer[1]);
      lcd.print(Buffer[2]);
      lcd.print(Buffer[3]);
      lcd.print(Buffer[4]);
      lcd.print(Buffer[5]);
      lcd.print(Buffer[6]);
      lcd.print(Buffer[7]);
      lcd.print(Buffer[8]);
      lcd.print("    ");
      lcd.setCursor(3+Index,1);
    }

    
    Key = customKeypad.getKey();  
    if(Key!=NO_KEY){
      switch(Key){
        case 'A':
          if(Index<8){
            Index++;
          }else{
            Index=0;
          }
          Display = true;
        break;
        

        case 'B':
          if(Index>0){
            Index--;
          }else{
            Index=8;
          }
          Display = true;
        break;
        
        case 'C':
          for(byte i=0; i<9; i++){
            Buffer[i] = '-';
          }
          Display = false;
        break;
        
        case 'D':
          if(Index>0){
            Index--;
          }else{
            Index=8;
          }
          Buffer[Index] = '-';
          Display = true;
        break;
        
        case '*':
          lcd.noBlink();
          return(String(Buffer[0]) + String(Buffer[1]) + String(Buffer[2]) + String(Buffer[3]) + String(Buffer[4]) + String(Buffer[5]) + String(Buffer[6]) + String(Buffer[7]) + String(Buffer[8]));
        break;
        
        case '#':
          lcd.noBlink();
          Exit = true; 
        break;

        default : 
          Buffer[Index] = Key;
          if(++Index==9){
            Index = 0;
          }
          Display = true;
        break;
      
      }
    }
  }  
}




/***********************************************************************************/
/* FUNCTION : bool GetCore (String ID,int *Score)                                  */
/***********************************************************************************/
bool GetCore (String ID,int *Score)
{
  bool Status;
  HTTPClient http;
  String url;

  url = "http://172.20.10.11/AutoBin/ReadScore.php?ID="+ID;
  Serial.print(url);
  Serial.println(ID);
 
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    *Score = parseScore(payload);
    Status = true;
  } else {
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
    Status = false;
  }

  http.end();

  return(Status);
}


/***********************************************************************************/
/* FUNCTION : bool SendScore (String ID,int Score)                                 */
/***********************************************************************************/
bool SendScore (String ID,int Score)
{
  bool Status;
  HTTPClient http;
  String url;
  url = "http://172.20.10.11/AutoBin/WriteScore.php?ID=" + ID + "&Score=" + String(Score) ;
  

  Serial.println(url);
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    Status = true;
  } else {
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
    Status = false;
  }

  http.end();

  return(Status);
}



/***********************************************************************************/
/* FUNCTION : int parseScore(String payload)                                       */
/***********************************************************************************/
int parseScore(String payload) {
  StaticJsonDocument<200> doc; 
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return 0;
  }

  int score = doc["Score"];

  return score;
}


/***********************************************************************************/
/* FUNCTION : bool Login ()                                                        */
/***********************************************************************************/
bool Login (void)
{
  bool Exit = false;
  bool Display = true;
  char Key;
  byte Index = 0;
  char Buffer[6] = {'-','-','-','-','-','-'};
  unsigned long Timeout = millis();

  lcd.blink();

  while(Exit == false){

    if(Display == true){
      Display = false;
      lcd.setCursor(0,0);
      lcd.print("  Admin Login   ");
      lcd.setCursor(0,1);
      lcd.print("    ");  
      lcd.print(Buffer[0]);
      lcd.print(Buffer[1]);
      lcd.print(Buffer[2]);
      lcd.print(Buffer[3]);
      lcd.print(Buffer[4]);
      lcd.print(Buffer[5]);
      lcd.print("    ");
      lcd.setCursor(4+Index,1);
    }

    Key = customKeypad.getKey();  
    if(Key!=NO_KEY){
      switch(Key){

        case 'A':
          if(Index<5){
            Index++;
          }else{
            Index=0;
          }
          Display = true;
        break;
        

        case 'B':
          if(Index>0){
            Index--;
          }else{
            Index=5;
          }
          Display = true;
        break;
        
        case 'C':
          for(byte i=0; i<6; i++){
            Buffer[i] = '-';
          }
          Display = false;
        break;
        
        case 'D':
          if(Index>0){
            Index--;
          }else{
            Index=5;
          }
          Buffer[Index] = '-';
          Display = true;
        break;
        
        case '*':
          lcd.noBlink();
          if((Pass[0]==Buffer[0])&&(Pass[1]==Buffer[1])&&(Pass[2]==Buffer[2])&&(Pass[3]==Buffer[3])&&(Pass[4]==Buffer[4])&&(Pass[5]==Buffer[5])){
            lcd.setCursor(0,0);
            lcd.print("    Password    ");
            lcd.setCursor(0,1);
            lcd.print("    Correct.    ");
            delay(1000);
            return(1);
          }else{
            lcd.setCursor(0,0);
            lcd.print("    Password    ");
            lcd.setCursor(0,1);
            lcd.print("   Incorrect!   ");
            delay(1000);
            return(0);
          }
        break;
        
        case '#':
          lcd.noBlink();
          Exit = true; 
        break;

        default : 
          Buffer[Index] = Key;
          if(++Index==6){
            Index = 0;
          }
          Display = true;
        break;
      }  

      Timeout = millis();
    }

    if((millis()-Timeout)>30000){
      Exit = true;  
    }

  }

  return(0);
}


/***********************************************************************************/
/* FUNCTION : SelectMenu()                                                         */
/***********************************************************************************/
void SelectMenu (void)
{
  bool Exit = false;
  bool Display = true;
  char Key;
  byte Menu = 0;
  const char MenuTxt[2][17] = {"1: Set Network  ",
                               "2: Charge Pass  "};

  while(Exit == false){
    
    if(Display == true){
      Display = false;
      lcd.setCursor(0,0);
      lcd.print(" Select Menu 1-2");
      lcd.setCursor(0,1);
      lcd.print(MenuTxt[Menu]);
    }

    Key = customKeypad.getKey();  
    if(Key!=NO_KEY){
      switch(Key){

        case 'A': break;
        case 'B': break;
        case 'C': break;
        case 'D': break;
        
        case '*':
          switch(Menu){
            case 0: SetNetwork();     break;
            case 1: ChangePassword(); break;  
          }
          Display = true;
        break;

        case '#':
          Exit = true;
        break;

        default : 
          if(Key>='1'&&Key<='2'){
            Menu = Key - '1';  
          }
          Display = true;
        break;  

      }
    }

  }

}



/***********************************************************************************/
/* FUNCTION : SetNetwork()                                                         */
/***********************************************************************************/
void SetNetwork (void)
{
  bool Exit = false;
  char Key;

  lcd.setCursor(0,0);
  lcd.print("WIFI: WIFIconfig");
  lcd.setCursor(0,1);
  lcd.print(" IP: 191.168.4.1");
  
  WiFi.mode(WIFI_AP);         
  WiFi.softAP("WIFIConfig");   
  server.on("/",handleRoot);
  server.on("/UserSave",handleUserSave);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");     

  while(Exit == false){
    server.handleClient(); 
    Key = customKeypad.getKey();  
    if(Key!=NO_KEY){
      if(Key == '#'){
        Exit = true;  
      }  
    }
  }

}


/***********************************************************************************/
/* FUNCTION : ChangePassword()                                                     */
/***********************************************************************************/
void ChangePassword (void)
{
  bool Exit = false;
  bool Display = true;
  char Key;
  byte Index = 0;
  char Buffer[6] = {'-','-','-','-','-','-'};
  unsigned long Timeout = millis();

  lcd.blink();

  while(Exit == false){

    if(Display == true){
      Display = false;
      lcd.setCursor(0,0);
      lcd.print(" Change Password");
      lcd.setCursor(0,1);
      lcd.print("    ");  
      lcd.print(Buffer[0]);
      lcd.print(Buffer[1]);
      lcd.print(Buffer[2]);
      lcd.print(Buffer[3]);
      lcd.print(Buffer[4]);
      lcd.print(Buffer[5]);
      lcd.print("    ");
      lcd.setCursor(4+Index,1);
    }

    Key = customKeypad.getKey();  
    if(Key!=NO_KEY){
      switch(Key){

        case 'A':
          if(Index<5){
            Index++;
          }else{
            Index=0;
          }
          Display = true;
        break;
        

        case 'B':
          if(Index>0){
            Index--;
          }else{
            Index=5;
          }
          Display = true;
        break;
        
        case 'C':
          for(byte i=0; i<6; i++){
            Buffer[i] = '-';
          }
          Display = false;
        break;
        
        case 'D':
          if(Index>0){
            Index--;
          }else{
            Index=5;
          }
          Buffer[Index] = '-';
          Display = true;
        break;
        
        case '*':
          for(byte i=0; i<6; i++){
            Pass[i] = Buffer[i];
            EEPROM.write(200+i,Pass[i]);
          }
          EEPROM.commit();

          lcd.noBlink();
          lcd.setCursor(0,1);
          lcd.print("    Complete    ");
          delay(1000);
          Exit = true;
        break;
        
        case '#':
          lcd.noBlink();
          Exit = true; 
        break;

        default : 
          Buffer[Index] = Key;
          if(++Index==6){
            Index = 0;
          }
          Display = true;
        break;
      }  

      Timeout = millis();
    }

    if((millis()-Timeout)>30000){
      Exit = true;  
    }

  }

}



/***********************************************************************************/
/* FUNCTION : handleRoot()                                                         */
/***********************************************************************************/
void handleRoot(){
  String header;    
  String content = "<!DOCTYPE html>\r\n";
  content += "<html lang=\"en\">";
  content += "<head>\r\n";
  content += "<meta charset=\"utf-8\">\r\n";
  content += "<title> User management  </title>\r\n";
  
  content += "<style>input[type=text], select {";
  content += "width: 30%;";  
  content += "padding: 12px 20px;";
  content += "margin: 8px 0;";
  content += "display: inline-block;";
  content += "border: 1px solid #ccc;";
  content += "border-radius: 4px;";
  content += "box-sizing: border-box;";
  content += "}";
  content += "input[type=password], select {";
  content += "width: 30%;";  
  content += "padding: 12px 20px;";
  content += "margin: 8px 0;";
  content += "display: inline-block;";
  content += "border: 1px solid #ccc;";
  content += "border-radius: 4px;";
  content += "box-sizing: border-box;";
  content += "}";
  content += "input[type=submit] {";
  content += "width: 20%;";
  content += "background-color: #4CAF50;";
  content += "color: white;";
  content += "padding: 14px 20px;";
  content += "margin: 8px 0;";
  content += "border: none;";
  content += "border-radius: 4px;";
  content += "cursor: pointer;";
  content += "}";
  content += "input[type=submit]:hover {";
  content += "background-color: #45a049;";
  content += "}";
  content += "div {";
  content += "width: 80%;";
  content += "border-radius: 5px;";
  content += "background-color: #f2f2f2;";
  content += "padding: 20px;";
  content += "}";
  content += "</style>";

  content += "<script>";
  content += "function startTime() {";
  content += "var today = new Date();";
  content += "var h = today.getHours();";
  content += "var m = today.getMinutes();";
  content += "var s = today.getSeconds();";
  content += "var dy = today.getDate();";
  content += "var mt = today.getMonth() + 1;";
  content += "var yr = today.getFullYear();";
  content += "m = checkTime(m);";
  content += "s = checkTime(s);";
  
  content += "document.getElementById('txt').innerHTML =";
  content += "dy + \"/\" + mt + \"/\" + yr + \" \" + h + \":\" + m + \":\" + s;";
      
  content += "var t = setTimeout(startTime, 500);";
  content += "}";
  content += "function checkTime(i) {";
  content += "if (i < 10) {i = \"0\" + i};";// add zero in front of numbers < 10
  content += "return i;";
  content += "}";
  content += "</script>";
  content += "</head>";
  
  content +="<body onload=\"startTime()\">\r\n";
  content +="<BR>\r\n";
  content +="<BR>\r\n";
  content +="<center>\r\n";
  content +="<div><form action=\"/UserSave\" method=\"POST\">\r\n";

  content +="<div align=\"right\" id=\"txt\"></div>";
  content +="<fieldset>";
  content +="<legend>Config Network</legend>";
  content +="Wifi Name: <input type=\"text\" name=\"WifiName\" placeholder=\'" + String(Ssid) + "'" + "maxlength=\"32\"><br>";
  content +="Wifi Passord: <input type=\"password\" name=\"WifiPass\" placeholder=\'" + String(Password) + "'" + "maxlength=\"32\"><br>";
  content +="</fieldset>";

  content +="<input type=\"submit\" value=\"Save\">\r\n";
  content +="</form></div>\r\n";
  content +="</center>\r\n";
  
  content += "</body></html>";
  server.send(200, "text/html", content);
}

/***********************************************************************************/
/* FUNCTION : handleUserSave()                                                     */
/***********************************************************************************/
void handleUserSave (void)
{
String str;  

  if(server.hasArg("WifiName")){
    if(server.arg("WifiName")!=NULL){
      str = server.arg("WifiName");  
      str.toCharArray(Ssid,str.length()+1);
      #ifdef _DEBUG
      Serial.print("Ssid=");
      Serial.println(Ssid);
      #endif      
      EEPROM.put(EEPROM_SSID_ADDR,Ssid);
      EEPROM.commit();
    }
  }

  if(server.hasArg("WifiPass")){
    if(server.arg("WifiPass")!=NULL){
      str = server.arg("WifiPass");  
      str.toCharArray(Password,str.length()+1);
      #ifdef _DEBUG
      Serial.print("Password=");
      Serial.println(Password);
      #endif
      EEPROM.put(EEPROM_PASS_ADDR,Password);
      EEPROM.commit();
    }
  }


  server.sendHeader("Location","/");        
  server.send(303); 
}




/***********************************************************************************/
/* FUNCTION : handleNotFound()                                                     */
/***********************************************************************************/
void handleNotFound(){ // เรียกฟังก์ชั่นนี้ ถ้า url หาไม่เจอ
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}







