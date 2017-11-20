#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h" // From DirectX Tool Kit
#include "DDSTextureLoader.h" // For loading skyboxes (cube maps)

// For the DirectX Math library
using namespace DirectX;


Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexBuffer = 0;
	indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;
	waterVS = 0;
	waterPS = 0;
	refractionVS = 0;
	refractionPS = 0;
	_reflectionTexture = 0;
	_refractionTexture = 0;
	camera = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}


Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
	
	sampler->Release();
	textureSRV->Release();
	normalMapSRV->Release();
	bathSRV->Release();
	bathNormalMapSRV->Release();
	waterNormalMapSRV->Release();

	skySRV->Release();
	rsSky->Release();
	dsSky->Release();

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete skyVS;
	delete skyPS;
	delete waterVS;
	delete waterPS;
	delete refractionVS;
	delete refractionPS;
	delete _refractionTexture;
	delete _reflectionTexture;

	// Clean up resources
	for(auto& e : entities) delete e;
	for(auto& m : meshes) delete m;
	delete camera;
	delete water;
	delete bathBottom;
	delete bathLeft;
	delete bathRight;
	delete bathBack;
	delete bathFront;
}


void Game::Init()
{
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();
	//Initialize water class
	water = new Water();
	water->Initialize(device, L"Debug/Textures/waterNormal.dds", 2.75f, 1.0f);

	//Create the refraction render to the texture object
	_refractionTexture = new RenderTexture();
	_refractionTexture->Initialize(device, width, height, 100.0f, 0.1f);

	// Create the reflection render to texture object.
	_reflectionTexture = new RenderTexture();
	_reflectionTexture->Initialize(device, width, height, 100.0f, 0.1f);

	

	// Load ground texture stuff
	CreateWICTextureFromFile(device,context, L"Debug/Textures/ground.jpg", 0,&textureSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Textures/groundNormalMap.jpg", 0, &normalMapSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Textures/bath.tiff", 0, &bathSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Textures/bathNormal.tiff", 0, &bathNormalMapSRV);

	CreateDDSTextureFromFile(device, L"Debug/Textures/SunnyCubeMap.dds", 0, &skySRV);
	CreateDDSTextureFromFile(device, L"Debug/Textures/waterNormal.dds", 0, &waterNormalMapSRV);

	// Create a sampler state for texture sampling
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	// Ask the device to create a state
	device->CreateSamplerState(&samplerDesc, &sampler);

	// Set up sky stuff
	
	// Set up the rasterize state
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &rsSky);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &dsSky);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");		

	pixelShader = new SimplePixelShader(device, context);
	if(!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))	
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	skyVS = new SimpleVertexShader(device, context);
	if (!skyVS->LoadShaderFile(L"Debug/SkyVS.cso"))
		skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	if (!skyPS->LoadShaderFile(L"Debug/SkyPS.cso"))
		skyPS->LoadShaderFile(L"SkyPS.cso");

	waterVS = new SimpleVertexShader(device, context);
	if (!waterVS->LoadShaderFile(L"Debug/WaterVS.cso"))
		waterVS->LoadShaderFile(L"WaterVS.cso");

	waterPS = new SimplePixelShader(device, context);
	if (!waterPS->LoadShaderFile(L"Debug/WaterPS.cso"))
		waterPS->LoadShaderFile(L"WaterPS.cso");

	refractionVS = new SimpleVertexShader(device, context);
	if (!refractionVS->LoadShaderFile(L"Debug/RefractionVS.cso"))
		refractionVS->LoadShaderFile(L"RefractionVS.cso");

	refractionPS = new SimplePixelShader(device, context);
	if (!refractionPS->LoadShaderFile(L"Debug/RefractionPS.cso"))
		refractionPS->LoadShaderFile(L"RefractionPS.cso");

}




void Game::CreateMatrices()
{
	XMStoreFloat4x4(&worldMatrix1, XMMatrixIdentity());

	camera = new Camera(0, 0, -5);
	camera->UpdateProjectionMatrix((float)width / height);
}



