#include <DHT.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#define brocheDeBranchementDHT 4    // La ligne de communication du DHT22 sera donc branchée sur la pin D6 de l'Arduino
#define typeDeDHT DHT22             // Ici, le type de DHT utilisé est un DHT22 (que vous pouvez changer en DHT11, DHT21, ou autre, le cas échéant)

// Instanciation de la librairie DHT
DHT dht(brocheDeBranchementDHT, typeDeDHT);

const char* ssid = "Invite-ESIEA";
const char* password = "hQV86deaazEZQPu9a";
// const char* mqttServer = "fadb89c7ba204c94a8431ce6bc100de0.s2.eu.hivemq.cloud";
// const int mqttPort = 8884;
// const char* mqttUser = "admin";
// const char* mqttPassword = "adminadmin";
// const char* topic = "hello";

// const char* ssid = "Network_Machine";
// const char* password = "Network_Machine";
const char* mqttServer = "10.8.128.250";
const int mqttPort = 1883;
// const char* mqttUser = "admin";
// const char* mqttPassword = "adminadmin";
const char* topicHumidity = "aslan/humidity";
const char* topicTemperature = "aslan/temperature";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ========================
// Initialisation programme
// ========================
void setup () {
  
  // Initialisation de la liaison série (pour retourner les infos au moniteur série de l'ordi)
  Serial.begin(9600);

  connectToWiFi();
  client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);

  Serial.println("Programme de test du DHT22");
  Serial.println("==========================");
  Serial.println();

  // Initialisation du DHT22;
  dht.begin();
}
 
// =================
// Boucle principale
// =================
void loop () {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Lecture des données
  float tauxHumidite = dht.readHumidity();              // Lecture du taux d'humidité (en %)
  float temperatureEnCelsius = dht.readTemperature();   // Lecture de la température, exprimée en degrés Celsius

  // Vérification si données bien reçues
  if (isnan(tauxHumidite) || isnan(temperatureEnCelsius)) {
    Serial.println("Aucune valeur retournée par le DHT22. Est-il bien branché ?");
    delay(2000);
    return;         // Si aucune valeur n'a été reçue par l'Arduino, on attend 2 secondes, puis on redémarre la fonction loop()
  }



  // Calcul de la température ressentie
  float temperatureRessentieEnCelsius = dht.computeHeatIndex(temperatureEnCelsius, tauxHumidite, false); // Le "false" est là pour dire qu'on travaille en °C, et non en °F
  
  // Affichage des valeurs
  Serial.print("Humidité = "); Serial.print(tauxHumidite); Serial.println(" %");
  Serial.print("Température = "); Serial.print(temperatureEnCelsius); Serial.println(" °C");
  Serial.print("Température ressentie = "); Serial.print(temperatureRessentieEnCelsius); Serial.println(" °C");
  Serial.println();

  DynamicJsonDocument jsonTemp(1024);
  DynamicJsonDocument jsonHum(1024);
  jsonTemp["temperature"] = temperatureEnCelsius;
  jsonHum["humidity"] = tauxHumidite;
  //json["temp_feel"] = temperatureRessentieEnCelsius;
  String jsonStringTemp;
  String jsonStringHum;
  serializeJson(jsonTemp, jsonStringTemp);
  serializeJson(jsonHum, jsonStringHum);
  client.publish(topicTemperature, jsonStringTemp.c_str());
  client.publish(topicHumidity, jsonStringHum.c_str());
  delay(1000);
  
  // Temporisation de 2 secondes (pour rappel : il ne faut pas essayer de faire plus d'1 lecture toutes les 2 secondes, avec le DHT22, selon le fabricant)
  delay(2000);
}





void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void reconnect() {
  while (!client.connected()) {
    // if (client.connect("your_mqtt_client_id", mqttUser, mqttPassword)) {
    if (client.connect(mqttServer)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}




