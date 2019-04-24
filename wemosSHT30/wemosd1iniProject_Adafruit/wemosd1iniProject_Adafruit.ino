#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WEMOS_SHT3X.h>
 
SHT3X sht30(0x45);
 
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
 
 
void setup() 
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}
 
void loop() {
 
  
  display.clearDisplay();
  display.invertDisplay(0);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(INVERSE);
 
  if(sht30.get()==0){
    display.print("T: ");
    display.setTextSize(2);
    display.print((int)sht30.cTemp);
    display.setTextSize(1);
    display.print((char)247);
    display.println("C");
    display.setTextSize(2);
    display.println();
    
    display.setTextSize(1);
    display.print("H: ");
    display.setTextSize(2);
    display.print((int)sht30.humidity);
    display.setTextSize(1);
    display.println(" %");
  }
  else
  {
    display.println("Error!");
  }
  display.display();
  delay(1000);
 
}
