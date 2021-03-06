#pragma once

#include "struct.h"
#include "resource.h"
#include "GameObject.h"
#include "Camera.h"
#include "OBJLoader.h"


using namespace DirectX;


class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*			_pVertexGroundBuffer; //for ground
	ID3D11Buffer*			_pIndexGroundBuffer; // for ground
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world;
	XMFLOAT4X4              _world2;
	XMFLOAT4X4              _world3;
	XMFLOAT4X4              _world4;
	XMFLOAT4X4              _groundWorld;
	int						cameraNumber;


	ID3D11DepthStencilView*	_depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;
	ID3D11RasterizerState* _wireFrame;
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT3 LightDirection;
	MeshData				cubes_data;
	GameObject		*		cube[4];
	Camera			*		camera[1];
	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11SamplerState * _pSamplerLinear = nullptr;
	MeshData objMeshData;
	GameObject * sphere;
	



private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

	void CheckInput();

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

