#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <wrl.h>

class DebugRenderer
{
public:
    DebugRenderer();
    ~DebugRenderer() {}

    void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
    

	void DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color);
	void DrawCylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);
	void DrawBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& color);

private:
	void CreateSphereMesh(const float& radius, const int& slices, const int& stacks);
	void CreateCylinderMesh(const float& radius1, const float& radius2, const float& start, const float& height, const int& slices, const int& stacks);
	void CreateBoxMesh(const float& size);

private:
	struct Constants
	{
		DirectX::XMFLOAT4X4	wvp;
		DirectX::XMFLOAT4	color;
	};

	struct Sphere
	{
		DirectX::XMFLOAT4	color_;
		DirectX::XMFLOAT3	center_;
		float				radius_;
	};

	struct Cylinder
	{
		DirectX::XMFLOAT4	color_;
		DirectX::XMFLOAT3	position_;
		float				radius_;
		float				height_;
	};

	struct Box
	{
		DirectX::XMFLOAT3   center_;
		DirectX::XMFLOAT3   rotation_;
		DirectX::XMFLOAT3   scale_;
		DirectX::XMFLOAT4   color_;
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer>			sphereVertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			cylinderVertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			boxVertexBuffer_;	
	Microsoft::WRL::ComPtr<ID3D11Buffer>			constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout_;

	std::vector<Sphere>		spheres;
	std::vector<Cylinder>	cylinders;
	std::vector<Box>		boxes;

	UINT	sphereVertexCount = 0;
	UINT	cylinderVertexCount = 0;
	UINT	boxVertexCount = 0;

};

