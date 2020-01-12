#ifndef VECTOR_H
#define VECTOR_H

class Vector3
{
public:
	float x;
	float y;
	float z;
	Vector3();
	Vector3(float x_, float y_, float z_);
	Vector3(float x_, float y_);
	bool isEmpty();
	void normalize();
};

#endif 