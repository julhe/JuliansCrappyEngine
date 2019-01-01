#include "pch.h"
#include "Texture2D.h"
#include "WICTextureLoader\WICTextureLoader.h"
#include "DDSTextureLoader\DDSTextureLoader.h"
// create texture from path
Texture2D::Texture2D(std::wstring path, ID3D11Device* device, ID3D11DeviceContext* context, bool linear ) : name(path)
{
	
	//auto result = DirectX::CreateWICTextureFromFile(
	//	device, 
	//	context, 
	//	path.c_str(),
	//	textureResource.GetAddressOf(), 
	//	textureView.GetAddressOf());
	auto result = DirectX::CreateDDSTextureFromFileEx(
		device, //device
		context, //context
		path.c_str(), //file
		0, //maxsize
		D3D11_USAGE_DEFAULT, // usage
		D3D11_BIND_SHADER_RESOURCE, // bindflags
		0, // cpu fla
		0, // misc
		linear ? DirectX::WIC_LOADER_FLAGS::WIC_LOADER_IGNORE_SRGB : DirectX::WIC_LOADER_FLAGS::WIC_LOADER_DEFAULT,
		textureResource.GetAddressOf(),
		textureView.GetAddressOf());

	//auto result = DirectX::CreateWICTextureFromFileEx(
	//	device,
	//	context,
	//	path.c_str(),
	//	0,
	//	D3D11_USAGE_DEFAULT,
	//	D3D11_BIND_SHADER_RESOURCE,
	//	0,
	//	0,
	//	linear ? DirectX::WIC_LOADER_FLAGS::WIC_LOADER_IGNORE_SRGB : DirectX::WIC_LOADER_FLAGS::WIC_LOADER_DEFAULT,
	//	textureResource.GetAddressOf(),
	//	textureView.GetAddressOf());

	DX::ThrowIfFailed(result);
	D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;

	DX::ThrowIfFailed(device->CreateSamplerState(&desc, samplerState.GetAddressOf()));
}



