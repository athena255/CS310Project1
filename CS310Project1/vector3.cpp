#include "vector3.h"
#include <math.h>

constexpr float PI = 3.1415926535897932384626433832f;

Vector3::Vector3() { z = 0; }

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

Vector3::Vector3(const Vector3& obj)
{
	x = obj.x; 
	y = obj.y;
	z = obj.z;
}

void subtract(const Vector3& src, const Vector3& dst, Vector3* diff)
{
	diff->x = src.x - dst.x;
	diff->y = src.y - dst.y;
	diff->z = src.z - dst.z;
}

void norm(Vector3& vec)
{
	float rad = mag(vec);
	if (rad == 0) return;

	vec.x /= rad;
	vec.y /= rad;
	vec.z /= rad;
}

float mag(const Vector3& vec)
{
	return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

void Vector3::vecAdd(const Vector3& toAdd)
{
	x += toAdd.x;
	y += toAdd.y;
	z += toAdd.z;
}
void Vector3::vecScale(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

float dist(const Vector3& src, const Vector3& dst)
{
	Vector3 pNewAng;
	subtract(src, dst, &pNewAng);
	return(mag(pNewAng));
}
void calcAngle(const Vector3& src, const Vector3& dst, Vector3* newAngle)
{
	float radToDeg = 180.0 / PI;
	Vector3 angleTo;
	subtract(dst, src, &angleTo);

	// atan2 undefined for (x, y) == 0
	if (angleTo.y == 0 && angleTo.x == 0)
	{
		newAngle->y = 0;
		newAngle->x = 0;
	}
	else
	{
		newAngle->y = (atan2(angleTo.y, angleTo.x)*radToDeg);
		float tmp = sqrt(angleTo.x*angleTo.x + angleTo.y*angleTo.y);
		newAngle->x = (atan2(-angleTo.z, tmp)*radToDeg);
		if (angleTo.x <= -360)
		{
			newAngle->y -= 360;
		}
	}
	newAngle->z = 0;
}

void clamp(const Vector3& oldAngle, Vector3& newAng) {
	float length = newAng.y - oldAngle.y;
	if (length >= 180)
		newAng.y -= 360;
	else if (length <= -180)
		newAng.y += 360;

}

float innerProd(const Vector3& u, const Vector3& v)
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

void copy(Vector3& src, const Vector3& dst)
{
	src.x = dst.x;
	src.y = dst.y;
	src.z = dst.z;
}
