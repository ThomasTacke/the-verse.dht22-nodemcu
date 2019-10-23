#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
// Update these with values suitable for your network.
#include "../include/settings.h"

#ifdef TLS
WiFiClientSecure espClient;
static unsigned int const mqttPort = 8883;
#else
WiFiClient espClient;
static unsigned int const mqttPort = 1883;
#endif

#define DHTPIN 2
#define DHTTYPE DHT22

PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

const long interval = 60000; // 1 minute
unsigned long previousMillis = 0;
unsigned long currentMillis = millis();
char stgFromFloat[10];

void callback(char *topic, byte *payload, unsigned int length)
{
#ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
#endif
    char message[length];
    for (unsigned int i = 0; i < length; i++)
    {
#ifdef DEBUG
        Serial.print((char)payload[i]);
#endif
        message[i] = (char)payload[i];
    }
#ifdef DEBUG
    Serial.println();
#endif
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
#ifdef DEBUG
        Serial.print("Attempting MQTT connection...");
#endif
        // Attempt to connect
        // client.connect(clientId.c_str(), mqttUser.c_str(), mqttPassword.c_str()); // User Auth
        if (client.connect(clientId.c_str()))
        {
#ifdef DEBUG
            Serial.println("connected");
#endif
            // Once connected, publish an announcement...
            // client.publish("outTopic", "hello world");
            // ... and resubscribe
            // client.subscribe(lights433topic.c_str());
        }
        else
        {
#ifdef DEBUG
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
#endif
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void reconnectWiFi()
{
// We start by connecting to a WiFi network
#ifdef DEBUG
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
#ifdef DEBUG
        Serial.print(".");
#endif
    }

#ifdef DEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IPv4 address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
#endif
}

void setup()
{
    pinMode(internalLED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    Serial.begin(115200);

    // Start sensor
    dht.begin();

    delay(10);
// We start by connecting to a WiFi network
#ifdef DEBUG
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
#ifdef DEBUG
        Serial.print(".");
#endif
    }

#ifdef DEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IPv4 address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
#endif

    client.setServer(server, mqttPort);
    client.setCallback(callback);

    // Turn the internal LED off by making the voltage HIGH
    digitalWrite(internalLED, HIGH);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        reconnectWiFi();
    }
    else if (!client.connected())
    {
        reconnect();
    }

    currentMillis = millis();

    if ((currentMillis - previousMillis >= (interval * 5)) || (previousMillis == 0)) // interval is 1 minute. we want to publish every 5 minutes
    {
        previousMillis = currentMillis;
#ifdef DEBUG
        Serial.print("Temp: ");
        Serial.print(dht.readTemperature());
        Serial.print("\t\tHum: ");
        Serial.println(dht.readHumidity());
#endif
        dtostrf(dht.readTemperature(), 4, 2, stgFromFloat);
        client.publish("the-verse/kitchen/temperature", stgFromFloat, true);
        dtostrf(dht.readHumidity(), 4, 2, stgFromFloat);
        client.publish("the-verse/kitchen/humidity", stgFromFloat, true);
    }

    client.loop();
}
