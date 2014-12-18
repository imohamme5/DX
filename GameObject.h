#pragma once

#include "struct.h"


class GameObject
{
private:
	MeshData _meshData;

	XMFLOAT4X4 _world;

	XMFLOAT4X4 _scale;
	XMFLOAT4X4 _rotate;
	XMFLOAT4X4 _translate;

public:
	GameObject(void);
	~GameObject(void);

	XMFLOAT4X4 GetWorld() const { return _world; };

	void UpdateWorld();

	void SetScale(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetTranslation(float x, float y, float z);

	void Initialise(MeshData meshData);
	void Update(float elapsedTime);
	void Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext);
};

