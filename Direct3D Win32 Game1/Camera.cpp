#include "pch.h"
#include "Camera.h"
#include <DirectXPackedVector.h>

CameraClass::CameraClass() 
{
	Position = XMFLOAT3(0.f,0.f,0.f);
	Rotation = XMFLOAT3(0.f, 0.f, 0.f);
}

void CameraClass::SetActive()
{
	XMFLOAT3 up, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = XMConvertToRadians(Rotation.x);
	yaw = XMConvertToRadians(Rotation.y);
	roll = XMConvertToRadians(Rotation.z);

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(roll, pitch, yaw);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	XMStoreFloat3(&lookAt, XMVector3Transform(XMLoadFloat3(&lookAt), rotationMatrix));
	XMStoreFloat3(&up, XMVector3Transform(XMLoadFloat3(&up), rotationMatrix));

	// Translate the rotated camera position to the location of the viewer.
	//lookAt = position + lookAt;
	XMStoreFloat3(&lookAt, XMLoadFloat3(&Position) + XMLoadFloat3(&lookAt));

	// Finally create the view matrix from the three updated vectors.
	viewMatrix = XMMatrixLookAtLH(
		XMLoadFloat3(&Position),
		XMLoadFloat3(&lookAt),
		XMLoadFloat3(&up));
	//D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);
	projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(FOV), AspectRatio, ClipNear, ClipFar);
	worldToClip = viewMatrix * projectionMatrix;
}

XMMATRIX CameraClass::GetViewMatrix(){ return viewMatrix; }
XMMATRIX CameraClass::GetWorldToClip() { return worldToClip; }

void CameraClass::Release() {}