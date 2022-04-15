#include <SDI12.h>
#include <M5Core2.h>
#include <WiFi.h>
#include "Free_Fonts.h"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"



WiFiClient client;

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
#define DATA_PIN 26        /*!< The pin of the SDI-12 data bus */
#define POWER_PIN -1       /*!< The sensor power pin (or -1 if not switching power) */

bool tft_backlight = true;
// screen off counter
int tft_counter = 0;
// pictures on the sd card
String iot_picture = "";
const char *ssid =  "OIZOM_3G";
const char *pass =  "9510810866";
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "mausam5555"
#define MQTT_PASS "aio_zTIC36fS92mzdZ7lH0LzCQpjfiNn"

///////////////////////////////////////////////
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Publish atmosphericPressure = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/atmosphericPressure");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Temperature");
Adafruit_MQTT_Publish humiditySensorTemperature = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/humiditySensorTemperature");
Adafruit_MQTT_Publish vaporPressure = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/vaporPressure");
Adafruit_MQTT_Publish Precipitation = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Precipitation");
Adafruit_MQTT_Publish Solar = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Solar");
Adafruit_MQTT_Publish xOrientation = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/xOrientation");
Adafruit_MQTT_Publish yOrientation = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/yOrientation");
Adafruit_MQTT_Publish windDirection = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/windDirection");
Adafruit_MQTT_Publish windSpeed = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/windSpeed");
//////////////////////////////////////////////

unsigned long lastConnectionTime = 10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)


float temperature, atmosphericpressure, humiditysensortemperature, vaporpressure, xorientation, yorientation, precipitation, solar, winddirection, windspeed;
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 10000; // wait ten seconds between measurement attempts.

/** Define the SDI-12 bus */
SDI12 mySDI12(DATA_PIN);

// keeps track of active addresses
bool isActive[64] = {
  0,
};

uint8_t numSensors = 0;

float atmosValues[20] = {0};

/**
   @brief converts allowable address characters ('0'-'9', 'a'-'z', 'A'-'Z') to a
   decimal number between 0 and 61 (inclusive) to cover the 62 possible
   addresses.
*/
byte charToDec(char i) {
  if ((i >= '0') && (i <= '9')) return i - '0';
  if ((i >= 'a') && (i <= 'z')) return i - 'a' + 10;
  if ((i >= 'A') && (i <= 'Z'))
    return i - 'A' + 36;
  else
    return i;
}

/**
   @brief maps a decimal number between 0 and 61 (inclusive) to allowable
   address characters '0'-'9', 'a'-'z', 'A'-'Z',

   THIS METHOD IS UNUSED IN THIS EXAMPLE, BUT IT MAY BE HELPFUL.
*/
char decToChar(byte i) {
  if (i < 10) return i + '0';
  if ((i >= 10) && (i < 36)) return i + 'a' - 10;
  if ((i >= 36) && (i <= 62))
    return i + 'A' - 36;
  else
    return i;
}



/**
   @brief gets identification information from a sensor, and prints it to the serial
   port

   @param i a character between '0'-'9', 'a'-'z', or 'A'-'Z'.
*/

void printInfo(char i) {
  String command = "";
  command += (char)i;
  command += "I!";
  mySDI12.sendCommand(command);
  delay(100);

  String sdiResponse = mySDI12.readStringUntil('\n');
  sdiResponse.trim();
  // allccccccccmmmmmmvvvxxx...xx<CR><LF>
  Serial.print(sdiResponse.substring(0, 1));  // address
  Serial.print(", ");
  Serial.print(sdiResponse.substring(1, 3).toFloat() / 10);  // SDI-12 version number
  Serial.print(", ");
  Serial.print(sdiResponse.substring(3, 11));  // vendor id
  Serial.print(", ");
  Serial.print(sdiResponse.substring(11, 17));  // sensor model
  Serial.print(", ");
  Serial.print(sdiResponse.substring(17, 20));  // sensor version
  Serial.print(", ");
  Serial.print(sdiResponse.substring(20));  // sensor id
  Serial.print(", ");
}



