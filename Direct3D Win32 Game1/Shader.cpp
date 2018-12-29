#include "pch.h"
#include "Shader.h"
#include <d3d10shader.h>
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>
#include "Utility.h"
#include "Material.h"

Shader::Shader() : vertexShader(NULL), pixelShader(NULL), layout(NULL), perDrawCallBuffer(NULL) {}

bool Shader::Initalize(ID3D11Device* device, HWND hwnd) {
	return InitializeShader(device, hwnd, L"UnlitVS.hlsl", L"UnlitPS.hlsl");
}

bool Shader::SetActive(ID3D11DeviceContext* deviceContext)
{
	bool result = true;

	// Set the shader parameters that it will use for rendering.
	//result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);

	return true;
}

//TODO: this is a shit function, and you know it!
long readFileBytes(const char *name, char*& bytes)
{
	FILE *fl;
	fopen_s(&fl, name, "rb");
	fseek(fl, 0, SEEK_END);
	long len = ftell(fl);
	bytes = (char*)malloc(len);
	fseek(fl, 0, SEEK_SET);
	fread(bytes, 1, len, fl);
	fclose(fl);
	return len;
}

bool Shader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	ID3D10Blob* errorMessage;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	char* shaderVS = { 0 };
	size_t shaderVSsize = readFileBytes("UnlitVS.cso", shaderVS);

	// Create the vertex shader from the buffer.
	DX::ThrowIfFailed(device->CreateVertexShader((void*)shaderVS, shaderVSsize, NULL, &vertexShader));

	char* shaderPS = { 0 };
	size_t shaderPSsize = readFileBytes("UnlitPS.cso", shaderPS);
	// Create the pixel shader from the buffer.
	DX::ThrowIfFailed(device->CreatePixelShader((void*)shaderPS, shaderPSsize, NULL, &pixelShader));

	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{"POSITION"	, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0								, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT	, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT	, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT		, 0, D3D11_APPEND_ALIGNED_ELEMENT	, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Get a count of the elements in the layout.
	size_t numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	DX::ThrowIfFailed(device->CreateInputLayout(
		polygonLayout,
		numElements,
		shaderVS,
		shaderVSsize,
		&layout));


	PerDrawCallBuffer perCallBuferData;
	memset(&perCallBuferData, 0, sizeof(PerDrawCallBuffer));
	perCallBuferData.objectToClip = XMFLOAT4X4();
	perCallBuferData.camera_positionWS = XMFLOAT4(0,0,0,0);
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &perCallBuferData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(PerDrawCallBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	perDrawCallBuffer = NULL;
	 //Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	DX::ThrowIfFailed(device->CreateBuffer(
		&cbDesc, 
		&InitData,
		&perDrawCallBuffer));

	return true;
}


void Shader::Release()
{
	SAFE_RELEASE(perDrawCallBuffer);
	SAFE_RELEASE(layout);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(vertexShader);
}

void Shader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool Shader::SetShaderParameters(
	ID3D11DeviceContext* deviceContext,
	XMFLOAT3 cameraPosition,
	XMMATRIX objectToClip, 
	std::shared_ptr<Material> mat)
{
	// Lock the constant buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DX::ThrowIfFailed(deviceContext->Map(perDrawCallBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// Get a pointer to the data in the constant buffer.
	PerDrawCallBuffer*  perCallBufferData = (PerDrawCallBuffer*)mappedResource.pData;

	XMStoreFloat4x4(&perCallBufferData->objectToClip, objectToClip);
	perCallBufferData->camera_positionWS = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0);
	// Unlock the constant buffer.
	deviceContext->Unmap(perDrawCallBuffer, 0);

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(0, 1, &perDrawCallBuffer);

	deviceContext->PSSetShaderResources(0, 1, mat->albedo->textureView.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, mat->roughness->textureView.GetAddressOf());
	deviceContext->PSSetShaderResources(2, 1, mat->metalness->textureView.GetAddressOf());
	deviceContext->PSSetShaderResources(3, 1, mat->normal->textureView.GetAddressOf());
	deviceContext->PSSetConstantBuffers(0, 1, &perDrawCallBuffer);
	deviceContext->PSSetSamplers(0, 1, mat->albedo->samplerState.GetAddressOf());
	return true;
}
