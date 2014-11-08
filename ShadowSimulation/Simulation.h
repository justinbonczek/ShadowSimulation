///
// Environment Simulation made by Justin Bonczek using DirectX 11
// Justin Bonczek
///

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

#include "Game.h"
#include "Camera.h"
#include "GameObject.h"
#include "MeshGenerator.h"
#include "Lights.h"
#include "ShadowMap.h"

struct PerFrameData
{
	DirectionalLight dLight;
	PointLight pLight;
	SpotLight sLight;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT3 eyePos;
	float time;
	XMFLOAT4 fogColor;
	float fogStart;
	float fogRange;
	float pad[2];
};

struct PerObjectData
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldInverseTranspose;
	LightMaterial lightMat;
	float tileX;
	float tileZ;
	float pad[2];
};

struct ShadowData
{
	XMFLOAT4X4 sView;
	XMFLOAT4X4 sProj;
};

class Simulation : public Game
{
public:
	Simulation(HINSTANCE hInstance);
	virtual ~Simulation();

	/// <summary>Sets up the game</summary>
	bool Initialize();

	/// <summary>Called every time the window is resized</summary>
	void OnResize();

	/// <summary>Called many times per second to update simulation logic</summary>
	void Update(float dt);

	/// <summary>Called many times per second to draw the simulation</summary>
	void Draw();
private:
	/// <summary>Functions called during initialization</summary>
	void LoadAssets();

	/// <summary>Sets up the input layouts and other DirectX 11 states</summary>
	void InitializePipeline();

	/// <summary>Handles camera motion</summary>
	void MoveCamera(float dt);

	Camera m_Camera;

	PerFrameData perFrameData;
	PerObjectData perObjectData;
	ShadowData shadowData;

	ID3D11Buffer* perFrameBuffer;
	ID3D11Buffer* perObjectBuffer;
	ID3D11Buffer* shadowBuffer;

	ShadowMap* shadowMap;
	DirectionalLight dLight;
	PointLight pLight;
	SpotLight sLight;

	GameObject* fullScreenQuad;

	ID3D11InputLayout* inputLayout;
	
	ID3D11BlendState* blendState;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilState* noDoubleBlendDSS;

	ID3D11RasterizerState* solid;
	ID3D11RasterizerState* wireframe;
	ID3D11RasterizerState* current;
	float totalTime;
	float time;

	std::vector<GameObject*> objects;
};

#endif