void Game::CreateBasicGeometry()
{
	Mesh* groundMesh = new Mesh("Models/cube.obj", device);
	Mesh* cubeMesh = new Mesh("Models/cube.obj", device);

	meshes.push_back(groundMesh);
	meshes.push_back(cubeMesh);

	// Make some entities
	GameEntity* ground = new GameEntity(groundMesh);	
    bathBottom = new GameEntity(cubeMesh);
	bathRight = new GameEntity(cubeMesh);
	bathLeft = new GameEntity(cubeMesh);
	bathBack = new GameEntity(cubeMesh);
	bathFront = new GameEntity(cubeMesh);
	GameEntity* cube = new GameEntity(cubeMesh);

	entities.push_back(ground);	
	entities.push_back(cube);

	ground->SetScale(5.0f, 0.025f, 5.0f);
	ground->SetPosition(0.0f, -0.20f, 0.0f);

	bathBottom->SetPosition(0.0f, -0.15f, 0.0f);
	bathBottom->SetScale(2.0f, 0.15f, 2.0f);

	bathRight->SetPosition(1.0f, 0.0f, 0.0f);
	bathRight->SetScale(0.15f, 0.5f, 2.0f);

	bathLeft->SetPosition(-1.0f, 0.0f, 0.0f);
	bathLeft->SetScale(0.15f, 0.5f, 2.0f);

	bathBack->SetPosition(0.0f, 0.0f, 1.0f);
	bathBack->SetScale(2.0f, 0.5f, 0.15f);

	bathFront->SetPosition(0.0f, 0.0f, -1.0f);
	bathFront->SetScale(2.0f, 0.5f, 0.15f);

	cube->SetScale(10, 10, 10);


//	//Water Vertices
//	WaterVertex* waterVertices;
//	waterVertices = new WaterVertex[6];
//	waterVertices[0].Position = XMFLOAT3(-2.0f,0.0f,2.0f);  // Top left.
//	waterVertices[0].UV = XMFLOAT2(0.0f,0.0f);
//
//	waterVertices[1].Position = XMFLOAT3(2.0f,0.0f,-2.0f);  // Top right.
//	waterVertices[1].UV = XMFLOAT2(1.0f, 0.0f);
//
//	waterVertices[2].Position = XMFLOAT3(-2.0f,0.0f,-2.0f);  // Bottom left.
//	waterVertices[2].UV = XMFLOAT2(0.0f, 1.0f);
//
//	waterVertices[3].Position = XMFLOAT3(2.0f,0.0f,2.0f);  // Bottom left.
//	waterVertices[3].UV = XMFLOAT2(0.0f, 1.0f);
//
//	waterVertices[4].Position = XMFLOAT3(2.0f, 0.0f, -2.0f);  // Top right.
//	waterVertices[4].UV = XMFLOAT2(1.0f, 0.0f);
//
//	waterVertices[5].Position = XMFLOAT3(-2.0f, 0.0f, 2.0f);  // Bottom right.
//	waterVertices[5].UV = XMFLOAT2(1.0f, 1.0f);
//
//	/*WaterVertex waterVertices[]
//	{
//		{ XMFLOAT3(+0.0f, +0.0f, +0.0f),XMFLOAT4(1.0f,0.0f,0.0f,1.0f) },
//		{ XMFLOAT3(+0.0f, +1.0f, +0.0f),XMFLOAT4(0.0f,0.0f,0.0f,1.0f) },
//		{ XMFLOAT3(+1.0f, +1.0f, +0.0f), XMFLOAT4(0.0f,0.0f,0.0f,1.0f) },
//		{ XMFLOAT3(+1.0f, +0.0f, +0.0f),XMFLOAT4(0.0f,0.0f,0.0f,1.0f) },
//	};
//*/
//	//int indices2[] = { 0,1,2,0,2,3 };
//
//	// Load the index array with data.
//	unsigned long* indices;
//	indices = new unsigned long[6];
//	indices[0] = 0;
//	indices[1] = 1;
//	indices[2] = 2;
//	indices[3] = 3;
//	indices[4] = 4;
//	indices[5] = 5;
//
//	indexCount = sizeof(indices) / sizeof(int);
//
//	// Create the vertex buffer
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_DEFAULT;
//	vbd.ByteWidth = sizeof(WaterVertex) * 6;
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//	vbd.StructureByteStride = 0;
//
//	// Give the subresource structure a pointer to the vertex data.
//	D3D11_SUBRESOURCE_DATA vertexData;
//	vertexData.pSysMem = waterVertices;
//	vertexData.SysMemPitch = 0;
//	vertexData.SysMemSlicePitch = 0;
//
//	// Now finally create the vertex buffer.
//	device->CreateBuffer(&vbd, &vertexData, &waterVertexBuffer);
//
//	// Create the index buffer
//	D3D11_BUFFER_DESC ibd;
//	ibd.Usage = D3D11_USAGE_DEFAULT;
//	ibd.ByteWidth = sizeof(unsigned long) * 6;
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.CPUAccessFlags = 0;
//	ibd.MiscFlags = 0;
//	ibd.StructureByteStride = 0;
//
//	// Give the subresource structure a pointer to the index data.
//	D3D11_SUBRESOURCE_DATA indexData;
//	indexData.pSysMem = indices;
//	indexData.SysMemPitch = 0;
//	indexData.SysMemSlicePitch = 0;
//
//	// Create the index buffer.
//	device->CreateBuffer(&ibd, &indexData, &waterIndexBuffer);
//
	currentEntity = 0;
}

