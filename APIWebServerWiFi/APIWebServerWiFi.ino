/*
 *   For the server side:
 *     A server that accepts the following HTTP requests:
 *
 *     > GET / HTTP/1.1                      : Responds w/ "Hello from Arduino Server"
 *     > GET /ring/status/ HTTP/1.1          : Responds w/ "ON"/"OFF" for the power state of the strip
 *     > GET /ring/fade/ HTTP/1.1            : Responds w/ a JSON representation of 
 *                                             the fading parameters -> {"n":x,"d":x}
 *     > GET /ring/color/ HTTP/1.1           : Responds w/ a JSON representation of 
 *                                             the strip color -> {"r":x,"g":x,"b":x}
 *     > GET /ring/pixel/ HTTP/1.1           : Responds w/ a JSON representation of 
 *                                             the pixel color -> {"n":x,"r":x,"g":x,"b":x}
 *
 *
 *     > PUT /ring/on/ HTTP/1.1              : Turns the LED strip on
 *     > PUT /ring/off/ HTTP/1.1             : Turns the LED strip off
 *     > PUT /ring/fade/ HTTP/1.1            : Color starts fading
 *                                             The fading parameters (number, delay) are provided as a JSON object: {"n": x,"d": x}
 *     > PUT /ring/rainbow/ HTTP/1.1         : Starts a color rainbow
 *     > PUT /ring/rainbow/circle/ HTTP/1.1  : Starts a color rainbow cycle
 *     > PUT /ring/color/ HTTP/1.1           : Changes the color of the LED strip
 *                                             The color values (red, green, blue) are provided as a JSON object: {"r": x,"g": x,"b": x}
 *     > PUT /ring/pixel/ HTTP/1.1           : Changes the color of a LED pixel
 *                                             The pixel values (number, red, green, blue) are provided as a JSON object: {"n": x,"r": x,"g": x, "b":x}
 *     > PUT /ring/pixel/range/ HTTP/1.1      : Changes the color of a LED pixel range
 *                                             The pixel values (number1, number2, red, green, blue) are provided as a JSON object: {"n1": x, "n2":x ,"r": x,"g": x, "b":x}
 *     > PUT /speaker/beep/ HTTP/1.1         : Plays a beep
 *     > PUT /speaker/melody/ HTTP/1.1       : Plays a melody
 
 // Implement: Blinking, Fading, Rainbow
 Play, Pause, Stop Song
 
 * 
 * ArduinoPixel:
 *   You can find the app's source on github:
 *   https://github.com/pAIgn10/ArduinoPixel
 * 
 * Author:
 *   Nick Lamprianidis { paign10.ln [at] gmail [dot] com }
 * 
 * License:
 *   Copyright (c) 2014 Nick Lamprianidis 
 *   This code is released under the MIT license
 *   http://www.opensource.org/licenses/mit-license.php
 */

 /*
  Melody
 
 Plays a melody 
 
 circuit:
 * 8-ohm speaker on digital pin 8
 
 created 21 Jan 2010
 modified 30 Aug 2011
 by Tom Igoe 

This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/Tone
 
 */

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WiFi.h>
#include "pitches.h"

#define DEBUG

#define PIN 2
Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

//char ssid[] = "EnGenius052136-2.4G";      //  your network SSID (name)
//char ssid[] = "Dresden";      //  your network SSID (name) 
//char pass[] = "AjT>:#;,7";   // your network password
char ssid[] = "ou-celstec";      //  your network SSID (name) 
char pass[] = "ou-c3lst3c";   // your network password
//char ssid[] = "Almonacid";      //  your network SSID (name) 
//char pass[] = "tolentino";   // your network password
//char ssid[] = "FeedbackCube";      //  your network SSID (name) 
//char key[] = "7072316d40";   // your network password
//int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

WiFiClient client;

// HTTP request methods
enum Method
{
    GET,
    PUT
};

