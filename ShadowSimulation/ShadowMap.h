#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <d3d11.h>

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* dev, UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView* GetDepthMapSrv();

	void SetSRVToShaders(ID3D11DeviceContext* devCon);
	void BindDSVAndSetNullRenderTarget(ID3D11DeviceContext* devCon);
private:
	UINT width;
	UINT height;

	ID3D11ShaderResourceView* shadowMap;
	ID3D11DepthStencilView* dsv;

	D3D11_VIEWPORT viewport;
};
#endif