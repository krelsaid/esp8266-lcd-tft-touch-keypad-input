/*
* Arduino ESP8266 Tft Lcd Touch Full Keypad Demo
* Using TFT_eSPI library <https://github.com/Bodmer/TFT_eSPI>
*
* By Khairil_Said 
* 20/12/18
*/


#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h> // https://github.com/Bodmer/TFT_eSPI

#define KEY_DEFAULT_CNT 113
#define KEY_PAD_X 30
#define KEY_PAD_Y 180
#define KEY_PAD_W 25
#define KEY_PAD_H 25
#define KEY_PAD_SPACE_X 8
#define KEY_PAD_SPACE_Y 5
#define OFFSET_UPPERLETTER 28
#define OFFSET_SYMBOL1 56 
#define OFFSET_SYMBOL2 72 
#define OFFSET_CAPS 102
#define OFFSET_SYMBOL 105 
#define OFFSET_DEL 104
#define OFFSET_SELECT 103 
#define OFFSET_SPACE 101
#define OFFSET_DOT 100
#define OFFSET_CURL 111
#define OFFSET_CURR 112
#define OFFSET_CLR 106
#define OFFSET_COPY 107
#define OFFSET_PASTE 108
#define OFFSET_SAVE 109
#define OFFSET_CONNECT 110
#define STR_LEN 128
#define LBL_KEY_CNT 21
#define KEY_TEXTSIZE 1

#define TFT_GREY 0x5AEB

char strBuffer[STR_LEN + 1] = "";
uint16_t numberIndex = 0;
char _temp_copy_buffer[STR_LEN+1]="";
char cursorBuffer[STR_LEN+1]="";
boolean cursormoved = false;
boolean markwhitespace=false;
boolean redrawwificfg=true;
boolean onsymbolkey = false;
boolean onsymbolkey1 = false;
boolean oncapskey =false;
boolean cfgssid = false;
boolean cfgpass = false;
boolean cfgssid2 = false;
boolean cfgpass2 = false;
boolean cfgwifidefault = false;
int16_t cursorIndex = 0,origpos = 0; 

char _tmp_str_buffer[STR_LEN+1] ="";
char _temp_ssid[32]="";
char _temp_pass[64]="";
char _temp_ssid2[32]="";
char _temp_pass2[64]="";
char _temp_ssid_default[2]="";
char keyLabel[LBL_KEY_CNT][8] PROGMEM = {"Alt", "Ctrl", "Num","Tab","Enter","<<",">>","Shift","Space","<-Del","Clear","Send","Save","Copy","Paste","Connect","Caps","Symbol","ABC","1/1","1/2"};
const char keypadrow_Number_P[] PROGMEM ={"1234567890"};
const char keypadrow_LowLetter_P[] PROGMEM ={"qwertyuiopasdfghjkl..zxcvbnm"};
const char keypadrow_UpperLetter_P[] PROGMEM ={"QWERTYUIOPASDFGHJKL..ZXCVBNM"};
const char keypadrow_Symbol_P[] PROGMEM ={"`~!@#$%^&*()_+-={}|;',./<>?[]:\\\""};
char fs_ssid[32];
char fs_password[64];
char fs_ssid2[32];
char fs_password2[64];
char fs_ssid_default[2];
int adj_expos = 375;

typedef enum{
  CFGWIFIPASSWORD,
  CFGWIFIPASSWORD2,
  CFGWIFISSID,
  CFGWIFISSID2,
  CFGWIFISEND,
  CFGWIFISAVE,
  CFGWIFICONNECT
}cfgtyp;

TFT_eSPI tft = TFT_eSPI();      
TFT_eSPI_Button keypadalt[KEY_DEFAULT_CNT];

#define CALIBRATION_FILENAME "/touchcaldata"

