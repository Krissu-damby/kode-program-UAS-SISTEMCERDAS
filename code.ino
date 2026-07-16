#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

const char* WIFI_SSID = "HAHAY";
const char* WIFI_PASS = "Konveksiaja12!";
WiFiClient netClient;

unsigned long tsChannel = 3410197;
const char* tsWriteKey = "M6ZKK3JU4W5S42NM";

const int PIN_DHT = D4;
const int PIN_TRIG = D6;
const int PIN_ECHO = D5;
const int PIN_LED_A = D0; 
const int PIN_LED_B = D2; 

DHT sensorSuhu(PIN_DHT, DHT11);

float dapatkanJarakCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long jedaPulsa = pulseIn(PIN_ECHO, HIGH, 30000);
  
  if (jedaPulsa == 0) return -1.0;
  
  return (jedaPulsa * 0.0343) / 2.0;
}

void kendaliIndikator(float rentangJarak) {
  if (rentangJarak < 0) return; 

  if (rentangJarak <= 30.0) {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, HIGH);
  } 
  else if (rentangJarak <= 50.0) {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, LOW);
  } 
  else {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
  }
}

void unggahKeCloud(float tC, float tF, float hum, float dist) {
  ThingSpeak.setField(1, tC);
  ThingSpeak.setField(2, tF);
  ThingSpeak.setField(3, hum);
  ThingSpeak.setField(4, dist);
  
  int statusCode = ThingSpeak.writeFields(tsChannel, tsWriteKey);
  
  if (statusCode == 200) {
    Serial.println("[CLOUD] Sinkronisasi berhasil.");
  } else {
    Serial.printf("[CLOUD] Gagal mengirim data. Kode Error: %d\n", statusCode);
  }
}

void setup() {
  Serial.begin(115200);
  sensorSuhu.begin();
  
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  
  digitalWrite(PIN_LED_A, LOW);
  digitalWrite(PIN_LED_B, LOW);
  
  Serial.print("\nMenghubungkan ke Access Point");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("#");
  }
  
  Serial.printf("\n[WIFI] Terkoneksi. IP: %s\n", WiFi.localIP().toString().c_str());
  ThingSpeak.begin(netClient);
}

void loop() {
  float kelembaban = sensorSuhu.readHumidity();
  float celcius = sensorSuhu.readTemperature();
  float fahrenheit = sensorSuhu.readTemperature(true);
  float jarakSaatIni = dapatkanJarakCm();

  if (isnan(kelembaban) || isnan(celcius)) {
    Serial.println("[WARNING] Sensor DHT tidak terbaca atau bermasalah!");
    delay(5000);
    return;
  }

  kendaliIndikator(jarakSaatIni);

  Serial.println("\n--- [TELEMETRI SENSOR] ---");
  Serial.printf("Suhu Ruangan  : %.2f °C | %.2f °F\n", celcius, fahrenheit);
  Serial.printf("Kelembaban Air: %.2f %%\n", kelembaban);
  Serial.printf("Jarak Objek   : %.2f cm\n", jarakSaatIni);
  Serial.println("--------------------------");

  unggahKeCloud(celcius, fahrenheit, kelembaban, jarakSaatIni);

  delay(5000);
}