
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

/*Network Settings*/
#define STASSID

#ifndef STASSID
#define STASSID "UPC2888929"
#define STAPSK  "Xtjmbpxs7bws"
#endif

const char * ssid = "UPC2888929"; // your network SSID (name)
const char * pass = "Xtjmbpxs7bws";  // your network password
unsigned int localPort = 2390;      // local port to listen for UDP packets
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.hu.pool.ntp.org";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
/*End of Network settings*/


/*NeoPixel Settings*/
#define PIN 5
int hourIndex =0, minIndex =0, secIndex=0;
//How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 24
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
/*End of NeoPixel Settings*/

void setup() 
{
  pixels.begin();
  pixels.clear();

  
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void loop() 
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(100);

  int cb = udp.parsePacket();
  if (!cb) 
  {
    Serial.println("no packet yet");
    pixels.clear();
    pixels.setPixelColor(hourIndex, pixels.Color(0, 70, 0));
    pixels.show();
    pixels.setPixelColor(minIndex, pixels.Color(50, 0, 0));
    pixels.show();
  } 
  else 
  {
   // Serial.print("packet received, length=");
   // Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
   // Serial.print("Seconds since Jan 1 1900 = ");
   // Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    //Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900+7200 - seventyYears; //Added 7200 because the pool is lagging by 2 hours
    // print Unix time:
    //Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The local time is ");       // UTC is the time at Greenwich Meridian (GMT)
    /*Hours Calculation and Display*/
    
    int hours = (epoch  % 86400L) / 3600;
    
    if(hours>=12)
    {

      hourIndex = (hours - 12)*2;
      
    }
    else
    {
      hourIndex = hours*2;  
    }
    //Set RED LED to indicate hours
    pixels.setPixelColor(hourIndex, pixels.Color(0, 70, 0)); //pixels.Color(G,R,B);
    pixels.show();   // Send the updated pixel colors to the hardware.
    
    //Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(hours);
    
    Serial.print(':');
 
    /*Minutes Index Calculation and Display*/
    float minutes, seconds;
    minutes = (float)((epoch  % 3600) / 60);
    seconds = epoch % 60;
    minIndex = (int)((minutes + (seconds/60))/2.5);
    
    //Set GREEN LED for Minutes
    
    pixels.setPixelColor(minIndex, pixels.Color(50, 0, 0));
    pixels.show();
    
    if (minutes < 10) 
    {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((int)minutes); // print the minute (3600 equals secs per minute)
    
    
    /*Seconds Calculation and Dispay*/
    
    Serial.print(':');

    secIndex = (int)(seconds/2.5);
    
    //Set BLUE LED for Seconds
    
    pixels.setPixelColor(secIndex, pixels.Color(0, 0, 20));
    pixels.show();
    
     if (seconds<10) 
    {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println((int)seconds); // print the second
  }
  // wait ten seconds before asking for the time again
  delay(2400);
  pixels.clear();
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) 
{
  //Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
