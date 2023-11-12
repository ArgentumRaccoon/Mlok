#pragma once

#include "Defines.h"

typedef union Vec2
{
    float Elements[2];
    struct
    {
        union
        {
            float X, S, U;
        };
        union
        {
            float Y, T, V;
        };
    };
} Vec2;

typedef union Vec3
{
    float Elements[3];
    struct
    {
        union
        {
            float X, R, U;
        };
        union
        {
            float Y, G, V;
        };
        union
        {
            float Z, B, W;
        };
    };
} Vec3;

typedef union Vec4
{
#ifdef MUSE_SIMD
    alignas(16) __m128 Data;
#endif
    alignas(16) float Elements[4];
    struct
    {
        union
        {
            float X, R, S;
        };
        union
        {
            float Y, G, T;
        };
        union
        {
            float Z, B, P;
        };
        union
        {
            float W, A, Q;
        };
    };
} Vec4;

typedef Vec4 Quat;


typedef union Mat4
{
    alignas(16) float Data[16];
#ifdef MUSE_SIMD
    alignas(16) Vec4 Rows[4];
#endif
} Mat4;

typedef struct Vertex3D
{
    Vec3 Position;
} Vertex3D;