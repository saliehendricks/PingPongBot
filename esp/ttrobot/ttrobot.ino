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
#include <WiFiManager.h>
#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <Servo.h>
#include <FS.h> // Include the SPIFFS library
#include <Stepper.h>
/********************
 Constant Definition
********************/

//define the motor pins
#define MTOP_PIN D5
#define MLEFT_PIN D6
#define MRIGHT_PIN D7
#define STEPS 2048
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
#define MIN_PULSE_LENGTH 1000 // Minimum pulse length in µs
#define MAX_PULSE_LENGTH 2000 // Maximum pulse length in µs

//define runtime objects
WiFiManager wifiManager;
ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

Servo topSpin;
Servo leftSpin;
Servo rightSpin;

int topSpeed = 15;
int leftSpeed = 15;
int rightSpeed = 15;

//stepper definitions
long stepinterval = 4; //millisecs
unsigned long previousMillis = 0;

Stepper stepper(STEPS, IN4, IN2, IN3, IN1);

File fsUploadFile; // a File object to temporarily store the received file

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS
void handleSetMotor();

void setup()
{

    Serial.begin(115200);
    Serial.println("Starting up");

    //setup wifi
    Serial.println("..");
    wifiManager.autoConnect("PROTEA-TERMINATOR");

    Serial.println("HTTP server starting...");
    server.on("/upload", HTTP_GET, []() { // if the client requests the upload page
                                          //if (!handleFileRead("/upload.html"))                  // send it if it exists
        //server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
        server.send(200, "text/html", "<form method=\"post\" enctype=\"multipart/form-data\">    <input type=\"file\" name=\"name\">    <input class=\"button\" type=\"submit\" value=\"Upload\"></form>");
    });

    server.on("/upload", HTTP_POST,       // if the client posts to the upload page
              []() { server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
              handleFileUpload            // Receive and save the file
    );

    server.on("/setmotor", handleSetMotor);

    server.onNotFound([]() {                                  // If the client requests any URI
        if (!handleFileRead(server.uri()))                    // send it if it exists
            server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    server.begin(); // Actually start the server
    Serial.println("HTTP server started");

    Serial.println("File system starting");
    SPIFFS.begin();
    Serial.println("File system started");

    //setup 3 launcher motors
    topSpin.attach(MTOP_PIN, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    leftSpin.attach(MLEFT_PIN, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    rightSpin.attach(MRIGHT_PIN, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);

    topSpin.writeMicroseconds(MIN_PULSE_LENGTH);
    leftSpin.writeMicroseconds(MIN_PULSE_LENGTH);
    rightSpin.writeMicroseconds(MIN_PULSE_LENGTH);

    Serial.println("Launch Motors ready");
    //TODO setup 1 feeder stepper motor
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    Serial.println("Feeder ready");

}

void loop()
{
    server.handleClient();

    handleStepper();
}

void handleStepper()
{
    //stepper feeder turn
    if (stepinterval <= 0)
    {
        stepinterval = 0;
        return;
    }
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= stepinterval)
    {
        // save the last time we stepped
        previousMillis = currentMillis;
        stepper.step(1);
    }
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

void handleSetMotor()
{
    Serial.println("handleSetMotor:");
    if (!server.hasArg("m") || !server.hasArg("s"))
    {
        Serial.println(" no motors adjusted");
        return;
    }

    String m = server.arg("m");
    String s = server.arg("s");
    long speed = s.toInt();
    
    if (m == "top")
    {
        speed = adjustSpeedForLaunchers(speed);        
        topSpin.writeMicroseconds(speed);        
    }
    else if (m == "left")
    {
        speed = adjustSpeedForLaunchers(speed);                
        leftSpin.writeMicroseconds(speed);
    }
    else if (m == "right")
    {
        speed = adjustSpeedForLaunchers(speed);        
        rightSpin.writeMicroseconds(speed);
    }
    else if (m == "leftandright")
    {
        speed = adjustSpeedForLaunchers(speed);        
        leftSpin.writeMicroseconds(speed);
        rightSpin.writeMicroseconds(speed);
    }
    else if (m == "fi")
    {
        speed = adjustSpeedForFeeder(speed);        
        stepinterval = speed;        
    }

    Serial.print(" motor speed set to ");
    Serial.println(speed);
    server.send(200, "text/plain", String(speed));
}

long adjustSpeedForLaunchers(long speed)
{
    if (speed > MAX_PULSE_LENGTH)
    {
        speed = MAX_PULSE_LENGTH;
    }
    if (speed < MIN_PULSE_LENGTH)
    {
        speed = MIN_PULSE_LENGTH;
    }
    Serial.print(speed);
    return speed;
}

long adjustSpeedForFeeder(long speed)
{
    if (speed < 2)
    {
        speed = 0;
    }
    else if (speed > 100)
    {
        speed = 100;
    }
    
    Serial.print(speed);
    return speed;
}


String getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {                                                       // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                      // If there's a compressed version available
            path += ".gz";                                  // Use the compressed verion
        File file = SPIFFS.open(path, "r");                 // Open the file
        size_t sent = server.streamFile(file, contentType); // Send it to the client
        file.close();                                       // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    Serial.println(String("\tFile Not Found: ") + path); // If the file doesn't exist, return false
    return false;
}

void handleFileUpload()
{ // upload a new file to the SPIFFS
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        Serial.print("handleFileUpload Name: ");
        Serial.println(filename);
        fsUploadFile = SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
            Serial.println("    Writing file contents..");
        }
        else
        {
            Serial.println("    No file to write contents to..");
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {                         // If the file was successfully created
            fsUploadFile.close(); // Close the file again
            Serial.print("handleFileUpload Size: ");
            Serial.println(upload.totalSize);
            server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
            server.send(303);
        }
        else
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}