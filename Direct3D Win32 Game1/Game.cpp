//
// Game.cpp
//
#define YAP_IMPL
#define YAP_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "pch.h"
#include "Game.h"
#include "Camera.h"
#include "ModelClass.h"
#include "Shader.h"
#include "Utility.h"
#include "Renderer.h"
#include "OBJ-Loader\Source\OBJ_Loader.h"
#include "Texture2D.h"
#include "ECS_Types.h"
#include "Material.h"
#include <locale>
#include <codecvt>
#include <chrono>


extern void ExitGame();

const std::wstring L_ROOT_ASSET_PATH = L"..//..//Content//";
const std::string	ROOT_ASSET_PATH = "..//..//Content//";

using namespace DirectX;

using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::make_shared;
using std::make_unique;

// save some typing
namespace cr = std::chrono;

// you can replace this with steady_clock or system_clock
typedef cr::high_resolution_clock system_clock;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{}

void StringToWString(std::wstring &ws, const std::string &s) {
	std::wstring wsTmp(s.begin(), s.end());
	ws = wsTmp;
}

std::wstring ToWString(const std::string s) {
	std::wstring wsTmp(s.begin(), s.end());
	return wsTmp;
}

using convert_type = std::codecvt_utf8<wchar_t>;
std::string ToString(const std::wstring& wstr) //todo remove
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

unique_ptr<CameraClass> m_Camera;
unique_ptr<vector<shared_ptr<ModelClass>>> m_Models;

unique_ptr<vector<shared_ptr<Material>>> materials;
shared_ptr<Material> FindMaterial(std::string name) {
	for (size_t i = 0; i < materials->size(); i++) {
		if (materials->at(i)->name == name)
			return materials->at(i);
	}
	return nullptr;
}

unique_ptr<vector<shared_ptr<Texture2D>>> textures = make_unique<vector<shared_ptr<Texture2D>>>();;
shared_ptr<Texture2D> FindTexture(std::wstring name) {
	for (size_t i = 0; i < textures->size(); i++) {
		if (textures->at(i)->name == name)
			return textures->at(i);
	}
	return nullptr;
}

shared_ptr<Shader> m_Shader;
unique_ptr<Renderer> renderer;
unique_ptr<std::string> consoleString;
void Log(std::string string) {
	consoleString->append(string + "\n");
}

shared_ptr<Texture2D> GetTextureOrDefault(
	const std::string path, 
	shared_ptr<Texture2D> defaultTex, 
	ID3D11Device* device, 
	ID3D11DeviceContext* context,
	bool linear = false)
{
	auto start_time = system_clock::now();
	shared_ptr<Texture2D> tex = defaultTex; 
	if (fileExists(path)) {
		std::wstring textureName = ToWString(path);
		tex = FindTexture(textureName);
		if (tex == nullptr) {		
			tex = make_unique<Texture2D>(textureName, device, context, linear);
			textures->push_back(tex);
		}
	}
	auto end_time = system_clock::now();
	auto diff = end_time - start_time;
	auto durration = cr::duration_cast<cr::milliseconds>(diff);
	Log("loaded-tex: " + path + " in " + std::to_string(durration.count()) + "ms");
	return tex;
}

