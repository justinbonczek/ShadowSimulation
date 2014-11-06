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
	matrix sView;
	matrix sProj;
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

struct VertexInput
{
	float3 position : POSITION;
	float4 color    : COLOR;
	float2 uv		: TEXCOORD0;
	float3 normal   : NORMAL;
	float4 tangent  : TANGENT;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float3 worldpos : POSITION0;
	float4 color    : COLOR;
	float2 uv		: TEXCOORD0;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 shadowpos: POSITION1;
};

VertexOutput main(VertexInput input)
{
	VertexOutput o;

	matrix worldViewProj = mul(mul(world, view), projection);

	o.position = mul(float4(input.position, 1.0), worldViewProj);
	o.worldpos = mul(float4(input.position, 1.0), world).xyz;

	o.normal = mul(input.normal, (float3x3)worldInverseTranspose );
	o.tangent = mul(input.tangent, (float3x3)world);

	o.color = input.color;
	o.uv = input.uv;

	matrix lightViewProj = mul(mul(world, sView), sProj);

	o.shadowpos = mul(float4(input.position, 1.0), lightViewProj);

	return o;
}