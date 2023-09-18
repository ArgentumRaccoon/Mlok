#include "MlokClock.h"
#include "platform/Platform.h"

void MlokClock::Update()
{
    if (StartTime != 0.f)
    {
        Elapsed = Platform::Get()->GetAbsoluteTime() - StartTime;
    }
}

void MlokClock::Start()
{
    StartTime = Platform::Get()->GetAbsoluteTime();
    Elapsed = 0.f;
}

void MlokClock::Stop()
{
    StartTime = 0.f;
}

double MlokClock::GetStartTime() const
{
    return StartTime;
}

double MlokClock::GetElapsed() const
{
    return Elapsed;
}