// Available URIs
enum Uri
{
    ROOT,            // "/"
    STATUS,          // "/ring/status/"
    TURN_ON,         // "/ring/on/"
    TURN_OFF,        // "/ring/off/"
    BLINK,           // "/ring/blink/"
    FADE_GET,        // "/ring/fade/"
    FADE_PUT,        // "/ring/fade/"
    RAINBOW,         // "/ring/rainbow/"
    RAINBOW_CIRCLE,  // "/ring/rainbow/circle/"
    COLOR_GET,       // "/ring/color/"
    COLOR_PUT,       // "/ring/color/"
    PIXEL_GET,         // "/ring/pixel/"
    PIXEL_PUT,         // "/ring/pixel/"
    PIXELRANGE_GET,         // "/ring/pixel/range/"
    PIXELRANGE_PUT,         // "/ring/pixel/range/"
    BEEP,            // "/speaker/beep/"
    MELODY,           // "/speaker/melody/1/"
    MELODY2           // "/speaker/melody/2/"
};

// Specifies whether the LED strip is on or off
boolean power = false;

// Keeps the current color of the LED strip
uint8_t color[] = { 128, 128, 128 };
// Keeps the current fading parameters
uint8_t fading[] = { 1, 20 };
// Keeps the current pie parameters
uint8_t pie[] = { 4, 4, 4, 4 };
// Keeps the current pixelparameters
uint8_t pixel[] = { 0, 128, 128, 128 };
// Keeps the current pixelparameters
uint8_t pixelRange[] = { 0, 0, 128, 128, 128 };

void setup() {
  #if defined(DEBUG)
  Serial.begin(9600);      // initialize serial communication
  #endif
  
  ring.begin();
  ring.setBrightness(128);
  ring.show(); // Initialize all pixels to 'off'
  
  colorWipe(0xFFFFFF,50);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    #if defined(DEBUG)
    Serial.println("WiFi shield not present");
    #endif
    while(true);        // don't continue
  } 

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    #if defined(DEBUG)
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);
    #endif
        
    //status = WiFi.begin(ssid);                  //Open
    //status = WiFi.begin(ssid, keyIndex,key);  //WEP
    status = WiFi.begin(ssid, pass);          //WPA/WPA2
    delay(10000);
  } 
  server.begin();                           // start the web server on port 80
  #if defined(DEBUG)
    printWifiStatus();                        // you're connected now, so print out the status
  #endif

  colorWipe(0x000000,50);
  
  // to calculate the note duration, take one second divided by the note type, e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  int noteDuration = 1000/4;
  tone(8, NOTE_A3,noteDuration);
  // to distinguish the notes, set a minimum time between them. > the note's duration + 30% seems to work well:
  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);
  // stop the tone playing:
  noTone(8);
}