bool Game::RenderRefractionToTexture()
{
	XMFLOAT4 clipPlane;

	//Setting up a clip plane based on the height of the water to clip everything above it to create a refraction.
	clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, water->GetWaterHeight() + 0.1f);

	//Set the render target to be the refraction render to texture
	_refractionTexture->SetRenderTarget(context);

	//Clear the refraction render to texture
	_refractionTexture->ClearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

	//Generate view matrix based on camera's position
	camera->UpdateViewMatrix();

	//Render the ground

	ID3D11Buffer* vb = bathBottom->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = bathBottom->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	refractionVS->SetMatrix4x4("world", *bathBottom->GetWorldMatrix());
	refractionVS->SetMatrix4x4("view", camera->GetView());
	refractionVS->SetMatrix4x4("projection", camera->GetProjection());

	refractionVS->SetFloat4("clipPlane", clipPlane);

	refractionVS->CopyAllBufferData();
	refractionVS->SetShader();

	refractionPS->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	refractionPS->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	refractionPS->SetFloat3("PointLightPosition", XMFLOAT3(0.5, 0, 0));
	refractionPS->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	refractionPS->SetFloat3("CameraPosition", camera->GetPosition());

	refractionPS->SetSamplerState("Sampler", sampler);
	refractionPS->SetShaderResourceView("shaderTexture", bathSRV);
	refractionPS->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	refractionPS->SetShaderResourceView("Sky", skySRV);

	refractionPS->CopyAllBufferData();
	refractionPS->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathBottom->GetMesh()->GetIndexCount(), 0, 0);

	//Set the render target back to back buffer
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	return true;
}



void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the projection matrix assuming the
	// camera exists
	if( camera ) 
		camera->UpdateProjectionMatrix((float)width / height);
}

void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	// Update the camera
	camera->Update(deltaTime);

	// Always update current entity's world matrix
	entities[currentEntity]->UpdateWorldMatrix();
	bathBottom->UpdateWorldMatrix();
	bathRight->UpdateWorldMatrix();
	bathLeft->UpdateWorldMatrix();
	bathBack->UpdateWorldMatrix();
	bathFront->UpdateWorldMatrix();

	//Do water frame processing
	water->Update();

	////Render the refraction of scene to a texture
	RenderRefractionToTexture();

	////Render the reflection
	//RenderReflectionToTexture();

	Draw(deltaTime, totalTime);
}


