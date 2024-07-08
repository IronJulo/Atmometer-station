#include <Arduino.h>
#include <Wire.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "LoRa.h"

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

#define SOCKET_LAYOUT_PACKET_ID 0x01
#define DATA_PACKET_ID 0x02

void scanSensorLayout();
void printSensorLayout();
void printSocketGrid();
void printSocket0Data();
void fillSocketLayoutPacket();
void fillDataPacket();
uint16_t computeCRC(uint8_t *data, uint8_t length);
uint8_t sendSocketLayoutPacket();
uint8_t sendDataPacket();
void printSocketLayoutPacket(com::epitech::atmos::protobuf::Socket_layout_packet<64> &socketLayoutPacket);
void printDataPacket(com::epitech::atmos::protobuf::Data_packet<64> &dataPacket);

uint32_t get_sensor_raw_value(TwoWire &i2cHandle, uint8_t address);

Socket sockets[MAX_SOCKET_COUNT];
uint32_t sensor_values[MAX_SOCKET_COUNT];

com::epitech::atmos::protobuf::Socket_layout_packet<64> socketLayoutPacket;
com::epitech::atmos::protobuf::Data_packet<64> dataPacket;

EmbeddedProto::WriteBufferFixedSize<256> writeBuffer;

const double FREQUENCY = 433E6;
const double BANDWIDTH = 125E3;
const int SPREADING_FACTOR = 7;
const int CODING_RATE_DENOM = 8;
const int PREAMBLE_LENGTH = 8;

const int LORA_SCK = 3;
const int LORA_MISO = 7;
const int LORA_MOSI = 6;
const int LORA_SS = 2;
const int LORA_RST = 0;
const int LORA_DIO0 = 1;

int globalCounter = 0;

void setup()
{
	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setTimeOut(200);
	Wire.setClock(10000);

	Serial.begin(9600);
	Serial.println("Atmometer station starting...");
	LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
	if (!LoRa.begin(FREQUENCY))
	{
		Serial.println("Starting LoRa failed!");
		while (1)
			;
	}
	LoRa.setSignalBandwidth(BANDWIDTH);
	LoRa.setSpreadingFactor(SPREADING_FACTOR);
	LoRa.setCodingRate4(CODING_RATE_DENOM);
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

	scanSensorLayout();
}

bool resendSocketLayoutPacket = false;
bool resendDataPacket = false;
uint64_t timeOnDPSent = 0;
#define DP_SEND_INTERVAL 10000

