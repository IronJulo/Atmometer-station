#ifndef ATMOMETER_MH_Z19_MODULE_DRIVER_H
#define ATMOMETER_MH_Z19_MODULE_DRIVER_H

#include "atmometer_basic_module_driver.h"

namespace Atmometer
{
    class MH_Z19Module : public BasicModule
    {
    public:
        MH_Z19Module() = default;
        ~MH_Z19Module() = default;

        static float get_module_co2_ppm(TwoWire &i2cHandle, uint8_t address);
    };

    float MH_Z19Module::get_module_co2_ppm(TwoWire &i2cHandle, uint8_t address)
    {
        uint64_t data = BasicModule::get_module_value_1(i2cHandle, address);
        return (float)data;
    }
}

#endif /* ATMOMETER_MH_Z19_MODULE_DRIVER_H */