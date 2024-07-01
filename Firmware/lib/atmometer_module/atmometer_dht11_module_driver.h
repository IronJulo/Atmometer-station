#ifndef ATMOMETER_DHT11_MODULE_DRIVER_H
#define ATMOMETER_DHT11_MODULE_DRIVER_H

#include "atmometer_basic_module_driver.h"

namespace Atmometer
{
    enum DHT11_Register
    {
        module_VALUE_2_MSB1 = (BASIC_REGISTER_READ_ONLY_END + 1),
        module_VALUE_2_MSB2,
        module_VALUE_2_LSB1,
        module_VALUE_2_LSB2,

        DHT11_REGISTER_ENUM_END,
    };

    class DHT11Module : public BasicModule
    {
    public:
        DHT11Module() = default;
        ~DHT11Module() = default;

        static float get_module_temperature(TwoWire &i2cHandle, uint8_t address);
        static float get_module_humidity(TwoWire &i2cHandle, uint8_t address);
    };

    float DHT11Module::get_module_humidity(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, DHT11_Register::module_VALUE_2_MSB1, &value);
        // TODO: HANDLE NEGAIVE TEMPERATURE
        return (float)value / 1000;
    }

    float DHT11Module::get_module_temperature(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        // TODO: HANDLE NEGAIVE TEMPERATURE
        return (float)data / 1000;
    }
}

#endif /* ATMOMETER_DHT11_MODULE_DRIVER_Hs */