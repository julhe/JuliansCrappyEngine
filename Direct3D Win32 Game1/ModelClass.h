#pragma once
#include "pch.h"
#include <vector>
using namespace DirectX;
class ModelClass {
public:
	struct VertexInputType {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT4 Color;
		XMFLOAT2 UV;
	};
	ModelClass();

	int GetIndexCount() const;
	bool Initalize(ID3D11Device*);
	void Release();
	void SetActive(ID3D11DeviceContext*);
	void SetVerticies(std::vector<VertexInputType>*, ID3D11Device*);
	void SetIndicies(std::vector<unsigned long>*, ID3D11Device*);

private:
	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int m_vertexCount, m_indexCount;
};