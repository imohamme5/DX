#include "Application.h"
#include "DDSTextureLoader.h"
 
 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
	cameraNumber = 1;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

	

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }


	//objMeshData = OBJLoader::Load("sphere.obj", _pd3dDevice, false); //this is for 3ds max

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMFLOAT4 eye = XMFLOAT4(0.0f, 1.0f, -20.0f, 0.0f);
	XMFLOAT4 at = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 up = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camera[0] = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);

	XMFLOAT4 eye1 = XMFLOAT4(0.0f, 15.0f, -50.0f, 0.0f);
	XMFLOAT4 at1 = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 up1 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	camera[1] = new Camera(eye1, at1, up1, _WindowWidth, _WindowHeight, 0.01f, 100.0f);


	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);
	//Load the texture
	CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);


	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout 
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	 
	LightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
	Ambient = XMFLOAT4(0.08f, 0.08f, 0.08f, 1.0f);
	Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);
	//Tells DirectX which texture we would like to use in our shader
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
	//Tells DirectX which sampler we would like to use in our shader
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;


    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        /*{ XMFLOAT3( -1.0f, 1.0f, 0.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 0.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 0.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 0.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },

		{ XMFLOAT3(-1.0f, 1.0f, 2.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 2.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 2.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 2.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },*/


		/*{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },*/

		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f ) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

 
    };

	SimpleVertex GroundVertices[] = 
	{
		/*{ XMFLOAT3(-100.0f, 1.0f, 10.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-100.0f, 1.0f, 200.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(150.0f, 1.0f, 10.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(150.0f, 1.0f, 200.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },*/

		{ XMFLOAT3(-100.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-100.0f, 1.0f, 200.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(150.0f, 1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(150.0f, 1.0f, 200.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	
	D3D11_SAMPLER_DESC sampDesc; //Texture Desc
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8; //change - * no. of vertices (how many lines)
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);
	
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	cubes_data.VBStride = stride;
	cubes_data.VBOffset = offset;
	cubes_data.VertexBuffer = _pVertexBuffer;

	// Disc for Ground vertices
	D3D11_BUFFER_DESC bd_ground;
	ZeroMemory(&bd_ground, sizeof(bd_ground));
	bd_ground.Usage = D3D11_USAGE_DEFAULT;
	bd_ground.ByteWidth = sizeof(SimpleVertex) * 5; //change - * no. of vertices (how many lines)
	bd_ground.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd_ground.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitGroundData; // edit variable for ground
	ZeroMemory(&InitGroundData, sizeof(InitGroundData));
	InitGroundData.pSysMem = GroundVertices;

	hr = _pd3dDevice->CreateBuffer(&bd_ground, &InitGroundData, &_pVertexGroundBuffer);



    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
  
	WORD indices[] =
    {
		//front face
        0,1,2, // drawing triangles
        2,1,3,

		//left face
		0, 2, 6,
		4, 0, 6,

		//back face
		7, 4, 6,
		7, 5, 4,

		// right face
		3,5,7,
		3,1,5,

		//bottom face
		3,6,2,
		3,7,6,

		//top face
		0,4,5,
		0,5,1,

    };

	WORD GroundIndices[] = 
	{
		0, 1, 2,
		1, 2, 3,

	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36; // increase to 36 vertices    
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	cubes_data.IndexBuffer = _pIndexBuffer;
	cubes_data.IndexCount = 36; //amount of points a cube has

	//buffer for ground indices
	D3D11_BUFFER_DESC bd_ground;
	ZeroMemory(&bd_ground, sizeof(bd_ground));

	bd_ground.Usage = D3D11_USAGE_DEFAULT;
	bd_ground.ByteWidth = sizeof(WORD) * 6; // increase to 6 vertices    
	bd_ground.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd_ground.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitDataGround; //edit variable for gorund
	ZeroMemory(&InitDataGround, sizeof(InitDataGround));
	InitDataGround.pSysMem = GroundIndices;
	hr = _pd3dDevice->CreateBuffer(&bd_ground, &InitDataGround, &_pIndexGroundBuffer);




    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
	
	/* Define depth / stencil buffer */
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	 //Create the depth buffer
		_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

	InitIndexBuffer();

	
	for (int i = 0; i < 4; i++)
	{
		cube[i] = new GameObject();
		cube[i]->Initialise(cubes_data); //initialised the cube class
	}
    
	objMeshData = OBJLoader::Load("star.obj", _pd3dDevice); // this is to load with 3ds max 
	sphere = new GameObject();
	sphere->Initialise(objMeshData);
	
    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);
	
	//Create rasterizer state
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);
	

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
	if (_depthStencilView)	_depthStencilView->Release();
	if (_depthStencilBuffer)	_depthStencilBuffer->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_wireFrame) _wireFrame->Release();
}

void Application::Update()
{
	CheckInput();

	camera[0]->CalculateViewProjection(); // call camera class
	camera[1]->CalculateViewProjection();

	sphere->UpdateWorld();


    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    //
    // Animate the cube
    //
	XMStoreFloat4x4(&_groundWorld, XMMatrixTranslation(-20.0f, -50.0f, 50.0f));
	XMStoreFloat4x4(&_world, XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationX(t) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&_world4, XMMatrixTranslation(0.0f, 5.0f, 4.0f) * XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationX(t));

	XMStoreFloat4x4(&_world2, XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationZ(t) * XMMatrixTranslation(3.0f, 0.0f, 3.0f));
	XMStoreFloat4x4(&_world3, XMMatrixTranslation(3.0f, 2.0, 0.0f) * XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationZ(-t) * XMMatrixTranslation(3.0f, 0.0f, 3.0f));
 
}

void Application::Draw()
{

    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.0f, 1.0f, 1.0f}; // red,green,blue,alpha - changes color of screen
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX world2 = XMLoadFloat4x4(&_world2);
	XMMATRIX world3 = XMLoadFloat4x4(&_world3);
	XMMATRIX world4 = XMLoadFloat4x4(&_world4);
	XMMATRIX groundWorld = XMLoadFloat4x4(&_groundWorld);
	XMMATRIX view;
	XMMATRIX projection;

	if (cameraNumber == 1)
	{
		view = XMLoadFloat4x4(&camera[0]->GetView());
		projection = XMLoadFloat4x4(&camera[0]->GetProjection());
	}
	else if (cameraNumber == 2)
	{
		view = XMLoadFloat4x4(&camera[1]->GetView());
		projection = XMLoadFloat4x4(&camera[1]->GetProjection());
	}

	
	//
    // Update variables
    //
	 
	XMFLOAT4 MaterialColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4 AmbientMaterialColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	

    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);					
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	cb.diffuseLight = Diffuse;
	cb.diffuseMaterial = MaterialColor;
	cb.lightVecW = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f); //hardcoded value!
	cb.ambientLight = Ambient;
	cb.ambientMaterial = AmbientMaterialColor;
	 
	cb.gSpecularLight = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	cb.gSpecularMtrl = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.gSpecularPower = 10.0f;
	cb.gEyePosW = XMFLOAT3(0.0f, 0.0f, -3.0f);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	sphere->Draw(_pd3dDevice, _pImmediateContext);
