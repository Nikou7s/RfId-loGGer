#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// ================= PIN CONFIGURATION =================

// OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// RC522 RFID reader
#define RFID_SS   5
#define RFID_RST  27
#define RFID_SCK  18
#define RFID_MISO 19
#define RFID_MOSI 23

// ================= HARDWARE OBJECTS =================

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

MFRC522 rfid(RFID_SS, RFID_RST);

bool oledOK = false;
bool rfidOK = false;

// ================= STUDENT DATABASE =================

struct Student {
  String uid;
  String name;
};

// Add your real UIDs and Names here!
Student students[] = {
  {"7A 4B B7 4F", "Mike Wheeler"},
  {"1A 2F B3 4F", "Max Mayfield"}
};
const int numStudents = sizeof(students) / sizeof(students[0]);

// ================= OLED INITIALIZATION =================

uint8_t findOLEDAddress() {
  for (uint8_t address = 0x3C; address <= 0x3D; address++) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      return address;
    }
  }

  return 0;
}

// ================= DRAW IDLE SCREEN =================

void showReadyScreen() {
  if (!oledOK) return;

  display.clearDisplay();

  // Outer frame
  display.drawRoundRect(0, 0, 128, 64, 5, SSD1306_WHITE);

  // Header bar
  display.fillRoundRect(5, 5, 118, 14, 3, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(16, 9);
  display.print("RFID ATTENDANCE");

  display.setTextColor(SSD1306_WHITE);

  // Draw a small card icon
  display.drawRoundRect(10, 28, 27, 21, 3, SSD1306_WHITE);
  display.drawLine(14, 34, 32, 34, SSD1306_WHITE);
  display.fillCircle(16, 42, 2, SSD1306_WHITE);
  display.drawLine(22, 42, 32, 42, SSD1306_WHITE);

  // Instructions
  display.setCursor(43, 29);
  display.print("READY TO SCAN");

  display.setCursor(43, 40);
  display.print("Tap your card");

  // Footer
  display.drawLine(8, 53, 120, 53, SSD1306_WHITE);
  display.fillCircle(12, 58, 2, SSD1306_WHITE);

  display.setCursor(19, 55);
  display.print("READER ONLINE");

  display.display();
}

// ================= DRAW ERROR SCREEN =================

void showErrorScreen() {
  if (!oledOK) return;

  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 5, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(12, 10);
  display.print("RFID ATTENDANCE");

  display.drawLine(8, 22, 120, 22, SSD1306_WHITE);

  display.setCursor(15, 31);
  display.print("READER ERROR");

  display.setCursor(8, 44);
  display.print("Check the wiring");

  display.display();
}

// ================= DRAW CARD DETAILS =================

void showCardScreen(String studentName, bool isKnown) {
  if (!oledOK) return;

  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 5, SSD1306_WHITE);

  // Inverted title bar
  display.fillRoundRect(5, 5, 118, 14, 3, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(25, 9);
  display.print("CARD DETECTED");

  display.setTextColor(SSD1306_WHITE);

  // Status icon: Checkmark for known, X for unknown
  display.drawCircle(15, 34, 8, SSD1306_WHITE);
  if (isKnown) {
    display.drawLine(10, 34, 14, 38, SSD1306_WHITE);
    display.drawLine(14, 38, 20, 29, SSD1306_WHITE);
  } else {
    display.drawLine(12, 31, 18, 37, SSD1306_WHITE);
    display.drawLine(18, 31, 12, 37, SSD1306_WHITE);
  }

  // Label
  display.setCursor(29, 25);
  if (isKnown) {
    display.print("STUDENT NAME");
  } else {
    display.print("UNKNOWN CARD");
  }

  // Print the actual name where the UID used to be
  display.setCursor(29, 36);
  display.print(studentName);

  // Footer
  display.drawLine(8, 54, 120, 54, SSD1306_WHITE);
  display.setCursor(7, 56);
  display.print("Remove card to scan");

  display.display();
}

// ================= READ AND FORMAT UID =================

String formatByte(byte value) {
  String result = "";

  if (value < 0x10) {
    result += "0";
  }

  result += String(value, HEX);
  result.toUpperCase();

  return result;
}

void handleCard() {
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String fullUID = "";

  // Create the UID in hexadecimal format
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i > 0) {
      fullUID += " ";
    }
    fullUID += formatByte(rfid.uid.uidByte[i]);
  }

  // Look up the student in the database
  String studentName = "UNREGISTERED";
  bool isKnown = false;
  for (int i = 0; i < numStudents; i++) {
    if (students[i].uid == fullUID) {
      studentName = students[i].name;
      isKnown = true;
      break;
    }
  }

  // Send the UID to Python/Docker exactly as it expects
  Serial.println();
  Serial.println("========== CARD SCANNED ==========");
  Serial.print("Card UID: ");
  Serial.println(fullUID);
  Serial.println("==================================");

  // Show name on your custom OLED layout
  showCardScreen(studentName, isKnown);

  // Finish communication with this card
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // Keep the result visible before returning to idle
  delay(1800);

  showReadyScreen();
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("RFID ATTENDANCE SYSTEM");
  Serial.println("Initializing hardware...");

  // Start I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // Detect OLED address
  uint8_t address = findOLEDAddress();

  if (address != 0) {
    oledOK = display.begin(SSD1306_SWITCHCAPVCC, address);
  }

  if (oledOK) {
    Serial.println("OLED: OK");
    showReadyScreen();
  } else {
    Serial.println("OLED: NOT DETECTED");
  }

  // Start SPI and RFID reader
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();
  delay(100);

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.printf("RC522 version: 0x%02X\n", version);

  rfidOK = (version != 0x00 && version != 0xFF);

  if (rfidOK) {
    Serial.println("RC522: OK");
    Serial.println("Present a compatible RFID card.");
    showReadyScreen();
  } else {
    Serial.println("RC522: NOT RESPONDING");
    showErrorScreen();
  }
}

// ================= MAIN LOOP =================

void loop() {
  if (!rfidOK) {
    delay(1000);
    return;
  }

  if (!rfid.PICC_IsNewCardPresent()) {
    delay(40);
    return;
  }

  handleCard();
}
