/*
  raduino
  Touchscreen FM radio: Arduino Mega 2560 + TEA5767N + Adafruit 2.8" TFT Touch Shield
  (capacitive version: ILI9341 display + FT6206 touch controller)

  Wiring: see docs/wiring.png
  Libraries: Adafruit GFX, Adafruit ILI9341, Adafruit FT6206, TEA5767N (see README.md)
*/

#include <Wire.h>
#include <TEA5767N.h>

//Constants:
TEA5767N Radio = TEA5767N(); // I2C: SDA = pin 20, SCL = pin 21 on the Arduino Mega 2560

#define MUTEPIN             // D2 pin
#define STATIONPIN          // D3 pin

//Variables:
int flag=0;
double stations[] = {88.3, 92.3, 93.9, 97.1, 99.5, 105.9, 107.5}; // Your station presets
int currentstation = 0;
int maxstation = sizeof(stations) / sizeof(stations[0]) - 1; // Index of the last preset (zero indexed)
bool mutestate = 0;

#define adafruit320

#ifdef adafruit320
 //(screen 320x240)
 //#include <Adafruit_TFTLCD.h>
 #include <Adafruit_GFX.h>    // Core graphics library
 #include <SPI.h>       // this is needed for display
 #include <Adafruit_ILI9341.h>
 #include <Wire.h>      // this is needed for FT6206
 #include <Adafruit_FT6206.h>
 #define TFT_CS 10
 #define TFT_DC 9
 Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
 // This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

 #define MINPRESSURE 10
 #define MAXPRESSURE 1000
 #define FRAME_W 320
 #define FRAME_H 240

#endif 

Adafruit_FT6206 tp = Adafruit_FT6206();  

int touchx,touchy,touched;

uint16_t BLACK,BLUE,RED,GREEN,CYAN,MAGENTA,YELLOW,WHITE,GRAY,DARK,GRAY2,BLACKF,WHITEF;

uint16_t getColor(uint8_t red, uint8_t green, uint8_t blue)
{
  red   >>= 3;
  green >>= 2;
  blue  >>= 3;
  return (red << 11) | (green << 5) | blue;
}

void initplayer() {
  tft.fillScreen(BLACK);        
  tft.setTextSize(2);    
  tft.setCursor(78,20); tft.setTextColor(WHITE);      
  tft.print(F("Raduino"));
  tft.setCursor(50,50);
  tft.setTextColor(GREEN); 
  tft.print(F("Now Playing:"));
}

void definecolors() {
 BLACK  =ILI9341_BLACK;
 BLUE   =ILI9341_BLUE;  //0x001F;
 RED    =ILI9341_RED;
 GREEN  =ILI9341_GREEN;
 CYAN   =ILI9341_CYAN;
 MAGENTA=ILI9341_MAGENTA;
 YELLOW =ILI9341_YELLOW;
 WHITE  =ILI9341_WHITE;
 GRAY   =0x7BEF;
 DARK   =getColor(32,32,32); 
 GRAY2   =getColor(16,16,16); 
 BLACKF  =getColor(94,58,0); 
 WHITEF  =getColor(180,114,0); 
}
class Button {
public:
int bx,by,bw,bh,bshift;
String bs;
const uint8_t* bp;
 Button(int x,int y,int w,int h,int x1,int y1,int w1,int h1,String s,const uint8_t* p, int shift);
 void Show(int shift); 
 void Show(String s); 
 void Hide();
 boolean IsPressed();
private:
boolean active;
 void ishow(uint16_t textcolor,uint16_t backcolor);
};
Button::Button(int x,int y,int w,int h,int x1,int y1,int w1,int h1,String s,const uint8_t* p=NULL, int shift=0) {
  if (FRAME_H==320) {
   bx=x; by=y; bw=w; bh=h; 
  } else {
   bx=x1; by=y1; bw=w1; bh=h1;  
  }
  bs=s; bp=p; bshift=shift;
};
void Button::Show(int shift=0) {
 bshift=shift; 
 active=true;
 ishow(WHITE,DARK);  
};
void Button::Show(String s) {
 bs=s; 
 active=true;
 ishow(WHITE,DARK);  
};
void Button::Hide() { 
 active=false; 
 ishow(GRAY,DARK);  
};
boolean Button::IsPressed() {  
  if (active&&abs(bx+bw/2-touchx)<bw/2&&abs(by+bh/2-touchy)<bh/2) {
    ishow(WHITE,GRAY); 
    touchx=0; touchy=0;        
    return true;
  } 
  return false;
};  
void Button::ishow(uint16_t textcolor,uint16_t backcolor) {  
  tft.fillRoundRect(bx+2,by+2,bw,bh,3,GRAY2);               
  tft.fillRoundRect(bx,by,bw,bh,3,backcolor);               
  tft.setTextColor(textcolor);
  if (FRAME_H==320) {
   tft.setTextSize(2);    
   tft.setCursor(bx+(bw-bs.length()*12)/2,by+(bh-12)/2);       
  }  else {
   tft.setTextSize(1);    
   tft.setCursor(bx+(bw-bs.length()*6)/2,by+(bh-8)/2);           
  }
  if (bp!=NULL) { 
   tft.drawBitmap(1+bx+(bw-16)/2, by+(bh-16)/2, bp+bshift*32, 16, 16,textcolor);          
  } 
  else  tft.print(bs);    
}

