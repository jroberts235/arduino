#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define RINGPIXEL_PIN   6
#define BAR0PIXEL_PIN   12
#define BAR1PIXEL_PIN   13
#define RINGPIXEL_COUNT 24
#define BAR0PIXEL_COUNT 8
#define BAR1PIXEL_COUNT 8

byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0x51, 0x60 }; // Feather-MO-0
//byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0x51, 0x60 }; // Feather-MO-0
//byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4A, 0xD5 }; // Arduino-Ethernet-0

IPAddress mqttServer(10, 0, 0, 1);

Adafruit_NeoPixel ring_0 = Adafruit_NeoPixel(RINGPIXEL_COUNT, RINGPIXEL_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel bar_0 = Adafruit_NeoPixel(BAR0PIXEL_COUNT, BAR0PIXEL_PIN, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel bar_1 = Adafruit_NeoPixel(BAR1PIXEL_COUNT, BAR1PIXEL_PIN, NEO_RGBW + NEO_KHZ800);

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

int showType; // try getting rid of this

void setup()
{
  Serial.begin(115200);

  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);

  Ethernet.begin(mac);
  delay(1500);

  //// Initialize all pixels to 'off'
  ring_0.begin();
  ring_0.show();
  bar_0.begin();
  bar_0.show();
  bar_1.begin();
  bar_1.show();
  //// End

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
  Serial.print(topic);

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

  int showType = root["showType"];
  const char* displayObj = root["displayObj"];
  // const char* displayObj = root["displayObj"];

  // REMOVE BEFORE FLIGHT
  //displayObj = "ring_0";
  //showType = 2;

  startShow(displayObj, showType);
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

constexpr unsigned int str2int(const char* str, int h = 0) {
  return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

void startShow(const char* displayObj, int showType) {
  // for debuging
  Serial.println("~=Inside startShow=~");
  Serial.print("displayObj: ");
  Serial.println(displayObj);
  Serial.print("showType: ");
  Serial.println(showType);

  Adafruit_NeoPixel* obj;

  switch (str2int(displayObj)) {
    case str2int("ring_0"):
      obj = &ring_0;
      Serial.println("obj set to ring_0");
      break;
    case str2int("bar_0"):
      obj = &bar_0;
      Serial.println("obj set to bar_0");
      break;
    case str2int("bar_1"):
      obj = &bar_1;
      Serial.println("obj set to bar_1");
      break;
    default:
      Serial.print("displayObj: default");
  }

  switch (showType) {
    case 0:
      colorWipe(obj->Color(0, 0, 0), 50, obj);    // Black/off
      Serial.println("showType: 0");
    break;
    case 1:
      colorWipe(obj->Color(255, 0, 0), 50, obj);  // Red
      Serial.println("showType: 1");
    break;
    case 2:
      colorWipe(obj->Color(0, 255, 0), 50, obj);  // Green
      Serial.println("showType: 2");
    break;
    case 3:
      colorWipe(obj->Color(0, 0, 255), 50, obj);  // Blue
      Serial.println("showType: 3");
    break;
    default:
      Serial.println("showType: default");
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, Adafruit_NeoPixel* obj) {
  for(uint16_t i=0; i<obj->numPixels(); i++) {
    obj->setPixelColor(i, c);
    obj->show();
    delay(wait);
  }
}
