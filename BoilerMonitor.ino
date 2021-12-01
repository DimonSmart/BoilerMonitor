#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include "DebugMacros.h"
#include "consts.h"

DHTesp dhts[3];
const int sensors = 1;


HTTPSRedirect* client = nullptr;

void setup()
{
  Serial.begin(115200);
  Serial.flush();

  client = CreateClient();

  DPRINTLN("Temperature sensors setup begin");
  try
  {
    dhts[0].setup(TEMPERATURE_PIN_1, DHTesp::DHT11);
  }
  catch (...)
  {
    DPRINTLN("Temperature sensors setup ERROR");
  }

  DPRINTLN("Temperature sensors setup end");
}

HTTPSRedirect* CreateClient()
{
  client = new HTTPSRedirect(HTTPS_PORT);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  return client;
}

bool ConnectToWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    DPRINTLN("WiFi Already connected");
    return true;
  }

  DPRINTLN("Connecting to wifi: " + String(SSID));

  WiFi.begin(SSID, PWD);
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(1000 * 15);
    Serial.print(".");
    return false;
  }

  DPRINTLN("");
  // DPRINTLN(String("WiFi connected. IP address: ") + WiFi.localIP());

  return true;
}

bool ConnectToHost()
{
  if (client == nullptr)
  {
    DPRINTLN("No HTTPS client created");
    return false;
  }

  if (client->connected())
  {
    return true;
  }

  for (int i = 0; i < 10; i++)
  {
    int retval = client->connect(HOST, HTTPS_PORT);
    if (retval == 1)
    {
      return true;
    }

    DPRINTLN("Connection to host failed. Retrying...");
  }

  return false;
}

void loop()
{
  if (!ConnectToWiFi())
  {
    return;
  }

  if (!ConnectToHost())
  {
    return;
  }

  String payloadBody = "";
  for (int i = 0; i < sensors; i++)
  {
    float humidity = 0.0, temperature = 0.0;
    try
    {
      temperature = dhts[i].getTemperature();
      humidity = dhts[i].getHumidity();
    }
    catch (...)
    {
      DPRINTLN("Sensors reading error.");
    }

    if (isnan(temperature) || isnan(humidity))
    {
      continue;
    }

    payloadBody = payloadBody +
                  "\"temperature" + String(i + 1) + "\":" + String(temperature, 3) +
                  ",\"humidity" + String(i + 1) + "\":" + String(humidity, 3);
    if (i + 1 < sensors)
    {
      payloadBody = payloadBody + ",";
    }
  }

  if (payloadBody == "")
  {
    DPRINTLN("No data to upload");
    return;
  }

  payloadBody = "{" + payloadBody + "}";
  DPRINTLN(payloadBody);
  client->POST(GOOGLE_SCRIPT_URL, HOST, payloadBody, false);

  delay(60 * 1000);
}