entt::registry registry;
static bool profilerOpened(true);

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
std::string replacePngByDDS(const std::string s) {
	std::string newS = s;
	replace(newS, "PNG", "dds");
	return newS;
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dContext.Get());

	ImGui::StyleColorsDark();
	
	YAP::Init(256, 1, 1024, 256);

	{
		m_Camera = make_unique<CameraClass>();// new CameraClass();
		m_Camera->Position = XMFLOAT3(-1000.0f, 100.0f, 8.0f);
		m_Camera->Rotation = XMFLOAT3(90.0f, 00.0f, 0.0f);
		textures->push_back(make_shared<Texture2D>(L_ROOT_ASSET_PATH + L"gi_flag.DDS", m_d3dDevice.Get(), m_d3dContext.Get()));
		textures->push_back(make_shared<Texture2D>(L_ROOT_ASSET_PATH + L"black.DDS", m_d3dDevice.Get(), m_d3dContext.Get()));
		textures->push_back(make_shared<Texture2D>(L_ROOT_ASSET_PATH + L"white.DDS", m_d3dDevice.Get(), m_d3dContext.Get()));
		textures->push_back(make_shared<Texture2D>(L_ROOT_ASSET_PATH + L"normal.DDS", m_d3dDevice.Get(), m_d3dContext.Get(), true));
		m_Models = make_unique<vector<shared_ptr<ModelClass>>>();
		HRESULT result;
		//m_Models->push_back(std::move(triangle));
		//if (!result)
		//{
		//	MessageBox(window, L"Could not initialize the model object.", L"Error", MB_OK);
		//}

		m_Shader = make_unique<Shader>();
		result = m_Shader->Initalize(m_d3dDevice.Get(), window);
		if (!result)
		{
			MessageBox(window, L"Could not initialize the color shader object.", L"Error", MB_OK);
		}

		materials = make_unique<vector<shared_ptr<Material>>>();
		materials->push_back(make_shared<Material>("default", m_Shader, textures->at(0), textures->at(0), textures->at(0), textures->at(0)));

		consoleString = make_unique<std::string>("Welcome!\n");
	}
	objl::Loader Loader;
	auto start_time = system_clock::now();
	bool loadout = Loader.LoadFile(ROOT_ASSET_PATH + "sponza.obj");
	auto end_time = system_clock::now();
	auto diff = end_time - start_time;
	auto durration = cr::duration_cast<cr::milliseconds>(diff);
	Log("loaded mainMesh in " + std::to_string(durration.count()) + " ms");

	for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
	{
		objl::MeshRenderer curMesh = Loader.LoadedMeshes[i];
		if (curMesh.MeshName == "sponza_04") //skip the flag in the middle
			continue;

		shared_ptr<ModelClass> subMesh = make_shared<ModelClass>();

		auto indices = vector<unsigned long>();
		for (const auto index : curMesh.Indices) {
			indices.push_back((unsigned long)index);
		}
		subMesh->SetIndicies(&indices, m_d3dDevice.Get());
		
		auto vertices = vector<ModelClass::VertexInputType>();
		const float scale = 1.0f;
		for (const auto vertex : curMesh.Vertices) {
			ModelClass::VertexInputType meshVert;
			meshVert.Color = XMFLOAT4(1, 1, 1, 1);
			meshVert.Normal = XMFLOAT3(
				vertex.Normal.X,
				vertex.Normal.Y,
				vertex.Normal.Z);

			meshVert.Position = XMFLOAT3(
				vertex.Position.X,
				vertex.Position.Y,
				vertex.Position.Z);

			meshVert.UV = XMFLOAT2(
				vertex.TextureCoordinate.X,
				-vertex.TextureCoordinate.Y);

			vertices.push_back(meshVert);
		}

		subMesh->SetVerticies(&vertices, m_d3dDevice.Get());
		m_Models->push_back(subMesh);



		shared_ptr<Material> mat = FindMaterial(curMesh.MeshMaterial.name);
		if (mat == nullptr) {
			shared_ptr<Texture2D> albedo = GetTextureOrDefault(
				ROOT_ASSET_PATH + replacePngByDDS(curMesh.MeshMaterial.map_Kd),
				textures->at(0),
				m_d3dDevice.Get(), 
				m_d3dContext.Get());

			shared_ptr<Texture2D> roughness = GetTextureOrDefault(
				ROOT_ASSET_PATH + replacePngByDDS(curMesh.MeshMaterial.map_Ns),
				textures->at(2),
				m_d3dDevice.Get(),
				m_d3dContext.Get(), true);

			shared_ptr<Texture2D> metalness = GetTextureOrDefault(
				ROOT_ASSET_PATH + replacePngByDDS(curMesh.MeshMaterial.map_Ka),
				textures->at(1),
				m_d3dDevice.Get(),
				m_d3dContext.Get(), true);

			shared_ptr<Texture2D> normalMap = GetTextureOrDefault(
				ROOT_ASSET_PATH + replacePngByDDS(curMesh.MeshMaterial.map_bump),
				textures->at(3),
				m_d3dDevice.Get(),
				m_d3dContext.Get(), true);
			mat = make_shared<Material>(curMesh.MeshMaterial.name, m_Shader, albedo, roughness, metalness, normalMap);
			materials->push_back(mat);
		}

		auto ent = registry.create(); //use raw pointer here
		registry.assign<Types::Transform>(ent);
		registry.assign<Types::MeshRenderer>(ent, subMesh, mat);
	}

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

// Executes the basic game loop.
void Game::Tick()
{

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    SetActive();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());


    // TODO: Add your game logic here.
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	YAP::PushPhase(Update);	
	YAP::PushSection(Update Main);

	bool bTrue = false;
	ImGui::ShowDemoWindow(&bTrue);
	ImGui::Begin("Main");                        
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::BeginGroup();
	ImGui::DragFloat3("Camera Pos", &m_Camera->Position.x, 1.0f);
	ImGui::DragFloat3("Camera Rot", &m_Camera->Rotation.x);
	ImGui::SliderFloat("Camera FOV", &m_Camera->FOV, 10.0f, 90.0f);
	ImGui::SliderFloat("Near Clip", &m_Camera->ClipNear, 0.0001f, 10.0f);
	ImGui::SliderFloat("Far Clip", &m_Camera->ClipFar, 10.0f, 5000.0f);
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		ImGui::TextUnformatted(consoleString->c_str());
	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::BeginGroup();
	//ImGui::Text("Active Entities");
	//ImGui::BeginChild("ActiveEntites", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	//	std::string logString("");
	//	for (auto ent: world->all())
	//	{
	//		logString.append(std::to_string(ent->getEntityId()) + "/n");
	//	}
	//	ImGui::TextUnformatted(logString.c_str());
	//ImGui::EndChild();
	ImGui::EndGroup();
	YAP::PopSection();
	YAP::PopPhase();
	
	YAP::ImGuiLogger(&profilerOpened);
	ImGui::End();
	
    elapsedTime;
}

// Draws the scene.
void Game::SetActive()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }
	YAP::PushPhase(Rendering);
	YAP::PushSection(RenderCamera);
    Clear();
	m_Camera->AspectRatio = m_outputWidth / (float)m_outputHeight;
    
	// Render World
	m_Camera->SetActive();
	renderer->RenderCamera(registry, m_Camera.get());
	YAP::PopSection();
	YAP::PopPhase();
	
	// Render GUI
	ImGui::SetActive();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    Present();
	YAP::NewFrame();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}


void Game::OnKeyDown(wchar_t key) {

	switch (key)
	{
		case 'W': 

			m_Camera->Position.z += 10.0f; 
		break;	
	}
	Log(std::to_string(key));
}
// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        //D3D_FEATURE_LEVEL_10_1,
        //D3D_FEATURE_LEVEL_10_0,
        //D3D_FEATURE_LEVEL_9_3,
        //D3D_FEATURE_LEVEL_9_2,
        //D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
	renderer = make_unique<Renderer>(m_d3dContext.Get());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
	renderer->SetD3DDevice(m_d3dDevice.Get());
	
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();

}