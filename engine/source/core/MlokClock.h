#pragma once

#include "Defines.h"

class MAPI MlokClock
{
    private:
        double StartTime;
        double Elapsed;

    public:
        void Update();
        void Start();
        void Stop();

        double GetStartTime() const;
        double GetElapsed() const;
};