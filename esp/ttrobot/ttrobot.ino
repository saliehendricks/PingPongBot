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

/********************
 Constant Definition
********************/

//define the motor pins
#define MTOP_PIN D6
int MLEFT_PIN = 4;
int MRIGHT_PIN = 5;
int MFEED_PIN = 6;

//define runtime objects
WiFiManager wifiManager;
ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

Servo topSpin;
Servo leftSpin;
Servo rightSpin;

int topSpeed = 15;
int leftSpeed = 15;
int rightSpeed = 15;

File fsUploadFile; // a File object to temporarily store the received file

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS
void handleSetMotor();

void setup()
{

    Serial.begin(115200);
    Serial.println("Starting up");

    pinMode(D0, OUTPUT);
    pinMode(D4, OUTPUT);

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

    server.on("/setmotor",handleSetMotor);

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
    topSpin.attach(MTOP_PIN,1000,2000);
    //leftSpin.attach(MRIGHT_PIN,1000,2000);
    //rightSpin.attach(MFEED_PIN,1000,2000);

    //setup 1 feeder motor
}

void loop()
{
    server.handleClient();

    digitalWrite(D0, LOW);  // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(D4, HIGH); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(1000);            // Wait for a second
    digitalWrite(D0, HIGH); // Turn the LED off by making the voltage HIGH
    digitalWrite(D4, LOW);  // Turn the LED on (Note that LOW is the voltage level
    delay(2000);
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
    if(!server.hasArg("m") || !server.hasArg("s"))
    {
        Serial.println(" no motors adjusted");
        return;
    }

    String m = server.arg("m");
    String s = server.arg("s");
    int speed = s.toInt();
    if (speed > 80)
        speed = 80;
    if (speed < 15)
        speed = 15;

    Serial.print(m);
    if (m == "top")
    {
        //topSpin.write(speed);
        topSpeed = speed;
        topSpin.write(topSpeed);
    }
    else if (m == "left")
    {
    }
    else if (m == "right")
    {
    }
    else if (m == "leftandright")
    {
    }
    else if (m == "feeder")
    {
    }
    Serial.print(" motor speed set to ");
    Serial.println(speed);
    server.send(200, "text/plain", "ok");
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