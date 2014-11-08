#include "Lighting.hlsli"

cbuffer perFrame : register(b0)
{
	DirectionalLight dLight;
	PointLight pLight;
	SpotLight sLight;
	matrix view;
	matrix projection;
	float3 eyePos;
	float time;
	float4 fogColor;
	float fogStart;
	float fogRange;
	float pad[2];
};

cbuffer perObject : register(b1)
{
	matrix world;
	matrix worldInverseTranspose;
	LightMaterial lightMat;
	float tileX;
	float tileZ;
	float padO[2];
};

cbuffer shadow : register(b2)
{
	matrix sView;
	matrix sProj;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldpos		: POSITION;
	float4 color		: COLOR;
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 shadowpos	: POSITION1;
};

Texture2D _Texture : register(t0);
Texture2D _Normal  : register(t1);
Texture2D _ShadowMap : register(t3);
SamplerState _Sampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float distToEye = length(eyePos - input.worldpos);

	float3 toEye = normalize(eyePos - input.worldpos);
	float4 ambient = float4(0, 0, 0, 0);
	float4 diffuse = float4(0, 0, 0, 0);
	float4 spec = float4(0, 0, 0, 0);

	float4 A, D, S;

	ComputeDirectionalLight(lightMat, dLight, input.normal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputePointLight(lightMat, pLight, input.worldpos, input.normal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputeSpotLight(lightMat, sLight, input.worldpos, input.normal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	float shadowDepth = _ShadowMap.Sample(_Sampler, input.shadowpos.xy).r;
	float lightDepth = input.shadowpos.z;

	float4 texColor = _Texture.Sample(_Sampler, float2(input.uv.x * tileX, input.uv.y * tileZ));

	float4 litColor = texColor * (ambient + diffuse) + spec;

	if (lightDepth > shadowDepth)
	{
		litColor = litColor / 2.0;
	}

	float fogLerp = saturate((distToEye - fogStart) / fogRange);

	litColor = lerp(litColor, fogColor, fogLerp);

	litColor.a = lightMat.diffuse.a;

	return litColor;
	//return input.color;
}