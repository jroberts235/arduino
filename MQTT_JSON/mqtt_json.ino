#include <SPI.h>
#include <Ethernet.h>
//#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    7 // Arduino-Ethernet-0
//#define PIXEL_PIN    6
#define PIXEL_COUNT 6
//#define PIXEL_COUNT 24
//byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0x51, 0x60 }; // Feather-MO-0
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4A, 0xD5 }; // Arduino-Ethernet-0

IPAddress mqttServer(10, 0, 0, 1);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

int showType = 0;

void setup()
{
  Serial.begin(115200);

  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);

  Ethernet.begin(mac);
  delay(1500);

  // NeoPixel related
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  // End

  Serial.println("Running...");
  Serial.println();

  // Echo local IP address:
  /*Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  */

}

void loop()
{
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Raw: ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  StaticJsonBuffer<200> jsonBuffer;
  char inData[80];

  for(int i = 0; i<length; i++){
    inData[(i)] = (char)payload[i];
  }

  JsonObject& root = jsonBuffer.parseObject(inData);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  for (JsonObject::iterator it=root.begin(); it!=root.end(); ++it)
  {
    Serial.println(it->key);
    Serial.println(it->value.asString());
  }

  //const char* sensor = root["sensor"];
  //long time = root["time"];
  //double latitude = root["data"][0];
  //double longitude = root["data"][1];
//
  //Serial.println(sensor);
  //Serial.println(time);
  //Serial.println(latitude, 6);
  //Serial.println(longitude, 6);

  // {"showType":0}
  // {"showType":1}
  // {"showType":2}
  // {"showType":3}
  // {"showType":4}
  // {"showType":5}
  // {"showType":6}
  // {"showType":7}
  // {"showType":8}
  // {"showType":9}

  startShow(root["showType"]);
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection ");
    Serial.print("to: ");
    Serial.print(mqttServer);
    Serial.print("... ");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...

      char outData[80];
      char outTopic[80];
      char inTopic[80];

      // construct a payload
      String payload = "Hello From: ";
             payload += my_ip_address();
             payload += "\n";
      payload.toCharArray(outData, (payload.length() + 1));
      Serial.print("Payload: ");
      Serial.println(outData);

      // construct the topic name
      String pubTo = my_mac_address();
             pubTo += "out";
      pubTo.toCharArray(outTopic, (pubTo.length() + 1));
      Serial.print("pubTo: ");
      Serial.println(outTopic);

      // send the message
      mqttClient.publish(outTopic, outData);

      // ... and resubscribe
      String subTo = my_mac_address();
             subTo += "in";
      subTo.toCharArray(inTopic, (subTo.length() + 1));

      mqttClient.subscribe(inTopic);
      Serial.print("resubscribed to: ");
      Serial.println(inTopic);

      Serial.println("Done.");

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

String my_mac_address() {
  String address = "";
  for (byte thisByte = 0; thisByte < 6; thisByte++) {
    // print the value of each byte of the MAC address:
    address += String(mac[thisByte], HEX);
    address += ":";
  }
  return address;
}

String my_ip_address() {
  String address = "";
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    address += String(Ethernet.localIP()[thisByte], DEC);
    address += ".";
  }
  return address;
}

void startShow(int i) {
  switch(i){
    case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
            break;
    case 1: colorWipe(strip.Color(255, 0, 0), 50);  // Red
            break;
    case 2: colorWipe(strip.Color(0, 255, 0), 50);  // Green
            break;
    case 3: colorWipe(strip.Color(0, 0, 255), 50);  // Blue
            break;
    case 4: theaterChase(strip.Color(127, 127, 127), 50); // White
            break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50); // Red
            break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 50); // Blue
            break;
    case 7: rainbow(20);
            break;
    case 8: rainbowCycle(20);
            break;
    case 9: theaterChaseRainbow(50);
            break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
