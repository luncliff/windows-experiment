#include "texture_renderer.hpp"

#include <DirectXMath.h>
#include <array>
#include <fstream>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

using namespace std::filesystem;

HRESULT create_shader(ID3D11Device* device, std::wstring_view fpath, ID3D11VertexShader** output) {
    std::wstring path{fpath};
    winrt::com_ptr<ID3DBlob> blob = nullptr;
    if (auto hr = D3DReadFileToBlob(path.c_str(), blob.put()); FAILED(hr))
        return hr;
    return device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, output);
}

HRESULT create_shader(ID3D11Device* device, std::wstring_view fpath, ID3D11PixelShader** output) {
    std::wstring path{fpath};
    winrt::com_ptr<ID3DBlob> blob = nullptr;
    if (auto hr = D3DReadFileToBlob(path.c_str(), blob.put()); FAILED(hr))
        return hr;
    return device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, output);
}

texture_renderer_t::texture_renderer_t(ID3D11Device* device, ID3D11DeviceContext* context,
                                       winrt::take_ownership_from_abi_t ownership) noexcept(false)
    : device{device, ownership}, context{context, ownership} {
}
texture_renderer_t::texture_renderer_t(ID3D11Device* device, ID3D11DeviceContext* context) noexcept(false)
    : device{nullptr}, context{nullptr} {
    if (device == nullptr)
        throw winrt::hresult_invalid_argument{};
    if (auto hr = device->QueryInterface(this->device.put()); FAILED(hr))
        throw winrt::hresult_error{hr};

    if (context == nullptr) {
        device->GetImmediateContext(this->context.put());
        return;
    }

    if (auto hr = context->QueryInterface(this->context.put()); FAILED(hr))
        throw winrt::hresult_error{hr};
}

void texture_renderer_t::setup(const std::filesystem::path& folder) noexcept(false) {
    const auto shader0 = folder / "shader-vertex1.cso";
    if (auto hr = create_shader(device.get(), shader0.native(), vertex_shader.put()); FAILED(hr))
        throw winrt::hresult_error{hr};
    if (auto hr = make_input_layout(shader0.native(), input_layout.put()); FAILED(hr))
        throw winrt::hresult_error{hr};

    const auto shader1 = folder / "shader-pixel1.cso";
    if (auto hr = create_shader(device.get(), shader1.native(), pixel_shader.put()); FAILED(hr))
        throw winrt::hresult_error{hr};

    if (auto hr = make_buffer(D3D11_BIND_VERTEX_BUFFER, vertex_buffer.put()); FAILED(hr))
        throw winrt::hresult_error{hr};
    if (auto hr = make_buffer(D3D11_BIND_INDEX_BUFFER, index_buffer.put()); FAILED(hr))
        throw winrt::hresult_error{hr};

    if (auto hr = make_sampler_state(sampler.put()); FAILED(hr))
        throw winrt::hresult_error{hr};
}

HRESULT texture_renderer_t::make_sampler_state(ID3D11SamplerState** output) noexcept {
    CD3D11_SAMPLER_DESC desc{CD3D11_DEFAULT{}};
    return device->CreateSamplerState(&desc, output);
}

inline namespace builtin {
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;

struct VertexShaderInput final {
    XMFLOAT3 position;
    XMFLOAT2 texcoord;
};

VertexShaderInput vertices[4]{
    {XMFLOAT3(-1, 1, 0), XMFLOAT2(0, 0)},  // top left
    {XMFLOAT3(1, 1, 0), XMFLOAT2(1, 0)},   // top right
    {XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 1)}, // bottom left
    {XMFLOAT3(1, -1, 0), XMFLOAT2(1, 1)},  // bottom right
};

// for DXGI_FORMAT_R16_UINT, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
uint16_t indices[6]{
    0, 1, 2, // triangle 1
    2, 1, 3, // triangle 2
};

} // namespace builtin

HRESULT texture_renderer_t::make_input_layout(std::wstring_view p, ID3D11InputLayout** output) noexcept {
    using namespace builtin;

    const path fpath{p};
    auto buflen = file_size(fpath);
    std::ifstream fin{fpath, std::ios::binary};
    auto bytecode = std::make_unique<std::byte[]>(buflen);
    fin.read(reinterpret_cast<char*>(bytecode.get()), buflen);

    D3D11_INPUT_ELEMENT_DESC elements[2]{
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexShaderInput, position), //
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexShaderInput, texcoord), //
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    return device->CreateInputLayout(elements, 2, bytecode.get(), buflen, output);
}

HRESULT texture_renderer_t::make_buffer(UINT binding, ID3D11Buffer** output) noexcept {
    using namespace builtin;
    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA init{};
    if (binding & D3D11_BIND_VERTEX_BUFFER) {
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        init.pSysMem = vertices;
        desc.ByteWidth = sizeof(VertexShaderInput) * 4;
    } else if (binding & D3D11_BIND_INDEX_BUFFER) {
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        init.pSysMem = indices;
        desc.ByteWidth = sizeof(uint16_t) * 6;
    }
    return device->CreateBuffer(&desc, &init, output);
}

void texture_renderer_t::draw(ID3D11ShaderResourceView* texture0, //
                              ID3D11RenderTargetView* target0, const D3D11_VIEWPORT& viewport) noexcept(false) {
    using namespace builtin;
    std::array<ID3D11Buffer*, 1> iab{vertex_buffer.get()}; // input assembly
    std::array<UINT, iab.max_size()> strides{sizeof(VertexShaderInput)};
    std::array<UINT, iab.max_size()> offsets{};
    std::array<ID3D11ShaderResourceView*, 1> resources{texture0};
    std::array<ID3D11RenderTargetView*, 1> targets{target0};
    std::array<ID3D11SamplerState*, 1> samplers{sampler.get()};
    //std::array<ID3D11Buffer*, 1> contstants;
    XMFLOAT4 blending{};

    context->OMSetBlendState(nullptr, reinterpret_cast<float*>(&blending), 0xFFFF'FFFF);
    context->OMSetRenderTargets(targets.size(), targets.data(), nullptr);

    UINT slot = 0;
    context->PSSetShader(pixel_shader.get(), nullptr, 0);
    slot = 0;
    context->PSSetShaderResources(slot, resources.size(), resources.data());
    context->PSSetSamplers(slot, samplers.size(), samplers.data());

    context->VSSetShader(vertex_shader.get(), nullptr, 0);
    slot = 0;
    context->IASetVertexBuffers(slot, iab.size(), iab.data(), strides.data(), offsets.data());
    context->IASetIndexBuffer(index_buffer.get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(input_layout.get());

    context->RSSetViewports(1, &viewport);
    context->DrawIndexed(6, 0, 0);
}
