#pragma once
#include "pch.h"
#include <vector>
namespace DXHelper{
	template <typename T>
	HRESULT CreateBuffer(
		std::vector<T>* bufferData, 
		D3D11_USAGE usage, 
		D3D11_BIND_FLAG bindFlags, 
		ID3D11Device* device, 
		ID3D11Buffer* &buffer) 
	{
		SAFE_RELEASE(buffer);

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = usage;
		desc.ByteWidth = sizeof(T) * bufferData->size();
		desc.BindFlags = bindFlags;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &bufferData->front();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		return device->CreateBuffer(&desc, &data, &buffer);
	}
};

