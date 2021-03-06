//
// Struct used to specify vertex data to be sent to the shaders
// Justin Bonczek
//

#ifndef VERTEX_H
#define VERTEX_H

#include <DirectXMath.h>
using namespace DirectX;

#define PI 3.14159265359f

struct Vertex
{
	Vertex(){}
	Vertex(XMFLOAT3 position, XMFLOAT2 uv){ Position = position; UV = uv; }
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT2 UV;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
};
#endif