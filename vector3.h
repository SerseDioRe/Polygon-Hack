#include <Windows.h>
#include <iostream>
#include <math.h>

#include <d3d9types.h>

#define UCONST_Pi           3.1415926535
#define RadianToURotation 180.0f / UCONST_Pi

class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	float x;
	float y;
	float z;

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)) / 100.0F);
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	inline float Length()
	{
		return sqrtf((x * x) + (y * y) + (z * z));
	}

};

struct Vector2
{
public:
	float x;
	float y;
	inline Vector2() : x(0), y(0) {}
	inline Vector2(float x, float y) : x(x), y(y) {}
	inline float Distance(Vector2 v)
	{
		return sqrtf(((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y)));
	}
	inline Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}
	inline Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}
};