#include <Arduino.h>
#include <Wire.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "atmometer_basic_module_driver.h"
#include "atmometer_dht11_module_driver.h"
#include "atmometer_mh_z19_module_driver.h"
#include "atmometer_pms5003_module_driver.h"
#include "protocol.h"
#include "module.h"
#include "module_available.h"

#include <WriteBufferFixedSize.h>

#define I2C_SCL 9
#define I2C_SDA 8

#define LED_ERROR 10
#define LED_OK 20
#define LED_SENDING 21

#define SOCKET_VD_INPUT 5

#define MAX_SOCKET_COUNT 16

#define DEVICE_ID 0x01

void mapSensorLayout();
void printSensorLayout();
void printSocketGrid();
void printSocket0Data();
uint32_t get_sensor_raw_value(TwoWire &i2cHandle, uint8_t address);

Socket sockets[MAX_SOCKET_COUNT];
uint32_t sensor_values[MAX_SOCKET_COUNT];

com::epitech::atmos::protobuf::Socket_layout_packet<64> socketLayoutPacket;
com::epitech::atmos::protobuf::Data_packet<64> dataPacket;

EmbeddedProto::WriteBufferFixedSize<256> writeBuffer;

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

	mapSensorLayout();
}

void loop()
{
	mapSensorLayout();
	printSocketGrid();

	Serial.println("waiting");
	delay(2000);
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

			sensor_values[address] = get_sensor_raw_value(Wire, address + 1);
		}
		else
		{
			sockets[address].setModuleID(0);
			sockets[address].setModuleType(0);
			sockets[address].setModulePresent(false);
			sensor_values[address] = 0;
		}
		delay(100);
	}
}

uint32_t get_sensor_raw_value(TwoWire &i2cHandle, uint8_t address)
{
	switch (sockets[address - 1].getModuleType())
	{
	case MODULE_TYPE_DHT11_T:
		// DHT11 module for humidity
		return Atmometer::DHT11Module::get_raw_module_temperature(i2cHandle, address);
		break;
	case MODULE_TYPE_DHT11_H:
		// DHT11 module for temperature
		return Atmometer::DHT11Module::get_raw_module_humidity(i2cHandle, address);
		break;
	case MODULE_TYPE_MH_Z19:
		// MH-Z19 module for CO2
		return Atmometer::MH_Z19Module::get_raw_module_co2_ppm(i2cHandle, address);
		break;
	case MODULE_TYPE_PMS5003_1_0_um:
		// PMS7003 module for 1.0 um particles
		return Atmometer::PMS5003Module::get_raw_module_1_0_um(i2cHandle, address);
		break;
	case MODULE_TYPE_PMS5003_2_5_um:
		// PMS7003 module for 2.5 um particles
		return Atmometer::PMS5003Module::get_raw_module_2_5_um(i2cHandle, address);
		break;
	case MODULE_TYPE_PMS5003_10_um:
		// PMS7003 module for 10 um particles
		return Atmometer::PMS5003Module::get_raw_module_10_um(i2cHandle, address);
		break;
	case MODULE_TYPE_DHT22_T:
		// DHT22 module for temperature
	case MODULE_TYPE_DHT22_H:
		// DHT22 module for humidity
	case MODULE_TYPE_NOT_CONFIGURED:
	default:
		return 0;
		break;
	}
}

void printSocketGrid()
{
	const int rows = 1; // 4;
	const int cols = 3;
	int col = 0;
	const int cellWidth = 20;
	std::ostringstream oss;

	oss << "+---------------+---------------+---------------+\n";

	for (int row = 0; row < rows; ++row)
	{
		oss << "|               |               |               |\n";
		oss << "|               |               |               |\n";
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 0;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 1;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 2;
		oss << "|\n";
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(sensor_values[(col * cols) + 0]);
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(sensor_values[(col * cols) + 1]);
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(sensor_values[(col * cols) + 2]);
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

void fillSocketLayoutPacket()
{
	for (int i = 0; i < MAX_SOCKET_COUNT; i++)
	{
		socketLayoutPacket.mutable_socket_data()[i].clear();
		socketLayoutPacket.mutable_socket_data()[i].set_socket_id(i);
		if (!sockets[i].isModulePresent())
		{
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_id(0XFFFFFFF);
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_type(0XFFFFFFF);
		}
		else
		{
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_id(sockets[i].getModuleID());
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_type(sockets[i].getModuleType());
		}
	}
}

void fillDataPacket()
{
	dataPacket.clear_sensor_data();

	dataPacket.mutable_header().set_device_id(DEVICE_ID);
	dataPacket.mutable_header().set_packet_crc(0);

	for (int i = 0; i < MAX_SOCKET_COUNT; i++)
		dataPacket.add_sensor_data(sensor_values[i]);
}

uint16_t computeCRC(uint8_t *data, uint8_t length)
{
	uint16_t crc = 0;

	for (uint8_t i = 0; i < length; i++)
	{
		crc += data[i];
	}

	return crc;
}