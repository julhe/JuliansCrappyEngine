#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Material.h"
using namespace DirectX;

class Shader {

public:
	struct PerDrawCallBuffer {		
		XMFLOAT4X4 objectToClip;
		XMFLOAT4 camera_positionWS;
	};
	Shader();
	bool Initalize(ID3D11Device*, HWND);
	void Release();

	bool SetActive(ID3D11DeviceContext*);
	bool SetShaderParameters(ID3D11DeviceContext*, XMFLOAT3, XMMATRIX, std::shared_ptr<Material>);
private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

private:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11Buffer* perDrawCallBuffer;
};
