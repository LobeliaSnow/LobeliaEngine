#pragma once
namespace Lobelia::Graphics {
	enum class BLEND_PRESET { MIN = -1, NONE, COPY, ADD, SUB, REPLACE, MULTIPLY, LIGHTENESS, DARKENESS, SCREEN, MAX };
	enum class SAMPLER_PRESET { MIN, POINT, LINEAR, ANISOTROPIC, COMPARISON_POINT, COMPARISON_LINEAR, COMPARISON_ANISOTROPIC, MAX, };
	enum class RASTERIZER_PRESET { MIN, FRONT, BACK, NONE, MAX, };
	enum class DEPTH_PRESET { MIN, NEVER, LESS, EQUAL, LESS_EQUAL, GREATER, NOT_EQUAL, GREATER_EQUAL, ALWAYS, MAX, };
	struct StencilDesc {
		//読み書きしないときは0を指定してね。
		UINT8 readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		UINT8 writeMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		struct Face {
			enum class Operation :BYTE { KEEP = D3D11_STENCIL_OP_KEEP, ZERO = D3D11_STENCIL_OP_ZERO, REPLACE = D3D11_STENCIL_OP_REPLACE, INCR_CLAMP = D3D11_STENCIL_OP_INCR_SAT, DECR_CLAMP = D3D11_STENCIL_OP_DECR_SAT, INVERT = D3D11_STENCIL_OP_INVERT, INCR_WRAP = D3D11_STENCIL_OP_INCR, DECR_WRAP = D3D11_STENCIL_OP_DECR };
			//テスト失敗
			Operation faile;
			//深度テスト失敗
			Operation depthFail;
			//テスト合格
			Operation pass;
			//左項 ソース値 右項 ステンシル値
			enum class Function :BYTE { ALWAYS_FAIL = D3D11_COMPARISON_NEVER, LESS = D3D11_COMPARISON_LESS, EQUAL = D3D11_COMPARISON_EQUAL, LESS_EQUAL = D3D11_COMPARISON_LESS_EQUAL, GREATER = D3D11_COMPARISON_GREATER, NOT_EQUAL = D3D11_COMPARISON_NOT_EQUAL, GREATER_EQUAL = D3D11_COMPARISON_GREATER_EQUAL, ALWAYS_PASS = D3D11_COMPARISON_ALWAYS };
			//ステンシルテストを行う関数
			Function testFunc;
		};
		Face front;
		Face back;
	};
	using StencilOperation = StencilDesc::Face::Operation;
	using StencilFunction = StencilDesc::Face::Function;
	class BlendState final :private Origin<BlendState> {
	private:
		/**@brief ブレンドステート本体*/
		ComPtr<ID3D11BlendState> state;
	public:
		//作成に失敗した場合、Exception型で例外を投げます
		BlendState(BLEND_PRESET preset, bool blend, bool alpha_coverage);
		~BlendState();
		void SettingPreset(D3D11_BLEND_DESC* desc, int preset)const;
		void Set(bool force_set = false)noexcept;
	};
	class SamplerState final :private Origin<SamplerState> {
	private:
		/**@brief サンプラーステート本体*/
		ComPtr<ID3D11SamplerState> state;
	public:
		//作成に失敗した場合、Exception型で例外を投げます
		SamplerState(SAMPLER_PRESET preset, int max_anisotropy, bool is_border = false);
		~SamplerState();
		void SettingPreset(D3D11_SAMPLER_DESC* desc, int preset)const;
		void Set(bool force_set = false)noexcept;
		void Set(int slot, bool force_set = false)noexcept;
	};
	class RasterizerState final :private Origin<RasterizerState> {
	private:
		/**@brief ラスタライザーステート本体*/
		ComPtr<ID3D11RasterizerState> state;
	public:
		//作成に失敗した場合、Exception型で例外を投げます
		RasterizerState(RASTERIZER_PRESET preset, bool wire_frame = false);
		~RasterizerState();
		void SettingPreset(D3D11_RASTERIZER_DESC* desc, int preset)const;
		void Set(bool force_set = false)noexcept;
	};
	class DepthStencilState final :private Origin<DepthStencilState> {
	private:
		ComPtr<ID3D11DepthStencilState> state;
	public:
		//作成に失敗した場合、Exception型で例外を投げます
		//stencilがまだよくわかっていない。
		DepthStencilState(DEPTH_PRESET preset, bool depth, StencilDesc sdesc, bool stencil);
		~DepthStencilState();
		void SettingPreset(D3D11_DEPTH_STENCIL_DESC* desc, int preset)const;
		void Set(bool force_set = false)noexcept;
	};

}