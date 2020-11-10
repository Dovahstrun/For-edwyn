#include "LightShader.h"

LightShader::LightShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"light_vs.cso", L"light_ps.cso");
}


LightShader::~LightShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC attnBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	//Set up attenuation buffer
	attnBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	attnBufferDesc.ByteWidth = sizeof(AttnBufferType);
	attnBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	attnBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	attnBufferDesc.MiscFlags = 0;
	attnBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&attnBufferDesc, NULL, &attnBuffer);


}


void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, Light* light[2])
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	
	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	//lightPtr->ambient = light->getAmbientColour();
	XMFLOAT4 tempPos;
	for (int i = 0; i < 2; ++i)
	{
		lightPtr->diffuse[i] = light[i]->getDiffuseColour();
	}
	for (int i = 0; i < 2; ++i)
	{
		tempPos.x = light[i]->getPosition().x;
		tempPos.y = light[i]->getPosition().y;
		tempPos.z = light[i]->getPosition().z;
		tempPos.w = 0;
		lightPtr->position[i] = tempPos;
	}
	
	
	//lightPtr->padding = 0.0f;
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}
//
//void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, Light* light, attnVal attenValues)
//{
//	HRESULT result;
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//	MatrixBufferType* dataPtr;
//
//	XMMATRIX tworld, tview, tproj;
//
//
//	// Transpose the matrices to prepare them for the shader.
//	tworld = XMMatrixTranspose(worldMatrix);
//	tview = XMMatrixTranspose(viewMatrix);
//	tproj = XMMatrixTranspose(projectionMatrix);
//	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//	dataPtr = (MatrixBufferType*)mappedResource.pData;
//	dataPtr->world = tworld;// worldMatrix;
//	dataPtr->view = tview;
//	dataPtr->projection = tproj;
//	deviceContext->Unmap(matrixBuffer, 0);
//	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
//
//	//Additional
//	// Send light data to pixel shader
//	LightBufferType* lightPtr;
//	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//	lightPtr = (LightBufferType*)mappedResource.pData;
//	lightPtr->ambient = light->getAmbientColour();
//	lightPtr->diffuse = light->getDiffuseColour();
//	lightPtr->position = light->getPosition();
//	lightPtr->padding = 0.0f;
//	deviceContext->Unmap(lightBuffer, 0);
//	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
//
//	//Send attenuation data to pixel shader
//	AttnBufferType* attnPtr;
//	deviceContext->Map(attnBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//	attnPtr = (AttnBufferType*)mappedResource.pData;
//	attnPtr->constFactor = attenValues.constFactor;
//	attnPtr->linFactor = attenValues.linFactor;
//	attnPtr->quadFactor = attenValues.quadFactor;
//	attnPtr->padding = 0.0f;
//	deviceContext->Unmap(attnBuffer, 0);
//	deviceContext->PSSetConstantBuffers(1, 1, &attnBuffer);
//
//
//
//
//	// Set shader texture resource in the pixel shader.
//	deviceContext->PSSetShaderResources(0, 1, &texture);
//	deviceContext->PSSetSamplers(0, 1, &sampleState);
//}
