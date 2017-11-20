#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>

#include "Camera.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "RenderTexture.h"
#include "Water.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:



	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	// Input and mesh swapping
	bool prevTab;
	unsigned int currentEntity;

	// Keep track of "stuff" to clean up
	std::vector<Mesh*> meshes;
	std::vector<GameEntity*> entities;
	Camera* camera;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();

	//Water stuff
	Mesh* waterMesh;
	//GameEntity* water;
	Water* water;
	GameEntity* bathBottom;
	GameEntity* bathRight;
	GameEntity* bathLeft;
	GameEntity* bathBack;
	GameEntity* bathFront;

	SimpleVertexShader* waterVS;
	SimplePixelShader* waterPS;
	SimpleVertexShader* refractionVS;
	SimplePixelShader* refractionPS;

	RenderTexture* _refractionTexture;
	RenderTexture* _reflectionTexture;

	bool RenderRefractionToTexture();
	bool RenderReflectionToTexture();


	// Texture related DX stuff
	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalMapSRV;
	ID3D11ShaderResourceView* waterNormalMapSRV;
	ID3D11ShaderResourceView* bathSRV;
	ID3D11ShaderResourceView* bathNormalMapSRV;
	ID3D11SamplerState* sampler;

	// Sky stuff
	ID3D11ShaderResourceView* skySRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	ID3D11RasterizerState* rsSky;
	ID3D11DepthStencilState* dsSky;

	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	DirectX::XMFLOAT4X4 worldMatrix1;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};

