#pragma once

#include "Defines.h"

#include "MlokClock.h"
#include "MlokMemory.h"

#include <memory>

typedef struct ApplicationConfig
{
    int16_t StartPosX;
    int16_t StartPosY;
    int16_t StartWidth;
    int16_t StartHeight;
    std::string Name;
} ApplicationConfig;

class MAPI Application
{
    public:
        bool Create(const ApplicationConfig& Config); // TODO: move config to game object

        bool Run();
        void Stop();

        void GetFramebufferSize(uint16_t* OutWidth, uint16_t* OutHeight) const;
        void SetFramebufferSize(const uint16_t inWidth, const uint16_t inHeight);

        bool IsRunning() const;
        bool IsSuspended() const;
        void SetSuspended(const bool bValue);

    private:
        struct AppState
        {
            bool bIsRunning;
            bool bIsSuspended;
            int16_t Width;
            int16_t Height;
            double LastTime;
        } State;

        std::unique_ptr<MlokLinearAllocator> SubsystemsAllocator;

        std::unique_ptr<MlokClock> AppClock;
};