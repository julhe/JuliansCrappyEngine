#pragma once
#include "pch.h"
#include "ECS.h"
#include "DXUtility.h"
#include "Shader.h"
#include "ModelClass.h"
#include "Renderer.h"
#include "Material.h"

ECS_TYPE_IMPLEMENTATION;

using namespace ECS;
using namespace DirectX;

namespace Types {
	struct Transform {
		ECS_DECLARE_TYPE;
		XMFLOAT3 position = XMFLOAT3(0,0,0), scale = XMFLOAT3(1, 1, 1);
		XMFLOAT4 rotation = XMFLOAT4(0,0,0,1);

		Transform(XMFLOAT3 position, XMFLOAT4 rotation, XMFLOAT3 scale) 
			: position(position), rotation(rotation), scale(scale) {}
		Transform() {}

		inline XMMATRIX ObjectToWorldMatrix() const {
			return XMMatrixAffineTransformation(
				XMLoadFloat3(&scale),
				XMLoadFloat3(&position),
				XMLoadFloat4(&rotation),
				XMLoadFloat3(&position));
		}
	};
	ECS_DEFINE_TYPE(Transform);

	struct MeshRenderer {
		ECS_DECLARE_TYPE;
		std::shared_ptr<ModelClass> mesh;
		std::shared_ptr<Material> material;
		MeshRenderer(
			std::shared_ptr<ModelClass> mesh,
			std::shared_ptr<Material> material)
			: mesh(mesh), material(material) {}
	}; 
	ECS_DEFINE_TYPE(MeshRenderer);

	struct Camera {
		ECS_DECLARE_TYPE;
		float clipNear, clipFar, FOV;
		Camera(float clipNear, float clipFar, float FOV) : clipNear(clipNear), clipFar(clipFar), FOV(FOV) {}
	};
	ECS_DEFINE_TYPE(Camera);
}