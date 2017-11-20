#include "Water.h"
#include "DDSTextureLoader.h"


Water::Water()
{
}


Water::~Water()
{
	//release texture
	waterSRV->Release();

	//Release vertex and index buffer
	// Release the index buffer.
	if (indexBuffer)
	{
		indexBuffer->Release();
		indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = 0;
	}

	return;
}

bool Water::Initialize(ID3D11Device * device, WCHAR * textureFile, float waterHeight, float waterRadius)
{

	//Store the water height
	_waterHeight = waterHeight;

	//Initialize index and vertex buffer
	InitializeBuffers(device, waterRadius);

	//Load Texture
	LoadTexture(device, textureFile);

	//Set the tiling for normal map
	_normalMapTiling.x = 0.01f;
	_normalMapTiling.y = 0.02f;

	//Initialize water translation to 0
	_waterTranslation = 0.0f;


	//Scaling value for water normal map
	_reflectRefractScale = 0.03f;

	// Set the tint of the refraction.
	_refractionTint = XMFLOAT4(0.0f, 0.8f, 1.0f, 1.0f);

	//Set specular shininess
	_specularShininess = 200.0f;

	return true;
}

void Water::Update()
{
	// Update the position of the water to simulate motion.
	_waterTranslation += 0.001f;
	if (_waterTranslation > 1.0f)
	{
		_waterTranslation -= 1.0f;
	}

	return;
}

void Water::Render(ID3D11DeviceContext * context)
{
	RenderBuffers(context);
}

int Water::GetIndexCount()
{
	return _indexCount;
}

ID3D11ShaderResourceView * Water::GetTexture()
{
	return waterSRV;
}

bool Water::InitializeBuffers(ID3D11Device * device, float waterRadius)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	
	// Set the number of vertices in the vertex array.
	_vertexCount = 6;

	// Set the number of indices in the index array.
	_indexCount = 6;

	// Create the vertex array.
	vertices = new VertexType[_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[_indexCount];
	if(!indices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-waterRadius, 0.15f, waterRadius);  // Top left.
	vertices[0].uv = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(waterRadius, 0.15f, waterRadius);  // Top right.
	vertices[1].uv = XMFLOAT2(1.0f, 0.0f);
	
	vertices[2].position = XMFLOAT3(-waterRadius, 0.15f, -waterRadius);  // Bottom left.
	vertices[2].uv = XMFLOAT2(0.0f, 1.0f);

	vertices[3].position = XMFLOAT3(-waterRadius, 0.15f, -waterRadius);  // Bottom left.
	vertices[3].uv = XMFLOAT2(0.0f, 1.0f);

	vertices[4].position = XMFLOAT3(waterRadius, 0.15f, waterRadius);  // Top right.
	vertices[4].uv = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(waterRadius, 0.15f, -waterRadius);  // Bottom right.
	vertices[5].uv = XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
	indices[4] = 4;
	indices[5] = 5;

	// Set up the description of the vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * _vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	

	// Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * _indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void Water::RenderBuffers(ID3D11DeviceContext * context)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool Water::LoadTexture(ID3D11Device * device, WCHAR * textureFile)
{
	CreateDDSTextureFromFile(device, textureFile, 0, &waterSRV);

	return true;
}
