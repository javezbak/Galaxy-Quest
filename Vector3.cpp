#include "Vector3.h"
#include <cmath>
//constructors
Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f){};
Vector3::Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_){};
Vector3::Vector3(float x_, float y_) : x(x_), y(y_), z(0.0f){};

void Vector3::normalize(){
	//float xNormal = y;
	//float yNormal = -x;
	float length = sqrt((x*x) + (y*y));
	x /= length;
	y /= length;
}

bool Vector3::isEmpty(){ return (x == 0.0f && y == 0.0f && z == 0.0f) ? true : false; }