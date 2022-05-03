/*
 * Pin layout:
 * ------------------------------------
 *             MFRC522      Arduino
 *             Reader/PCD   Pro Micro
 * Signal      Pin          Pin
 * ------------------------------------
 * SPI SS      SDA(SS)      10
 * SPI MOSI    MOSI         16
 * SPI MISO    MISO         14
 * SPI SCK     SCK          15
 */

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

MFRC522DriverPinSimple ss_pin(10); // Set the SS pin
MFRC522DriverSPI driver{ss_pin};   // Create SPI driver
MFRC522 mfrc522{driver};		   // Create MFRC522 instance

volatile bool buttonPressed = false;

void buttonISR() {
	buttonPressed = true;
}

void setup() {
	Serial.begin(115200); // Initialize serial communications with the PC for debugging

	mfrc522.PCD_Init(); // Init MFRC522 board

	pinMode(5, OUTPUT);	  // LED
	pinMode(7, OUTPUT);	  // Taser
	digitalWrite(5, LOW); // LED off
	digitalWrite(7, LOW); // Taser off

	// Interrupt on button press
	pinMode(3, INPUT_PULLUP); // Button IRQ
	attachInterrupt(digitalPinToInterrupt(3), buttonISR, FALLING);

	while (!Serial)
		; // Wait for serial port to connect. Needed for native USB port only

	Serial.println("I am alive!");
}

byte computeCardHash() {
	if (!mfrc522.PICC_ReadCardSerial())
	{
		Serial.println("Failed to read card");
		return 0xFF;
	}

	// Compute hash
	byte hash = 0;
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		hash ^= mfrc522.uid.uidByte[i];
	}

	mfrc522.PICC_HaltA(); // Stop reading

	return hash & 0b11;
}

void loop() {
	if (mfrc522.PICC_IsNewCardPresent()) {
		byte cardHash = computeCardHash();
		if (cardHash != 0xFF) {
			digitalWrite(5, HIGH);
			Serial.println(cardHash);

			unsigned long timeStart = millis();
			buttonPressed = false;
			while (millis() - timeStart < 3000) {
				if (buttonPressed) {
					buttonPressed = false;

					if (cardHash == 2) {
						digitalWrite(7, HIGH);
					}
				}
			}

			digitalWrite(5, LOW);
			digitalWrite(7, LOW);
		}
	}
}