void screen1() {
  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.print("Button A pressed");
  delay(100);

  M5.Lcd.fillRect(0, 0, 400, 80, WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("Solar : ");

  M5.Lcd.print(atmosValues[0]);
  Serial.print("Solar : "); Serial.println(atmosValues[0]);

  solar = atmosValues[0];

  Serial.print("precipitation : "); Serial.println(atmosValues[1]);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.fillRect(0, 80, 400, 80, WHITE);
  M5.Lcd.print("precipitation : ");

  M5.Lcd.print(atmosValues[1]);
  precipitation = atmosValues[1];


  Serial.print("strikes : "); Serial.println(atmosValues[2]);
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.fillRect(0, 160, 400, 80, WHITE);
  M5.Lcd.print("strikes : ");

  M5.Lcd.print(atmosValues[2]);

  Serial.print("strikeDistance : "); Serial.println(atmosValues[3]);
  M5.Lcd.setCursor(10, 200);
  M5.Lcd.fillRect(0, 240, 400, 80, WHITE);
  M5.Lcd.print("strikeDistance : ");

  M5.Lcd.print(atmosValues[3]);

  M5.Lcd.fillRect(0, 60, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 180, 320, 4, BLUE); // horizontal line

  //delay(10000);
}


void screen2() {
  M5.Lcd.fillScreen(WHITE);
  Serial.print("windSpeed : "); Serial.println(atmosValues[4]);
  M5.Lcd.fillRect(0, 0, 400, 80, WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("windSpeed : ");

  M5.Lcd.print(atmosValues[4]);
  windspeed = atmosValues[4];


  Serial.print("windDirection : "); Serial.println(atmosValues[5]);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.fillRect(0, 80, 400, 80, WHITE);
  M5.Lcd.print("windDirection : ");

  M5.Lcd.print(atmosValues[5]);
  winddirection = atmosValues[5];


  Serial.print("gustWindSpeed : "); Serial.println(atmosValues[6]);
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.fillRect(0, 160, 400, 80, WHITE);
  M5.Lcd.print("gustWindSpeed : ");

  M5.Lcd.print(atmosValues[6]);


  Serial.print("airTemperature : "); Serial.println(atmosValues[7]);
  M5.Lcd.setCursor(10, 200);
  M5.Lcd.fillRect(0, 240, 400, 80, WHITE);
  M5.Lcd.print("airTemperature : ");

  M5.Lcd.print(atmosValues[7]);
  temperature = atmosValues[7];


  M5.Lcd.fillRect(0, 60, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 180, 320, 4, BLUE); // horizontal line
}

void screen3() {
  M5.Lcd.fillScreen(WHITE);
  Serial.print("vaporPress : "); Serial.println(atmosValues[8]);
  M5.Lcd.fillRect(0, 0, 400, 80, WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("vaporPress : ");

  M5.Lcd.print(atmosValues[8]);
  vaporpressure = atmosValues[8];

  Serial.print("atmosphericPress : "); Serial.println(atmosValues[9]);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.fillRect(0, 80, 400, 80, WHITE);
  M5.Lcd.print("atmosphericPress : ");

  M5.Lcd.print(atmosValues[9]);
  atmosphericpressure = atmosValues[9];

  Serial.print("relativeHumidity : "); Serial.println(atmosValues[10]);
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.fillRect(0, 160, 400, 80, WHITE);
  M5.Lcd.print("relativeHumidity : ");

  M5.Lcd.print(atmosValues[10]);

  Serial.print("humiditySensorTemp : "); Serial.println(atmosValues[11]);
  M5.Lcd.setCursor(10, 200);
  M5.Lcd.fillRect(0, 240, 400, 80, WHITE);
  M5.Lcd.print("humiditySensorTemp : ");

  M5.Lcd.print(atmosValues[11]);
  humiditysensortemperature = atmosValues[11];

  M5.Lcd.fillRect(0, 60, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 180, 320, 4, BLUE); // horizontal line
}

void screen4() {

  M5.Lcd.fillScreen(WHITE);
  Serial.print("xOrientation : "); Serial.println(atmosValues[12]);
  M5.Lcd.fillRect(0, 0, 400, 80, WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("xOrientation : ");

  M5.Lcd.print(atmosValues[12]);
  xorientation = atmosValues[12];


  Serial.print("yOrientation : "); Serial.println(atmosValues[13]);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.fillRect(0, 80, 400, 80, WHITE);
  M5.Lcd.print("yOrientation : ");

  M5.Lcd.print(atmosValues[13]);
  yorientation = atmosValues[13];


  Serial.print("NorthWindSpeed : "); Serial.println(atmosValues[15]);
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.fillRect(0, 160, 400, 80, WHITE);
  M5.Lcd.print("NorthWindSpeed : ");

  M5.Lcd.print(atmosValues[15]);


  Serial.print("EastWindSpeed : "); Serial.println(atmosValues[16]);
  M5.Lcd.setCursor(10, 200);
  M5.Lcd.fillRect(0, 240, 400, 80, WHITE);
  M5.Lcd.print("EastWindSpeed : ");

  M5.Lcd.print(atmosValues[16]);

  M5.Lcd.fillRect(0, 60, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 180, 320, 4, BLUE); // horizontal line
}

void homescreen() {
  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.setCursor(100, 120);
  M5.Lcd.fillRect(0, 60, 400, 65, YELLOW);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.setCursor(30, 220);
  M5.Lcd.print("BtnA");
  M5.Lcd.setCursor(140, 220);
  M5.Lcd.print("BtnB");
  M5.Lcd.setCursor(240, 220);
  M5.Lcd.print("BtnC");
  M5.Lcd.setCursor(100, 90);
  M5.Lcd.print("HOME SCREEN");
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.print("LD College of Engineering");

  M5.Lcd.setCursor(20, 160);
  M5.Lcd.print(" By - Mausam Prajapati  ");

  M5.Lcd.setCursor(100, 180);
  M5.Lcd.print(" Lipika Sarkar  ");

}

bool getContinuousResults(char i, int resultsExpected) {
  uint8_t resultsReceived = 0;
  uint8_t cmd_number      = 0;

  while (resultsReceived < resultsExpected && cmd_number <= 9) {
    String command = "";
    // in this example we will only take the 'DO' measurement
    command = "";
    command += i;
    command += "R";
    command += cmd_number;
    command += "!";  // SDI-12 command to get data [address][D][dataOption][!]
    mySDI12.sendCommand(command);

    uint32_t start = millis();
    while (mySDI12.available() < 3 && (millis() - start) < 1500) {}
    mySDI12.read();           // ignore the repeated SDI12 address
    char c = mySDI12.peek();  // check if there's a '+' and toss if so
    if (c == '+') {
      mySDI12.read();
    }
    int v = 0;
    while (mySDI12.available()) {
      char c = mySDI12.peek();
      if (c == '-' || (c >= '0' && c <= '9') || c == '.') {
        float result = mySDI12.parseFloat(SKIP_NONE);
        atmosValues[v] = result;
        v++;
        Serial.print(String(result, 10));
        if (result != -9999) {
          resultsReceived++;
        }
      } else if (c == '+') {
        mySDI12.read();
        Serial.print(", ");
      } else {
        mySDI12.read();
      }
      delay(10);  // 1 character ~ 7.5ms
    }
    if (resultsReceived < resultsExpected) {
      Serial.print(", ");
    }
    cmd_number++;
  }
  mySDI12.clearBuffer();


  return resultsReceived == resultsExpected;
}

// this checks for activity at a particular address
// expects a char, '0'-'9', 'a'-'z', or 'A'-'Z'

boolean checkActive(char i) {
  String myCommand = "";
  myCommand        = "";
  myCommand += (char)i;  // sends basic 'acknowledge' command [address][!]
  myCommand += "!";

  for (int j = 0; j < 3; j++) {  // goes through three rapid contact attempts
    mySDI12.sendCommand(myCommand);
    delay(100);
    if (mySDI12.available()) {  // If we here anything, assume we have an active sensor
      mySDI12.clearBuffer();
      return true;
    }
  }
  mySDI12.clearBuffer();
  return false;
}


void MQTT_connect()
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected())
  {
    Serial.println("MQTT Connected");
    return;
  }
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println("MQTT not Connected");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
}

void sensorDetails() {
  M5.Lcd.fillScreen(ORANGE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("Sensor Address - 0");
  M5.Lcd.setCursor(10, 60);
  M5.Lcd.print("Protocol Version - 1.30");
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.print("Sensor Vendor - METER");
  M5.Lcd.setCursor(10, 140);
  M5.Lcd.print("Sensor Model - ATM41");
  M5.Lcd.setCursor(10, 180);
  M5.Lcd.print("Sensor Version - 529");
  M5.Lcd.setCursor(10, 220);
  M5.Lcd.print("Sensor ID- ATM-410005622");


  M5.Lcd.fillRect(0, 40, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 80, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 160, 320, 4, BLUE); // horizontal line
  M5.Lcd.fillRect(0, 200, 320, 4, BLUE); // horizontal line
}



void setup() {

  M5.begin(true, false, true); //Init M5Core
  M5.IMU.Init();  //Init IMU sensor.

  M5.Lcd.setTextColor(BLACK); //Sets the foreground color and background color of the displayed text.

  M5.Lcd.setTextSize(2);  //Set the font size.
  Serial.begin(SERIAL_BAUD);
  while (!Serial)
    ;
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    M5.Lcd.fillScreen(WHITE);
    M5.Lcd.fillRect(0, 0, 400, 50, BLUE);
    M5.Lcd.setCursor(80, 20);
    M5.Lcd.print("M5-STACK CORE2");
    M5.Lcd.setCursor(10, 140);
    M5.Lcd.print("Connecting to...");
    M5.Lcd.print(ssid);
    Serial.print(".");  // print ... till not connected
    delay(3000);
  }

  M5.Lcd.fillScreen(0);
  Serial.println("");
  Serial.println("WiFi connected to ");
  Serial.println(ssid);
  M5.Lcd.fillRect(0, 0, 350, 135, GREEN);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.print("Wifi connected !");


  Serial.println("Opening SDI-12 bus...");
  M5.Lcd.fillRect(0, 120, 350, 135, YELLOW);
  M5.Lcd.setCursor(10, 170);
  M5.Lcd.print("Opening SDI-12 bus !");
  mySDI12.begin();

  M5.Lcd.fillRect(0, 120, 320, 4, BLUE); // horizontal line

  delay(5000);  // allow things to settle

  // Quickly Scan the Address Space

  Serial.println("Scanning all addresses, please wait...");

  Serial.println("Sensor Address, Protocol Version, Sensor Vendor, Sensor Model, ");
  Serial.println("Sensor Version, Sensor ID");

  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.fillRect(0, 70, 400, 80, YELLOW);
  M5.Lcd.setCursor(10, 90);
  M5.Lcd.print("Scanning all addresses");
  M5.Lcd.setCursor(10, 120);
  M5.Lcd.print("please wait...");
  delay(2000);

  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.fillRect(0, 70, 400, 50, YELLOW);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.print("Looking for the sensor...");

  sensorDetails();

  for (byte i = 0; i < 62; i++) {
    char addr = decToChar(i);
    if (checkActive(addr)) {
      numSensors++;
      isActive[i] = 1;
      printInfo(addr);
      Serial.println();
    }
  }
  Serial.print("Total number of sensors found:  ");
  Serial.println(numSensors);

  if (numSensors == 0) {
    Serial.println();

    Serial.println("No sensors found, please check connections and restart the Arduino.");
    while (true) {
      delay(10);  // do nothing forever
    }
  }

  Serial.println("Time Elapsed (s), Sensor Address, Est Measurement Time (s), Number Measurements, ");
  Serial.println("Real Measurement Time (ms), Measurement 1, Measurement 2, ... etc.");
  Serial.println("-------------------------------------------------------------------------------");

  homescreen();
}

void loop() {

  unsigned long currentTime = millis();
  unsigned long currentMillis = millis();


  M5.update();

  if ((M5.BtnB.isPressed()) || (M5.BtnB.wasReleased())) {
    homescreen();
    delay(1000);
  }

  else if (M5.BtnA.isPressed()) {
    if (currentMillis - previousMillis >= 3000) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      screen1();
    }
  }

  else if (M5.BtnA.wasReleased()) {
    if (currentMillis - previousMillis >= 3000) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
    }
    screen2();
  }

  else if (M5.BtnC.isPressed()) {
    if (currentMillis - previousMillis >= 3000) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      screen3();
    }
  }

  else if (M5.BtnC.wasReleased()) {
    if (currentMillis - previousMillis >= 3000) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
    }
    screen4();
  }

  if (currentMillis - previousMillis >= 300000) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;// wait ten seconds between measurement attempts.
    MQTT_connect();
    Temperature.publish(temperature);
    atmosphericPressure.publish(atmosphericpressure);
    humiditySensorTemperature.publish(humiditysensortemperature);
    vaporPressure.publish(vaporpressure);
    Precipitation.publish(precipitation);
    Solar.publish(solar);
    xOrientation.publish(xorientation );
    yOrientation .publish(yorientation );
    windDirection.publish(winddirection);
    windSpeed.publish(windspeed);
  }

  if (currentMillis - previousMillis >= interval) {
    //save the last time you blinked the LED
    previousMillis = currentMillis;// wait ten seconds between measurement attempts.
    //   measure one at a time
    for (byte i = 0; i < 62; i++) {
      char addr = decToChar(i);
      if (isActive[i]) {
        //Serial.print(millis() / 1000);
        Serial.print(millis());
        Serial.print(", ");
        getContinuousResults(addr, 4);
        Serial.println();
      }
    }
  }

}
