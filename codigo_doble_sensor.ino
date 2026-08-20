
#include <WiFi.h>
#include <WebServer.h>

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

// Setup

void setup() {
    Serial.begin(57600);
    delay(10);

    // Configuracion de pines
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, OUTPUT);

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