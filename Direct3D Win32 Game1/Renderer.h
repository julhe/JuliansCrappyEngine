#pragma once
#include <vector>
#include "pch.h"
#include "ModelClass.h"
#include "Shader.h"
#include "entt\entt.hpp"
#include "ECS_Types.h"
#include "Camera.h"
#include "Texture2D.h"
using Microsoft::WRL::ComPtr;
using std::shared_ptr;

struct Drawcall {
	ModelClass* model;
	Shader* shader;
	XMMATRIX transform;
	Texture2D* texture;
};

class Renderer
{
private:
	std::vector<Drawcall> drawQueue = std::vector<Drawcall>();
	ComPtr<ID3D11Device1>           m_d3dDevice;
	ComPtr<ID3D11DeviceContext1>    m_d3dContext;
public:
	void SetD3DDevice(ID3D11Device1*);
	Renderer(ID3D11DeviceContext1*);
	void DrawFrame(CameraClass*);
	void AddToFrame(Drawcall);

	void RenderCamera(entt::registry<> &registry, CameraClass* camera);

	~Renderer();
};

