///
// Environment Simulation made by Justin Bonczek using DirectX 11
///

#include <d3dcompiler.h>

#include "Simulation.h"
#include "Vertex.h"
#include "Timer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int showCmd)
{
	Simulation simulation(hInstance);

	if (!simulation.Initialize())
		return 0;

	return simulation.Run();
}

Simulation::Simulation(HINSTANCE hInstance) : 
Game(hInstance)
{
	windowTitle = L"Environment Simulation";
	windowWidth = 1280;
	windowHeight = 720;
}

Simulation::~Simulation()
{
	for (GameObject* obj : objects)
	{
		delete obj;
		obj = 0;
	}
	ReleaseMacro(inputLayout);
	ReleaseMacro(perFrameBuffer);
	ReleaseMacro(perObjectBuffer);
	ReleaseMacro(blendState);
	ReleaseMacro(solid);
	ReleaseMacro(wireframe)
}

bool Simulation::Initialize()
{
	if (!Game::Initialize())
		return false;

	LoadAssets();
	InitializePipeline();

	m_Camera.SetPosition(0.0, 5.0, -10.0);
	return true;
}

void Simulation::LoadAssets()
{
	ID3D11SamplerState* wrapSampler;
	D3D11_SAMPLER_DESC wsd;
	ZeroMemory(&wsd, sizeof(D3D11_SAMPLER_DESC));
	wsd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	wsd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	wsd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	wsd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	wsd.MaxAnisotropy = 0;
	wsd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	wsd.MinLOD = 0;
	wsd.MaxLOD = 0;
	wsd.MipLODBias = 0;
	dev->CreateSamplerState(&wsd, &wrapSampler);

	///
	// Lights
	///
	dLight.ambient =	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dLight.diffuse =	XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	dLight.specular =	XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	dLight.direction =	XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	pLight.ambient =	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	pLight.diffuse =	XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	pLight.specular =	XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pLight.attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pLight.position =	 XMFLOAT3(0.0f, -44.0f, 10.0f);
	pLight.range =		40.0f;

	sLight.ambient =	XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	sLight.diffuse =	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sLight.specular =	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sLight.attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	sLight.position =	XMFLOAT3(10.0f, 10.0f, 10.0f);
	sLight.direction =	XMFLOAT3(0.0f, 0.0f, 0.0f);
	sLight.spot = 90.0f;
	sLight.range = 1000.0f;

	perFrameData.dLight = dLight;
	perFrameData.pLight = pLight;
	perFrameData.sLight = sLight;

	///
	// Fog data
	///
	perFrameData.fogStart = 50.0f;
	perFrameData.fogRange = 100.0f;
	perFrameData.fogColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.2f);

	///
	// GameObject Initialization
	// No separate class needed to manage them for a small simulation
	// Sets up meshes, materials, light materials, shaders and handles position/rotation/scaling
	///
	MeshData plane;
	MeshGenerator::CreatePlane(25.0, 25.0, 2, 2, plane);
	Mesh* planeMesh = new Mesh(plane, dev);

	MeshData screenQuad;
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(-1.0, -1.0, 0), XMFLOAT2(0.0, 0.0)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(1.0, -1.0, 0), XMFLOAT2(1.0, 0.0)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(-1.0, 1.0, 0), XMFLOAT2(0.0, 1.0)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(1.0, 1.0, 0), XMFLOAT2(1.0, 1.0)));
	screenQuad.indices.push_back(0);
	screenQuad.indices.push_back(1);
	screenQuad.indices.push_back(2);
	screenQuad.indices.push_back(1);
	screenQuad.indices.push_back(3);
	screenQuad.indices.push_back(2);
	Mesh* screenQuadMesh = new Mesh(screenQuad, dev);

	Material* testMat = new Material(L"FullScreenQuadVert.cso", L"FullScreenQuadPixel.cso", wrapSampler, dev);
	fullScreenQuad = new GameObject(screenQuadMesh, testMat);

	Material* brickMat = new Material(L"Textures/floor_tiles.png", wrapSampler, dev);
	brickMat->LoadShader(L"DefaultVertex.cso", Vert, dev);
	brickMat->LoadShader(L"DefaultPixel.cso", Pixel, dev);
	brickMat->LoadNormal(L"Textures/floor_tiles_normal.png", dev);
	brickMat->SetTileX(3.0f);
	brickMat->SetTileZ(3.0f);

	Material* defaultMat = new Material(L"Textures/default.png", wrapSampler, dev);
	defaultMat->LoadShader(L"DefaultVertex.cso", Vert, dev);
	defaultMat->LoadShader(L"PixelNoNormal.cso", Pixel, dev);
	defaultMat->LoadNormal(L"Textures/brick_normal.png", dev);
	defaultMat->SetTileX(1.0f);
	defaultMat->SetTileZ(1.0f);

	LightMaterial* tileLightMat = new LightMaterial();
	tileLightMat->ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	tileLightMat->diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	tileLightMat->specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 64.0f);

	LightMaterial* chairLightMat = new LightMaterial();
	chairLightMat->ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	chairLightMat->diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	chairLightMat->specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	brickMat->SetLightMaterial(tileLightMat);
	defaultMat->SetLightMaterial(chairLightMat);

	GameObject* obj = new GameObject(planeMesh, brickMat);
	obj->SetPosition(XMFLOAT3(0.0f, 0.0f, 10.0f));
	objects.push_back(obj);

	for (int i = 0; i < 5; i++)
	{
		GameObject* chair = new GameObject(new Mesh("Models/chair.fbx", dev), defaultMat);
		chair->SetPosition(XMFLOAT3(-5.0f, 2.0f, (float)i * 5.0f));
		chair->SetScale(XMFLOAT3(1.0f, 0.25f, 1.0f));
		chair->SetRotation(XMFLOAT3(0.0f, PI / 2.0f, 0.0f));
		objects.push_back(chair);
	}

	for (int i = 0; i < 5; i++)
	{
		GameObject* chair = new GameObject(new Mesh("Models/chair.fbx", dev), defaultMat);
		chair->SetPosition(XMFLOAT3(5.0f, 2.0f, (float)i * 5.0f));
		chair->SetScale(XMFLOAT3(1.0f, 0.25f, 1.0f));
		chair->SetRotation(XMFLOAT3(0.0, -PI/ 2.0f, 0.0f));
		objects.push_back(chair);
	}

	objects.push_back(fullScreenQuad);
}	

