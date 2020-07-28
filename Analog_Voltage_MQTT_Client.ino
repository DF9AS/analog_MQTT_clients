#include <MCP4725.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h> // MQTT Publish nur mit QoS 0 möglich (Speicher ist zu knapp)

// Delta Electronica properties
char voltage_max = 30;
char current_max = 5;

// Variables for Analog Inputs connected to Delta Electronica monitor output
float voltage_sensor;
float current_sensor;

//Variables for Analog Outputs
short voltage_output;
short current_output;

char voltage_sensor_str[8];
char current_sensor_str[8];

// Address of MCP4725 board
MCP4725 dac(0x60); //0x60 default address

// MQTT Network Parameters
// Set your MAC address and IP address here
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(10, 42, 0, 10); // DHCP possible, but expensive
IPAddress server_ip(192, 168, 2, 161);

// Set MQTT Topic to publish and subscribe
char topic_current_publish[] = "mbe/load-lock-lamp/current/update";
char topic_voltage_publish[] = "mbe/load-lock-lamp/voltage/update";
char topic_current_subscribe[] = "mbe/load-lock-lamp/current/targetstate";
char topic_voltage_subscribe[] = "mbe/load-lock-lamp/voltage/targetstate";

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void setup()
{
  // Start the ethernet connection
  Ethernet.begin(mac);

  // Ethernet takes some time to boot!
  delay(3000);

  // Set the MQTT server to the server stated above ^
  mqttClient.setServer(server_ip, 1883);

  // Attempt to connect to the server with the ID "myClientID"
  mqttClient.connect("LoadLockClient");
  mqttClient.setCallback(subscribeReceive);

  // Initialize DAC
  dac.begin();
  dac.setValue(0);

   // If control is needed
  mqttClient.subscribe(topic_current_subscribe, 1);
  mqttClient.subscribe(topic_voltage_subscribe, 1);

}

void loop()
{
  // This is needed at the top of the loop!
  mqttClient.loop();

  // Attempt to publish a value to the topics
  voltage_sensor = analogRead(0) * (voltage_max / 1023.0);
  dtostrf(voltage_sensor, 6,2, voltage_sensor_str);
  mqttClient.publish(topic_voltage_publish, voltage_sensor_str);

  current_sensor = analogRead(1) * (current_max / 1023.0);
   dtostrf(current_sensor, 6,2, current_sensor_str);
  mqttClient.publish(topic_current_publish, current_sensor_str);

  // Approx. Sending Intervall
  delay(1000);
}

void subscribeReceive(char *topic, byte *payload, unsigned int length)
{
  String str_message = "";
  for (int i=0;i<length;i++) {
    str_message += ((char)payload[i]);
  }

  if (strcmp(topic, topic_current_subscribe) == 0) {
    current_output = (int)(str_message.toFloat() * (4095.0 / current_max));
    dac.setValue(current_output);
  }
  else if (strcmp(topic,topic_voltage_subscribe)==0) {
    voltage_output = (int)(str_message.toFloat() * (4095.0 / voltage_max));
    //dac.setValue(voltage_output)
  }
}