
#include <Ethernet.h>
#include <MCP4725.h>
#include <Wire.h>
#include <SPI.h>
#include <PubSubClient.h> // MQTT Publish nur mit QoS 0 möglich (Speicher ist zu knapp)
#include <ADS1115_WE.h>
#include <ArduinoJson.h>


const bool DAC_ENABLED = true;

// Delta Electronica properties
const char voltage_max = 70;
const char voltage_max_measure = 5;
const char current_max = 20;
const char current_max_measure = 5;

//Variables for Analog Outputs
short voltage_output;
short current_output;

// MQTT Network Parameters
// Set your MAC address and IP address here
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
const IPAddress ip(192, 168, 2, 230); // DHCP possible, but expensive
const IPAddress server_ip(192, 168, 2, 161);

// Set MQTT Topic to publish and subscribe
const char topic_publish[] = "mbe/loadlock-lamp/update";
const char topic_subscribe[] = "mbe/loadlock-lamp/targetstate";
const char topic[] = "mbe/loadlock-lamp";


// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient;

// 188uV/bit
ADS1115_WE adc(0x48);


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Connecting...");
  // Start the ethernet connection
  Ethernet.begin(mac);

  // Ethernet takes some time to boot!
  delay(3000);
  
  Serial.println("Ethernet connected");

  if(!adc.init()){
    Serial.println("ADS1115 not connected!");
  }

  // Set the MQTT server to the server stated above ^
  mqttClient.setClient(ethClient);
  mqttClient.setServer(server_ip, 1883);

  Serial.println("MQTT Server connected");

  // Attempt to connect to the server with the ID "myClientID"
  mqttClient.connect("sputter-sample");
  mqttClient.setCallback(subscribeReceive);

  if (DAC_ENABLED) {
    mqttClient.subscribe(topic_subscribe, 1);
    Serial.println("MQTT topic subscribed");

    // Address of MCP4725 board
    MCP4725 dac(0x60); //0x60 default address
    // Initialize DAC
    dac.begin();
    dac.setValue(0);
    Serial.println("DAC Initialized");
  }
}

void loop()
{
  // This is needed at the top of the loop!
  mqttClient.loop();
  Ethernet.maintain(); 
  float voltage_sensor = 0.0;
  float current_sensor = 0.0;
  float voltage_measured;
  float current_measured;
  char buffer[256];

  //JSON properties
  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;

  Serial.println("Start Measurement");
  // Attempt to publish a value to the topics
  adc.setVoltageRange_mV(ADS1115_RANGE_6144);
  adc.setCompareChannels(ADS1115_COMP_0_GND);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage_sensor = adc.getResult_V();

  adc.setCompareChannels(ADS1115_COMP_1_GND);
  adc.startSingleMeasurement();
  current_sensor = adc.getResult_V();
  while(adc.isBusy()){}
  current_sensor = adc.getResult_V();
  Serial.println("Finished Measurement");

  doc["topic"] = topic;
  doc["voltage"] = voltage_sensor / voltage_max_measure * voltage_max;
  doc["current"] = current_sensor / current_max_measure * current_max;

  serializeJsonPretty(doc, buffer);
  mqttClient.publish(topic_publish, buffer);
  Serial.println(buffer);

  // Approx. Sending Intervall
  delay(1000);
}

void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;
  deserializeJson(doc, payload, length);
}