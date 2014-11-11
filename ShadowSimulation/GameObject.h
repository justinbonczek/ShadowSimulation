//
// Abstract class for defining game entities
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <d3d11.h>
#include <DirectXMath.h>

#include "Mesh.h"
#include "Material.h"
#include "Lights.h"

using namespace DirectX;

class GameObject
{
public:
	///
	// Constructors
	///
	GameObject(Mesh* mesh);
	GameObject(Material* mat);
	GameObject(Mesh* mesh, Material* mat);
	GameObject(Mesh* mesh, Material* mat, LightMaterial* lightMat);
	virtual ~GameObject();

	/// <summary>Updates object logic to time
	/// </summary>
	virtual void Update(float dt);

	/// <summary>Sets proper graphics pipeline values and renders the object
	/// </summary>
	virtual void Draw(ID3D11DeviceContext* devCon);

	/// <summary>Sets the position of the object to the new value
	/// </summary>
	virtual void SetPosition(XMFLOAT3 newPosition);

	/// <summary>Sets the scale of the object to the new value
	/// </summary>
	virtual void SetScale(XMFLOAT3 newScale);

	/// <summary>Sets the rotation of the object to the new value
	/// </summary>
	virtual void SetRotation(XMFLOAT3 newRotation);

	/// <summary>Binds the sampler(s) to the pipeline
	/// </summary>
	void SetSampler(ID3D11SamplerState* _sampler);

	/// <summary>Binds SRV(s) to the pipeline
	/// </summary>
	void SetSRV(ID3D11ShaderResourceView* srv);

	/// <summary>Sets the shadowPass bool value
	/// </summary>
	void SetShadowPass(bool val);
	
	/// <summary>Returns the Texture tiling in the x (u) coordinate
	/// </summary>
	float const GetTextureTileX();

	/// <summary>Returns the Texture tiling in the z (v) coordinate
	/// </summary>
	float const GetTextureTileZ();

	/// <summary>Returns the object's current world matrix
	/// </summary>
	XMFLOAT4X4 const GetWorldMatrix();

	/// <summary>Returns the object's light material
	/// </summary>
	LightMaterial const GetLightMaterial();
protected:
	Mesh* mesh;
	Material* mat;

	bool shadowPass;
	UINT stride;
	UINT offset;

	ID3D11Buffer* vBuffer;
	ID3D11Buffer* iBuffer;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* sampler;

	XMFLOAT4X4 worldMat;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
};

#endif