//	sphere->Draw(_pd3dDevice, _pImmediateContext);
    //
    // Drawing cubes
    //

	
	 // Set vertex buffers
	 UINT stride = sizeof(SimpleVertex);
	 UINT offset = 0;
	 _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
	
	 // Set index buffers
	 _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	 //draw cubes
/*	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);*/   

	 
	 cb.mWorld = XMMatrixTranspose(world);
	 _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	 cube[0]->Draw(_pd3dDevice, _pImmediateContext);
	

	
	cb.mWorld = XMMatrixTranspose(world2);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);
	
	cb.mWorld = XMMatrixTranspose(world3);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);

	cb.mWorld = XMMatrixTranspose(world4);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);

    //Drawing Ground plane vertex buffers
	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexGroundBuffer, &stride, &offset);
	//Drawing Ground plane Index buffers
	_pImmediateContext->IASetIndexBuffer(_pIndexGroundBuffer, DXGI_FORMAT_R16_UINT, 0);

	cb.mWorld = XMMatrixTranspose(groundWorld);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(6, 0, 0);


	
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}

void Application::CheckInput()
{
	

	SHORT f1KeyState = GetAsyncKeyState(VK_F1); // CTRL AND SPACE TO SEE LIST OF ENTRYS
	// Test high bit - if set, key was down when GetAsyncKeyState was called.
	if ((1 << 16) & f1KeyState)
	{
		_pImmediateContext->RSSetState(_wireFrame); 	// draw the wireframe
	}
	SHORT f2KeyState = GetAsyncKeyState(VK_F2); //GetAsyncKeyState gets the input from the keyboard
	if ((1 << 16) & f2KeyState)
	{
		_pImmediateContext->RSSetState(nullptr); //set cube to nullptr to display solids
	}

	SHORT wKey = GetAsyncKeyState(0x57);
	if ((1 << 16) & wKey)
	{

		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.z += 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT sKey = GetAsyncKeyState(0x53);
	if ((1 << 16) & sKey)
	{

		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.z -= 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT aKey = GetAsyncKeyState(0x41);
	if ((1 << 16) & aKey)
	{
		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.x += 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT dKey = GetAsyncKeyState(0x44);
	if ((1 << 16) & dKey)
	{
		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.x -= 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT pgUP = GetAsyncKeyState(VK_PRIOR);
	if ((1 << 16) & pgUP)
	{
		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.y += 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT pgDOWN = GetAsyncKeyState(VK_NEXT);
	if ((1 << 16) & pgDOWN)
	{
		XMFLOAT4 newEye = camera[0]->GetEye();
		newEye.y -= 0.005f;
		camera[0]->SetEye(newEye);
	}

	SHORT cam1 = GetAsyncKeyState(0x31);
	if ((1 << 16) & cam1)
	{
		cameraNumber = 1;
	}

	SHORT cam2 = GetAsyncKeyState(0x32);
	if ((1 << 16) & cam2)
	{
		cameraNumber = 2;
	}

}