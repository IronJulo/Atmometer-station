#include <Arduino.h>

#include <Wire.h>

#include "atmometer_basic_module_driver.h"

#define I2C_SCL 9
#define I2C_SDA 8

#define LED_ERROR 10
#define LED_OK 20
#define LED_SENDING 21

#define SOCKET_VD_INPUT 5

#define SLAVE_ADDRESS 1

void print_module_info(uint8_t address);

void setup()
{
	Wire.begin(I2C_SDA, I2C_SCL);
	Serial.begin(9600);
	Serial.println("\nI2C Scanner");
	delay(1000);

	pinMode(LED_ERROR, OUTPUT);
	// pinMode(LED_OK, OUTPUT);
	// pinMode(LED_SENDING, OUTPUT);
	digitalWrite(LED_ERROR, LOW);
	// digitalWrite(LED_OK, LOW);
	// digitalWrite(LED_SENDING, LOW);

	pinMode(SOCKET_VD_INPUT, OUTPUT);
	digitalWrite(SOCKET_VD_INPUT, HIGH);
	delay(1000);
}

void loop()
{
	Wire.beginTransmission(SLAVE_ADDRESS);
	byte error = Wire.endTransmission();

	if (error == 0)
	{
		digitalWrite(LED_ERROR, LOW);
		Serial.println("I2C device found");
		// print_module_info(SLAVE_ADDRESS);
		Serial.print("Module vlaue: ");
		Serial.println(Atmometer::BasicModule::get_module_value_1(Wire, SLAVE_ADDRESS));
	}
	else
	{
		digitalWrite(LED_ERROR, HIGH);
		Serial.println("Error");
	}

	Serial.println("waiting");
	delay(2000);
}

void print_module_info(uint8_t address)
{
	uint64_t module_id = Atmometer::BasicModule::get_module_id(Wire, address);
	uint64_t module_type = Atmometer::BasicModule::get_module_id(Wire, address);

	Serial.print("Module ID: ");
	Serial.println(module_id);
	Serial.print("Module type: ");
	Serial.println(module_type);
}