void setup(void) {

  uint16_t calData[5];
  uint8_t calOK = 0;

  Serial.begin(115200);
  Serial.println("start keypad demo..");

  tft.init();

  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  
  tft.setTextSize(1);

  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formating..");

    SPIFFS.format();
    SPIFFS.begin();
  }

  if (SPIFFS.exists(CALIBRATION_FILENAME)) {
    File f = SPIFFS.open(CALIBRATION_FILENAME, "r");
    if (f) {
      if (f.readBytes((char *)calData, 14) == 14)
        calOK = 1;
      f.close();
    }
  }
  if (calOK) {

    tft.setTouch(calData);
  } else {
    tft.calibrateTouch(calData, TFT_WHITE, TFT_RED, 15);
    File f = SPIFFS.open(CALIBRATION_FILENAME, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(1);
  tft.setTextPadding(0);
  tft.setTextDatum(TL_DATUM); // Reset datum to normal
  drawkeyboard_alt(0);
  
}


void loop() {
	
	uint16_t t_x = 0, t_y = 0; 
	boolean pressed = tft.getTouch(&t_x, &t_y);
	configwifi(t_x,t_y,pressed,redrawwificfg);
	
}

void enumswitch(int enumtype){

cfgssid = enumtype == CFGWIFISSID?true:false;
cfgpass = enumtype == CFGWIFIPASSWORD?true:false;
cfgssid2 = enumtype == CFGWIFISSID2?true:false;
cfgpass2 = enumtype == CFGWIFIPASSWORD2?true:false;
origpos = 0;
}

void clearwifibuffer(){
  
      if(cfgssid)
      clrbuffer(_temp_ssid,32);
      if(cfgpass)
      clrbuffer(_temp_pass,64);
      if(cfgpass2)
      clrbuffer(_temp_pass2,64);
      if(cfgssid2)
      clrbuffer(_temp_ssid2,32);
  
}

void copywifibuffer(){
     if(cfgpass)
        strcpy(_temp_pass,strBuffer);
     if(cfgssid)
        strcpy(_temp_ssid,strBuffer);
     if(cfgpass2)
        strcpy(_temp_pass2,strBuffer);
     if(cfgssid2)
        strcpy(_temp_ssid2,strBuffer);
        
        clrbffr();
       
}

void clrbffr(){
        for(int j=0;j<STR_LEN;j++)
        strBuffer[j] = '\0';
        numberIndex=0;
        cursorIndex=0;
}

void clrbuffer(char *buffers,int len){
       for(int j=0;j<len;j++)
        buffers[j] = '\0';
        numberIndex=0;
        cursorIndex=0;
}

char* keyStrA(const char * keypadpntr, uint8_t k) 
{
   static char keybuffer[2];
   keybuffer[0] = pgm_read_byte(&(keypadpntr[k]));  
   keybuffer[1] = 0; 
   return keybuffer;
}


void drawkeyboard_alt(uint8_t sw){

  int b = 0;
  int rows = 5;
  int column = 10;
  int adj_x =0;

  for(int i=10; i < KEY_DEFAULT_CNT;i++){
       keypadalt[i].initButton(0, 0, 0, 0, 0, TFT_WHITE, TFT_GREY, TFT_WHITE,"", 0);
  }

          if(sw==2 || sw==3 ){
           tft.fillRect(0 , 170, 480, 200, TFT_BLACK);
          }
            
    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < column; col++) {
         b = col + (row * column);
        //NUMPAD 0-9
          if(b < 10){
            
                  keypadalt[b].initButton(&tft, KEY_PAD_X + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyStrA(keypadrow_Number_P,b), KEY_TEXTSIZE);
                  keypadalt[b].drawButton();
          }
       
        if(row > 0){
  
          if(sw==0){
             if((row == 2 && col == 9)||(row == 3 && col == 0)||(row == 3 && col == 9)||(row == 4 && col == 0)||(row == 4 && col == 2)||(row == 4 && col == 8)){
          
            }else{
              
            if(b > 9 && b < 38){ //26 lowerlatter
               
               if(row == 2 || row == 3){
                 adj_x = KEY_PAD_SPACE_X+KEY_PAD_W/2;
               }

                  keypadalt[b].initButton(&tft, KEY_PAD_X + adj_x+ col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyStrA(keypadrow_LowLetter_P,b-10), KEY_TEXTSIZE);
                  keypadalt[b].drawButton();
            }
          }
        }

        if(sw==1){
           if((row == 2 && col == 9)||(row == 3 && col == 0)||(row == 3 && col == 9)||(row == 4 && col == 0)||(row == 4 && col == 2)||(row == 4 && col == 8)){
                
           }else{
           
            if(b > 9 && b < 38){ //26 upperlatter
               if(row == 2 || row == 3){
                 adj_x = KEY_PAD_SPACE_X+KEY_PAD_W/2;
               }
                  keypadalt[b+OFFSET_UPPERLETTER].initButton(&tft, KEY_PAD_X + adj_x + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyStrA(keypadrow_UpperLetter_P,b-10), KEY_TEXTSIZE);
                  keypadalt[b+OFFSET_UPPERLETTER].drawButton();
                }
            }
          }

        if(sw==2){
  
           if((row == 3 && col == 0)||(row == 3 && col == 9)||(row == 4 && col == 0)||(row == 4 && col == 2)||(row == 4 && col == 8)){
           }else{   
               if(row == 2 || row == 3){
                 adj_x = KEY_PAD_SPACE_X+KEY_PAD_W/2;
               }   

            if(b > 9 && b < 26){ //16 symbol1

                  keypadalt[b+OFFSET_SYMBOL1].initButton(&tft, (KEY_PAD_X + adj_x)+ col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyStrA(keypadrow_Symbol_P,b-10), KEY_TEXTSIZE);
                  keypadalt[b+OFFSET_SYMBOL1].drawButton();
              }
            }
          }

        if(sw==3){
           if((row == 3 && col == 0)||(row == 3 && col == 9)||(row == 4 && col == 0)||(row == 4 && col == 2)||(row == 4 && col == 8)){
            }else{      
               if(row == 2 || row == 3){
                 adj_x = KEY_PAD_SPACE_X+KEY_PAD_W/2;
               }
            if(b > 9 && b < 26){ //16 symbol2
                  keypadalt[b+OFFSET_SYMBOL2].initButton(&tft,( KEY_PAD_X + adj_x)+ col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyStrA(keypadrow_Symbol_P,b+6), KEY_TEXTSIZE);
                  keypadalt[b+OFFSET_SYMBOL2].drawButton();
              }
            }
          }

       if(b > 29 && b < 40){ 
         
          if(row == 3){

           if(sw<2){
            if(col==0){ //caps
                  keypadalt[OFFSET_CAPS].initButton(&tft, (KEY_PAD_X + adj_x-10)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[16], KEY_TEXTSIZE);
                  keypadalt[OFFSET_CAPS].drawButton();
            }
           }else{
            if(col==0){ //symbol
            char sy[]="";
            strcpy(sy,(sw==3)?keyLabel[20]:keyLabel[19]);
            keypadalt[OFFSET_SYMBOL].initButton(&tft, (KEY_PAD_X + adj_x-10)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  sy, KEY_TEXTSIZE);
                  keypadalt[OFFSET_SYMBOL].drawButton();
            }
           }
           if(col==9){ //del
                  keypadalt[OFFSET_DEL].initButton(&tft, (KEY_PAD_X + adj_x-25)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+15, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[9], KEY_TEXTSIZE);
                  keypadalt[OFFSET_DEL].drawButton();
            }
            
          }
       }

       if(b > 39 && b < 50){ 
         
        if(row == 4){
            char sy[]="";
            strcpy(sy,(sw>1)?keyLabel[18]:keyLabel[17]);
            if(col==0){ //symbol
                  keypadalt[OFFSET_SELECT].initButton(&tft, (KEY_PAD_X + adj_x-10)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  sy, KEY_TEXTSIZE);
                  keypadalt[OFFSET_SELECT].drawButton();
            }
          
          if(col==2){ //space
                  keypadalt[OFFSET_SPACE].initButton(&tft, (KEY_PAD_X + adj_x+65)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+190, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[8], KEY_TEXTSIZE);
                  keypadalt[OFFSET_SPACE].drawButton();
            }

          if(col==8){ //dot
                  keypadalt[OFFSET_DOT].initButton(&tft, (KEY_PAD_X + adj_x+8)+col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+15, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  ".", KEY_TEXTSIZE);
                  keypadalt[OFFSET_DOT].drawButton();
            }
            
          }
     
        }
    
      }//row > 0
     
      }
    }


    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 2; col++) {
          if(row == 0 ){
            if(col == 0 ){
                  keypadalt[OFFSET_CURL].initButton(&tft, (KEY_PAD_X+adj_expos-30) + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[5], KEY_TEXTSIZE);
                  keypadalt[OFFSET_CURL].drawButton();
            }
          if(col == 1 ){
                  keypadalt[OFFSET_CURR].initButton(&tft, (KEY_PAD_X+adj_expos) + col * (KEY_PAD_W - 5 + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[6], KEY_TEXTSIZE);
                  keypadalt[OFFSET_CURR].drawButton();
            }
          }

          if(row == 1 ){
            if(col == 0 ){
                  keypadalt[OFFSET_CLR].initButton(&tft, (KEY_PAD_X+adj_expos) + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+80, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[10], KEY_TEXTSIZE);
                  keypadalt[OFFSET_CLR].drawButton();
            }
          }

          if(row == 2 ){
            if(col == 0 ){
                  keypadalt[OFFSET_COPY].initButton(&tft, (KEY_PAD_X+adj_expos-30) + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[13], KEY_TEXTSIZE);
                  keypadalt[OFFSET_COPY].drawButton();
            }
            if(col == 1 ){
                  keypadalt[OFFSET_PASTE].initButton(&tft, (KEY_PAD_X+adj_expos) + col * (KEY_PAD_W - 5 + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+20, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[14], KEY_TEXTSIZE);
                  keypadalt[OFFSET_PASTE].drawButton();
            }
          }

          if(row == 3 ){
            if(col == 0 ){
                  keypadalt[OFFSET_SAVE].initButton(&tft, (KEY_PAD_X+adj_expos) + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+80, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[12], KEY_TEXTSIZE);
                  keypadalt[OFFSET_SAVE].drawButton();
            }
          }

          if(row == 4 ){
            if(col == 0 ){
                  keypadalt[OFFSET_CONNECT].initButton(&tft, (KEY_PAD_X+adj_expos) + col * (KEY_PAD_W + KEY_PAD_SPACE_X), KEY_PAD_Y + row * (KEY_PAD_H + KEY_PAD_SPACE_Y) , 
                                KEY_PAD_W+80, KEY_PAD_H, TFT_WHITE, TFT_GREY, TFT_WHITE,
                  keyLabel[15], KEY_TEXTSIZE);
                  keypadalt[OFFSET_CONNECT].drawButton();
            }
          }
        
      }
    }
   
}

