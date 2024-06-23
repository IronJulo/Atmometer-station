#ifndef ATMOMETER_BASIC_MODULE_DRIVER_H
#define ATMOMETER_BASIC_MODULE_DRIVER_H

#include <stdint.h>
#include <Wire.h>

// 1 if set else 0
#define GET_BIT_VALUE_AT(variable, position) \
    (((variable) & (1 << (position))) >> (position))

#define SET_BIT_VALUE_AT(variable, position, value) \
    ((variable) |= ((value) << (position)))

namespace Atmometer
{
    enum Register
    {
        // Read Only
        module_TYPE_MSB,
        module_TYPE_LSB,

        module_ID_MSB1,
        module_ID_MSB2,
        module_ID_LSB1,
        module_ID_LSB2,

        module_VALUE_1_MSB1,
        module_VALUE_1_MSB2,
        module_VALUE_1_LSB1,
        module_VALUE_1_LSB2,

        module_ERROR_MSB,
        module_ERROR_LSB,

        // Read/Write
        module_CONFIG = 0X81,
        module_UPDATE_PERIOD,
        module_READ_ADDRESS,

        REGISTER_ENUM_END,
    };

#define BASIC_REGISTER_READ_ONLY_START module_TYPE_MSB
#define BASIC_REGISTER_READ_ONLY_END module_ERROR_LSB

#define BASIC_REGISTER_READ_WRITE_START module_CONFIG
#define BASIC_REGISTER_READ_WRITE_end module_READ_ADDRESS

    enum ConfigBitField
    {
        CBF_READ_IN_PROGRESS,
        CBF_IDLE,
        CBF_SEQUENTIAL_READ,
    };

    class BasicModule
    {
    public:
        BasicModule() = default;
        ~BasicModule() = default;

        static uint8_t set_module_config(TwoWire &i2cHandle, uint8_t address, uint8_t value);
        static uint8_t set_module_config_read_in_progress(TwoWire &i2cHandle, uint8_t address, bool value);
        static uint8_t set_module_config_idle(TwoWire &i2cHandle, uint8_t address, bool value);
        static uint8_t set_module_config_sequential_read(TwoWire &i2cHandle, uint8_t address, bool value);

        static uint8_t set_module_update_period(TwoWire &i2cHandle, uint8_t address, uint8_t value);
        static uint8_t set_module_read_address(TwoWire &i2cHandle, uint8_t address, uint8_t value);

        static uint16_t get_module_type(TwoWire &i2cHandle, uint8_t address);
        static uint64_t get_module_id(TwoWire &i2cHandle, uint8_t address);
        static uint64_t get_module_value_1(TwoWire &i2cHandle, uint8_t address);
        static uint16_t get_module_error(TwoWire &i2cHandle, uint8_t address);
        static uint8_t get_module_config(TwoWire &i2cHandle, uint8_t address);
        static uint8_t get_module_update_period(TwoWire &i2cHandle, uint8_t address);
    };

    uint8_t simple_write(TwoWire &i2cHandle, uint8_t i2cAddress, uint8_t registerAddress, uint8_t value)
    {
        uint8_t error;

        Wire.beginTransmission(i2cAddress);
        Wire.write(registerAddress);
        Wire.write(value);
        error = Wire.endTransmission();
        return error;
    }

    uint8_t simple_read(TwoWire &i2cHandle, uint8_t i2cAddress, uint8_t registerAddress, uint8_t *value)
    {
        uint8_t error = simple_write(i2cHandle, i2cAddress, Register::module_READ_ADDRESS, registerAddress);
        // TODO handle errors
        Wire.requestFrom(i2cAddress, 1);
        uint8_t realValue = Wire.read();

        *value = realValue;

        return error;
    }

    uint8_t simple_read(TwoWire &i2cHandle, uint8_t i2cAddress, uint8_t registerAddress, uint16_t *value)
    {
        uint8_t error = 0;
        error = simple_write(i2cHandle, i2cAddress, Register::module_READ_ADDRESS, registerAddress);

        *value = 0;
        uint8_t read = 0;

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 8);

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 0);

        return error;
    }

    uint8_t simple_read(TwoWire &i2cHandle, uint8_t i2cAddress, uint8_t registerAddress, uint64_t *value)
    {
        uint8_t error = 0;
        error = simple_write(i2cHandle, i2cAddress, Register::module_READ_ADDRESS, registerAddress);

        *value = 0;
        uint8_t read = 0;

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 24);

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 16);

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 8);

        Wire.requestFrom(i2cAddress, 1);
        read = Wire.read();
        *value += (read << 0);

        return error;
    }

    uint8_t BasicModule::set_module_config(TwoWire &i2cHandle, uint8_t address, uint8_t value)
    {
        return simple_write(i2cHandle, address, Register::module_CONFIG, value);
    }

    uint8_t BasicModule::set_module_config_read_in_progress(TwoWire &i2cHandle, uint8_t address, bool value)
    {
        uint8_t config_value = 0;
        uint8_t error = simple_read(i2cHandle, address, Register::module_CONFIG, &config_value);

        if (value)
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_READ_IN_PROGRESS, 1);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        else
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_READ_IN_PROGRESS, 0);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        return error;
    }

    uint8_t BasicModule::set_module_config_idle(TwoWire &i2cHandle, uint8_t address, bool value)
    {
        uint8_t config_value = 0;
        uint8_t error = simple_read(i2cHandle, address, Register::module_CONFIG, &config_value);

        if (value)
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_IDLE, 1);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        else
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_IDLE, 0);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        return error;
    }

    uint8_t BasicModule::set_module_config_sequential_read(TwoWire &i2cHandle, uint8_t address, bool value)
    {
        uint8_t config_value = 0;
        uint8_t error = simple_read(i2cHandle, address, Register::module_CONFIG, &config_value);

        if (value)
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_SEQUENTIAL_READ, 1);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        else
        {
            SET_BIT_VALUE_AT(config_value, ConfigBitField::CBF_SEQUENTIAL_READ, 0);
            error = simple_write(i2cHandle, address, Register::module_CONFIG, config_value);
        }
        return error;
    }

    uint8_t BasicModule::set_module_update_period(TwoWire &i2cHandle, uint8_t address, uint8_t value)
    {
        return simple_write(i2cHandle, address, Register::module_UPDATE_PERIOD, value);
    }

    uint8_t BasicModule::set_module_read_address(TwoWire &i2cHandle, uint8_t address, uint8_t value)
    {
        return simple_write(i2cHandle, address, Register::module_READ_ADDRESS, value);
    }

    uint16_t BasicModule::get_module_type(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint16_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_TYPE_MSB, &value);

        return value;
    }

    uint64_t BasicModule::get_module_id(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_ID_MSB1, &value);

        return value;
    }

    uint64_t BasicModule::get_module_value_1(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint64_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_VALUE_1_MSB1, &value);

        return value;
    }

    uint16_t BasicModule::get_module_error(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint16_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_ERROR_MSB, &value);

        return value;
    }

    uint8_t BasicModule::get_module_config(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint8_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_CONFIG, &value);

        return value;
    }

    uint8_t BasicModule::get_module_update_period(TwoWire &i2cHandle, uint8_t address)
    {
        uint8_t error;
        uint8_t value = 0;
        error = simple_read(i2cHandle, address, Register::module_UPDATE_PERIOD, &value);

        return value;
    }
}

#endif /* ATMOMETER_BASIC_MODULE_DRIVER_H */