void Simulation::InitializePipeline()
{
	///
	// Input Layout
	///
	D3D11_INPUT_ELEMENT_DESC vDesc[] = 
	{
		{ "POSITION", NULL, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, NULL },
		{ "COLOR", NULL, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, NULL },
		{ "TEXCOORD", NULL, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, NULL },
		{ "NORMAL", NULL, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, NULL },
		{ "TANGENT", NULL, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, NULL }
	};

	ID3DBlob* vertexByte;
	D3DReadFileToBlob(L"DefaultVertex.cso", &vertexByte);

	dev->CreateInputLayout(vDesc, ARRAYSIZE(vDesc), vertexByte->GetBufferPointer(), vertexByte->GetBufferSize(), &inputLayout);

	ReleaseMacro(vertexByte);

	///
	// Pipeline buffers/ states
	// cBuffers, blend state, rasterizer state, stencil states, etc
	///
	D3D11_BUFFER_DESC cd;
	ZeroMemory(&cd, sizeof(D3D11_BUFFER_DESC));
	cd.ByteWidth = sizeof(perFrameData);
	cd.Usage = D3D11_USAGE_DEFAULT;
	cd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cd.CPUAccessFlags = 0;
	cd.MiscFlags = 0;
	cd.StructureByteStride = 0;
	dev->CreateBuffer(&cd, NULL, &perFrameBuffer);

	cd.ByteWidth = sizeof(perObjectData);
	dev->CreateBuffer(&cd, NULL, &perObjectBuffer);

	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	dev->CreateBlendState(&bd, &blendState);

	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
	rd.FillMode = D3D11_FILL_WIREFRAME;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = false;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = true;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false;
	rd.AntialiasedLineEnable = false;
	dev->CreateRasterizerState(&rd, &wireframe);

	rd.FillMode = D3D11_FILL_SOLID;
	dev->CreateRasterizerState(&rd, &solid);

	current = solid;

	D3D11_DEPTH_STENCIL_DESC dsd;
	ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsd.StencilEnable = false;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dev->CreateDepthStencilState(&dsd, &depthStencilState);

	D3D11_DEPTH_STENCIL_DESC ndsd;
	ZeroMemory(&ndsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ndsd.DepthEnable = true;
	ndsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ndsd.DepthFunc = D3D11_COMPARISON_LESS;
	ndsd.StencilEnable = true;
	ndsd.StencilReadMask = 0xff;
	ndsd.StencilWriteMask = 0xff;

	ndsd.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	ndsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ndsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	ndsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	ndsd.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	ndsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ndsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	ndsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dev->CreateDepthStencilState(&ndsd, &noDoubleBlendDSS);

	// Configure input assembly
	devCon->IASetInputLayout(inputLayout);
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set up constant buffers
	devCon->VSSetConstantBuffers(0, 1, &perFrameBuffer);
	devCon->VSSetConstantBuffers(1, 1, &perObjectBuffer);
	devCon->PSSetConstantBuffers(0, 1, &perFrameBuffer);
	devCon->PSSetConstantBuffers(1, 1, &perObjectBuffer);

	shadowMap = new ShadowMap(dev, 1024, 1024);
}

void Simulation::OnResize()
{
	Game::OnResize();

	m_Camera.SetLens(0.25f * 3.1415926535f, AspectRatio(), 0.1f, 200.0f);
	XMStoreFloat4x4(&(perFrameData.projection), XMMatrixTranspose(m_Camera.Proj()));
}

void Simulation::Update(float dt)
{
	///
	// Rudimentary implementation to handle rasterizer state change (space to switch to wireframe/ back)
	///
	time += dt;
	totalTime += dt;

	perFrameData.time = totalTime;
	if (GetAsyncKeyState(VK_SPACE) && 0x8000 && time > 0.25)
	{
		if (current == solid)
		{
			current = wireframe;
			time = 0;
		}
		else
		{
			current = solid;
			time = 0;
		}
		if (time > 100)
			time -= 100;
	}
	MoveCamera(dt);
	perFrameData.eyePos = m_Camera.GetPosition();

	///
	// Spotlight animation
	///
	sLight.position = XMFLOAT3(40.0f * cos(totalTime * 1.0), sLight.position.y, 40.0f * sin(totalTime * 1.0) + 10.0f);
	XMFLOAT3 direction(-(sLight.position.x), -sLight.position.y, 10 - (sLight.position.z));
	XMStoreFloat3(&sLight.direction, XMVector3Normalize(XMLoadFloat3(&direction)));

	perFrameData.sLight = sLight;

	for (GameObject* obj : objects)
	{
		obj->Update(dt);
	}
}

void Simulation::Draw()
{
	// Update camera/ send view matrix to pipeline
	m_Camera.UpdateViewMatrix();
	XMStoreFloat4x4(&(perFrameData.view), XMMatrixTranspose(m_Camera.View()));

	// Set up the render target (back buffer) and clear it
	const float clearColor[4] = { 0.0f, 0.0, 0.0f, 1.0f };

	devCon->ClearRenderTargetView(renderTargetView, clearColor);
	devCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set various states
	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	devCon->OMSetBlendState(blendState, blendFactors, 0xFFFFFF);
	devCon->RSSetState(current);
	devCon->OMSetDepthStencilState(depthStencilState, 0);
	
	// Render the scene from the light's point of view to create a shadow map
	shadowMap->BindDSVAndSetNullRenderTarget(devCon);
	XMMATRIX sView = XMMatrixLookAtLH(XMLoadFloat3(&sLight.position), XMVectorSet(0.0f, 0.0f, 10.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX sProj = XMMatrixOrthographicLH(10.0, 10.0, 0.1, 200.0);
	XMStoreFloat4x4(&perFrameData.view, XMMatrixTranspose(sView));
	XMStoreFloat4x4(&perFrameData.projection, XMMatrixTranspose(sProj));
	XMStoreFloat4x4(&perFrameData.sView, XMMatrixTranspose(sView));
	XMStoreFloat4x4(&perFrameData.sProj, XMMatrixTranspose(sProj));
	devCon->UpdateSubresource(perFrameBuffer, 0, NULL, &perFrameData, 0, 0);
	for (GameObject* obj : objects)
	{
		XMStoreFloat4x4(&(perObjectData.world), XMLoadFloat4x4(&(obj->GetWorldMatrix())));
		XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix())))));
		perObjectData.lightMat = obj->GetLightMaterial();
		perObjectData.tileX = obj->GetTextureTileX();
		perObjectData.tileZ = obj->GetTextureTileZ();
		devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);
		devCon->PSSetShader(0, 0, 0);
		obj->Draw(devCon);
	}

	XMStoreFloat4x4(&(perFrameData.view), XMMatrixTranspose(m_Camera.View()));
	OnResize();
	shadowMap->SetSRVToShaders(devCon);

	devCon->UpdateSubresource(perFrameBuffer, 0, NULL, &perFrameData, 0, 0);
	// Render the geometry from the camera to the back buffer
	for (GameObject* obj : objects)
	{		
		XMStoreFloat4x4(&(perObjectData.world), XMLoadFloat4x4(&(obj->GetWorldMatrix())));
		XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix())))));
		perObjectData.lightMat = obj->GetLightMaterial();
		perObjectData.tileX = obj->GetTextureTileX();
		perObjectData.tileZ = obj->GetTextureTileZ();
		devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);

		obj->Draw(devCon);
	}
	swapChain->Present(0, 0);
}

void Simulation::MoveCamera(float dt)
{
	if (GetAsyncKeyState('W') & 0x8000)
		m_Camera.Walk(10.0f*dt);
	if (GetAsyncKeyState('S') & 0x8000)
		m_Camera.Walk(-10.0f*dt);
	if (GetAsyncKeyState('A') & 0x8000)
		m_Camera.Strafe(-10.0f*dt);
	if (GetAsyncKeyState('D') & 0x8000)
		m_Camera.Strafe(10.0f*dt);
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		m_Camera.Pitch(-1.0f * dt);
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		m_Camera.RotateY(-1.0f * dt);
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		m_Camera.RotateY(1.0f * dt);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		m_Camera.Pitch(1.0f * dt);
}