void loop() {
  client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    #if defined(DEBUG)
    Serial.println("new client");           // print a message out the serial port
    #endif
    
    uint8_t i = 0;
    char reqChar;
    char request[255]; //35
    char buf[255]; //60
    
    while (client.connected()) {            // loop while the client's connected
      if(client.available()) {
        reqChar = client.read();             // read a byte, then
        
        if (reqChar != '\r' && reqChar != '\n' && i < 255) { //EDITED
            request[i++] = reqChar;
        }
        else {
            client.read();  // Gets rid of '\n'
            request[i] = '\0';
            break;
        }
      }
    }
    
    // Parses the request line
    int8_t method, uri;
    parseRequestLine(method, uri, String(request));
    
    // Initiates the appropriate response
    switch (method)
    {
        case GET:
            switch (uri)
            {
                case ROOT:
                    respond(200, "OK", false, "This is the Feedback Cube WIFI. You can use my API to control my functions...");
                    break;

                case STATUS:
                    respond(200, "OK", false, power?"ON":"OFF");
                    break;
                    
                case FADE_GET:
                    	{
                    	String fadeData = "{\"n\":" + String(fading[0]) + 
                    	                   ",\"d\":" + String(fading[1]) + "}";
                    	respond(200, "OK", false, fadeData);
                    	}
                    	break;
                    
                case COLOR_GET:
                    	{
                    	String colorData = "{\"r\":" + String(color[0]) + 
                    	                   ",\"g\":" + String(color[1]) + 
                    	                   ",\"b\":" + String(color[2]) + "}";
                    	respond(200, "OK", false, colorData);
                    	}
                    	break;
                
                case PIXEL_GET:
                    	{
                    	String pixelData = "{\"n\":" + String(pixel[0]) + 
                    	                   ",\"r\":" + String(pixel[1]) + 
                                           ",\"g\":" + String(pixel[2]) +
                    	                   ",\"b\":" + String(pixel[3]) + "}";
                    	respond(200, "OK", false, pixelData);
                    	}
                    	break;
                case PIXELRANGE_GET:
                    	{
                    	String pixelRangeData = "{\"n1\":" + String(pixelRange[0]) + 
                                           ",\"n2\":" + String(pixelRange[1]) + 	                   
                                           ",\"r\":" + String(pixelRange[2]) + 
                                           ",\"g\":" + String(pixelRange[3]) +
                    	                   ",\"b\":" + String(pixelRange[4]) + "}";
                    	respond(200, "OK", false, pixelRangeData);
                    	}
                    	break;

                default:
                    respond(404, "Not Found", false, "\0");
            }
            break;

        case PUT:
            switch (uri)
            {
                 // Turns the LED strip on
                 case TURN_ON:
                    power = true;
                    //colorWipe(0xFFFFFF,0);
                    colorWipe(ring.Color(color[0], color[1], color[2]),0);
                    respond(200, "OK", false, "\0");
                    break;

                 // Turn the LED strip off
                 case TURN_OFF:
                    power = false;
                    //colorWipe(0x000000,0);
                    colorWipe(ring.Color(0, 0, 0),0);
                    respond(200, "OK", false, "\0");
                    break;
                 
                 //Color starts fading up/down
                 case FADE_PUT:
                    getJson(buf);  // Retrieves the JSON data
                    parseFade(String(buf));  // Parses the data
                    if (power)
                      colorFade(ring.Color(color[0], color[1], color[2]),fading[0],fading[1]);
                    respond(200, "OK", false, "\0");
                    break;
                 
                 //Start a rainbow
                 case RAINBOW:
                    if (power) {
                      rainbow(20);
                      colorWipe(ring.Color(color[0], color[1], color[2]),0);
                    }
                    respond(200, "OK", false, "\0");
                    break;
                 
                 //Start a rainbow cycle
                 case RAINBOW_CIRCLE:
                    if (power) {
                      rainbowCircle(20);
                      colorWipe(ring.Color(color[0], color[1], color[2]),0);
                    }
                    respond(200, "OK", false, "\0");
                    break;
                   
                 // Parses the JSON data (and updates the LED strip color)
                 case COLOR_PUT:
                    getJson(buf);  // Retrieves the JSON data
                    parseColors(String(buf));  // Parses the data
                    if (power)
                        colorWipe(ring.Color(color[0], color[1], color[2]),0);
                    respond(200, "OK", false, "\0");
                    break;
                    
                 // Parses the JSON data (and updates the LED pixel range color)
                 case PIXELRANGE_PUT:
                    getJson(buf);  // Retrieves the JSON data
                    parsePixelRange(String(buf));  // Parses the data
                    if (power)
                        pixelRangeWipe(pixelRange[0],pixelRange[1],ring.Color(pixelRange[2], pixelRange[3], pixelRange[4]),0);
                    respond(200, "OK", false, "\0");
                    break;
                    
                 // Parses the JSON data (and updates the LED pixel color)
                 case PIXEL_PUT:
                    getJson(buf);  // Retrieves the JSON data
                    parsePixel(String(buf));  // Parses the data
                    if (power)
                        pixelWipe(pixel[0],ring.Color(pixel[1], pixel[2], pixel[3]),0);
                    respond(200, "OK", false, "\0");
                    break;
                 
                 //Plays a beep
                 case BEEP:
                    tone(8, NOTE_A3, 1000/4);
                    // to distinguish the notes, set a minimum time between them. > the note's duration + 30% seems to work well:
                    delay((1000/4) * 1.30);
                    // stop the tone playing:
                    noTone(8);
                    respond(200, "OK", false, "\0");
                    break;
                    
                //Plays a melody
                case MELODY:
                    playMelody(1);
                    respond(200, "OK", false, "\0");
                    break;
                    
                //Plays a melody
                case MELODY2:
                    playMelody(2);
                    respond(200, "OK", false, "\0");
                    break;

                default:
                    respond(404, "Not Found", false, "\0");
            }
            break;

        default:
            respond(404, "Not Found", false, "\0");
    }
    
    //// close the connection:
    //client.stop();
    //Serial.println("client disonnected");
  }
  //printWifiStatus();
}

