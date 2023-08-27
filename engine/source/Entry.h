#include "core/Application.h"

static ApplicationConfig AppConfig;
static Application App;

int main()
{
    AppConfig.StartPosX = 50;
    AppConfig.StartPosY = 50;
    AppConfig.StartWidth = 1280;
    AppConfig.StartHeight = 720;
    AppConfig.Name = (char*)"Mlok Engine";

    if (!App.Create(AppConfig))
    {
        return 1;
    }

    if (!App.Run())
    {
        return 2;
    }
    
    return 0;
}