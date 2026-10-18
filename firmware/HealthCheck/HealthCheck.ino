#include <esp_system.h>

// The pins used in your RFID + OLED project
int projectPins[] = {5, 18, 19, 21, 22, 23, 27};
int numPins = 7;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n==================================");
  Serial.println("       ESP32 CORE HEALTH CHECK    ");
  Serial.println("==================================");

  // 1. Check CPU and Memory
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  
  Serial.printf("CPU Cores:      %d\n", chip_info.cores);
  Serial.printf("Chip Revision:  %d\n", chip_info.revision);
  Serial.printf("Flash Memory:   %d MB\n", spi_flash_get_chip_size() / (1024 * 1024));
  Serial.printf("Free Heap RAM:  %d bytes\n", ESP.getFreeHeap());
  Serial.print("MAC Address:    ");
  Serial.println(WiFi.macAddress());

  // 2. Check GPIO Pin internal states
  Serial.println("\n--- PIN MULTIPLEXER TEST ---");
  for (int i = 0; i < numPins; i++) {
    // Set to INPUT_PULLUP to provide a known default state (HIGH)
    pinMode(projectPins[i], INPUT_PULLUP);
    int state = digitalRead(projectPins[i]);
    
    Serial.printf("GPIO %02d: ", projectPins[i]);
    if (state == HIGH) {
      Serial.println("OK (Reads HIGH as expected)");
    } else {
      // If it reads LOW while pulled UP and nothing is connected, the pin might be grounded/damaged
      Serial.println("WARNING (Reads LOW - check if wired to Ground)");
    }
  }
  
  Serial.println("==================================");
  Serial.println("Health check complete.");
}

void loop() {
  // Do nothing
}
