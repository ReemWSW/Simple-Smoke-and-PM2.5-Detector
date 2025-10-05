// เครื่องตรวจจับควัน + ฝุ่น PM2.5 แบบง่าย
// Simple Smoke + PM2.5 Detector

#include "PMS.h"
#include <LiquidCrystal_I2C.h>

// ตั้งค่าอุปกรณ์
HardwareSerial pmsSerial(2);
PMS pms(pmsSerial);
PMS::DATA data;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16x2

int smokePin = 34;      // MQ-2 ควัน
int ledPin = 2;         // LED แจ้งเตือน

// ค่าเตือน
int smokeAlert = 2000;  // ค่าเตือนควัน
int pm25Alert = 75;     // ค่าเตือน PM2.5

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // เริ่มต้น LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smoke & PM2.5");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  Serial.println("=== เครื่องตรวจจับควัน + ฝุ่น ===");
  
  // เริ่มต้น PMS5003
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
  Serial.println("เริ่มระบบ PMS5003...");
  
  // ต่อ VCC ของ PMS5003 เข้า ADC pin เพื่อวัดแรงดัน
  int voltage = analogRead(35); // ถ้าต่อ VCC เข้า GPIO 35
  float volt = (voltage * 3.3) / 4095;
  Serial.print("แรงดันไฟเลี้ยง: ");
  Serial.print(volt);
  Serial.println("V");
  
  if (volt < 4.8) {
    Serial.println("⚠️ แรงดันไฟต่ำ! ต้องการ 5V");
  }

  // เช็คว่า Serial พร้อมหรือไม่
  Serial.print("PMS Serial available: ");
  Serial.println(pmsSerial.available());
  
  // ส่งคำสั่งเปิดเซ็นเซอร์
  pms.wakeUp();
  delay(2000);

  // เปลี่ยนเป็น passive mode
  pms.passiveMode();
  delay(1000);
  
  // เช็คข้อมูลดิบจาก Serial
  testRawSerial();

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

  lcd.clear();
}

void loop() {
  // อ่านค่าควัน
  int smokeLevel = analogRead(smokePin);
  
  // อ่านค่า PM2.5
  int pm25 = 0;
  bool havePM25 = false;

  pms.requestRead();
  if (pms.readUntil(data)) {
    pm25 = data.PM_AE_UG_2_5;
    havePM25 = true;
  }
  
  // แสดงผลบน LCD
  lcd.setCursor(0, 0);
  lcd.print("Smoke:");
  lcd.print(smokeLevel);
  lcd.print("    ");  // ลบตัวอักษรเก่า

  lcd.setCursor(0, 1);
  lcd.print("PM2.5:");
  if (havePM25) {
    lcd.print(pm25);
    lcd.print(" ug/m3  ");
  } else {
    lcd.print("--     ");
  }

  // แสดงผล Serial
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
  
  delay(2000);
}


void testRawSerial() {
  Serial.println("ทดสอบอ่านข้อมูลดิบจาก PMS5003...");
  
  unsigned long startTime = millis();
  int byteCount = 0;
  
  while (millis() - startTime < 5000) {  // เช็ค 5 วินาที
    if (pmsSerial.available()) {
      uint8_t byte = pmsSerial.read();
      Serial.print("0x");
      if (byte < 16) Serial.print("0");
      Serial.print(byte, HEX);
      Serial.print(" ");
      byteCount++;
      
      if (byteCount % 16 == 0) {
        Serial.println();
      }
    }
    delay(10);
  }
  
  Serial.println();
  Serial.print("รับข้อมูล: ");
  Serial.print(byteCount);
  Serial.println(" bytes");
}

