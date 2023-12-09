#pragma once

#include "Defines.h"
#include "math/MathTypes.h"

typedef struct RenderPacket
{
    float DeltaTime;
} RenderPacket;

typedef struct GlobalUniformObject
{
    Mat4 Projection;
    Mat4 View;

    // By nvidia requirements, need to be aligned to 256 bytes
    Mat4 Reserved0;
    Mat4 Reserved1;
} GlobalUniformObject;