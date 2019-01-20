#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "RenderState.hpp"

namespace Lobelia::Graphics {
	BlendState::BlendState(BLEND_PRESET preset, bool blend, bool alpha_coverage) {
		HRESULT hr = S_OK;
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = alpha_coverage;
		desc.IndependentBlendEnable = false;
		desc.RenderTarget[0].BlendEnable = blend;
		SettingPreset(&desc, static_cast<int>(preset));
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = Device::Get()->CreateBlendState(&desc, state.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ブレンドステート設定失敗");
	}
	BlendState::~BlendState() = default;
	void BlendState::SettingPreset(D3D11_BLEND_DESC* desc, int preset)const {
		BLEND_PRESET p = static_cast<BLEND_PRESET>(preset);
		switch (p) {
		default:
		case BLEND_PRESET::NONE:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::COPY:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::ADD:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::SUB:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::REPLACE:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::MULTIPLY:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::LIGHTENESS:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
			break;
		case BLEND_PRESET::DARKENESS:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BLEND_PRESET::SCREEN:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_INV_DEST_COLOR;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		}
	}
	void BlendState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		float blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
		Device::GetContext()->OMSetBlendState(state.Get(), blendFactor, 0xFFFFFFFF);
	}

	SamplerState::SamplerState(SAMPLER_PRESET preset, int max_anisotropy, bool is_border) {
		HRESULT hr = S_OK;
		D3D11_SAMPLER_DESC desc = {};
		SettingPreset(&desc, static_cast<int>(preset));
		if (is_border) {
			desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		}
		else {
			desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		}
		desc.MaxAnisotropy = max_anisotropy;
		desc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = +FLT_MAX;
		DirectX::XMFLOAT4 block(0.0f, 0.0f, 0.0f, 0.0f);
		memcpy(desc.BorderColor, &block, sizeof(DirectX::XMFLOAT4));

		hr = Device::Get()->CreateSamplerState(&desc, state.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("サンプラーステートの作成に失敗");
	}
	SamplerState::~SamplerState() = default;
	void SamplerState::SettingPreset(D3D11_SAMPLER_DESC* desc, int preset)const {
		SAMPLER_PRESET p = static_cast<SAMPLER_PRESET>(preset);
		switch (p) {
		default:
		case	SAMPLER_PRESET::POINT:										desc->Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
		case	SAMPLER_PRESET::LINEAR:										desc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case	SAMPLER_PRESET::ANISOTROPIC:							desc->Filter = D3D11_FILTER_ANISOTROPIC; break;
		case 	SAMPLER_PRESET::COMPARISON_POINT:				desc->Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT; break;
		case	SAMPLER_PRESET::COMPARISON_LINEAR:				desc->Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; break;
		case	SAMPLER_PRESET::COMPARISON_ANISOTROPIC:	desc->Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC; break;
		}
	}
	void SamplerState::Set(bool force_set)noexcept { Set(0, force_set); }
	void SamplerState::Set(int slot, bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->PSSetSamplers(slot, 1, state.GetAddressOf());
		Device::GetContext()->DSSetSamplers(slot, 1, state.GetAddressOf());
	}

	RasterizerState::RasterizerState(RASTERIZER_PRESET preset, bool wire_frame) {
		HRESULT hr = S_OK;
		//ラスタライズの設定
		D3D11_RASTERIZER_DESC desc = {};
		SettingPreset(&desc, static_cast<int>(preset));
		//2:ワイヤーフレーム表示 3:面表示
		desc.FillMode = wire_frame ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		//表面が反時計回りが表(TRUE)か時計回りが表(FALSE)かの設定
		desc.FrontCounterClockwise = false;
		desc.AntialiasedLineEnable = true;
		desc.MultisampleEnable = true;
		hr = Device::Get()->CreateRasterizerState(&desc, state.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ラスタライザ作成に失敗");
	}
	RasterizerState::~RasterizerState() = default;
	void RasterizerState::SettingPreset(D3D11_RASTERIZER_DESC* desc, int preset)const {
		RASTERIZER_PRESET p = static_cast<RASTERIZER_PRESET>(preset);
		switch (p) {
		default:
		case RASTERIZER_PRESET::FRONT:	desc->CullMode = D3D11_CULL_FRONT; break;
		case RASTERIZER_PRESET::BACK:		desc->CullMode = D3D11_CULL_BACK;	break;
		case RASTERIZER_PRESET::NONE:	desc->CullMode = D3D11_CULL_NONE;	break;
		}
	}
	void RasterizerState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->RSSetState(state.Get());
	}
	DepthStencilState::DepthStencilState(DEPTH_PRESET preset, bool depth, StencilDesc sdesc, bool stencil) {
		HRESULT hr = S_OK;
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = depth;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = stencil;
		if (stencil) {
			desc.StencilReadMask = sdesc.readMask;
			desc.StencilWriteMask = sdesc.writeMask;
			//表面をどうするか
			desc.FrontFace.StencilDepthFailOp = s_cast<D3D11_STENCIL_OP>(sdesc.front.depthFail);
			desc.FrontFace.StencilFailOp = s_cast<D3D11_STENCIL_OP>(sdesc.front.faile);
			desc.FrontFace.StencilPassOp = s_cast<D3D11_STENCIL_OP>(sdesc.front.pass);
			desc.FrontFace.StencilFunc = s_cast<D3D11_COMPARISON_FUNC>(sdesc.front.testFunc);
			//裏面をどうするか
			desc.BackFace.StencilDepthFailOp = s_cast<D3D11_STENCIL_OP>(sdesc.back.depthFail);
			desc.BackFace.StencilFailOp = s_cast<D3D11_STENCIL_OP>(sdesc.back.faile);
			desc.BackFace.StencilPassOp = s_cast<D3D11_STENCIL_OP>(sdesc.back.pass);
			desc.BackFace.StencilFunc = s_cast<D3D11_COMPARISON_FUNC>(sdesc.back.testFunc);
		}
		hr = Device::Get()->CreateDepthStencilState(&desc, state.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("深度ステンシルステート作成に失敗");
	}
	DepthStencilState::~DepthStencilState() = default;
	void DepthStencilState::SettingPreset(D3D11_DEPTH_STENCIL_DESC* desc, int preset)const {
		DEPTH_PRESET p = static_cast<DEPTH_PRESET>(preset);
		switch (p) {
		default:
		case DEPTH_PRESET::NEVER:					desc->DepthFunc = D3D11_COMPARISON_NEVER;					break;
		case DEPTH_PRESET::LESS:						desc->DepthFunc = D3D11_COMPARISON_LESS;						break;
		case DEPTH_PRESET::EQUAL:					desc->DepthFunc = D3D11_COMPARISON_EQUAL;					break;
		case DEPTH_PRESET::LESS_EQUAL:			desc->DepthFunc = D3D11_COMPARISON_NEVER;					break;
		case DEPTH_PRESET::GREATER:				desc->DepthFunc = D3D11_COMPARISON_GREATER;				break;
		case DEPTH_PRESET::NOT_EQUAL:			desc->DepthFunc = D3D11_COMPARISON_NOT_EQUAL;			break;
		case DEPTH_PRESET::GREATER_EQUAL:	desc->DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;	break;
		case DEPTH_PRESET::ALWAYS:					desc->DepthFunc = D3D11_COMPARISON_ALWAYS;				break;
		}
	}
	void DepthStencilState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->OMSetDepthStencilState(state.Get(), 1);
	}

}