Button BPrev(354, 20, 100, 30, 10, 240, 60, 60, "<<");
Button BNext(354, 58, 100, 30, 170, 240, 60, 60, ">>");
Button BPlay(354, 96, 100, 30, 85, 240, 70, 60, "PLAY");
Button BPause(354, 134, 100, 30, 85, 240, 70, 60, "PAUSE");

void gui()
{
  if(gettouch()){
    if(BPlay.IsPressed())
    {
      BPlay.Hide();
      BPause.Show("PAUSE");
      Serial.println("Unmuting radio");
      Radio.turnTheSoundBackOn();
    }
    else if(BPause.IsPressed())
    {
      BPause.Hide();
      BPlay.Show("PLAY");
      Serial.println("Muting radio");
      Radio.mute();
    }
    else if(BNext.IsPressed())
    {
      Serial.println("Tuning to next station");
      currentstation++;
      if (currentstation>maxstation) {
        currentstation = 0;
      }
      Radio.selectFrequency(stations[currentstation]);
      initplayer();
      tft.fillScreen(BLACK);
      tft.setCursor(78,20); tft.setTextColor(WHITE);      
      tft.print(F("Raduino"));
      tft.setCursor(50,50);
      tft.setTextColor(GREEN); 
      tft.print(F("Now Playing:"));
      tft.setTextSize(2);
      tft.setCursor(90,90);
      tft.setTextColor(WHITE);
      tft.print(stations[currentstation]); 
      flag = 0; // Flag to fetch stats and display over serial
      delay(250); // Added this delay to ensure tuning stats read from the module are correct - module has time to tune properly  

      if (flag == 0) {
        Serial.print("Current freq: ");
        Serial.print(Radio.readFrequencyInMHz());
        Serial.print("MHz Signal: ");
        Serial.print(Radio.isStereo() ? "STEREO" : "MONO");
        Serial.print(" ");
        Serial.print(Radio.getSignalLevel());
        Serial.println("/15");
        flag=1;
      }
      BNext.Hide();
      BNext.Show(">>");
      BPrev.Show("<<");
      BPause.Show("PAUSE");
    }
    //function for the previous button
    else if(BPrev.IsPressed())
    {
      Serial.println("Tuning to previous station");
      if (currentstation == 0) {
        currentstation = maxstation;   // wrap around to the last station
      } else {
        currentstation--;
      }
      Radio.selectFrequency(stations[currentstation]);
      initplayer();
      tft.fillScreen(BLACK);
      tft.setCursor(78,20); tft.setTextColor(WHITE);
      tft.print(F("Raduino"));
      tft.setCursor(50,50);
      tft.setTextColor(GREEN);
      tft.print(F("Now Playing:"));
      tft.setTextSize(2);
      tft.setCursor(90,90);
      tft.setTextColor(WHITE);
      tft.print(stations[currentstation]);
      flag = 0; // Flag to fetch stats and display over serial
      delay(250); // Give the module time to tune properly before reading stats

      if (flag == 0) {
        Serial.print("Current freq: ");
        Serial.print(Radio.readFrequencyInMHz());
        Serial.print("MHz Signal: ");
        Serial.print(Radio.isStereo() ? "STEREO" : "MONO");
        Serial.print(" ");
        Serial.print(Radio.getSignalLevel());
        Serial.println("/15");
        flag = 1;
      }
      BPrev.Hide();
      BPrev.Show("<<");
      BNext.Show(">>");
      BPause.Show("PAUSE");
    }
    //end of the Bprev code    
  }
}

void guistart() {
  //tft.reset();  
  tft.begin(tft.read16()); 
  tft.setRotation(0);     
  definecolors();
  initplayer();
  BPrev.Show("<<");
  BNext.Show(">>");
  BPlay.Show("PLAY");
  if (! tp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
  }

  Serial.println("Capacitive touchscreen started");
}  

boolean gettouch() {
  //digitalWrite(13, HIGH);
  if (! tp.touched()) {
    return false;
  }

  // Retrieve a point  
  TS_Point p = tp.getPoint();
  //digitalWrite(13, LOW);   
  // if sharing pins, you'll need to fix the directions of the touchscreen pins      
  p.x = map(p.x, 0, 240, 240, 0);
  p.y = map(p.y, 0, 320, 320, 0);
  touched=0;
  if ((abs(p.x-touchx)>5||abs(p.y-touchy)>5)) {    
    touched=1;  
    touchx=p.x;
    touchy=p.y; 
    //tft.drawPixel(x,y,GREEN);
    //Serial.println(tp.y);
    //Serial.println(tp.x);
  }
  return touched;    
}

void setup() {     
  while (!Serial) ;
  Serial.begin(115200);
  if (! tp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
  Serial.println(F("Start"));     
  guistart();
  tft.setTextSize(2);    
  tft.setCursor(90,90);
  tft.setTextColor(WHITE);
  Radio.selectFrequency(stations[currentstation]);
  tft.print(stations[currentstation]); 
}  

void loop() {
 gui();
 delay(100);

}
