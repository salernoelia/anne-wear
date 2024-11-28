#include <Arduino.h>
#include <M5Unified.h>

void setup(void)
{
    // Initialize Serial for debugging
    Serial.begin(9600);
    
    // Initialize M5Unified with default configuration
    auto cfg = M5.config();
    M5.begin(cfg);

    // Initialize the LCD
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2); // Set text size (Note: `setTextSize` might not be available; use `setTextFont` instead)
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color with background
    M5.Lcd.setCursor(0, 0); // Set initial cursor position
}

void loop() {
    M5.update(); // Handle background tasks

    float temperature;
    if (M5.Imu.getTemp(&temperature)) {
        Serial.printf("Temperature: %.2f °C\n", temperature);

        // Clear the area where the temperature is displayed
        // Increase the height to accommodate larger text
        M5.Lcd.fillRect(0, 0, M5.Lcd.width(), 24, TFT_BLACK);

        // Display the new temperature
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.printf("Temperature: %.2f °C", temperature);
    } else {
        Serial.println("Failed to read temperature.");

        // Clear the area where the error message is displayed
        M5.Lcd.fillRect(0, 0, M5.Lcd.width(), 24, TFT_BLACK);

        // Display the error message
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Failed to read temperature.");
    }

    delay(1000); // Wait for 1 second
}
