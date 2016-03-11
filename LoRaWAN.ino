/*	LoRaWAN Network Test v0.3
*	Rasmus Henriksson
*	rasmhenriksson@gmail.com
*/

// Cooking API
#include <arduinoUtils.h>
#include <arduinoUART.h>
#include <arduinoMultiprotocol.h>
#include <arduinoClasses.h>

// LoRaWAN
#include <arduinoLoRaWAN.h>

// Bluetooth serial
//#include <SoftwareSerial.h>
//SoftwareSerial btSerial(9, 10); // RX,TX
//String command = "";
//String message;

// Pins
const int errorLed = 13;
//const int btVcc = 8;

// Constants
uint8_t socket = SOCKET0;
uint8_t port = 1; // Range 1-223
int packetCount = 0;

// Device parameters for Back-End registration
char DEVICE_EUI[] = "2DEE0F9F3EA8459B";
char DEVICE_ADDR[] = "2DEE0F9F";
char NWK_SESSION_KEY[] = "2B7E151628AED2A6ABF7158809CF4F3C";
char APP_SESSION_KEY[] = "2B7E151628AED2A6ABF7158809CF4F3C";
char APP_KEY[] = "000102030405060708090A0B0C0D0E0F";

// Variables
uint8_t error;
uint8_t SNR;
char data[] = "00010203040506070809"; // Payload to send
uint32_t cFreq[] = { 867100000, 867300000, 867500000, 867700000, 867900000 };
uint32_t fOffset = 200000;
int errorCount = 0;

void showError(uint8_t e, int n) {
	if (e == 0) {
		digitalWrite(errorLed, LOW);
	}
	else {
		//btSerial.println("Error occured");
		for (int i = 1; i <= n; i++) {
			digitalWrite(errorLed, HIGH);
			delay(100);
			digitalWrite(errorLed, LOW);
			delay(100);
		}
	}
}

void softwareReset() {
	asm volatile ("  jmp 0");
	//wdt_enable(WDTO_15MS);
}


void setup() {
	pinMode(errorLed, OUTPUT);
	//pinMode(btVcc, OUTPUT);
	//btSerial.begin(9600);

	//digitalWrite(btVcc, HIGH);
	digitalWrite(errorLed, LOW);

	// 1. Activate LoRaWAN
	error = 1;
	LoRaWAN.ON(socket);
	LoRaWAN.factoryReset();

	// Channel parameters
	for (uint8_t i = 3; i < 8; i++) {
		LoRaWAN.setChannelStatus(i, "on");
		LoRaWAN.setChannelFreq(i, cFreq[i - 3]);
	}
	for (uint8_t i = 0; i < 3; i++) {
		LoRaWAN.setChannelDutyCycle(i, 302);
	}

	for (uint8_t i = 3; i < 8; i++) {
		LoRaWAN.setChannelDutyCycle(i, 99);
	}

	//btSerial.println("Setting channel parameters");
	LoRaWAN.setPower(5); // [N/A, 14, 11, 8, 5, 2] dBm
	LoRaWAN.setADR("off"); // Adaptive data rate
	LoRaWAN.setDataRate(5); // [250, 440, 980, 1760, 3125, 5470, 11000];



	// Set device EUI and address
	error = LoRaWAN.setDeviceEUI();
	LoRaWAN.setDeviceAddr(DEVICE_ADDR);
	showError(error, 2);


	// Keys and retries
	LoRaWAN.setNwkSessionKey(NWK_SESSION_KEY);
	LoRaWAN.setAppSessionKey(APP_SESSION_KEY);
	LoRaWAN.setAppKey(APP_KEY);
	LoRaWAN.setRetries(3);

	// 8. Save config
	error = LoRaWAN.saveConfig();
	showError(error, 5);

	LoRaWAN.joinABP();

}


void loop() {
	//LoRaWAN.joinABP();
	error = LoRaWAN.sendUnconfirmed(port, data);
	showError(error, 2);
	if (error != 0) {
		errorCount = errorCount + 1;
		if (errorCount > 3) {
			LoRaWAN.joinABP();
			if (errorCount > 50)
				softwareReset();
		}
	}
	else
		errorCount = 0;
	delay(3000);
	port = errorCount + 1;
}
