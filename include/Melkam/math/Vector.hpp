#pragma once

#include <cmath>

namespace Melkam
{
    struct Vector2f
    {
        float x = 0.0f;
        float y = 0.0f;

        Vector2f() = default;
        Vector2f(float xValue, float yValue) : x(xValue), y(yValue) {}

        Vector2f operator+(const Vector2f &other) const { return {x + other.x, y + other.y}; }
        Vector2f operator-(const Vector2f &other) const { return {x - other.x, y - other.y}; }
        Vector2f operator*(float scalar) const { return {x * scalar, y * scalar}; }
        Vector2f operator/(float scalar) const { return {x / scalar, y / scalar}; }

        Vector2f &operator+=(const Vector2f &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2f &operator-=(const Vector2f &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vector2f &operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        float &operator[](int index)
        {
            return index == 0 ? x : y;
        }

        const float &operator[](int index) const
        {
            return index == 0 ? x : y;
        }

        float length() const { return std::sqrt(x * x + y * y); }

        Vector2f normalized() const
        {
            const float len = length();
            return len > 0.0f ? Vector2f{x / len, y / len} : Vector2f{};
        }
    };

    struct Vector3f
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vector3f() = default;
        Vector3f(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}

        Vector3f operator+(const Vector3f &other) const { return {x + other.x, y + other.y, z + other.z}; }
        Vector3f operator-(const Vector3f &other) const { return {x - other.x, y - other.y, z - other.z}; }
        Vector3f operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
        Vector3f operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }

        Vector3f &operator+=(const Vector3f &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vector3f &operator-=(const Vector3f &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vector3f &operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        float &operator[](int index)
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }

        const float &operator[](int index) const
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }

        float length() const { return std::sqrt(x * x + y * y + z * z); }

        Vector3f normalized() const
        {
            const float len = length();
            return len > 0.0f ? Vector3f{x / len, y / len, z / len} : Vector3f{};
        }
    };

    struct Vector2i
    {
        int x = 0;
        int y = 0;

        Vector2i() = default;
        Vector2i(int xValue, int yValue) : x(xValue), y(yValue) {}

        int &operator[](int index)
        {
            return index == 0 ? x : y;
        }

        const int &operator[](int index) const
        {
            return index == 0 ? x : y;
        }
    };

    struct Vector3i
    {
        int x = 0;
        int y = 0;
        int z = 0;

        Vector3i() = default;
        Vector3i(int xValue, int yValue, int zValue) : x(xValue), y(yValue), z(zValue) {}

        int &operator[](int index)
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }

        const int &operator[](int index) const
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }
    };
}
