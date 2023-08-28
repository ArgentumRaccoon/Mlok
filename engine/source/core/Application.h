#pragma once

#include "Defines.h"

#include <string>

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

    private:
        struct AppState
        {
            bool bIsRunning;
            bool bIsSuspended;
            int16_t Width;
            int16_t Height;
        } State;

        void GetFramebufferSize(uint32_t* OutWidth, uint32_t* OutHeight) const;
};