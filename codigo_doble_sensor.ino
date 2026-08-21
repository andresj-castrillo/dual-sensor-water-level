
#include <WiFi.h>
#include <WebServer.h>
#include <HX711_ADC.h>

// Configuracion WIFI

const char* ssid = "nombre_de_red";
const char* password = "contraseña";

// Configuracion de pines

// Pines para el hx711
const int HX711_dout = 4;
const int HX711_sck = 5;

// Pines para el ultrasonido
const int trigPin = 17; 
const int echoPin = 16; 

// Variables Globales
// Celda
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// Web

// Ultrasonido
float cal_offset = 0.0;
float cal_factor = 1.0;
float sonico_alturaRef = 19.5; 

const int N_MUESTRAS = 7;
float muestras[N_MUESTRAS];

unsigned long lastLog = 0;

// Lecturas y filtado de ultrasonico
float readDistanceRaw() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); 
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

void ordenar(float arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

float getDistanceFiltered() {
  int validas = 0;
  int intentos = 0;
  while (validas < N_MUESTRAS && intentos < 20) {
    float d = readDistanceRaw();
    if (d > 0 && d < 400) {
      muestras[validas] = d;
      validas++;
    }
    intentos++;
    delay(5);
  }
  if (validas == 0) return sonico_alturaRef; 
  ordenar(muestras, validas);
  return muestras[validas / 2]; 
}

float getDistanceCalibrated() {
  float d_fil = getDistanceFiltered();
  return (d_fil * cal_factor) + cal_offset;
}

// Setup
void setup() {
  Serial.begin(57600);
  delay(10);

  // Configuracion de pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, OUTPUT);


  Serial.println("\n==================================================");
  Serial.println("         PANEL CONTROL DUAL DE AGUA - IOT         ");
  Serial.println("==================================================");

  // Inicializacion de galga
  LoadCell.begin();
  LoadCell.setSamplesInUse(64);
  unsigned long stabilizingtime = 2000;
  LoadCell.start(stabilizingtime, false);


  
  // Conexion WIFI
  WiFi.begin(ssid, password);
  Serial.print("Estableciendo conexion Wi-Fi");

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 15) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
}

// Bluce de ejecucion LOOP
void loop() {
  server.handleClient();
  if (millis() - lastLog > 2000) {
    lastLog = millis();
    
    float distS = getDistanceCalibrated();
    float nivelS = sonico_alturaRef - distS;
    if (nivelS < 0 || isnan(nivelS) || distS >= sonico_alturaRef) nivelS = 0;

    Serial.println("==================================================");
    Serial.print("[SONIDO] Dist: "); Serial.print(distS, 1);
    Serial.print(" cm | Nivel Real: "); Serial.print(nivelS, 2); Serial.println(" cm");
  }
}