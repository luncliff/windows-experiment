#pragma once
#include <winrt/Windows.Foundation.h>

#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <string_view>

HRESULT create_shader(ID3D11Device* device, std::wstring_view fpath, ID3D11VertexShader** output);
HRESULT create_shader(ID3D11Device* device, std::wstring_view fpath, ID3D11PixelShader** output);

class texture_renderer_t final {
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> context;
    winrt::com_ptr<ID3D11VertexShader> vertex_shader = nullptr;
    winrt::com_ptr<ID3D11PixelShader> pixel_shader = nullptr;
    winrt::com_ptr<ID3D11InputLayout> input_layout = nullptr;
    winrt::com_ptr<ID3D11Buffer> vertex_buffer = nullptr;
    winrt::com_ptr<ID3D11Buffer> index_buffer = nullptr;
    winrt::com_ptr<ID3D11SamplerState> sampler = nullptr;

  public:
    texture_renderer_t(ID3D11Device*, ID3D11DeviceContext*, winrt::take_ownership_from_abi_t) noexcept(false);
    texture_renderer_t(ID3D11Device*, ID3D11DeviceContext*) noexcept(false);

    void setup(const std::filesystem::path& folder) noexcept(false);

    void draw(ID3D11ShaderResourceView* texture0, //
              ID3D11RenderTargetView* target0, const D3D11_VIEWPORT& viewport) noexcept(false);

  private:
    HRESULT make_input_layout(std::wstring_view path, ID3D11InputLayout** output) noexcept;
    HRESULT make_buffer(UINT binding, ID3D11Buffer** output) noexcept;
    HRESULT make_sampler_state(ID3D11SamplerState** output) noexcept;
};
