#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>

#define DHTPIN D4     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

#define wifi_ssid "your_wifi_ssid"
#define wifi_password "your_wifi_password"

#define mqtt_server "your_ip_address"
#define mqtt_user "your_username"
#define mqtt_password "your_password"

#define h_topic "/sensor/dht22/humidity"
#define t_topic "/sensor/dht22/temperature_c"
#define f_topic "/sensor/dht22/temperature_f"
#define i_topic "/sensor/dht22/heatindex"

WiFiClient espClient;
PubSubClient client(espClient);

// sleep for this many seconds
const int sleepSeconds = 300;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("d1m_dht");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht.begin();
  Serial.println("Running...");
  }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
      if (client.connect("ESP8266Client")) {
    // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Read values from the sensor:
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float i = dht.computeHeatIndex(t, h, false);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  //Print values to serial and publish to MQTT
  Serial.print("Humidity: ");
  Serial.print(String(h).c_str());
  Serial.println(" %\t");
  client.publish(h_topic, String(h).c_str(), true);
  Serial.print("Temp_C: ");
  Serial.print(t);
  Serial.println(" C ");
  client.publish(t_topic, String(t).c_str(), true);
  Serial.print("Temp_F: ");
  Serial.print(f);
  Serial.println(" F ");
  client.publish(f_topic, String(f).c_str(), true);
  Serial.print("Heatindex: ");
  Serial.print(i);
  Serial.println(" C ");
  client.publish(i_topic, String(i).c_str(), true);
  delay(5000);
  //delay(300000);
  Serial.printf("Sleep for %d seconds\n\n", sleepSeconds);

  // convert to microseconds
  ESP.deepSleep(sleepSeconds * 1000000);
}