void loop()
{
	scanSensorLayout();
	if (resendSocketLayoutPacket)
	{
		Serial.println("Sending socket packet...");
		fillSocketLayoutPacket();
		resendSocketLayoutPacket = false;
		sendSocketLayoutPacket();
	}
	if (resendDataPacket)
	{
		Serial.println("Sending data packet...");
		fillDataPacket();
		resendDataPacket = false;
		sendDataPacket();
		timeOnDPSent = esp_timer_get_time();
	}

	if ((esp_timer_get_time() - timeOnDPSent) > DP_SEND_INTERVAL)
	{
		resendDataPacket = true;
	}

	globalCounter++;
	Serial.println("waiting");
	delay(3000);
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

void scanSensorLayout()
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
	socketLayoutPacket.mutable_header().clear();
	socketLayoutPacket.mutable_header().set_device_id(DEVICE_ID);
	socketLayoutPacket.mutable_header().set_packet_crc(50);
	for (int i = 0; i < MAX_SOCKET_COUNT; i++)
	{
		socketLayoutPacket.mutable_socket_data()[i].clear();
		socketLayoutPacket.mutable_socket_data()[i].set_socket_id(i);
		if (!sockets[i].isModulePresent())
		{
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_id(0x00);
			socketLayoutPacket.mutable_socket_data()[i].set_sensor_type(0x00);
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
	uint8_t crc = 0;

	for (uint8_t i = 0; i < length; i++)
	{
		crc += data[i];
	}

	return crc;
}

uint8_t sendSocketLayoutPacket()
{
	writeBuffer.clear();
	socketLayoutPacket.serialize(writeBuffer);

	uint8_t *data = writeBuffer.get_data();
	uint32_t length = writeBuffer.get_size();

	uint16_t crc = computeCRC(data, length);

	dataPacket.mutable_header().set_packet_crc(crc);

	writeBuffer.clear();
	socketLayoutPacket.serialize(writeBuffer);

	LoRa.beginPacket();
	LoRa.write(0x00);
	LoRa.write(0x01);
	LoRa.write(data, length);
	LoRa.endPacket();

	return 0;
}

uint8_t sendDataPacket()
{
	writeBuffer.clear();
	dataPacket.serialize(writeBuffer);

	uint8_t *data = writeBuffer.get_data();
	uint32_t length = writeBuffer.get_size();

	uint16_t crc = computeCRC(data, length);

	dataPacket.mutable_header().set_packet_crc(crc);

	writeBuffer.clear();
	dataPacket.serialize(writeBuffer);

	LoRa.beginPacket();
	LoRa.write(0x00);
	LoRa.write(0x02);
	LoRa.write(data, length);
	LoRa.endPacket();

	return 0;
}

void printSocketLayoutPacket(com::epitech::atmos::protobuf::Socket_layout_packet<64> &socketLayoutPacket)
{
	const int rows = 1; // 4;
	const int cols = 3;
	int col = 0;
	const int cellWidth = 20;

	std::ostringstream oss;
	oss << "+---------------+\n";
	oss << "|               |\n";
	oss << "|Device ID: " << std::setw(4) << std::left << static_cast<unsigned long long>(socketLayoutPacket.header().device_id()) << "|\n";
	oss << "|Packet CRC: " << std::setw(3) << std::left << static_cast<unsigned long long>(socketLayoutPacket.header().packet_crc()) << "|\n";
	oss << "+---------------+---------------+---------------+\n";

	for (int row = 0; row < rows; ++row)
	{
		oss << "|               |               |               |\n";
		oss << "|               |               |               |\n";
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 0;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 1;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 2;
		oss << "|\n";
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 0].get_sensor_type());
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 1].get_sensor_type());
		oss << "|Type: " << std::setw(9) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 2].get_sensor_type());
		oss << "|\n";
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 0].get_sensor_id());
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 1].get_sensor_id());
		oss << "|ID: " << std::setw(11) << std::left << static_cast<unsigned long long>(socketLayoutPacket.mutable_socket_data()[(col * cols) + 2].get_sensor_id());
		oss << "|\n";
		oss << "+---------------+---------------+---------------+\n";
		col++;
	}

	Serial.println(oss.str().c_str());
}

void printDataPacket(com::epitech::atmos::protobuf::Data_packet<64> &dataPacket)
{
	const int rows = 1; // 4;
	const int cols = 3;
	int col = 0;
	const int cellWidth = 20;

	std::ostringstream oss;
	oss << "+---------------+\n";
	oss << "|               |\n";
	oss << "|Device ID: " << std::setw(4) << std::left << static_cast<unsigned long long>(dataPacket.header().device_id()) << "|\n";
	oss << "|Packet CRC: " << std::setw(2) << std::left << static_cast<unsigned long long>(dataPacket.header().packet_crc()) << "|\n";
	oss << "+---------------+---------------+---------------+\n";

	for (int row = 0; row < rows; ++row)
	{
		oss << "|               |               |               |\n";
		oss << "|               |               |               |\n";
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 0;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 1;
		oss << "|SoID: " << std::setw(9) << std::left << (col * cols) + 2;
		oss << "|\n";
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(dataPacket.sensor_data()[(col * cols) + 0]);
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(dataPacket.sensor_data()[(col * cols) + 1]);
		oss << "|Val : " << std::setw(9) << std::left << static_cast<unsigned long long>(dataPacket.sensor_data()[(col * cols) + 2]);
		oss << "|\n";
		oss << "+---------------+---------------+---------------+\n";
		col++;
	}

	Serial.println(oss.str().c_str());
}