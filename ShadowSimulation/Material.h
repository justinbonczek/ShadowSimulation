//
// Class which holds important rendering information such as shaders, textures, light materials, etc.
// Justin Bonczek
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "Shader.h"
#include "Lights.h"
using namespace DirectX;

class Material
{
public:
	Material(wchar_t* vertfilepath, wchar_t* pixelfilepath, ID3D11SamplerState* _sampler, ID3D11Device* dev);
	Material(wchar_t* filepath, ID3D11SamplerState* sampler, ID3D11Device* dev);
	virtual ~Material();

	/// <summary>Sets the material's shader to the parameter
	/// </summary>
	void LoadShader(Shader* shader);

	/// <summary>Loads a shader from a file
	/// </summary>
	void LoadShader(wchar_t* filepath, ShaderType type, ID3D11Device* dev);

	/// <summary>Loads a normal map SRV
	/// </summary>
	void LoadNormal(wchar_t* filepath, ID3D11Device* dev);
	void LoadBump(wchar_t* filepath, ID3D11Device* dev);

	void SetShader(ID3D11DeviceContext* devCon);
	void SetLightMaterial(LightMaterial* _lightMat);
	void SetSampler(ID3D11DeviceContext* devCon);
	void SetResources(ID3D11DeviceContext* devCon);
	void SetTileX(float val);
	void SetTileZ(float val);

	float GetTileX();
	float GetTileZ();

	LightMaterial const GetLightMaterial();
	ID3D11ShaderResourceView*	GetSRV();
	ID3D11SamplerState*			GetSampler();
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* sampler;
private:
	ID3D11ShaderResourceView* normal;
	ID3D11ShaderResourceView* bump;

	Shader* m_Shader;
	LightMaterial* lightMat;
	ID3D11Buffer* cBuffer;

	UINT tileXZ[2];
};

#endif