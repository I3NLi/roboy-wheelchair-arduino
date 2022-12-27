/*
 * rosserial Publisher Example
 * Prints "hello world!"
 * This intend to connect to an Arduino Ethernet Shield
 * and a rosserial socket server.
 * You can launch the rosserial socket server with
 * roslaunch rosserial_server socket.launch
 * The default port is 11411
 *
 */

#include <SPI.h>
#include <Ethernet.h>

// To use the TCP version of rosserial_arduino
#define ROSSERIAL_ARDUINO_TCP

#include <ros.h>
#include <std_msgs/String.h>

// Set the shield settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);

// Set the rosserial socket server IP address
IPAddress server(192, 168, 0, 1);
// IPAddress server(192, 168, 3, 5);

// Set the rosserial socket server port
const uint16_t serverPort = 11411;

ros::NodeHandle nh;
// Make a chatter publisher
std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);

// Be polite and say hello
char hello[13] = "hello world!";
uint16_t period = 1000;
uint32_t last_time = 0;

void setup()
{
  // You can use Ethernet.init(pin) to configure the CS pin
  // Ethernet.init(10);  // Most Arduino shields
  // Ethernet.init(5);   // MKR ETH Shield
  // Ethernet.init(0);   // Teensy 2.0
  // Ethernet.init(20);  // Teensy++ 2.0
  Ethernet.init(15); // ESP8266 with Adafruit FeatherWing Ethernet
  // Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // Use serial to monitor the process
  Serial.begin(115200);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Serial connected");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }
  // Let some time for the Ethernet Shield to be initialized
  delay(1000);

  Serial.println("");
  Serial.println("Ethernet connected");
  Serial.println("IP address: ");
  Serial.println(Ethernet.localIP());

  // Set the connection to rosserial socket server
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();

  // Another way to get IP
  Serial.print("IP = ");
  Serial.println(nh.getHardware()->getLocalIP());

  // Start to be polite
  nh.advertise(chatter);
}

void loop()
{
  if (nh.connected())
  {
    Serial.println("Connected");
    // Say hello
    str_msg.data = hello;
    chatter.publish(&str_msg);
  }
  else
  {
    Serial.println("Not Connected");
  }
  nh.spinOnce();
  // Loop exproximativly at 1Hz
  delay(period);
}
