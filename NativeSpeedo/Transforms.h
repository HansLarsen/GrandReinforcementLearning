#pragma once

#include "..\..\inc\types.h"
#include <math.h>
double pi = 22 / 7;

struct transformMatrix {
	float a[4][4];

	/*float a00 = 0.0f, a01 = 0.0f, a02 = 0.0f, a03 = 0.0f;
	float a10 = 0.0f, a11 = 0.0f, a12 = 0.0f, a13 = 0.0f;
	float a20 = 0.0f, a21 = 0.0f, a22 = 0.0f, a23 = 0.0f;
	float a30 = 0.0f, a31 = 0.0f, a32 = 0.0f, a33 = 1.0f;*/
};

transformMatrix createMatrix(Vector3 position, Vector3 rotation) {
	rotation.x = rotation.x * (pi / 180.0);
	rotation.y = rotation.y * (pi / 180.0);
	rotation.z = rotation.z * (pi / 180.0);

	transformMatrix trix;
	trix.a[0][3] = position.x;
	trix.a[1][3] = position.y;
	trix.a[2][3] = position.z;

	trix.a[0][0] = cos(rotation.z) * cos(rotation.y);
	trix.a[1][0] = sin(rotation.z) * cos(rotation.y);
	trix.a[2][0] = -sin(rotation.y);

	trix.a[0][1] = cos(rotation.z) * sin(rotation.y) * sin(rotation.z) - sin(rotation.z) * cos(rotation.x);
	trix.a[1][1] = sin(rotation.z) * sin(rotation.y) * sin(rotation.z) + cos(rotation.z) * cos(rotation.x);
	trix.a[2][1] = cos(rotation.y) * sin(rotation.x);

	trix.a[0][2] = cos(rotation.x) * sin(rotation.y) * cos(rotation.x) + sin(rotation.z) * sin(rotation.x);
	trix.a[1][2] = cos(rotation.x) * sin(rotation.y) * sin(rotation.x) - cos(rotation.z) * sin(rotation.x);
	trix.a[2][2] = cos(rotation.y) * cos(rotation.x);

	trix.a[3][3] = 1.0f;

	return trix;
}

Vector3 getPosition(transformMatrix input) {
	Vector3 return_vec = Vector3();
	return_vec.x = input.a[0][3];
	return_vec.y = input.a[1][3];
	return_vec.z = input.a[2][3];
	return return_vec;
}

Vector3 getRotation(transformMatrix input) {
	double phi = atan2(input.a[2][1], input.a[2][2]);
	double psi = atan2(input.a[1][0], input.a[0][0]);
	double thetaa = 0.0;

	if (cos(psi) == 0) {
		thetaa = atan2(-input.a[2][0], (input.a[1][0] / sin(psi)));
	}
	else
	{
		thetaa = atan2(-input.a[2][0], (input.a[0][0] / sin(psi)));
	}

	double s = atan2((double)input.a[0][0], -sqrt((double)input.a[2][1] * (double)input.a[2][1] + (double)input.a[2][2] * (double)input.a[2][2]));

	Vector3 return_vec = Vector3();
	return_vec.x = phi;// * (180.0 / pi);
	return_vec.y = thetaa;// * (180.0 / pi);
	return_vec.z = psi;// * (180.0 / pi);
	return return_vec;
}

Vector3 transformVector(transformMatrix input, Vector3 position) {
	Vector3 return_vec = Vector3();
	return_vec.x = input.a[0][0] * position.x + input.a[0][1] * position.y + input.a[0][2] * position.z + input.a[0][3];
	return_vec.y = input.a[1][0] * position.x + input.a[1][1] * position.y + input.a[1][2] * position.z + input.a[1][3];
	return_vec.z = input.a[2][0] * position.x + input.a[2][1] * position.y + input.a[2][2] * position.z + input.a[2][3];
	return return_vec;
}

transformMatrix transformTransform(transformMatrix input, transformMatrix position) {
	transformMatrix mul = transformMatrix();

	for (int p = 0; p < 4; p++)
	{
		for (int j = 0; j < 4; j++)
		{
			mul.a[p][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				mul.a[p][j] += input.a[p][k] * position.a[k][j];
			}
		}
	}

	return mul;
}