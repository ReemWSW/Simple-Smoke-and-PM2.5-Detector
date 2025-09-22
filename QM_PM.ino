// เครื่องตรวจจับควัน + ฝุ่น PM2.5 แบบง่าย
// Simple Smoke + PM2.5 Detector

#include "PMS.h"

// ตั้งค่าอุปกรณ์
HardwareSerial pmsSerial(2);
PMS pms(pmsSerial);
PMS::DATA data;

int smokePin = 34;      // MQ-2 ควัน
int ledPin = 2;         // LED แจ้งเตือน

// ค่าเตือน
int smokeAlert = 2000;  // ค่าเตือนควัน
int pm25Alert = 75;     // ค่าเตือน PM2.5

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  Serial.println("=== เครื่องตรวจจับควัน + ฝุ่น ===");
  
  // เริ่มต้น PMS5003
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
  Serial.println("เริ่มระบบ PMS5003...");
  
  // อุ่นเซ็นเซอร์ควัน
  Serial.println("อุ่นเซ็นเซอร์ควัน 20 วินาที...");
  for(int i = 20; i > 0; i--) {
    Serial.print("เหลือ ");
    Serial.print(i);
    Serial.println(" วินาที");
    delay(1000);
  }
  
  Serial.println("พร้อมใช้งาน!");
  Serial.println("==================");
}

void loop() {
  // อ่านค่าควัน
  int smokeLevel = analogRead(smokePin);
  
  // อ่านค่า PM2.5
  int pm25 = 0;
  bool havePM25 = false;
  
  if (pms.read(data)) {
    pm25 = data.PM_AE_UG_2_5;
    havePM25 = true;
  }
  
  // แสดงผล
  Serial.print("ควัน: ");
  Serial.print(smokeLevel);
  Serial.print("/4095");
  
  if (havePM25) {
    Serial.print(" | PM2.5: ");
    Serial.print(pm25);
    Serial.print(" μg/m³");
  } else {
    Serial.print(" | PM2.5: --");
  }
  
  // ตรวจสอบเตือน
  bool smokeHigh = (smokeLevel > smokeAlert);
  bool pm25High = (havePM25 && pm25 > pm25Alert);
  
  if (smokeHigh || pm25High) {
    Serial.print(" >>> ");
    
    if (smokeHigh && pm25High) {
      Serial.println("เตือน! ควัน + ฝุ่นสูง!");
    } else if (smokeHigh) {
      Serial.println("เตือน! พบควัน!");
    } else {
      Serial.println("เตือน! ฝุ่น PM2.5 สูง!");
    }
    
    // กระพริบ LED
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(600);
    
  } else {
    Serial.println(" ปกติ");
    digitalWrite(ledPin, LOW);
  }
  
  delay(1000);
}