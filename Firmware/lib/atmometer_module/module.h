#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

class Socket
{
private:
    uint8_t m_SocketID = 0;
    uint8_t m_ModuleType = 0;
    uint32_t m_ModuleID = 0;

    bool m_ModulePresent = false;

public:
    Socket() = default;
    ~Socket() = default;

    void setSocketID(uint8_t socketID)
    {
        m_SocketID = socketID;
    }

    uint8_t getSocketID() const
    {
        return m_SocketID;
    }

    void setModuleType(uint8_t moduleType)
    {
        m_ModuleType = moduleType;
    }

    uint8_t getModuleType() const
    {
        return m_ModuleType;
    }

    void setModuleID(uint32_t moduleID)
    {
        m_ModuleID = moduleID;
    }

    uint32_t getModuleID() const
    {
        return m_ModuleID;
    }

    void setModulePresent(bool modulePresent)
    {
        m_ModulePresent = modulePresent;
    }

    bool isModulePresent() const
    {
        return m_ModulePresent;
    }
};

#endif /* MODULE_H */