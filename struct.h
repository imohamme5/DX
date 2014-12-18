#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>


using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;
	XMFLOAT4 lightVecW;

	XMFLOAT4 gSpecularMtrl;
	XMFLOAT4 gSpecularLight;
	FLOAT gSpecularPower;
	XMFLOAT3 gEyePosW;
};

struct MeshData // from gabeobject.h should create structuee class
{
	ID3D11Buffer * VertexBuffer;
	ID3D11Buffer * IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};