///
// Environment Simulation made by Justin Bonczek using DirectX 11
///

#include <d3dcompiler.h>

#include "Simulation.h"
#include "Vertex.h"
#include "Timer.h"

void Simulation::MoveLight(float dt)
{
	if (GetAsyncKeyState('I') & 0x8000)
		sLight.position.z += 10.0f * dt; 
	if (GetAsyncKeyState('K') & 0x8000)
		sLight.position.z -= 10.0f * dt;
	if (GetAsyncKeyState('J') & 0x8000)
		sLight.position.x -= 10.0f * dt;
	if (GetAsyncKeyState('L') & 0x8000)
		sLight.position.x += 10.0f * dt;
	if (GetAsyncKeyState('U') & 0x8000)
		sLight.position.y += 10.0f * dt;
	if (GetAsyncKeyState('O') & 0x8000)
		sLight.position.y -= 10.0f * dt;
}

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

	m_Camera.SetPosition(0.0f, 5.0f, -10.0f);
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

	ID3D11SamplerState* pcfSampler;
	wsd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	wsd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	wsd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	wsd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	wsd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	dev->CreateSamplerState(&wsd, &pcfSampler);
	devCon->PSSetSamplers(1, 1, &pcfSampler);
	
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
	MeshGenerator::CreatePlane(25.0f, 25.0f, 2, 2, plane);
	Mesh* planeMesh = new Mesh(plane, dev);

	MeshData sphere;
	MeshGenerator::CreateSphere(1.0f, 2, sphere);
	Mesh* sphereMesh = new Mesh(sphere, dev);

	MeshData screenQuad;
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(0.5f, -0.5f, 0.1f), XMFLOAT2(0.0f, 0.0f)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(1.0f, -0.5f, 0.1f), XMFLOAT2(1.0f, 0.0f)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(0.5f, -1.0f, 0.1f), XMFLOAT2(0.0f, 1.0f)));
	screenQuad.vertices.push_back(Vertex(XMFLOAT3(1.0f, -1.0f, 0.1f), XMFLOAT2(1.0f, 1.0f)));
	screenQuad.indices.push_back(0);
	screenQuad.indices.push_back(1);
	screenQuad.indices.push_back(2);
	screenQuad.indices.push_back(1);
	screenQuad.indices.push_back(3);
	screenQuad.indices.push_back(2);
	Mesh* screenQuadMesh = new Mesh(screenQuad, dev);

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

	Material* noLightMat = new Material(L"NoLightVert.cso", L"NoLightPixel.cso", wrapSampler, dev);
	Material* noLightTexMat = new Material(L"FullScreenQuadVert.cso", L"FullScreenQuadPixel.cso", wrapSampler, dev);

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
		chair->SetRotation(XMFLOAT3(0.0f, -PI/ 2.0f, 0.0f));
		objects.push_back(chair);
	}

	cameraDebugSphere = new GameObject(sphereMesh, noLightMat);
	quarterQuad = new GameObject(screenQuadMesh, noLightTexMat);
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

	cd.ByteWidth = sizeof(shadowData);
	dev->CreateBuffer(&cd, NULL, &shadowBuffer);

	//
	// Blend State
	//
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

	//
	// Rasterizer States
	//
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

	//
	// Depth Stencil States
	//
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
	devCon->VSSetConstantBuffers(2, 1, &shadowBuffer);
	devCon->PSSetConstantBuffers(2, 1, &shadowBuffer);

	shadowData.resolution = 2048.0;
	shadowMap = new ShadowMap(dev, (UINT)shadowData.resolution, (UINT)shadowData.resolution);
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
	if (GetAsyncKeyState(VK_SPACE) && 0x8000 && time > 0.25f)
	{
		if (current == solid)
		{
			current = wireframe;
			time = 0.0f;
		}
		else
		{
			current = solid;
			time = 0.0f;
		}
		if (time > 100.0f)
			time -= 100.0f;
	}
	MoveCamera(dt);
	perFrameData.eyePos = m_Camera.GetPosition();
	MoveLight(dt);
	cameraDebugSphere->SetPosition(sLight.position);
	cameraDebugSphere->Update(dt);
	///
	// Spotlight animation
	///
	sLight.position = XMFLOAT3(30.0f * cos(totalTime * 1.0f), sLight.position.y, 30.0f * sin(totalTime * 1.0f) + 10.0f);
	XMFLOAT3 direction(-(sLight.position.x), -sLight.position.y, 10.0f - (sLight.position.z));
	XMStoreFloat3(&sLight.direction, XMVector3Normalize(XMLoadFloat3(&direction)));

	perFrameData.sLight = sLight;

	for (GameObject* obj : objects)
	{
		obj->Update(dt);
	}

	quarterQuad->Update(dt);
}

