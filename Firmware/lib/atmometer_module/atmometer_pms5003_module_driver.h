#ifndef ATMOMETER_PMS5003_MODULE_DRIVER_H
#define ATMOMETER_PMS5003_MODULE_DRIVER_H

#include "atmometer_basic_module_driver.h"

namespace Atmometer
{
    enum PMS5003_Register
    {
        PMS5003_module_VALUE_2_MSB1 = (BASIC_REGISTER_READ_ONLY_END + 1),
        PMS5003_module_VALUE_2_MSB2,
        PMS5003_module_VALUE_2_LSB1,
        PMS5003_module_VALUE_2_LSB2,

        PMS5003_module_VALUE_3_MSB1,
        PMS5003_module_VALUE_3_MSB2,
        PMS5003_module_VALUE_3_LSB1,
        PMS5003_module_VALUE_3_LSB2,

        PMS5003_REGISTER_ENUM_END,
    };

    class PMS5003Module : public BasicModule
    {
    public:
        PMS5003Module() = default;
        ~PMS5003Module() = default;

        static float get_module_1_0_um(TwoWire &i2cHandle, uint8_t address);
        static float get_module_2_5_um(TwoWire &i2cHandle, uint8_t address);
        static float get_module_10_um(TwoWire &i2cHandle, uint8_t address);

        static float raw_to_1_0_um(uint32_t raw);
        static float raw_to_2_5_um(uint32_t raw);
        static float raw_to_10_um(uint32_t raw);

        static uint32_t get_raw_module_1_0_um(TwoWire &i2cHandle, uint8_t address);
        static uint32_t get_raw_module_2_5_um(TwoWire &i2cHandle, uint8_t address);
        static uint32_t get_raw_module_10_um(TwoWire &i2cHandle, uint8_t address);
    };

    float PMS5003Module::get_module_1_0_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        return raw_to_1_0_um(data);
    }

    float PMS5003Module::get_module_2_5_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, PMS5003_Register::PMS5003_module_VALUE_2_MSB1, &value);
        return raw_to_2_5_um(value);
    }

    float PMS5003Module::get_module_10_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, PMS5003_Register::PMS5003_module_VALUE_3_MSB1, &value);
        return raw_to_10_um(value);
    }

    float PMS5003Module::raw_to_1_0_um(uint32_t raw)
    {
        return (float)raw;
    }
    float PMS5003Module::raw_to_2_5_um(uint32_t raw)
    {
        return (float)raw;
    }
    float PMS5003Module::raw_to_10_um(uint32_t raw)
    {
        return (float)raw;
    }

    uint32_t PMS5003Module::get_raw_module_1_0_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        return data;
    }

    uint32_t PMS5003Module::get_raw_module_2_5_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, PMS5003_Register::PMS5003_module_VALUE_2_MSB1, &value);
        return value;
    }

    uint32_t PMS5003Module::get_raw_module_10_um(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, PMS5003_Register::PMS5003_module_VALUE_3_MSB1, &value);
        return value;
    }
}

#endif /* ATMOMETER_PMS5003_MODULE_DRIVER_H */