//Input Text Box Top Left
void makeinput_TL(uint16_t _x,uint16_t _y,uint16_t _w,uint16_t _h,String _fs_str,String lbl){
    int _str_len = _w/6;
    tft.setTextFont(1);
    tft.setCursor(_x, _y-10);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(lbl);
    tft.drawRect(_x, _y, _w,_h, TFT_GREY);
    tft.fillRect(_x+2, _y+2,_w-3,_h-3, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(_fs_str.substring(0,_str_len-1), _x + 6, _y + 7); 
}

//Input Text Box Bottom Left
void makeinput_BL(uint16_t _x,uint16_t _y,uint16_t _w,uint16_t _h,String _fs_str,String lbl){
    int _lbl_pass_y = (_y+_h)+5;
    int _pass_y = _lbl_pass_y+10;

    int _str_len = _w/6;
    tft.setTextFont(1);
    tft.setCursor(_x,_lbl_pass_y );
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(lbl);
    tft.drawRect(_x,_pass_y , _w,_h, TFT_GREY);
    tft.fillRect(_x+2,_pass_y+2,_w-3,_h-3, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(_fs_str.substring(0,_str_len-1), _x + 6, _pass_y + 7); 
}

//Input Text Box Top Right
void makeinput_TR(uint16_t _x,uint16_t _y,uint16_t _w,uint16_t _h,String _fs_str,String lbl){
  
    int _right_x = _x+_w+20;
    int _str_len = _w/6;
    tft.setTextFont(1);
    tft.setCursor(_right_x,_y-10 );
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(lbl);
    tft.drawRect(_right_x,_y , _w,_h, TFT_GREY);
    tft.fillRect(_right_x+2,_y+2,_w-3,_h-3, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(_fs_str.substring(0,_str_len-1), _right_x + 6, _y + 7); 
}

//Input Text Box Bottom Right
void makeinput_BR(uint16_t _x,uint16_t _y,uint16_t _w,uint16_t _h,String _fs_str,String lbl){
  
    int _lbl_pass_y = (_y+_h)+5;
    int _pass_y = _lbl_pass_y+10;
    int _right_x = _x+_w+20;
    int _str_len = _w/6;
    tft.setTextFont(1);
    tft.setCursor(_right_x,_lbl_pass_y );
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(lbl);
    tft.drawRect(_right_x,_pass_y , _w,_h, TFT_GREY);
    tft.fillRect(_right_x+2,_pass_y+2,_w-3,_h-3, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(_fs_str.substring(0,_str_len-1), _right_x + 6, _pass_y + 7); 
}


void configwifi(uint16_t t_x,uint16_t t_y,boolean pressed, boolean &draw){
  
    int _x = 10;
    int _y = 60;
    int _w = 200;
    int _h = 20;
    int _lbl_pass_y = (_y+_h)+5;
    int _pass_y = _lbl_pass_y+10;
    int _right_x = _x+_w+20;
    uint8_t _fn = 0;
    
    tft.setTextSize(1);
    tft.setTextFont(1);
    tft.setTextPadding(0);
  
  if(draw){
    draw = false;
    
    tft.setCursor(_x, _y-25);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println(F("Setting Wifi"));

    //ssid1
    makeinput_TL(_x,_y,_w,_h,fs_ssid,"SSID");
    strcpy(_temp_ssid,fs_ssid);

    //pass1
    makeinput_BL(_x,_y,_w,_h,fs_password,"PASSWORD");
    strcpy(_temp_pass,fs_password);

    //ssid2
    makeinput_TR(_x,_y,_w,_h,fs_ssid2,"SSID 2");
    strcpy(_temp_ssid2,fs_ssid2);

    //pass2
    makeinput_BR(_x,_y,_w,_h,fs_password2,"PASSWORD");
    strcpy(_temp_pass2,fs_password2);

    //ssid1 default checkbox
    tft.setCursor(_x+22,_pass_y+28 );
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(F("Set Default SSID 1"));
    tft.drawRect(_x,_pass_y+25 , 12,12, TFT_GREY);
    tft.fillRect(_x+1,_pass_y+25+1,11,11, TFT_WHITE);  
    tft.fillRect(_x+2,_pass_y+25+2,8,8, (atoi(fs_ssid_default) == 2 || strlen(fs_ssid_default)==0)?TFT_BLACK:TFT_WHITE);
    
    //ssid2 default checkbox
    tft.setCursor(_right_x+22,_pass_y+28 );
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(F("Set Default SSID 2"));
    tft.drawRect(_right_x,_pass_y+25 , 12,12, TFT_GREY);
    tft.fillRect(_right_x+1,_pass_y+25+1,11,11, TFT_WHITE);  
    tft.fillRect(_right_x+2,_pass_y+25+2,8,8, (atoi(fs_ssid_default) == 1)?TFT_BLACK:TFT_WHITE);
  }

//ssid1
  if(xytouch(t_x,t_y, _x,_y,_w,_h)){
    if(!cfgssid){
    tft.drawRect(_x, _y, _w,_h, TFT_YELLOW);
     tft.drawRect(_x,_pass_y , _w,_h, TFT_GREY);

     if(numberIndex != 0){
      
       copywifibuffer();
        
     }else{
		 
        clearwifibuffer();
		
     }

      if(strlen(_temp_ssid) > 0){
      strcpy(strBuffer,_temp_ssid);
      numberIndex = strlen(_temp_ssid);
      
     }

      enumswitch(CFGWIFISSID);
    }
  }

//pass1
  if(xytouch(t_x,t_y, _x,_pass_y,_w,_h)){
      if(!cfgpass){
     tft.drawRect(_x,_pass_y , _w,_h, TFT_YELLOW);
     tft.drawRect(_x, _y, _w,_h, TFT_GREY);
     
     if(numberIndex != 0){
      
        copywifibuffer();
     }else{
      
       clearwifibuffer();
     }

      if(strlen(_temp_pass) > 0){

      strcpy(strBuffer,_temp_pass);
      numberIndex = strlen(_temp_pass);
     }

    enumswitch(CFGWIFIPASSWORD);
      }
  }

//ssid2
  if(xytouch(t_x,t_y, _right_x,_y,_w,_h)){
    if(!cfgssid2){
         tft.drawRect(_x,_pass_y , _w,_h, TFT_GREY);
         tft.drawRect(_x, _y, _w,_h, TFT_GREY);
         tft.drawRect(_right_x, _y, _w,_h, TFT_YELLOW);
         tft.drawRect(_right_x,_pass_y , _w,_h, TFT_GREY);
    
         if(numberIndex != 0){
          
            copywifibuffer();
            
         }else{

           clearwifibuffer();
         }
    
          if(strlen(_temp_ssid2) > 0){
          strcpy(strBuffer,_temp_ssid2);
          numberIndex = strlen(_temp_ssid2);
          
         }
          enumswitch(CFGWIFISSID2);
      
    }
  }


 //pass2
  if(xytouch(t_x,t_y, _right_x,_pass_y,_w,_h)){
      if(!cfgpass2){
         tft.drawRect(_x,_pass_y , _w,_h, TFT_GREY);
         tft.drawRect(_x, _y, _w,_h, TFT_GREY);
         tft.drawRect(_right_x, _y, _w,_h, TFT_GREY);
         tft.drawRect(_right_x,_pass_y , _w,_h, TFT_YELLOW);
         
         if(numberIndex != 0){
          
            copywifibuffer();
            
         }else{
          
           clearwifibuffer();
         }
    
          if(strlen(_temp_pass2) > 0){
    
          strcpy(strBuffer,_temp_pass2);
          numberIndex = strlen(_temp_pass2);
         }

          enumswitch(CFGWIFIPASSWORD2);
      }
  }

//checkbox
//ssid1
  if(xytouch(t_x,t_y,_x,_pass_y+25,_w,_h)){
    if(cfgwifidefault){
      
    cfgwifidefault = false;
    
           tft.fillRect(_x+1,_pass_y+25+1,11,11, TFT_WHITE);
           tft.fillRect(_x+2,_pass_y+25+2,8,8, TFT_BLACK);
          //toggle
           tft.fillRect(_right_x+1,_pass_y+25+1,11,11, TFT_WHITE);
           tft.fillRect(_right_x+2,_pass_y+25+2,8,8, TFT_WHITE);
    }
  }
//ssid2
  if(xytouch(t_x,t_y, _right_x,_pass_y+25,_w,_h)){
    if(!cfgwifidefault){
      
       cfgwifidefault = true;
    
           tft.fillRect(_right_x+1,_pass_y+25+1,11,11, TFT_WHITE);
           tft.fillRect(_right_x+2,_pass_y+25+2,8,8, TFT_BLACK);
            //toggle
           tft.fillRect(_x+1,_pass_y+25+1,11,11, TFT_WHITE);
           tft.fillRect(_x+2,_pass_y+25+2,8,8, TFT_WHITE);
      
    }
  }
  
  if(cfgssid)
  _fn = onkeyboardtouch(t_x,t_y,pressed,_x+2,_y+3,_w-3,_h-3,TFT_BLACK,TFT_WHITE,32);

  if(cfgpass)
  _fn = onkeyboardtouch(t_x,t_y,pressed,_x+2,_pass_y+3,_w-3,_h-3,TFT_BLACK,TFT_WHITE,62);

  if(cfgssid2)
  _fn = onkeyboardtouch(t_x,t_y,pressed,_right_x+2,_y+3,_w-3,_h-3,TFT_BLACK,TFT_WHITE,32);

  if(cfgpass2)
  _fn = onkeyboardtouch(t_x,t_y,pressed,_right_x+2,_pass_y+3,_w-3,_h-3,TFT_BLACK,TFT_WHITE,62);
  
  switch(_fn){
    case CFGWIFISEND: //send
	
		    break;
    case CFGWIFISAVE: //save
	
        break;  
	case CFGWIFICONNECT: //connect
	
        break;
  }
}//wifi config


uint8_t onkeyboardtouch(uint16_t t_x,uint16_t t_y,boolean pressed,int disp_x,int disp_y,int disp_w,int disp_h,unsigned int strcolor,unsigned int backcolor,int len){
  
  uint8_t _fn = 0;  
  if(origpos == 0){
  origpos = strlen(strBuffer);
  }

  if(cursorIndex==0)
  cursormoved = false;

  if(!cursormoved)
  cursorIndex=0;


    // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < KEY_DEFAULT_CNT ; b++) {
    if (pressed && keypadalt[b].contains(t_x, t_y)) {
      keypadalt[b].press(true);  // tell the button it is pressed
    } else {
      keypadalt[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < KEY_DEFAULT_CNT; b++) {

    if (keypadalt[b].justReleased()) keypadalt[b].drawButton(); // draw normal

    if (keypadalt[b].justPressed()) {
      keypadalt[b].drawButton(true);  // draw invert

      if (b < 101 && numberIndex < len) {
      
        if (numberIndex < STR_LEN) {
          if(cursormoved){
            
            if(numberIndex == origpos){
              cursormoved = false;
              cursorIndex = 0;
            }

            if(b<10){
                strBuffer[numberIndex] = *keyStrA(keypadrow_Number_P,b);
            }else{

              if(oncapskey && !onsymbolkey && b > (OFFSET_UPPERLETTER+9)){
                strBuffer[numberIndex] = *keyStrA(keypadrow_UpperLetter_P,b-(OFFSET_UPPERLETTER+10));  
              }

              if(!oncapskey && !onsymbolkey && b < (OFFSET_UPPERLETTER+10)){
                strBuffer[numberIndex] = *keyStrA(keypadrow_LowLetter_P,b-10);  
              }

              if(onsymbolkey && b > (OFFSET_SYMBOL1+9) && b < (OFFSET_SYMBOL2+26)){
                strBuffer[numberIndex] = *keyStrA(keypadrow_Symbol_P,b-(OFFSET_SYMBOL1+10));  
              }

              if (b == OFFSET_DOT) {  
                strBuffer[numberIndex] = *keyStrA(keypadrow_Symbol_P,22);  
              }          
           }

            numberIndex++;

            if(cursorIndex > 0)
            cursorIndex--;
            
          }else{

            if(b<10){
                strBuffer[numberIndex] = *keyStrA(keypadrow_Number_P,b);
            }else{

              if(oncapskey && !onsymbolkey && b > (OFFSET_UPPERLETTER+9)){ 
                strBuffer[numberIndex] = *keyStrA(keypadrow_UpperLetter_P,b-(OFFSET_UPPERLETTER+10));  
              }

              if(!oncapskey && !onsymbolkey && b < (OFFSET_UPPERLETTER+10)){ 
                strBuffer[numberIndex] = *keyStrA(keypadrow_LowLetter_P,b-10);  
              }

              if(onsymbolkey && b > (OFFSET_SYMBOL1+9) && b < (OFFSET_SYMBOL2+26)){
                strBuffer[numberIndex] = *keyStrA(keypadrow_Symbol_P,b-(OFFSET_SYMBOL1+10)); 
              }

              if (b == OFFSET_DOT) {  
                strBuffer[numberIndex] = *keyStrA(keypadrow_Symbol_P,22);  
              } 
           }
            numberIndex++;
            strBuffer[numberIndex] = 0; 
            origpos = numberIndex;
          }
        }
      }

      // Del button
      if (b == OFFSET_DEL) {
   
        if (numberIndex > 0) {
          
          if(cursormoved){
   
           memmove(strBuffer+(numberIndex-1),strBuffer+(numberIndex),strlen(strBuffer)-numberIndex);
           strBuffer[strlen(strBuffer)-1] = 0;
           if (numberIndex >0)
           numberIndex--;
            
            origpos = strlen(strBuffer);
          }else{
          
          strBuffer[numberIndex] = 0;
          if (numberIndex >0)
           numberIndex--;
             
          strBuffer[numberIndex] = 0;
          origpos = numberIndex;
          cursorIndex = 0;
          }
        }

      }

      // Clr button
      if (b == OFFSET_CLR) {     
        for(int j=0;j<len;j++)
        strBuffer[j] = '\0';
        numberIndex = 0;
        cursorIndex = 0;
        origpos = 0;
      }

      // Space
      if (b == OFFSET_SPACE) {
     
        if (numberIndex < len) {
          
          if(cursormoved){
            markwhitespace = true;
            memmove(strBuffer+(numberIndex+1),strBuffer+numberIndex,strlen(strBuffer)-numberIndex);
            origpos = strlen(strBuffer);
          }
          strBuffer[numberIndex] = '\20';
          numberIndex++;
          
        }

      }

      // Send
      //if (b == 1000) { 
      //  _fn = CFGWIFISEND;
      //}

      // copy
      if (b == OFFSET_COPY) {
        strcpy(_temp_copy_buffer,strBuffer);
      }

      // paste
      if (b == OFFSET_PASTE) {

        numberIndex = strlen(_temp_copy_buffer);
        strcpy(strBuffer,_temp_copy_buffer);
        for(int j=0;j<len;j++)
        _temp_copy_buffer[j] = '\0';
        origpos = numberIndex;
      }

       // Save
      if (b == OFFSET_SAVE) {
        _fn = CFGWIFISAVE;
      }

      // connect
      if (b == OFFSET_CONNECT) {
        _fn = CFGWIFICONNECT;
      }

      // <- cursor
      if (b == OFFSET_CURL) {

        if (numberIndex >= 0 && numberIndex <= len ) {
        cursormoved = true;

        if(numberIndex > 0){
          cursorIndex++;
          numberIndex = numberIndex - 1;
		  
          }
        }
      }

      // -> cursor
      if (b == OFFSET_CURR) {

        cursormoved = true;

        if (numberIndex < origpos && numberIndex >= 0 ) {
            
          if(cursorIndex >= 0){
            cursorIndex--;
          }else{
            cursorIndex += -1;
          }
          
          numberIndex = numberIndex + 1;

        }
      }

      
      if (b == OFFSET_SELECT) {
        if(!onsymbolkey){
          onsymbolkey = true;
          drawkeyboard_alt(2);
        }else{
          onsymbolkey = false;
          oncapskey = false;
          onsymbolkey1 = false;
          drawkeyboard_alt(0);
        }
        return _fn; //ctrl fn
      }

      if (b == OFFSET_SYMBOL && onsymbolkey ) {
        if(!onsymbolkey1){
          onsymbolkey1 = true;
          drawkeyboard_alt(3);
        }else{
          onsymbolkey1 = false;
          drawkeyboard_alt(2);
        }
        return _fn; //ctrl fn
      }

      if (b == OFFSET_CAPS) {
        if(!oncapskey){
          oncapskey = true;
          drawkeyboard_alt(1);
        }else{
          oncapskey = false;
          drawkeyboard_alt(0);
        }
        return _fn; //ctrl fn
      }

      
      tft.setTextColor(strcolor, backcolor);

      if(markwhitespace){
        markwhitespace = false;
        tft.fillRect(disp_x + 4 , disp_y + 1, disp_w - 5, disp_h - 4, backcolor);//clear all
      }

      for(int i=0;i<sizeof(_tmp_str_buffer) ;i++)
      _tmp_str_buffer[i]='\0';
      
      int lblsz = disp_w/6;
      int jsz = disp_w/6;
      int jpos = 0;

      if(strlen(strBuffer) > jsz){
          jpos = strlen(strBuffer) - jsz;
          jsz += jpos;

      }else{
        jsz = strlen(strBuffer);
      }

      if(cursorIndex >= lblsz || strlen(strBuffer) >= jsz){
        markwhitespace = true;
      }
      
       if(cursorIndex > lblsz && numberIndex >= 0){
          jpos = numberIndex;
          jsz = cursorIndex - (cursorIndex-lblsz);
       }

      memcpy(_tmp_str_buffer,strBuffer+jpos,jsz);
      int xwidth = tft.drawString(_tmp_str_buffer, disp_x + 4, disp_y + 4);
      tft.fillRect(disp_x + 4 + xwidth, disp_y + 1, disp_w - xwidth - 5, disp_h - 4, backcolor);
      tft.fillRect(disp_x, disp_y + 14, disp_w, 2, backcolor);
      if(numberIndex < lblsz && cursorIndex < lblsz ){
      tft.drawFastHLine(disp_x + 4 + xwidth - (cursorIndex*6), disp_y + 14, 5, TFT_BLUE);
      }else{
        
        if(cursorIndex>=lblsz)
        tft.drawFastHLine(disp_x + 4 + xwidth - (lblsz*6), disp_y + 14, 5, TFT_BLUE);

        if(numberIndex >= lblsz && cursorIndex > 0 )
        tft.drawFastHLine(disp_x + 4 + xwidth - (cursorIndex*6), disp_y + 14, 5, TFT_BLUE);

        if(cursorIndex == 0 )
        tft.drawFastHLine(disp_x + 4 + xwidth - (1*6), disp_y + 14, 5, TFT_BLUE);

      }
       delay(10); // UI debouncing
    }//just pressed
    
  }

    return _fn; //ctrl fn
}

boolean xytouch(int tx,int ty, int x,int y,int w,int h){
        if ((tx > x) && (tx < (x + w))) {
        if ((ty > y) && (ty <= (y + h))) {
          return true;
        }
      }
      return false;
}
