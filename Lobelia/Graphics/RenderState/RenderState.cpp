#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Config/Config.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "RenderState.hpp"

namespace Lobelia::Graphics {
	BlendState::BlendState(BlendPreset preset, bool blend, bool alpha_coverage) {
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
		BlendPreset p = static_cast<BlendPreset>(preset);
		switch (p) {
		default:
		case BlendPreset::NONE:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::COPY:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::ADD:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::SUB:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::REPLACE:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::MULTIPLY:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::LIGHTENESS:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
			break;
		case BlendPreset::DARKENESS:
			desc->RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
			desc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			desc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case BlendPreset::SCREEN:
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
		float blendFactory[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
		Device::GetContext()->OMSetBlendState(state.Get(), blendFactory, 0xFFFFFFFF);
	}

	SamplerState::SamplerState(SamplerPreset preset, int max_anisotropy) {
		HRESULT hr = S_OK;
		D3D11_SAMPLER_DESC desc = {};
		SettingPreset(&desc, static_cast<int>(preset));
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
		SamplerPreset p = static_cast<SamplerPreset>(preset);
		switch (p) {
		default:
		case	SamplerPreset::POINT:										desc->Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
		case	SamplerPreset::LINEAR:										desc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case	SamplerPreset::ANISOTROPIC:							desc->Filter = D3D11_FILTER_ANISOTROPIC; break;
		case 	SamplerPreset::COMPARISON_POINT:				desc->Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT; break;
		case	SamplerPreset::COMPARISON_LINEAR:				desc->Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; break;
		case	SamplerPreset::COMPARISON_ANISOTROPIC:	desc->Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC; break;
		}
	}
	void SamplerState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->PSSetSamplers(0, 1, state.GetAddressOf());
		Device::GetContext()->DSSetSamplers(0, 1, state.GetAddressOf());
	}

	RasterizerState::RasterizerState(RasterizerPreset preset, bool wire_frame) {
		HRESULT hr = S_OK;
		//ラスタライズの設定
		D3D11_RASTERIZER_DESC desc = {};
		SettingPreset(&desc, static_cast<int>(preset));
		//2:ワイヤーフレーム表示 3:面表示
		desc.FillMode = wire_frame ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		//表面が反時計回りが表(TRUE)か時計回りが表(FALSE)かの設定
		desc.FrontCounterClockwise = false;
		desc.AntialiasedLineEnable = false;
		desc.MultisampleEnable = false;
		hr = Device::Get()->CreateRasterizerState(&desc, state.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ラスタライザ作成に失敗");
	}
	RasterizerState::~RasterizerState() = default;
	void RasterizerState::SettingPreset(D3D11_RASTERIZER_DESC* desc, int preset)const {
		RasterizerPreset p = static_cast<RasterizerPreset>(preset);
		switch (p) {
		default:
		case RasterizerPreset::FRONT:	desc->CullMode = D3D11_CULL_FRONT; break;
		case RasterizerPreset::BACK:		desc->CullMode = D3D11_CULL_BACK;	break;
		case RasterizerPreset::NONE:	desc->CullMode = D3D11_CULL_NONE;	break;
		}
	}
	void RasterizerState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->RSSetState(state.Get());
	}
	DepthStencilState::DepthStencilState(DepthPreset preset, bool depth, StencilDesc sdesc, bool stencil) {
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
		DepthPreset p = static_cast<DepthPreset>(preset);
		switch (p) {
		default:
		case DepthPreset::NEVER:					desc->DepthFunc = D3D11_COMPARISON_NEVER;					break;
		case DepthPreset::LESS:						desc->DepthFunc = D3D11_COMPARISON_LESS;						break;
		case DepthPreset::EQUAL:					desc->DepthFunc = D3D11_COMPARISON_EQUAL;					break;
		case DepthPreset::LESS_EQUAL:			desc->DepthFunc = D3D11_COMPARISON_NEVER;					break;
		case DepthPreset::GREATER:				desc->DepthFunc = D3D11_COMPARISON_GREATER;				break;
		case DepthPreset::NOT_EQUAL:			desc->DepthFunc = D3D11_COMPARISON_NOT_EQUAL;			break;
		case DepthPreset::GREATER_EQUAL:	desc->DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;	break;
		case DepthPreset::ALWAYS:					desc->DepthFunc = D3D11_COMPARISON_ALWAYS;				break;
		}
	}
	void DepthStencilState::Set(bool force_set)noexcept {
		if (!force_set&&IsSet())return;
		Device::GetContext()->OMSetDepthStencilState(state.Get(), 1);
	}

	void RenderStateBank::BlendFactory(std::string key, BlendPreset p, bool blend_enable, bool alpha_coverage)noexcept { ResourceBank<BlendState>::Factory(key, p, blend_enable, alpha_coverage); }
	void RenderStateBank::SamplerFactory(std::string key, SamplerPreset p, int max_anisotropy) { ResourceBank<SamplerState>::Factory(key, p, max_anisotropy); }
	void RenderStateBank::RasterizerFactory(std::string key, RasterizerPreset p, bool wire_frame) { ResourceBank<RasterizerState>::Factory(key, p, wire_frame); }
	void RenderStateBank::DepthStencilFactory(std::string key, DepthPreset p, bool depth, StencilDesc desc, bool stencil) { ResourceBank<DepthStencilState>::Factory(key, p, depth, desc, stencil); }
}