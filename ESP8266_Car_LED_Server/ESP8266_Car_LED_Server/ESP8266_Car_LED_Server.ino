#include "WiFiController.h"
#include "WebServerController.h"
#include <Adafruit_NeoPixel.h>

#define DNS_SSID "Audi TT WebServer"

#define LEFT_FRONT_LEDS 5
#define RIGHT_FRONT_LEDS 4
#define NUMPIXELS_FRONT 28
#define STAGES_FRONT 4
#define GAUGE_PIN 12

uint32_t wiFiTimer(0), animationTimer(0);
uint8_t dieCounter(0);

bool animation(false);
bool turnOnOff(true);
bool turboGauge(false);

uint8_t red(0), green(0), blue(0);

Adafruit_NeoPixel frontLeftPixels (NUMPIXELS_FRONT, LEFT_FRONT_LEDS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel frontRightPixels(NUMPIXELS_FRONT, RIGHT_FRONT_LEDS, NEO_GRB + NEO_KHZ800);
WebServerController WebServerContr;

char * ownWebSocketOnInit() {
	char * buff = new char[50];
	sprintf_P(buff, PSTR("I%d,%d,%d,%d,%d,%d,%d,%d,%d"), 
		red, green, blue, 
		WebServerContr.eeprom.getRed(), WebServerContr.eeprom.getGreen(), WebServerContr.eeprom.getBlue(),
		animation, turnOnOff, turboGauge);
	return buff;
}

void ownWebSocketOnSwitch(uint8_t sign, uint8_t * payload) {

	if (sign == 'a') {
		char temp[2] = { payload[0] , payload[1] };
		red = (uint8_t)strtol(temp,NULL,16);
		temp[0] = payload[2]; temp[1] = payload[3];
		green = (uint8_t)strtol(temp, NULL, 16);
		temp[0] = payload[4]; temp[1] = payload[5];
		blue = (uint8_t)strtol(temp, NULL, 16);

		frontLeftPixels.fill (frontLeftPixels.Color (red, green, blue));
		frontRightPixels.fill(frontRightPixels.Color(red, green, blue));
		frontLeftPixels.show();
		frontRightPixels.show();
		yield();
		Serial.printf_P(PSTR("MAC: %s Heap: %d\n"), WiFi.macAddress().c_str(), ESP.getFreeHeap());
		return;
	}

	if (sign == 'b') {
		uint8_t button(atoi((char*)&payload[0]));
		switch (button)
		{
		case 0:
		{
			WebServerContr.eeprom.setRed(red);
			WebServerContr.eeprom.setGreen(green);
			WebServerContr.eeprom.setBlue(blue);

			char * buff = new char[16];
			sprintf_P(buff, PSTR("B%d,%d,%d"), red, green, blue);
			WebServerContr.webSocketSendText(buff);
			delete[] buff;
			break;
		}
		case 1:
		{
			red = WebServerContr.eeprom.getRed();
			green = WebServerContr.eeprom.getGreen();
			blue = WebServerContr.eeprom.getBlue();

			frontLeftPixels.fill (frontLeftPixels.Color (red, green, blue));
			frontRightPixels.fill(frontRightPixels.Color(red, green, blue));
			frontLeftPixels.show();
			frontRightPixels.show();
			yield();

			char * buff = new char[16];
			sprintf_P(buff, PSTR("A%d,%d,%d"), red, green, blue);
			WebServerContr.webSocketSendText(buff);
			delete[] buff;
			break;
		}
		case 2:
		{ 
			frontStartup1();
			break;
		}
		case 3:
		{
			animation = !animation;
			WebServerContr.webSocketSend('C', animation);
			//WebServerContr.eeprom.setAnimation(animation);
			break;
		}
		case 4:
		{
			turnOnOff = !turnOnOff;
			WebServerContr.webSocketSend('D', turnOnOff);
			if(turnOnOff){
				frontLeftPixels.fill (frontLeftPixels.Color (red, green, blue));
				frontRightPixels.fill(frontRightPixels.Color(red, green, blue));
			}
			else
			{
				frontLeftPixels.clear();
				frontRightPixels.clear();
			}
			frontLeftPixels.show();
			frontRightPixels.show();
			yield();
			break;
		}
		case 5:
		{
			digitalWrite(GAUGE_PIN, !(turboGauge = !turboGauge));
			WebServerContr.webSocketSend('E', turboGauge);
			break;
		}
		default:
		{
			break;
		}
		}
		yield();
		return;
	}
}



void preinit() {
	ESP8266WiFiClass::preinitWiFiOff();
}

void setup()
{
	Serial.begin(115200);
	yield();

	Serial.printf_P(PSTR("MAC: %s Heap: %d BootReason: %d\n"), WiFi.macAddress().c_str(), ESP.getFreeHeap(), ESP.getResetInfoPtr()->reason);

	pinMode(BUILTIN_LED, OUTPUT);
	pinMode(GAUGE_PIN, OUTPUT);
    
	digitalWrite(BUILTIN_LED, HIGH);
	digitalWrite(GAUGE_PIN, HIGH);
	yield();

	red   = WebServerContr.eeprom.getRed();
	green = WebServerContr.eeprom.getGreen();
	blue  = WebServerContr.eeprom.getBlue();
	animation = WebServerContr.eeprom.getAnimation();
	yield();
  
	frontLeftPixels.begin();
	frontRightPixels.begin();
	yield();
 
	delay(1000);
	if(ESP.getResetInfoPtr()->reason != 4) frontStartup1();

	yield();
  
	//WebServerContr.WiFiContr.forceWifiERegister();
	if (WebServerContr.WiFiContr.begin(DNS_SSID)) {
		WebServerContr.beginSPIFFS();
		WebServerContr.beginWebSocket();
		WebServerContr.beginServer();
		//WebServerContr.beginOTA("root","root");
	}
 
	WebServerContr.webSocketOnInit(ownWebSocketOnInit);
	WebServerContr.webSocketOnSwitch(ownWebSocketOnSwitch);

}

void loop()
{
	yield();

	WebServerContr.webSocketLoop();
	WebServerContr.handleClientLoop();
	//WebServerContr.otaLoop();

	if (millis() - wiFiTimer >= 60000) {
		wiFiTimer = millis();
		WiFi.mode(WIFI_OFF);
		WebServerContr.WiFiContr.connect();
		WebServerContr.beginSPIFFS();
		WebServerContr.beginWebSocket();
		WebServerContr.beginServer();
	}

	if (animation && millis() - animationTimer >= 60000) {
		animation1();
		animationTimer = millis();
	}
  
	WebServerContr.PreventEspStuck();
}


//	LED Functions


void animation1(){
  int16_t tempRed(red), tempGreen(green), tempBlue(blue);
  
  for(int i=0; i<20; i++){
    tempRed   = tempRed   > 34 ? tempRed   - 11 : tempRed;
    tempGreen = tempGreen > 34 ? tempGreen - 11 : tempGreen;
    tempBlue  = tempBlue  > 34 ? tempBlue  - 11 : tempBlue;
    
    frontLeftPixels.fill(frontLeftPixels.Color(tempRed, tempGreen, tempBlue)); 
    frontRightPixels.fill(frontRightPixels.Color(tempRed, tempGreen, tempBlue));
      
    frontLeftPixels.show();
    frontRightPixels.show();
    delay(80);
  }
  
  delay(2000);
  
  int16_t secondTempRed(0),secondTempGreen(0),secondTempBlue(0);
  uint8_t divider = (red - tempRed)/7;
  Serial.println(divider);
  
  for(int i=0; i<NUMPIXELS_FRONT+5; i++) {
    secondTempRed = red; secondTempGreen = green; secondTempBlue = blue;
    for(int j=0; j<8; j++)
      if(i-j > -1 && i-j < NUMPIXELS_FRONT){
        secondTempRed   = secondTempRed-j*divider   <= tempRed ? tempRed     : secondTempRed   -j*divider;
        secondTempGreen = secondTempGreen-j*divider <= tempGreen ? tempGreen : secondTempGreen -j*divider;
        secondTempBlue  = secondTempBlue-j*divider  <= tempBlue ? tempBlue   : secondTempBlue  -j*divider;
        
        frontLeftPixels.setPixelColor(i-j, frontLeftPixels.Color(secondTempRed, secondTempGreen, secondTempBlue));
        frontRightPixels.setPixelColor(i-j, frontRightPixels.Color(secondTempRed, secondTempGreen, secondTempBlue));
      }
      
  frontLeftPixels.show();
  frontRightPixels.show();
  delay(70);
  
  }
  
  for(int i=NUMPIXELS_FRONT; i>-1; i--) {
    frontLeftPixels.setPixelColor(i, frontLeftPixels.Color(red, green, blue));
    frontRightPixels.setPixelColor(i, frontRightPixels.Color(red, green, blue));
    frontLeftPixels.show();
    frontRightPixels.show();
    delay(70);
  }

    delay(2000);
    frontLeftPixels.fill(frontLeftPixels.Color(red, green, blue)); 
    frontRightPixels.fill(frontRightPixels.Color(red, green, blue));
}

void frontStartup1()
{
  uint8_t stages = NUMPIXELS_FRONT/STAGES_FRONT;
  uint8_t tempRed(0), tempGreen(0), tempBlue(0);
  frontLeftPixels.clear();
  frontRightPixels.clear();
  
  for(int i=0; i<NUMPIXELS_FRONT; i++) {
    frontLeftPixels.setPixelColor(i, frontLeftPixels.Color(255, 0, 0));
    frontRightPixels.setPixelColor(i, frontRightPixels.Color(255, 0, 0));
    frontLeftPixels.show();
    frontRightPixels.show();
    delay(25);
  }

  for(int i=0; i<NUMPIXELS_FRONT+4; i++) {
    for(int j=0; j<=4; j++)
      if(i-j > -1 && i-j < NUMPIXELS_FRONT){
        frontLeftPixels.setPixelColor(i-j, frontLeftPixels.Color(200-j*50, 0, 0));
        frontRightPixels.setPixelColor(i-j, frontRightPixels.Color(200-j*50, 0, 0));
      }
      
  frontLeftPixels.show();
  frontRightPixels.show();
  delay(20);
  }
  
  delay(200);
  
  for(int i=0; i<STAGES_FRONT; i++) {
      for(int k=1; k<=16; k++){
        for(int j=0; j<stages; j++)
          if(k<13){
            frontLeftPixels.setPixelColor(i*stages+j, frontLeftPixels.Color(15+k*20, 0, 0));
            frontRightPixels.setPixelColor(i*stages+j, frontRightPixels.Color(15+k*20, 0, 0));
          }
          else{
            frontLeftPixels.setPixelColor(i*stages+j, frontLeftPixels.Color(tempRed = 255-(k-12)*40, 0, 0)); 
            frontRightPixels.setPixelColor(i*stages+j, frontRightPixels.Color(tempRed, 0, 0));
          }
        frontLeftPixels.show();
        frontRightPixels.show();   
        delay(15);
    }
    delay(200);
  }
  delay(500);

  while(tempRed != red || tempGreen != green || tempBlue != blue){
    
    if(tempRed < red)tempRed++;
    if(tempRed > red)tempRed--;
    if(tempGreen < green)tempGreen++;
    if(tempGreen > green)tempGreen--;    
    if(tempBlue < blue)tempBlue++;
    if(tempBlue > blue)tempBlue--;
        
    frontLeftPixels.fill(frontLeftPixels.Color(tempRed, tempGreen, tempBlue)); 
    frontRightPixels.fill(frontRightPixels.Color(tempRed, tempGreen, tempBlue));
      
    frontLeftPixels.show();
    frontRightPixels.show();
    delay(10);
  }
}
