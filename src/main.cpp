#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // Nick O'Leary (https://pubsubclient.knolleary.net/)
#include <EEPROM.h>

// Version
#define VERSION 1

// Eingaenge
#define TUER 4
#define KLAPPE 5

// an die eigenen Werte anpassen
const char *ssid = "SSID";
const char *password = "GEHEIM";
const char *mqtt_server = "192.168.xxx.xxx";

WiFiClient espClient;
PubSubClient client(espClient);

int geoeffnetcnt = 0;   // Zaehler fuer die Anzahl von Klappenoeffnungen
boolean offen = false;  // ist die Klappe oder Tuer offen?
uint8 klappe = 0;
uint8 tuer = 0;
uint8 post = 0;

void setup_wifi()
{

  delay(10);
  WiFi.hostname("briefkasten");
  WiFi.begin(ssid, password);

// mit lokalem Netzwerk verbinden
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

// mit dem MQTT-Server verbinden
void reconnect()
{
  while (!client.connected())
  {
    if (!client.connect("BriefkastenClient"))
    {
      delay(500);
    }
  }
}

// Payload erstellen und als String zurueckgeben
String getPayloadString(int raw, float akku, int signal, int klappe, int tuer, int post, int anz)
{
  String payload;

  payload = "{";
  payload += "\"Firmware\":" + String(VERSION) + ",";
  payload += "\"Raw\":" + String(raw) + ",";
  payload += "\"Akku\":" + String(akku) + ",";
  payload += "\"Signal\":" + String(signal) + ",";
  payload += "\"Klappe\":" + String(klappe) + ",";
  payload += "\"Tuer\":" + String(tuer) + ",";
  payload += "\"Post\":" + String(post) + ",";
  payload += "\"Anzahl\":" + String(anz);
  payload += "}";

  return payload;
}

void init()
{
  pinMode(TUER, INPUT_PULLUP);
  pinMode(KLAPPE, INPUT_PULLUP);

  delay(10);

  tuer = !digitalRead(TUER);
  klappe = !digitalRead(KLAPPE);

  // falls die Klappe geoeffnet ist nach kurzer Zeit noch mal abfragen (als debounce)
  if (klappe)
  {
    delay(20);
    klappe = (!digitalRead(KLAPPE));
  }

  // EEPROM initialisieren
  EEPROM.begin(3);

  // mit Wlan verbinden
  setup_wifi();

  // OTA Initialisieren
  ArduinoOTA.setHostname("briefkasten");
  ArduinoOTA.setPassword("geheim");
  ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);

  // zum MQTT-Server verbinden
  if (!client.connected())
  {
    reconnect();
  }

  // bei geloeschtem EEPROM ist alles auf 0xff
  // dann werden die Werte mit 0x00 Initialisiert
  boolean firstrun = EEPROM.read(2);
  if (firstrun != 0)
  {
    EEPROM.write(0, 0);
    EEPROM.write(1, 0);
    EEPROM.write(2, 0);
    EEPROM.commit();
  }
}

void setup()
{
  init();

  // Akkuspannung einlesen
  // !!! ACHTUNG !!! Spannungteiler anpassen (R5, R6)
  int raw = analogRead(A0);
  float volt = raw * (4.3 / 1023.0);

  // wenn die Klappe geoeffnet wurde wird angenommen das Post da ist
  post = klappe;

  // wenn die Tuer geoeffnet wird: Post wieder auf 0 setzen
  if (tuer)
  {
    post = 0;
  }

  offen = klappe || tuer;

  // Payload zum MQTT-Server schicken
  client.publish("briefkasten/status", getPayloadString(raw, volt, WiFi.RSSI(), klappe, tuer, post, geoeffnetcnt).c_str(), true);
  delay(10);

  // wenn die Klappe geoeffnet wurde wird der Zaehler fuer Oeffnungen hoch gezaehlt und im EEPROM gesichert
  if (klappe)
  {
    EEPROM.write(0, 1);
    {
      geoeffnetcnt = EEPROM.read(1);
      geoeffnetcnt++;
      EEPROM.write(1, geoeffnetcnt);
      EEPROM.commit();
    }
  }

  // wurde die Tuer geoeffnet werden die EEPROM-Daten auf 0x00 gesetzt
  if (tuer)
  {
    EEPROM.write(0, 0);
    EEPROM.write(1, 0);
    EEPROM.commit();
    geoeffnetcnt = 0;
  }

  // solange die Klappe oder Tuer geoeffnet ist wird gewartet
  // in dieser Zeit kann ein OTA-Update durchgefuehrt werden
  while (klappe || tuer)
  {
    klappe = !digitalRead(KLAPPE);
    tuer = !digitalRead(TUER);
    ArduinoOTA.handle();
    delay(10);
  }

  // falls die Klappe/Tuer vorher Offen war wird jetzt noch einmal eine Payloadmessage geschickt
  // jetzt ist ja die Klappe/Tuer wieder geschlossen
  if (offen)
  {
    offen = false;
    client.publish("briefkasten/status", getPayloadString(raw, volt, WiFi.RSSI(), klappe, tuer, post, geoeffnetcnt).c_str(), true);
  }
  delay(10);

  ESP.deepSleep(60 * 60e6); // 60e6 = 1 Minute * 60 = 1 Stunde
  delay(200);  // dem ESP noch etwas Zeit geben um in den DeepSleep zu wechseln
}

// die LOOP wird nicht benoetigt
void loop()
{
}