// Parses the HTTP request line
// Parameters: method  - the method of the request
//             uri     - the requested uri
//             request - string that holds the request line
void parseRequestLine(int8_t& method, int8_t& uri, String request)
{
    #if defined(DEBUG)
    Serial.println("Request: "+request);
    #endif
    
    // Parses the request method
    String reqMethod = request.substring(0, 3);
    if (reqMethod.equals("GET"))
        method = GET;
    else if (reqMethod.equals("PUT"))
        method = PUT;
    else
        method = -1;

    // Parses the URI
    String reqUri = request.substring(4, request.lastIndexOf(' '));
    if (reqUri.equals("/"))
        uri = ROOT;
    else if (reqUri.equals("/ring/status/"))
        uri = STATUS;
    else if (reqUri.equals("/ring/on/"))
        uri = TURN_ON;
    else if (reqUri.equals("/ring/off/"))
        uri = TURN_OFF;
    else if (reqUri.equals("/ring/fade/"))
        if (method == GET)
    		uri =FADE_GET;
    	else
        	uri = FADE_PUT;
    else if (reqUri.equals("/ring/rainbow/"))
        uri = RAINBOW;
    else if (reqUri.equals("/ring/rainbow/circle/"))
        uri = RAINBOW_CIRCLE;
    else if (reqUri.equals("/ring/color/"))
    	if (method == GET)
    		uri = COLOR_GET;
    	else
        	uri = COLOR_PUT;
    else if (reqUri.equals("/ring/pixel/"))
    	if (method == GET)
    		uri = PIXEL_GET;
    	else
        	uri = PIXEL_PUT;
    else if (reqUri.equals("/ring/pixel/range/"))
    	if (method == GET)
    		uri = PIXELRANGE_GET;
    	else
        	uri = PIXELRANGE_PUT;
    else if (reqUri.equals("/speaker/beep/"))
        uri = BEEP;
    else if (reqUri.equals("/speaker/melody/1/"))
        uri = MELODY;
    else if (reqUri.equals("/speaker/melody/2/"))
        uri = MELODY2;
    else
        uri = -1;
        
    #if defined(DEBUG)
    Serial.print("Method: ");
    Serial.println(method);
    Serial.print("URI: ");
    Serial.println(uri);
    #endif
}

// Sends an HTTP response
// Parameters: statusCode - Status code for the response line
//             statusMsg  - Status message for the response line
//             keepAlive  - Whether to keep the connection open or not
//             data       - Data to return to client
void respond(int statusCode, const char* statusMsg, boolean keepAlive, String data)
{
        #if defined(DEBUG)
        Serial.print("HTTP/1.1 ");
        Serial.print(statusCode);
        Serial.print(" ");
        Serial.println(statusMsg);
        if (!keepAlive) Serial.println("Connection: close");
        Serial.println();
        if (data) {
          Serial.print(data);
          Serial.println();
        }
        #endif

        client.print("HTTP/1.1 ");
        client.print(statusCode);
        client.print(" ");
        client.println(statusMsg);
        if (!keepAlive) client.println("Connection: close");
        if (data) { 
          client.println();
          client.print(data);
        }
        delay(1);
        client.flush();
        client.stop();
}

// Retrieves the JSON data
// Parameters: buf - the buffer the store the data
void getJson(char* buf)
{
    uint8_t i = 0;
    char readChar;
    char prevlastCharRead = '\0';
    char lastCharRead;

    // Gets rid of request headers
    while (client.available())
    {
        lastCharRead = client.read();
        if (prevlastCharRead == '\r' && lastCharRead == '\n')
        {
            char c1 = '\0', c2 = '\0';
            if (client.available()) c1 = client.read();
            if (client.available()) c2 = client.read();
            if (c1 == '\r' && c2 == '\n') break;
        }
        prevlastCharRead = lastCharRead;
    }

    // Retrieves the JSON data
    while (client.available())
    {
      readChar = client.read(); //EDITED
      if (i < 255) { //EDITED
            buf[i++] = readChar; //EDITED
        }
      //buf[i++] = client.read();
    }
    buf[i] = '\0';
    
    #if defined(DEBUG)
    Serial.println(buf);
    #endif
}

