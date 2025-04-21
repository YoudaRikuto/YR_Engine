#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "Graphics.h"
#include "Framework/Misc.h"

template<class T>
class ConstantBuffer
{
public:
    ConstantBuffer()
    {
        HRESULT result = S_OK;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = (sizeof(T) + 0x0f) & ~0x0f;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pSysMem = &data_;
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
        result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, buffer_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }
    virtual ~ConstantBuffer() = default;

    void Activate(const int& slot, const bool& vsSet = true, const bool& psSet = true,
        const bool& csSet = false, const bool& gsSet = false, const bool& hsSet = false)
    {
        ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

        deviceContext->UpdateSubresource(buffer_.Get(), 0, 0, &data_, 0, 0);

        if (vsSet)
        {
            deviceContext->VSSetConstantBuffers(slot, 1, buffer_.GetAddressOf());
        }
        if (psSet)
        {
            deviceContext->PSSetConstantBuffers(slot, 1, buffer_.GetAddressOf());
        }
        if (csSet) 
        {
            deviceContext->CSSetConstantBuffers(slot, 1, buffer_.GetAddressOf());
        }
        if (gsSet) 
        {
            deviceContext->GSSetConstantBuffers(slot, 1, buffer_.GetAddressOf());
        }
        if (hsSet) 
        {
            deviceContext->HSSetConstantBuffers(slot, 1, buffer_.GetAddressOf());
        }
    }
    void Deactivete() {}

    T* GetData() { return &data_; }

private:
    T data_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
};