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

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldpos		: POSITION;
	float4 color		: COLOR;
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 shadowpos	: TEXCOORD1;
};

Texture2D _Texture : register(t0);
Texture2D _Normal  : register(t1);
Texture2D _ShadowMap : register(t3);
SamplerState _Sampler : register(s0);
SamplerComparisonState _CmpSampler : register(s1);

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 normalT = _Normal.Sample(_Sampler, float2(input.uv.x * tileX, input.uv.y * tileZ));
	normalT = 2.0 * normalT - 1.0;
	
	float3 N = input.normal;
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 bumpedNormal = mul(normalT, TBN);
	bumpedNormal = normalize(bumpedNormal);

	float distToEye = length(eyePos - input.worldpos);
	float3 toEye = normalize(eyePos - input.worldpos);
	float4 ambient = float4(0, 0, 0, 0);
	float4 diffuse = float4(0, 0, 0, 0);
	float4 spec = float4(0, 0, 0, 0);

	float4 A, D, S;

	ComputeDirectionalLight(lightMat, dLight, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;
	
	ComputePointLight(lightMat, pLight, input.worldpos, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;
	
	ComputeSpotLight(lightMat, sLight, input.worldpos, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;

	input.shadowpos.xyz /= input.shadowpos.w;

	float shadowDepth = _ShadowMap.Sample(_Sampler, input.shadowpos.xy);
	float lightDepth = input.shadowpos.z;
	//float shadowDepth = _ShadowMap.SampleCmpLevelZero(_CmpSampler, input.shadowpos.xy, lightDepth).r;

	float4 texColor = _Texture.Sample(_Sampler, float2(input.uv.x * tileX, input.uv.y * tileZ));

	float4 litColor = texColor * (ambient + diffuse) + spec;

	if (lightDepth - 0.0005 > shadowDepth)
		litColor /= 2.0;
	//litColor *= shadowDepth;

	float fogLerp = saturate((distToEye - fogStart) / fogRange);

	litColor = lerp(litColor, fogColor, fogLerp);
	
	litColor.a = lightMat.diffuse.a;

	return litColor;
}