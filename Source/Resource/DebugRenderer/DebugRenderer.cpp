#include "DebugRenderer.h"
#include <stdio.h>
#include <memory>
#include "Graphics/Graphics.h"
#include "FrameWork/Misc.h"

DebugRenderer::DebugRenderer()
{
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	Graphics::Instance().CreateVsFromCso("./Resources/Shader/DebugVS.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	Graphics::Instance().CreatePsFromCso("./Resources/Shader/DebugPS.cso", pixelShader_.GetAddressOf());

	constantBuffer_ = std::make_unique<ConstantBuffer<Constants>>();

	// 球メッシュ作成
	CreateSphereMesh(1.0f, 16, 16);

	// 円柱メッシュ作成
	CreateCylinderMesh(1.0f, 1.0f, 0.0f, 1.0f, 16 * 2.0f, 1);

	// 箱メッシュ作成
	CreateBoxMesh(1.0f);
}

void DebugRenderer::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

	// シェーダー設定
	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
	deviceContext->IASetInputLayout(inputLayout_.Get());

	constantBuffer_->Activate(0);

	Graphics::Instance().SetBlendState(Shader::BlendState::Alpha);
	Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);
	Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);


	// ビュープロジェクション行列作成
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX VP = V * P;

	// プリミティブ設定
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 球描画
	deviceContext->IASetVertexBuffers(0, 1, sphereVertexBuffer_.GetAddressOf(), &stride, &offset);
	for (const Sphere& sphere : spheres)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(sphere.radius_, sphere.radius_, sphere.radius_);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(sphere.center_.x, sphere.center_.y, sphere.center_.z);
		DirectX::XMMATRIX W = S * T;
		DirectX::XMMATRIX WVP = W * VP;

		constantBuffer_->GetData()->color = sphere.color_;
		DirectX::XMStoreFloat4x4(&constantBuffer_->GetData()->wvp, WVP);
		constantBuffer_->Activate(0);

		deviceContext->Draw(sphereVertexCount, 0);
	}
	spheres.clear();

	// 円柱描画
	deviceContext->IASetVertexBuffers(0, 1, cylinderVertexBuffer_.GetAddressOf(), &stride, &offset);
	for (const Cylinder& cylinder : cylinders)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(cylinder.radius_, cylinder.height_, cylinder.radius_);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(cylinder.position_.x, cylinder.position_.y, cylinder.position_.z);
		DirectX::XMMATRIX W = S * T;
		DirectX::XMMATRIX WVP = W * VP;


		constantBuffer_->GetData()->color = cylinder.color_;
		DirectX::XMStoreFloat4x4(&constantBuffer_->GetData()->wvp, WVP);
		constantBuffer_->Activate(0);

		deviceContext->Draw(cylinderVertexCount, 0);
	}
	cylinders.clear();


	// 箱描画
	deviceContext->IASetVertexBuffers(0, 1, boxVertexBuffer_.GetAddressOf(), &stride, &offset);
	for (const Box& box : boxes)
	{
		// ワールドビュープロジェクション行列作成
		const DirectX::XMMATRIX S = DirectX::XMMatrixScaling(box.scale_.x, box.scale_.y, box.scale_.z);
		//// 回転行列を作成
		//DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
		DirectX::XMMATRIX X = DirectX::XMMatrixRotationX(box.rotation_.x);
		DirectX::XMMATRIX Y = DirectX::XMMatrixRotationY(box.rotation_.y);
		DirectX::XMMATRIX Z = DirectX::XMMatrixRotationZ(box.rotation_.z);
		// 先にX軸、Z軸の回転を確定させてからY軸を回転させる
		//（右から順番に掛けられる）
		DirectX::XMMATRIX R = Y * X * Z;
		const DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(box.center_.x, box.center_.y, box.center_.z);

		const DirectX::XMMATRIX W = S * R * T;
		const DirectX::XMMATRIX WVP = W * VP;

		constantBuffer_->GetData()->color = box.color_;
		DirectX::XMStoreFloat4x4(&constantBuffer_->GetData()->wvp, WVP);
		constantBuffer_->Activate(0);

		deviceContext->Draw(boxVertexCount, 0);
	}
	boxes.clear();
}

// 球描画
void DebugRenderer::DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color)
{
	Sphere sphere;
	sphere.center_ = center;
	sphere.radius_ = radius;
	sphere.color_ = color;
	spheres.emplace_back(sphere);
}

// 円柱描画
void DebugRenderer::DrawCylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color)
{
	Cylinder cylinder;
	cylinder.position_ = position;
	cylinder.radius_ = radius;
	cylinder.height_ = height;
	cylinder.color_ = color;
	cylinders.emplace_back(cylinder);
}

// 箱描画
void DebugRenderer::DrawBox(
	const DirectX::XMFLOAT3& center,
	const DirectX::XMFLOAT3& rotation,
	const DirectX::XMFLOAT3& scale,
	const DirectX::XMFLOAT4& color)
{
	Box box;
	box.center_ = center;
	box.rotation_ = rotation;
	box.scale_ = scale;
	box.color_ = color;
	boxes.emplace_back(box);
}

