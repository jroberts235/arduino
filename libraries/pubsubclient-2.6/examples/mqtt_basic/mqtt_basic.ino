
#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <ArduinoJson.h>


byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0x51, 0x60 };

// ip of MQTT server
IPAddress server(10, 0, 0, 1);

EthernetClient ethClient;
PubSubClient client(ethClient);

// char json[80];
StaticJsonBuffer<200> jsonBuffer;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Raw: ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  jsonParse(payload, length);
}


void jsonParse(byte* payload, unsigned int length) {
    char json[80];

    for(int i = 0; i<length; i++){
      json[(i)] = char(payload[i]);
    }

    JsonObject& root = jsonBuffer.parseObject(json);

    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    const char* sensor = root["sensor"];
    long time = root["time"];
    double latitude = root["data"][0];
    double longitude = root["data"][1];

    Serial.println(sensor);
    Serial.println(time);
    Serial.println(latitude, 6);
    Serial.println(longitude, 6);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection ");
    Serial.print("@ ");
    Serial.print(server);
    Serial.print("... ");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);
  // Allow the hardware to sort itself out
  delay(1500);

  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
