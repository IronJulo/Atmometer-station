#include <Arduino.h>
#include <Wire.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "atmometer_basic_module_driver.h"
#include "atmometer_dht11_module_driver.h"
#include "atmometer_mh_z19_module_driver.h"
#include "module.h"
#include "module_available.h"

#define I2C_SCL 9
#define I2C_SDA 8

#define LED_ERROR 10
#define LED_OK 20
#define LED_SENDING 21

#define SOCKET_VD_INPUT 5

#define MAX_SOCKET_COUNT 16

void mapSensorLayout();
void printSensorLayout();
void printSocketGrid();
void printSocket0Data();

Socket sockets[MAX_SOCKET_COUNT];

void setup()
{
	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setTimeOut(200);
	Wire.setClock(10000);
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
	mapSensorLayout();
	// printSocketGrid();

	uint32_t sensor_values[MAX_SOCKET_COUNT] = {0};

	for (int i = 0; i < MAX_SOCKET_COUNT; i++)
	{
		if (sockets[i].isModulePresent())
		{
			switch (sockets[i].getModuleType())
			{
			case MODULE_TYPE_DHT11_T:
				// DHT11 module for humidity
				sensor_values[i] = Atmometer::DHT11Module::get_module_temperature(Wire, sockets[i].getSocketID() + 1);
				break;
			case MODULE_TYPE_DHT11_H:
				// DHT11 module for temperature
				sensor_values[i] = Atmometer::DHT11Module::get_module_humidity(Wire, sockets[i].getSocketID() + 1);
				break;
			case MODULE_TYPE_MH_Z19:
				// MH-Z19 module for CO2
				sensor_values[i] = Atmometer::MH_Z19Module::get_module_co2_ppm(Wire, sockets[i].getSocketID() + 1);
				break;
			}
		}

		Serial.println("waiting");
		delay(2000);
	}
}

void printSocket0Data()
{
	if (sockets[0].isModulePresent())
	{
		Serial.print("Module ID: ");
		Serial.println(sockets[0].getModuleID());
		Serial.print("Module type: ");
		Serial.println(sockets[0].getModuleType());
		Serial.print("Module temperature: ");
		Serial.println(Atmometer::DHT11Module::get_module_temperature(Wire, 0x01));
		Serial.print("Module HUMIDITY: ");
		Serial.println(Atmometer::DHT11Module::get_module_humidity(Wire, 0x01));
	}
	else
	{
		Serial.println("No module present in Socket 0");
	}
}

void printSensorLayout()
{
	for (int i = 0; i < MAX_SOCKET_COUNT; i++)
	{
		if (!sockets[i].isModulePresent())
		{
			continue;
		}

		Serial.print("Socket: ");
		Serial.println(i);
		Serial.print("Module ID: ");
		Serial.println(sockets[i].getModuleID());
		Serial.print("Module type: ");
		Serial.println(sockets[i].getModuleType());
	}
}

void mapSensorLayout()
{
	for (int address = 0; address < MAX_SOCKET_COUNT; address++)
	{
		Wire.beginTransmission(address + 1);
		byte error = Wire.endTransmission();

		if (error == 0)
		{
			Serial.print("Detected module at address: ");
			Serial.println(address + 1);

			uint64_t deviceId = Atmometer::BasicModule::get_module_id(Wire, address + 1);
			delay(10);
			uint64_t deviceType = Atmometer::BasicModule::get_module_type(Wire, address + 1);
			delay(10);

			sockets[address].setSocketID(address);
			sockets[address].setModuleID(deviceId);
			sockets[address].setModuleType(deviceType);
			sockets[address].setModulePresent(true);
		}
		/*else
		{
			Serial.print("No module: ");
			Serial.print(address + 1);
			Serial.print("   ERROR: ");
			Serial.println(static_cast<unsigned long long>(error));

			sockets[address].setModuleID(0);
			sockets[address].setModuleType(0);
			sockets[address].setModulePresent(false);
		}*/
		delay(100);
	}
}

void printSocketGrid()
{
	const int rows = 4;
	const int cols = 3;
	int col = 0;
	const int cellWidth = 20;
	std::ostringstream oss;

	oss << "+---------------+---------------+---------------+\n";

	for (int row = 0; row < rows; ++row)
	{
		oss << "|               |               |               |\n";
		oss << "|               |               |               |\n";
		oss << "|Addr: " << std::setw(9) << std::left << (col * cols) + 0;
		oss << "|Addr: " << std::setw(9) << std::left << (col * cols) + 1;
		oss << "|Addr: " << std::setw(9) << std::left << (col * cols) + 2;
		oss << "|\n";
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 0].getModuleType());
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 1].getModuleType());
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 2].getModuleType());
		oss << "|\n";
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 0].getModuleID());
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 1].getModuleID());
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(sockets[(col * cols) + 2].getModuleID());
		oss << "|\n";
		oss << "+---------------+---------------+---------------+\n";
		col++;
	}

	Serial.println(oss.str().c_str());
}