void DebugRenderer::CreateSphereMesh(const float& radius, const int& slices, const int& stacks)
{
	sphereVertexCount = stacks * slices * 2 + slices * stacks * 2;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(sphereVertexCount);

	float phiStep = DirectX::XM_PI / stacks;
	float thetaStep = DirectX::XM_2PI / slices;

	DirectX::XMFLOAT3* p = vertices.get();

	for (int i = 0; i < stacks; ++i)
	{
		float phi = i * phiStep;
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);

		for (int j = 0; j < slices; ++j)
		{
			float theta = j * thetaStep;
			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;

			theta += thetaStep;

			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;
		}
	}

	thetaStep = DirectX::XM_2PI / stacks;
	for (int i = 0; i < slices; ++i)
	{
		DirectX::XMMATRIX M = DirectX::XMMatrixRotationY(i * thetaStep);
		for (int j = 0; j < stacks; ++j)
		{
			float theta = j * thetaStep;
			DirectX::XMVECTOR V1 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P1 = DirectX::XMVector3TransformCoord(V1, M);
			DirectX::XMStoreFloat3(p++, P1);

			int n = (j + 1) % stacks;
			theta += thetaStep;

			DirectX::XMVECTOR V2 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P2 = DirectX::XMVector3TransformCoord(V2, M);
			DirectX::XMStoreFloat3(p++, P2);
		}
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * sphereVertexCount);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT result = Graphics::Instance().GetDevice()->CreateBuffer(&desc, &subresourceData, sphereVertexBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
	}
}

void DebugRenderer::CreateCylinderMesh(const float& radius1, const float& radius2, const float& start, const float& height, const int& slices, const int& stacks)
{
	cylinderVertexCount = 2 * slices * (stacks + 1) + 2 * slices;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(cylinderVertexCount);

	DirectX::XMFLOAT3* p = vertices.get();

	float stackHeight = height / stacks;
	float radiusStep = (radius2 - radius1) / stacks;

	// vertices of ring
	float dTheta = DirectX::XM_2PI / slices;

	for (int i = 0; i < slices; ++i)
	{
		int n = (i + 1) % slices;

		float c1 = cosf(i * dTheta);
		float s1 = sinf(i * dTheta);

		float c2 = cosf(n * dTheta);
		float s2 = sinf(n * dTheta);

		for (int j = 0; j <= stacks; ++j)
		{
			float y = start + j * stackHeight;
			float r = radius1 + j * radiusStep;

			p->x = r * c1;
			p->y = y;
			p->z = r * s1;
			p++;

			p->x = r * c2;
			p->y = y;
			p->z = r * s2;
			p++;
		}

		p->x = radius1 * c1;
		p->y = start;
		p->z = radius1 * s1;
		p++;

		p->x = radius2 * c1;
		p->y = start + height;
		p->z = radius2 * s1;
		p++;
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * cylinderVertexCount);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT result = Graphics::Instance().GetDevice()->CreateBuffer(&desc, &subresourceData, cylinderVertexBuffer_.GetAddressOf());		
		_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
	}
}

void DebugRenderer::CreateBoxMesh(const float& size)
{
	boxVertexCount = 24;
	const std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(boxVertexCount);

	DirectX::XMFLOAT3* p = vertices.get();

	// 上と下の面
	for (int i = 0; i < 2; ++i)
	{
		// 上下に平らな四角形を作る
		const float sizeY = (i == 0) ? size : -size;

		{
			// 左上
			p->x = -size;
			p->y = sizeY;
			p->z = size;
			++p;

			// 右上
			p->x = size;
			p->y = sizeY;
			p->z = size;
			++p;
		}

		{
			// 右上
			p->x = size;
			p->y = sizeY;
			p->z = size;
			++p;

			// 右下
			p->x = size;
			p->y = sizeY;
			p->z = -size;
			++p;
		}

		{
			// 右下
			p->x = size;
			p->y = sizeY;
			p->z = -size;
			++p;

			// 左下
			p->x = -size;
			p->y = sizeY;
			p->z = -size;
			++p;
		}

		{
			// 左下
			p->x = -size;
			p->y = sizeY;
			p->z = -size;
			++p;

			// 左上
			p->x = -size;
			p->y = sizeY;
			p->z = size;
			++p;
		}
	}

	// 側面
	for (int i = 0; i < 2; ++i)
	{
		// 手前と奥でそれぞれ2本で合計4本の縦線を作る
		const float sizeZ = (i == 0) ? -size : size;

		{
			// 左上の線
			p->x = -size;
			p->y = size;
			p->z = sizeZ;
			++p;

			// 左下の線
			p->x = -size;
			p->y = -size;
			p->z = sizeZ;
			++p;
		}

		{
			// 右上の線
			p->x = size;
			p->y = size;
			p->z = sizeZ;
			++p;

			// 右下の線
			p->x = size;
			p->y = -size;
			p->z = sizeZ;
			++p;
		}
	}


	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * boxVertexCount);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT result = Graphics::Instance().GetDevice()->CreateBuffer(&desc, &subresourceData, boxVertexBuffer_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
	}
}