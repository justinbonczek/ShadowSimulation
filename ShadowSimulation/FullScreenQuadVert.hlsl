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
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput main( VertexInput input )
{
	VertexOutput o;
	o.position = float4(input.position, 1.0);
	o.uv = input.uv;
	return o;
}