// Parses the colors from the JSON data > Parameters: json - Holds the data
void parseFade(String json)
{
    uint8_t idx = 0;
    for (uint8_t i = 0; i < 2; ++i)
    {
        while (json[idx++] != ':') ;
        uint8_t startIdx = idx;
        while (json[idx] != ',' && json[idx] != '}') idx++;
        uint8_t endIdx = idx;
        String fadeStr = json.substring(startIdx, endIdx);
        fading[i] = (uint8_t) fadeStr.toInt();
    }
}

// Parses the colors from the JSON data > Parameters: json - Holds the data
void parseColors(String json)
{
    uint8_t idx = 0;
    for (uint8_t i = 0; i < 3; ++i)
    {
        while (json[idx++] != ':') ;
        uint8_t startIdx = idx;
        while (json[idx] != ',' && json[idx] != '}') idx++;
        uint8_t endIdx = idx;
        String colorStr = json.substring(startIdx, endIdx);
        color[i] = (uint8_t) colorStr.toInt();
    }
}

// Parses the colors from the JSON data > Parameters: json - Holds the data
void parsePixel(String json)
{
    uint8_t idx = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        while (json[idx++] != ':') ;
        uint8_t startIdx = idx;
        while (json[idx] != ',' && json[idx] != '}') idx++;
        uint8_t endIdx = idx;
        String pixelStr = json.substring(startIdx, endIdx);
        pixel[i] = (uint8_t) pixelStr.toInt();
    }
}

// Parses the colors from the JSON data > Parameters: json - Holds the data
void parsePixelRange(String json)
{
    uint8_t idx = 0;
    for (uint8_t i = 0; i < 5; ++i)
    {
        while (json[idx++] != ':') ;
        uint8_t startIdx = idx;
        while (json[idx] != ',' && json[idx] != '}') idx++;
        uint8_t endIdx = idx;
        String pixelRangeStr = json.substring(startIdx, endIdx);
        pixelRange[i] = (uint8_t) pixelRangeStr.toInt();
    }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Fade the color brightness down and up again
void colorFade(uint32_t c, uint16_t k, uint8_t wait) {
  uint16_t i;

  for(i=0; i<k; i++) {
    for(uint16_t m=128; m>1; m--) {
        ring.setBrightness(m);
        ring.show();
        delay(wait);
    }
    
    delay(wait);
    
    for(uint16_t n=1; n<129; n++) {
        ring.setBrightness(n);
        ring.show();
        delay(wait);
    }
  } 
}

// Fill the pixels one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<ring.numPixels(); i++) {
      ring.setPixelColor(i, c);
      ring.show();
      delay(wait);
  }
}

// Fill the pixel n with a color
void pixelWipe(uint16_t n, uint32_t c, uint8_t wait) {  
  ring.setPixelColor(n, c);
  ring.show();
  delay(wait);
}

// Fill the pixel n with a color
void pixelRangeWipe(uint16_t n1, uint16_t n2, uint32_t c, uint8_t wait) {  
  for(uint16_t i=n1; i<=n2; i++) {
    ring.setPixelColor(i, c);
    ring.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*3; j++) { // 3 cycles of all colors on wheel
    for(i=0; i<ring.numPixels(); i++) {
      ring.setPixelColor(i, Wheel((i+j) & 255));
    }
    ring.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCircle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*3; j++) { // 3 cycles of all colors on wheel
    for(i=0; i< ring.numPixels(); i++) {
      ring.setPixelColor(i, Wheel(((i * 256 / ring.numPixels()) + j) & 255));
    }
    ring.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// Fill the pixel n with a color
void playMelody(int number) {  
  
  int melody[] = {NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4 };
    
  int melody2[] = {NOTE_E3, NOTE_E3,NOTE_F3, NOTE_G3, NOTE_G3, NOTE_F3, NOTE_E3, NOTE_D3};
  int noteDurations2[] = {4, 4, 4, 4, 4, 4, 4, 4 };
  
  for (int thisNote = 0; thisNote < 8; thisNote++) {       
    
    if(number == 1) {
      int noteDuration = 1000/noteDurations[thisNote];
      tone(8, melody[thisNote],noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
    }
    else if(number == 2){
      int noteDuration = 1000/noteDurations2[thisNote];
      tone(8, melody2[thisNote],noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
    }
    
    noTone(8);
  }
}