void Game::Draw(float deltaTime, float totalTime)
{
	// Background color for clearing
	const float color[4] = {1,1,1,1};

	// Render the refraction of the scene to a texture.
	RenderRefractionToTexture();

	// Render the reflection of the scene to a texture.
	//RenderReflectionToTexture();

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	


	/********************************************************************/
	// Draw the sky ------------------------
	ID3D11Buffer* vb = meshes[1]->GetVertexBuffer();
	ID3D11Buffer* ib = meshes[1]->GetIndexBuffer();

	// Set buffers in the input assembler
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	skyVS->SetMatrix4x4("view", camera->GetView());
	skyVS->SetMatrix4x4("projection", camera->GetProjection());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	skyPS->SetShaderResourceView("Sky", skySRV);
	skyPS->CopyAllBufferData();
	skyPS->SetShader();

	context->RSSetState(rsSky);
	context->OMSetDepthStencilState(dsSky, 0);
	context->DrawIndexed(meshes[1]->GetIndexCount(), 0, 0);

	// Reset the render states we've changed
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	/******************************************************************/
	//Draw the ground ---------------------------------------
	// Grab the data from the first entity's mesh
	GameEntity* ge = entities[currentEntity];
	vb = ge->GetMesh()->GetVertexBuffer();
	ib = ge->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *ge->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(0.3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", textureSRV);
	pixelShader->SetShaderResourceView("NormalMap", normalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(ge->GetMesh()->GetIndexCount(), 0, 0);

	/***********************************************************************************/
	//Draw Bath model
	//Bottom
	vb = bathBottom->GetMesh()->GetVertexBuffer();
	ib = bathBottom->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *bathBottom->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", bathSRV);
	pixelShader->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathBottom->GetMesh()->GetIndexCount(), 0, 0);

	//Left
	vb = bathLeft->GetMesh()->GetVertexBuffer();
	ib = bathLeft->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *bathLeft->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", bathSRV);
	pixelShader->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathLeft->GetMesh()->GetIndexCount(), 0, 0);

	//Right
	vb = bathRight->GetMesh()->GetVertexBuffer();
	ib = bathRight->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *bathRight->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", bathSRV);
	pixelShader->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathRight->GetMesh()->GetIndexCount(), 0, 0);

	//back
	vb = bathBack->GetMesh()->GetVertexBuffer();
	ib = bathBack->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *bathBack->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", bathSRV);
	pixelShader->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathBack->GetMesh()->GetIndexCount(), 0, 0);

	//Front
	vb = bathFront->GetMesh()->GetVertexBuffer();
	ib = bathFront->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler

	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("world", *bathFront->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	pixelShader->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	pixelShader->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	pixelShader->SetFloat4("PointLightColor", XMFLOAT4(1, 0.3f, 0.3f, 1));
	pixelShader->SetFloat3("CameraPosition", camera->GetPosition());

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Texture", bathSRV);
	pixelShader->SetShaderResourceView("NormalMap", bathNormalMapSRV);
	pixelShader->SetShaderResourceView("Sky", skySRV);

	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(bathFront->GetMesh()->GetIndexCount(), 0, 0);


	/**********************************************************************/
	//Draw water ---------------------------------------------------
	water->Render(context);

	camera->UpdateReflectionViewMatrix(water->GetWaterHeight());

	//waterVS->SetMatrix4x4("world", worldMatrix1);
	//waterVS->SetMatrix4x4("view", camera->GetView());
	//waterVS->SetMatrix4x4("projection", camera->GetProjection());
	//waterVS->SetMatrix4x4("reflection", camera->GetReflectionView() );
	//waterVS->SetFloat3("CameraPosition", camera->GetPosition());
	//waterVS->SetFloat2("normalMapTiling", water->GetNormalMapTiling());
	//waterVS->CopyAllBufferData();
	//waterVS->SetShader();
	//waterPS->SetShaderResourceView("reflectionTexture", skySRV);
	//waterPS->SetShaderResourceView("refractionTexture", _refractionTexture->GetShaderResourceView());
	//waterPS->SetShaderResourceView("normalTexture", water->GetTexture());
	//waterPS->SetSamplerState("SampleType", sampler);
	//waterPS->SetFloat("WaterTranslation", water->GetWaterTranslation());
	//waterPS->SetFloat("reflectRefractScale", water->GetReflectRefractScale());
	//waterPS->SetFloat4("refractionTint", water->GetRefractionTint());
	////waterPS->SetFloat3("lightDirection", XMFLOAT3(1, 0, 0));
	//waterPS->SetFloat3("CameraPosition", camera->GetPosition());
	//waterPS->SetFloat("WaterTranslation", water->GetWaterTranslation());
	//waterPS->SetFloat("reflectRefractScale", water->GetReflectRefractScale());
	//waterPS->SetFloat("specularShininess", water->GetSpecularShininess());
	//
	//waterPS->CopyAllBufferData();
	//waterPS->SetShader();
	
	//context->IASetVertexBuffers(0, 1, , &stride, &offset);
	//context->IASetIndexBuffer(waterIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	camera->UpdateReflectionViewMatrix(water->GetWaterHeight());
	waterVS->SetMatrix4x4("world", worldMatrix1);
	waterVS->SetMatrix4x4("view", camera->GetView());
	waterVS->SetMatrix4x4("projection", camera->GetProjection());
	waterVS->SetMatrix4x4("reflection", camera->GetReflectionView());

	waterVS->CopyAllBufferData();
	waterVS->SetShader();

	waterPS->SetFloat4("DirLightColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1));
	waterPS->SetFloat3("DirLightDirection", XMFLOAT3(1, 0, 0));
	waterPS->SetFloat4("PointLightColor", XMFLOAT4(0.3, 0.3f, 0.3f, 1));
	waterPS->SetFloat3("PointLightPosition", XMFLOAT3(3, 0, 0));
	waterPS->SetFloat3("CameraPosition", camera->GetPosition());

	waterPS->SetFloat("waterTranslation", water->GetWaterTranslation());

	waterPS->SetShaderResourceView("reflectionTexture", skySRV);
	waterPS->SetShaderResourceView("refractionTexture",bathSRV);
	waterPS->SetShaderResourceView("NormalMap", water->GetTexture());
	waterPS->SetShaderResourceView("Sky", skySRV);
	waterPS->SetSamplerState("Sampler", sampler);
	
	waterPS->CopyAllBufferData();
	waterPS->SetShader();

	
	context->DrawIndexed(water->GetIndexCount(),0,0);
	//	


	/******************************************************************/




	
	

	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Check left mouse button
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion