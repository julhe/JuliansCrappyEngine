#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class CameraClass
{
public:
	CameraClass();
	XMFLOAT3 Position, Rotation;
	float FOV = 60.0f, AspectRatio, ClipNear = 1.0f, ClipFar = 4000.0f;

	void SetActive();
	XMMATRIX GetViewMatrix();
	XMMATRIX GetWorldToClip();
	void Release();

private:
	
	XMMATRIX viewMatrix, projectionMatrix, worldToClip;
};
