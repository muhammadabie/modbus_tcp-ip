#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include "DHT.h"

// ================== Konfigurasi PIN ==================
#define DHTPIN 14
#define DHTTYPE DHT11
#define RELAY1_PIN 25
#define RELAY2_PIN 26
#define TOUCH_PIN 4       // GPIO 4 untuk touch sensor

// ================== Alamat Register Modbus ==================
#define TEMPERATURE_ADDRESS 100
#define HUMIDITY_ADDRESS 101
#define RELAY1_ADDRESS 102
#define RELAY2_ADDRESS 103
#define TOUCH_ADDRESS 104  // register baru untuk touch sensor

// ================== Konfigurasi WiFi ==================
const char* ssid = "Ceponnn2";
const char* pass = "cepon555";

IPAddress local_IP(192, 168, 144, 113);
IPAddress gateway(192, 168, 144, 80);
IPAddress subnet(255, 255, 255, 0);

// ================== Objek DHT dan Modbus ==================
DHT dht(DHTPIN, DHTTYPE);
ModbusIP mb;

// ================== Konfigurasi Touch ==================
const int TOUCH_THRESHOLD = 1000;  // semakin kecil nilainya, semakin sensitif

void setup() {
  Serial.begin(115200);
  Serial.println(F("MODBUS TCP OVER WIFI ESP32"));

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Inisialisasi DHT & Relay
  dht.begin();
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Pastikan relay OFF saat awal
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);

  // Setup Modbus Server
  mb.server();

  // Tambahkan register Modbus
  mb.addHreg(TEMPERATURE_ADDRESS);
  mb.addHreg(HUMIDITY_ADDRESS);
  mb.addHreg(RELAY1_ADDRESS);
  mb.addHreg(RELAY2_ADDRESS);
  mb.addHreg(TOUCH_ADDRESS);  // register untuk touch sensor

  // Set nilai awal
  mb.Hreg(RELAY1_ADDRESS, 0);
  mb.Hreg(RELAY2_ADDRESS, 0);
  mb.Hreg(TOUCH_ADDRESS, 0);
}

void loop() {
  // === Baca sensor DHT11 ===
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (!isnan(humidity) && !isnan(temperature)) {
    mb.Hreg(TEMPERATURE_ADDRESS, temperature * 10);
    mb.Hreg(HUMIDITY_ADDRESS, humidity * 10);
  }

  // === Baca Touch Sensor ===
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  // Jika nilai lebih kecil dari threshold → disentuh
  if (touchValue < TOUCH_THRESHOLD) {
    mb.Hreg(TOUCH_ADDRESS, 1);
  } else {
    mb.Hreg(TOUCH_ADDRESS, 0);
  }

  // Jalankan Modbus
  mb.task();

  // === Logika Relay Normal ===
  bool relay1_state = (mb.Hreg(RELAY1_ADDRESS) == 1) ? LOW : HIGH;
  bool relay2_state = (mb.Hreg(RELAY2_ADDRESS) == 1) ? LOW : HIGH;

  digitalWrite(RELAY1_PIN, relay1_state);
  digitalWrite(RELAY2_PIN, relay2_state);

  delay(100);
}
