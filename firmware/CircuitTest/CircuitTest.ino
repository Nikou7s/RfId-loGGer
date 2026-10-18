#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define OLED_SDA 21
#define OLED_SCL 22

#define RFID_SS   5
#define RFID_RST  27
#define RFID_SCK  18
#define RFID_MISO 19
#define RFID_MOSI 23

MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n==================================");
  Serial.println("      CIRCUIT DIAGNOSTIC TOOL     ");
  Serial.println("==================================");

  // 1. I2C Bus Scan (OLED)
  Serial.println("\n1. Scanning I2C Bus (Checking OLED)...");
  Wire.begin(OLED_SDA, OLED_SCL);
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print(" -> SUCCESS: I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print(" -> ERROR: Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println(" -> FAILED: No I2C devices found. Check 3V3, GND, GPIO 21 (SDA), and GPIO 22 (SCL).");
  }

  // 2. SPI Bus Scan (RC522)
  Serial.println("\n2. Pinging SPI Bus (Checking RC522)...");
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();
  
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(" -> RC522 Firmware Version Read: 0x");
  Serial.print(version, HEX);
  
  if (version == 0x00 || version == 0xFF) {
    Serial.println("\n -> FAILED: RC522 not responding.");
    Serial.println("    Check 3V3, GND, GPIO 5 (SS), 27 (RST), 18 (SCK), 19 (MISO), 23 (MOSI).");
  } else {
    Serial.println("\n -> SUCCESS: RC522 is wired correctly and communicating.");
  }
  
  Serial.println("\n==================================");
  Serial.println("Diagnostic complete.");
}

void loop() {
  // Do nothing
}
