/*
 Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino WIZnet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 */

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// Set the rosserial socket server IP address
IPAddress rosServer(192, 168, 0, 1);
const uint16_t rosServerPort = 9090;

// Be polite and say hello
char hello[13] = "hello world!";

uint16_t period = 1000;
uint32_t last_time = 0;

EthernetClient client;
void setup()
{
  delay(1000);
  // You can use Ethernet.init(pin) to configure the CS pin
  // Ethernet.init(10);  // Most Arduino shields
  // Ethernet.init(5);   // MKR ETH Shield
  // Ethernet.init(0);   // Teensy 2.0
  // Ethernet.init(20);  // Teensy++ 2.0
  Ethernet.init(15); // ESP8266 with Adafruit FeatherWing Ethernet
  // Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("");

  initDHCP();

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  // listen for incoming clients
  if (millis() - last_time >= period)
  {

    // recieveData();
    delay(1);
  }
}

void recieveData()
{
  EthernetClient client = server.available();
  if (client)
  {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close"); // the connection will be closed after completion of the response
          client.println("Refresh: 5");        // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++)
          {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println(Ethernet.gatewayIP());
          client.println("<br />");
          client.println(Ethernet.dnsServerIP());
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void initDHCP()
{
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
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
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }
  else
  {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void connectRos()
{
  // close any connection before send a new request.
  // This will free the socket on the Ethernet shield

  client.stop();

  Serial.println("connecting...");
  if (!client.connect(rosServer, rosServerPort))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  // if there's a successful connection:

  // send the HTTP GET request:
  // client.println("GET /example.json HTTP/1.1 200 OK");
  // client.print("Host: arduinojson.org");
  client.println("User-Agent: arduino-ethernet");
  client.println("Connection: keep-alive");
  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    client.stop();
    return;
  }
  // client.println(millis());
  client.println("{ \"op\": \"publish\", \"topic\": \"test\",\"msg\": \"hello form esp8266\"}: ");
  // client.println("");
  // client.println();

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    client.stop();
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    client.stop();
    return;
  }

  // Allocate the JSON document
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    client.stop();
    return;
  }

  // Extract values
  Serial.println(F("Response:"));
  Serial.println(doc["sensor"].as<const char *>());
  Serial.println(doc["time"].as<long>());
  Serial.println(doc["data"][0].as<float>(), 6);
  Serial.println(doc["data"][1].as<float>(), 6);

  // Disconnect
  client.stop();
}
