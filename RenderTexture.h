#pragma once
#include "DXCore.h"
#include <DirectXMath.h>

using namespace DirectX;

class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear);
	void SetRenderTarget(ID3D11DeviceContext* context);
	void ClearRenderTarget(ID3D11DeviceContext* context, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView();
	XMFLOAT4X4 GetProjectionMatrix() { return _projectionMatrix; }
	XMFLOAT4X4 GetOrthoMatrix() { return _orthoMatrix; }

private:
	ID3D11Texture2D* _renderTargetTexture;
	ID3D11RenderTargetView* _renderTargetView;
	ID3D11ShaderResourceView* _shaderResourceView;
	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	D3D11_VIEWPORT _viewport;
	XMFLOAT4X4 _projectionMatrix;
	XMFLOAT4X4 _orthoMatrix;
};