void Simulation::Draw()
{
	// Update camera
	m_Camera.UpdateViewMatrix();

	// Set up the render target (back buffer) and clear it
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

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
	//XMMATRIX sProj = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, 1.0, 0.1, 50.0);
	XMMATRIX sProj = XMMatrixOrthographicLH(30.0f, 30.0f, 0.1f, 200.0f);
	XMStoreFloat4x4(&perFrameData.view, XMMatrixTranspose(sView));
	XMStoreFloat4x4(&perFrameData.projection, XMMatrixTranspose(sProj));
	XMStoreFloat4x4(&shadowData.sView, XMMatrixTranspose(sView));
	XMStoreFloat4x4(&shadowData.sProj, XMMatrixTranspose(sProj));
	devCon->UpdateSubresource(perFrameBuffer, 0, NULL, &perFrameData, 0, 0);
	devCon->UpdateSubresource(shadowBuffer, 0, NULL, &shadowData, 0, 0);
	for (GameObject* obj : objects)
	{
		XMStoreFloat4x4(&(perObjectData.world), XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix()))));
		XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix()))))));
		perObjectData.lightMat = obj->GetLightMaterial();
		perObjectData.tileX = obj->GetTextureTileX();
		perObjectData.tileZ = obj->GetTextureTileZ();
		devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);
		obj->SetShadowPass(true);
		obj->Draw(devCon);
	}
	// Reset render target/ view and projection matrices
	// Set shadowmap to the shader
	XMStoreFloat4x4(&(perFrameData.view), XMMatrixTranspose(m_Camera.View()));
	XMStoreFloat4x4(&(perFrameData.projection), XMMatrixTranspose(m_Camera.Proj()));
	devCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	devCon->RSSetViewports(1, &viewport);
	shadowMap->SetSRVToShaders(devCon);

	devCon->UpdateSubresource(perFrameBuffer, 0, NULL, &perFrameData, 0, 0);
	// Render the geometry from the camera to the back buffer
	for (GameObject* obj : objects)
	{		
		XMStoreFloat4x4(&(perObjectData.world), XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix()))));
		XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(obj->GetWorldMatrix()))))));
		perObjectData.lightMat = obj->GetLightMaterial();
		perObjectData.tileX = obj->GetTextureTileX();
		perObjectData.tileZ = obj->GetTextureTileZ();
		devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);
		obj->SetShadowPass(false);
		obj->Draw(devCon);
	}

	// Debug drawing
	XMStoreFloat4x4(&(perObjectData.world), XMMatrixTranspose(XMLoadFloat4x4(&(cameraDebugSphere->GetWorldMatrix()))));
	XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(cameraDebugSphere->GetWorldMatrix()))))));
	devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);
	cameraDebugSphere->Draw(devCon);

	XMStoreFloat4x4(&(perObjectData.world), XMMatrixTranspose(XMLoadFloat4x4(&(quarterQuad->GetWorldMatrix()))));
	XMStoreFloat4x4(&(perObjectData.worldInverseTranspose), XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&(quarterQuad->GetWorldMatrix()))))));
	devCon->UpdateSubresource(perObjectBuffer, 0, NULL, &perObjectData, 0, 0);
	quarterQuad->Draw(devCon);

	// Swap the buffer pointers!
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