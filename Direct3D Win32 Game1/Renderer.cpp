#include "Renderer.h"
#include "pch.h"
#include "ECS.h"
#include "ECS_Types.h"
#include "Camera.h"
using namespace Types;
using namespace ECS;
Renderer::Renderer(
	ID3D11DeviceContext1*  d3dContext) : 
	m_d3dContext(d3dContext)
{}

void Renderer::SetD3DDevice(ID3D11Device1* device) {
	m_d3dDevice = device;
}

void Renderer::AddToFrame(Drawcall drawCall) {
	drawQueue.push_back(drawCall);
}

void Renderer::DrawFrame(CameraClass* camera) {
	if (m_d3dDevice.Get() == nullptr)
		return;

	XMVECTOR cameraPosition = XMLoadFloat3(&camera->Position);
	for each (auto drawCall in drawQueue){
		drawCall.model->SetActive(m_d3dContext.Get());
		drawCall.shader->SetActive(m_d3dContext.Get());
		//drawCall.shader->SetShaderParameters(m_d3dContext.Get(), drawCall.transform, drawCall.texture);
		m_d3dContext->DrawIndexed(drawCall.model->GetIndexCount(), 0, 0);
	}

	drawQueue.clear();
}

void Renderer::RenderCamera(World* world, CameraClass* camera) {
	XMMATRIX worldToClip = camera->GetWorldToClip();
	world->each<Transform, MeshRenderer>([&]
		(Entity* ent, ComponentHandle<Transform> transform, ComponentHandle<MeshRenderer> meshRenderer) -> void {
		meshRenderer->mesh->SetActive(m_d3dContext.Get());
		XMMATRIX objectToWorld = transform->ObjectToWorldMatrix();
		XMMATRIX objectToClip = XMMatrixMultiply(objectToWorld, worldToClip);

		meshRenderer->material->shader->SetActive(m_d3dContext.Get());
		meshRenderer->material->shader->SetShaderParameters(
			m_d3dContext.Get(),
			camera->Position,
			worldToClip,
			meshRenderer->material);

		m_d3dContext->DrawIndexed(meshRenderer->mesh->GetIndexCount(), 0, 0);
	});
}

Renderer::~Renderer()
{
}
