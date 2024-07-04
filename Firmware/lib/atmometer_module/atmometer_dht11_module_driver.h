#ifndef ATMOMETER_DHT11_MODULE_DRIVER_H
#define ATMOMETER_DHT11_MODULE_DRIVER_H

#include "atmometer_basic_module_driver.h"

namespace Atmometer
{
    enum DHT11_Register
    {
        DHT11_module_VALUE_2_MSB1 = (BASIC_REGISTER_READ_ONLY_END + 1),
        DHT11_module_VALUE_2_MSB2,
        DHT11_module_VALUE_2_LSB1,
        DHT11_module_VALUE_2_LSB2,

        DHT11_REGISTER_ENUM_END,
    };

    class DHT11Module : public BasicModule
    {
    public:
        DHT11Module() = default;
        ~DHT11Module() = default;

        static float get_module_temperature(TwoWire &i2cHandle, uint8_t address);
        static float get_module_humidity(TwoWire &i2cHandle, uint8_t address);

        static float raw_to_temperature(uint32_t raw);
        static float raw_to_humidity(uint32_t raw);

        static uint32_t get_raw_module_temperature(TwoWire &i2cHandle, uint8_t address);
        static uint32_t get_raw_module_humidity(TwoWire &i2cHandle, uint8_t address);
    };

    float DHT11Module::get_module_humidity(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t data = 0;
        error = simple_read(i2cHandle, address, DHT11_Register::DHT11_module_VALUE_2_MSB1, &data);
        // TODO: HANDLE NEGAIVE TEMPERATURE
        return raw_to_humidity(data);
    }

    float DHT11Module::get_module_temperature(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        // TODO: HANDLE NEGAIVE TEMPERATURE
        return raw_to_temperature(data);
    }

    float DHT11Module::raw_to_temperature(uint32_t raw)
    {
        return (float)raw / 1000;
    }

    float DHT11Module::raw_to_humidity(uint32_t raw)
    {
        return (float)raw / 1000;
    }

    uint32_t DHT11Module::get_raw_module_humidity(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, DHT11_Register::DHT11_module_VALUE_2_MSB1, &value);
        return value;
    }

    uint32_t DHT11Module::get_raw_module_temperature(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        return data;
    }
}

#endif /* ATMOMETER_DHT11_MODULE_DRIVER_Hs */