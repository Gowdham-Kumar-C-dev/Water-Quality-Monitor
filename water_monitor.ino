/*
 * ============================================================
 *   Smart Water Quality Monitoring System
 *   Author  : Gowdham Kumar C
 *   College : Sri Krishna Arts and Science College, Coimbatore
 *   Role    : IoT Head — Google On Campus, SKASC
 * ============================================================
 *
 *  Hardware:
 *    - ESP32 Dev Module
 *    - TDS Sensor       → GPIO 34
 *    - Water Flow Sensor → GPIO 27
 *    - Servo Motor      → GPIO 14
 *
 *  Features:
 *    - Real-time TDS measurement (ppm)
 *    - Water flow rate & total volume tracking
 *    - Automatic servo valve control on contamination
 *    - Firebase RTDB upload (enable by uncommenting)
 *    - CSV serial output for data logging
 */

#include <WiFi.h>
#include <ESP32Servo.h>
// #include <Firebase_ESP_Client.h>   // Uncomment to enable Firebase

// ─────────────────────────────────────────────
//  CONFIGURATION
// ─────────────────────────────────────────────
#define WIFI_SSID       "your_wifi_name"
#define WIFI_PASSWORD   "your_wifi_password"

// Firebase — fill these in if using cloud upload
// #define API_KEY       "your_firebase_api_key"
// #define DATABASE_URL  "your_project.firebasedatabase.app"

// ─────────────────────────────────────────────
//  PIN DEFINITIONS
// ─────────────────────────────────────────────
#define TDS_PIN   34    // Analog input
#define FLOW_PIN  27    // Digital interrupt input
#define SERVO_PIN 14    // PWM output

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
#define VREF      3.3
#define ADC_RES   4095.0

const unsigned long READ_INTERVAL   = 5000;   // ms between readings
const unsigned long VALVE_RECHECK   = 10000;  // ms before re-checking valve

// ─────────────────────────────────────────────
//  GLOBALS
// ─────────────────────────────────────────────
// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;

volatile byte pulseCount      = 0;
float calibrationFactor       = 4.5;
float totalLiters             = 0;
float flowRate                = 0;

Servo valveServo;
bool  valveClosed             = false;
unsigned long valveCloseTime  = 0;
unsigned long previousMillis  = 0;

// ─────────────────────────────────────────────
//  INTERRUPT — Flow Sensor Pulse Counter
// ─────────────────────────────────────────────
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// ─────────────────────────────────────────────
//  READ TDS SENSOR
// ─────────────────────────────────────────────
float readTDS() {
  int   raw     = analogRead(TDS_PIN);
  float voltage = raw * VREF / ADC_RES;

  // Polynomial conversion: voltage → TDS (ppm)
  float tds = (133.42 * pow(voltage, 3)
             - 255.86 * pow(voltage, 2)
             + 857.39 * voltage) * 0.5;

  return tds;
}

// ─────────────────────────────────────────────
//  VALVE CONTROL
// ─────────────────────────────────────────────
void closeValve() {
  valveServo.write(0);
  valveClosed    = true;
  valveCloseTime = millis();
  Serial.println("[VALVE] CLOSED — Contaminated water detected!");
}

void openValve() {
  valveServo.write(90);
  valveClosed = false;
  Serial.println("[VALVE] OPENED — Water is safe.");
}

// ─────────────────────────────────────────────
//  CLASSIFY WATER QUALITY
// ─────────────────────────────────────────────
String classifyWater(float tds) {
  if (tds < 50)         return "POISON";
  else if (tds <= 300)  return "SAFE";
  else if (tds <= 500)  return "MODERATE";
  else                  return "CONTAMINATED";
}

// ─────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Flow sensor interrupt
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);

  // ADC config for TDS
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Servo init — open valve on startup
  valveServo.attach(SERVO_PIN);
  openValve();

  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

  // Firebase init (uncomment to enable)
  // config.api_key    = API_KEY;
  // config.database_url = DATABASE_URL;
  // if (Firebase.signUp(&config, &auth, "", "")) {
  //   Serial.println("Firebase SignUp OK");
  // }
  // Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);

  // Print CSV header
  Serial.println("timestamp_ms,flowRate_Lmin,totalLiters,tds_ppm,status");
}

// ─────────────────────────────────────────────
//  MAIN LOOP
// ─────────────────────────────────────────────
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= READ_INTERVAL) {

    // ── Flow Calculation ──────────────────────
    flowRate    = pulseCount / calibrationFactor;   // L/min
    pulseCount  = 0;
    totalLiters += flowRate / 60.0;                 // convert to liters

    // ── TDS Reading & Classification ──────────
    float  tds         = readTDS();
    String waterStatus = classifyWater(tds);

    // ── Valve Logic ───────────────────────────
    if ((waterStatus == "POISON" || waterStatus == "CONTAMINATED") && !valveClosed) {
      closeValve();
    }

    if (valveClosed && (millis() - valveCloseTime >= VALVE_RECHECK)) {
      float freshTDS = readTDS();
      if (freshTDS >= 50 && freshTDS <= 500) {
        openValve();
      } else {
        valveCloseTime = millis(); // reset timer, keep closed
      }
    }

    // ── Human-Readable Serial Output ──────────
    Serial.println("─────── WATER DATA ───────");
    Serial.printf("Flow Rate   : %.2f L/min\n",  flowRate);
    Serial.printf("Total Volume: %.3f L\n",       totalLiters);
    Serial.printf("TDS         : %.2f ppm\n",     tds);
    Serial.printf("Status      : %s\n",           waterStatus.c_str());
    Serial.println("──────────────────────────");

    // ── CSV Serial Output (for data logging) ──
    Serial.printf("%lu,%.2f,%.3f,%.2f,%s\n",
      millis(), flowRate, totalLiters, tds, waterStatus.c_str());

    // ── Firebase Upload (uncomment to enable) ─
    // if (Firebase.ready()) {
    //   Firebase.RTDB.setFloat(&fbdo,  "/water/flowRate",    flowRate);
    //   Firebase.RTDB.setFloat(&fbdo,  "/water/totalLiters", totalLiters);
    //   Firebase.RTDB.setFloat(&fbdo,  "/water/tds",         tds);
    //   Firebase.RTDB.setString(&fbdo, "/water/status",      waterStatus);
    //   Serial.println("Firebase: Data uploaded.");
    // }

    previousMillis = currentMillis;
  }
}
