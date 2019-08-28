#pragma once
class Vector3
{
public:
	Vector3(float x, float y, float z);
	Vector3();
	Vector3(const Vector3& obj);
	void vecAdd(const Vector3& toAdd);
	void vecScale(float scale);
	float x, y, z; // x is pitch, y is yaw
};
void subtract(const Vector3& src, const Vector3& dst, Vector3* diff);
void norm(Vector3& vec);
float mag(const Vector3& vec);
float dist(const Vector3& src, const Vector3& dst);
void calcAngle(const Vector3& src, const Vector3& dst, Vector3* newAngle);
void copy(Vector3& src, const Vector3& dst);
float innerProd(const Vector3& u, const Vector3& v);

void clamp(const Vector3& oldAngle, Vector3& newAng);

