#include <Wire.h>
#include <BH1750.h>
#include <WiFiNINA.h>

const char* ssid = "OPPO K10";
const char* password = "qwerty2007";

const char* iftttHost = "maker.ifttt.com";
const char* iftttKey = "nvF0N7MSzk6L-QF3c92cXOAtR4JW22OCOkvT1N4jDYS";

BH1750 lightMeter;

WiFiClient client;

const float LIGHT_THRESHOLD = 1000.0;

bool sunlightState = false;

void connectWiFi()
{
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.begin(ssid, password) != WL_CONNECTED)
  {
    delay(2000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendIFTTTEvent(const char* eventName, float lux)
{
  Serial.print("Sending IFTTT event: ");
  Serial.println(eventName);

  if (client.connect(iftttHost, 80))
  {
    String url = "/trigger/";
    url += eventName;
    url += "/with/key/";
    url += iftttKey;

    String body = "{\"value1\":\"";
    body += String(lux, 2);
    body += "\"}";

    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println("Connection: close");
    client.println();
    client.println(body);

    delay(1000);

    while (client.available())
    {
      String response = client.readStringUntil('\n');
      Serial.println(response);
    }

    client.stop();

    Serial.println("IFTTT event sent.");
  }
  else
  {
    Serial.println("Connection to IFTTT failed.");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Wire.begin();

  Serial.println("Starting BH1750...");

  if (!lightMeter.begin())
  {
    Serial.println("BH1750 not detected!");
    
    while (1)
    {
      delay(1000);
    }
  }

  Serial.println("BH1750 ready.");

  connectWiFi();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  float lux = lightMeter.readLightLevel();

  Serial.print("Light Level: ");
  Serial.print(lux);
  Serial.println(" lux");

  if (lux < 0)
  {
    Serial.println("Invalid light reading.");
    delay(5000);
    return;
  }

  bool newSunlightState = lux >= LIGHT_THRESHOLD;

  if (newSunlightState && !sunlightState)
  {
    Serial.println("Sunlight detected!");

    sendIFTTTEvent("sunlight_detected", lux);

    sunlightState = true;
  }

  else if (!newSunlightState && sunlightState)
  {
    Serial.println("Sunlight stopped!");

    sendIFTTTEvent("sunlight_stopped", lux);

    sunlightState = false;
  }

  delay(5000);
}
