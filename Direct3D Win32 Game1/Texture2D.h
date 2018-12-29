#pragma once
#include "pch.h"
#include "WICTextureLoader\WICTextureLoader.h"
#include <string>


using Microsoft::WRL::ComPtr;
class Texture2D
{
public:
	//static shared_ptr<Texture2D> errorTexture, defaultNormalMap;
	Texture2D(std::wstring, ID3D11Device*, ID3D11DeviceContext* , bool = false);

	void SetActive(ID3D11DeviceContext*);
	const std::wstring name;
	ComPtr<ID3D11ShaderResourceView> textureView;
	ComPtr<ID3D11Resource> textureResource;
	ComPtr<ID3D11SamplerState> samplerState;
};

