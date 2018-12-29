#include "ModelClass.h"
#include "DXUtility.h"
ModelClass::ModelClass() : m_vertexCount(0), m_indexCount(0), vertexBuffer(0), indexBuffer(0){}

bool ModelClass::Initalize(ID3D11Device* device) {
	std::vector<VertexInputType> vertices = std::vector<VertexInputType>(3);
	vertices[0].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[0].Color = XMFLOAT4(1.0f, 0.f, 0.f, 1.f);
	vertices[1].Position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.f);
	vertices[2].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.f);
	SetVerticies(&vertices, device);

	std::vector<unsigned long> indicesVec = std::vector<unsigned long>();
	indicesVec.push_back(0);
	indicesVec.push_back(1);
	indicesVec.push_back(2);
	SetIndicies(&indicesVec, device);

	return true;
}

void ModelClass::Release() {
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(vertexBuffer);
}

void ModelClass::SetIndicies(std::vector<unsigned long>* indices, ID3D11Device* device) {
	m_indexCount = indices->size();

	DXHelper::CreateBuffer(
		indices, 
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER, 
		device,
		indexBuffer);
}

void ModelClass::SetVerticies(std::vector<VertexInputType>* verticies, ID3D11Device* device) {
	m_vertexCount = verticies->size();
	DXHelper::CreateBuffer(
		verticies,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		device,
		vertexBuffer);
}
void ModelClass::SetActive(ID3D11DeviceContext* deviceContext) {
	unsigned int stride = sizeof(VertexInputType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int ModelClass::GetIndexCount() const { return m_indexCount; }