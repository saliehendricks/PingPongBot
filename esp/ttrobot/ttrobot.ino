

/**
 * License: GPL
 * Firmware Version:
 * Date: September 2019
 * 
 * Control a Table Tennis robot over self hosted wifi connection.
 * Features: ESP8266 / ESP32 firmware
 * Drives 3 FPV Drone Motors
 * Hosted Wifi page
 *  
 * */

#include <Servo.h>                //See: https://ubidots.com/blog/speed-control-for-brushless-motors-with-an-esp8266-2/
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/********************
 Constant Definition
********************/

//define the motor pins
#define MTOP_PIN  D3
int MLEFT_PIN  = 4;
int MRIGHT_PIN = 5;
int MFEED_PIN  = 6;

//define LED pins
 #define LEDPINA 12;

//define runtime objects
WiFiManager wifiManager;
Servo topSpin; 
Servo leftSpin;
Servo rightSpin;

void setup() 
{

    Serial.begin(115200);

    //setup wifi
    Serial.println("Should save config");
    wifiManager.autoConnect("PROTEA-TERMINATOR");
        
    //setup 3 launcher motors
    topSpin.attach(MTOP_PIN,1000,2000);
    leftSpin.attach(MRIGHT_PIN,1000,2000);
    rightSpin.attach(MFEED_PIN,1000,2000);

    //setup 1 feeder motor
    
}

void loop() 
{  

}

//Setup the system for topSpin at a specified speed
//m|l|h (medium, low or high)
void setTopSpin(char speed = 'm')
{
    //set topspin speed
    //reduce left/right speeds    
}

void setBackSpin(char speed = 'm')
{
    //reduce topspin speed
    //increase left/right speeds equal amounts
}

void setSideSpinChop(char speed = 'm')
{
    //reduce topspin speed
    //increase left speeds
    //reduce right speed
}

void setDrill(String drillSquence, char speed = 'm')
{
    
}