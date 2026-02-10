#pragma once

namespace Melkam
{
    struct Quaternionf
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f;

        Quaternionf() = default;
        Quaternionf(float xValue, float yValue, float zValue, float wValue)
            : x(xValue), y(yValue), z(zValue), w(wValue)
        {
        }

        static Quaternionf identity() { return Quaternionf{}; }
    };
}
