#pragma once
#include "DXCore.h"
#include <DirectXMath.h>

using namespace DirectX;

class Water
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

public:
	Water();
	~Water();

	bool Initialize(ID3D11Device* device, WCHAR* textureFile, float waterHeight, float waterRadius);
	void Update();
	void Render(ID3D11DeviceContext* context);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	float GetWaterHeight() { return _waterHeight; }
	XMFLOAT2 GetNormalMapTiling() { return _normalMapTiling; }
	float GetWaterTranslation() { return _waterTranslation; }
	float GetReflectRefractScale() { return _reflectRefractScale; }
	float GetSpecularShininess() { return _specularShininess; }
	XMFLOAT4 GetRefractionTint() { return _refractionTint; }

private:
	bool InitializeBuffers(ID3D11Device* device, float waterRadius);
	void RenderBuffers(ID3D11DeviceContext* context);

	bool LoadTexture(ID3D11Device* device, WCHAR* textureFile);

private:
	float _waterHeight;
	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int _vertexCount, _indexCount;
	XMFLOAT2 _normalMapTiling;
	float _waterTranslation;
	float _reflectRefractScale;
	float _specularShininess;
	XMFLOAT4 _refractionTint;
	ID3D11ShaderResourceView* waterSRV;

};

