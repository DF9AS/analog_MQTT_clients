#include <Wire.h>
#include <MCP4725.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h> // MQTT Publish nur mit QoS 0 möglich (Speicher ist zu knapp)


// Delta Electronica properties
unsigned int voltage_max = 30;
unsigned int current_max = 5;

// Variables for Analog Inputs connected to Delta Electronica monitor output
unsigned int voltage_sensor;
unsigned int current_sensor;
short voltage_measured;
short current_measured;

// Address of MCP4725 board
MCP4725 DAC(0x62); //0x62 or 0x63

// MQTT Network Parameters
// Set your MAC address and IP address here
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 42, 0, 10); // DHCP possible, but expensive
IPAddress server_ip(192,168,2,161);

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
  // Useful for debugging purposes
  Serial.begin(9600);
  
  // Start the ethernet connection
  Ethernet.begin(mac);              
  
  // Ethernet takes some time to boot!
  delay(3000);                          
 
  // Set the MQTT server to the server stated above ^
  mqttClient.setServer(server_ip, 1883);   
 
  // Attempt to connect to the server with the ID "myClientID"
  if (mqttClient.connect("myClientID")) 
  {
    Serial.println("Connection has been established, well done");
 
    // Establish the subscribe event
    mqttClient.setCallback(subscribeReceive);
  } 
  else 
  {
    Serial.println("Looks like the server connection failed...");
  }

  DAC.begin();
  DAC.setValue(0);

  // Set A0 and A1 as Inputs
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // Set A2 and A3 as Outputs to make them our GND and Vcc,
  //which will power the MCP4725
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  digitalWrite(A2, LOW);//Set A2 as GND
  digitalWrite(A3, HIGH);//Set A3 as Vcc
}

void loop()
{
  // This is needed at the top of the loop!
  mqttClient.loop();
 
  // If control is needed
  // mqttClient.subscribe(topic_prefix + topic_subscribe + topic_suffix_voltage);
 
  // Attempt to publish a value to the topic "MakerIOTopic"

  if(mqttClient.publish(topic_current_publish, "Hello World"))
  {
    Serial.println("Publish message success");
  }
  else
  {
    Serial.println("Could not send message :(");
  }
 
  // Dont overload the server!
  delay(4000);
}

void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  // Print the topic
  Serial.print("Topic: ");
  Serial.println(topic);
 
  // Print the message
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }
 
  // Print a newline
  Serial.println("");
}
