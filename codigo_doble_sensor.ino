
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

// Direcciones EEPROM para mapeo de memoria

const int calVal_eepromAdress = 0;   // float (4 bytes)
const int radio_eepromAdress  = 4;   // float (4 bytes)
const int altura_eepromAdress = 8;   // float (4 bytes)
const int EEPROM_OFFSET_ADDR  = 12;  // float (4 bytes)
const int EEPROM_FACTOR_ADDR  = 16;  // float (4 bytes)
const int EEPROM_ALT_REF_ADDR = 20;  // float (4 bytes)

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

// Calibracion secuemcial obligatorio en consola
void cambiarDimensionesGalga() {
  Serial.println("\n--- CONFIGURACION DE DIMENSIONES DEL RECIPIENTE (GALGA) ---");
  Serial.println("Ingrese el radio interno en cm:");
  while (!Serial.available());
  float r_temp = Serial.parseFloat();
  if(r_temp > 0) radio_cm = r_temp;
  Serial.print("Radio fijado: "); Serial.print(radio_cm); Serial.println(" cm");

  while (Serial.available()) Serial.read(); 

  Serial.println("Ingrese la altura interna en cm:");
  while (!Serial.available());
  float h_temp = Serial.parseFloat();
  if(h_temp > 0) altura_cm = h_temp;
  Serial.print("Altura fijada: "); Serial.print(altura_cm); Serial.println(" cm");

  EEPROM.put(radio_eepromAdress, radio_cm);
  EEPROM.put(altura_eepromAdress, altura_cm);
  EEPROM.commit();
}

void calibrarGalga() {
  Serial.println("\n--- CALIBRACION OBLIGATORIA DE LA GALGA ---");
  Serial.println("1. Retire todo el peso de la balanza.");
  Serial.println("Envie 't' por consola para establecer el TARE:");
  
  boolean _resume = false;
  while (!_resume) {
    LoadCell.update();
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') LoadCell.tareNoDelay();
    }
    if (LoadCell.getTareStatus()) _resume = true;
  }

  while (Serial.available()) Serial.read();

  Serial.println("\n2. Coloque un peso conocido sobre la galga.");
  Serial.println("Ingrese el valor exacto de ese peso en gramos:");
  float known_mass = 0;
  while (known_mass == 0) {
    LoadCell.update();
    if (Serial.available() > 0) known_mass = Serial.parseFloat();
  }
  
  LoadCell.refreshDataSet();
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
  
  if(isnan(newCalibrationValue) || newCalibrationValue == 0) newCalibrationValue = 1.0;
  
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  
  LoadCell.setCalFactor(newCalibrationValue);
  Serial.print("Factor de calibración guardado: "); Serial.println(newCalibrationValue);
}

void calibrarUltrasonico() {
  while (Serial.available() > 0) Serial.read();
  delay(100);

  Serial.println("\n--- CALIBRACION OBLIGATORIA DEL ULTRASONICO ---");
  Serial.println("1. Coloque el sensor apuntando a una distancia CONOCIDA fija.");
  Serial.println("Ingrese esa distancia patrón real en cm:");
  
  while (Serial.available() == 0) {
    delay(10); 
  }
  
  float d_real = Serial.parseFloat();
  Serial.print("Distancia real patrón registrada: "); Serial.print(d_real); Serial.println(" cm");

  while (Serial.available() > 0) Serial.read();

  Serial.println("Midiendo distancia actual del sensor...");
  float d_medida = getDistanceFiltered();
  Serial.print("Distancia medida bruta: "); Serial.print(d_medida); Serial.println(" cm");

  cal_offset = d_real - d_medida;
  cal_factor = 1.0; 

  Serial.println("\n2. Ingrese la Altura de Referencia del Tanque Vacio (cm) [Ej: 19.5]:");
  while (Serial.available() == 0) {
    delay(10); 
  }
  
  float alt_temp = Serial.parseFloat();
  if(alt_temp > 0) sonico_alturaRef = alt_temp;
  Serial.print("Nueva Altura de Referencia fijada a: "); Serial.print(sonico_alturaRef); Serial.println(" cm");

  EEPROM.put(EEPROM_OFFSET_ADDR, cal_offset);
  EEPROM.put(EEPROM_FACTOR_ADDR, cal_factor);
  EEPROM.put(EEPROM_ALT_REF_ADDR, sonico_alturaRef);
  EEPROM.commit();

  while (Serial.available() > 0) Serial.read();
  Serial.println("✓ Calibración de Ultrasonico almacenada con éxito.");
}

// Setup
void setup() {
  Serial.begin(57600);
  delay(10);

  // Configuracion de pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, OUTPUT);

  EEPROM.begin(512); 

  Serial.println("\n==================================================");
  Serial.println("         PANEL CONTROL DUAL DE AGUA - IOT         ");
  Serial.println("==================================================");

  // Inicializacion de galga
  LoadCell.begin();
  LoadCell.setSamplesInUse(64);
  unsigned long stabilizingtime = 2000;
  LoadCell.start(stabilizingtime, false);

  // Carga de datos preventivos desde EEPROM
  float f_galga, f_offset, f_cal_fact, f_alt_ref;
  EEPROM.get(calVal_eepromAdress, f_galga);
  EEPROM.get(EEPROM_OFFSET_ADDR, f_offset);
  EEPROM.get(EEPROM_FACTOR_ADDR, f_cal_fact);
  EEPROM.get(EEPROM_ALT_REF_ADDR, f_alt_ref);
  
  // Ejecución de calibraciones secuenciales
  cambiarDimensionesGalga();
  calibrarGalga();
  calibrarUltrasonico();

  if(!isnan(f_offset)) cal_offset = f_offset;
  if(!isnan(f_cal_fact) && f_cal_fact > 0) cal_factor = f_cal_fact;
  if(!isnan(f_alt_ref) && f_alt_ref > 0) sonico_alturaRef = f_alt_ref;


  // Conexion WIFI
  WiFi.begin(ssid, password);
  Serial.print("Estableciendo conexion Wi-Fi");

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 15) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  // Establecer el factor final obtenido de la calibración actual
  if(isnan(f_galga) || f_galga == 0) f_galga = 1.0; 
  LoadCell.setCalFactor(f_galga);
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