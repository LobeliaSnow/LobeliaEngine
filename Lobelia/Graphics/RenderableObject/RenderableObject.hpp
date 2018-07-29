#pragma once
namespace Lobelia::Graphics {
	template<class T> class RenderableObject {
	protected:
		static std::shared_ptr<BlendState> blend;
		static std::shared_ptr<SamplerState> sampler;
		static std::shared_ptr<RasterizerState> rasterizer;
		static std::shared_ptr<DepthStencilState> depthStencil;
		static std::shared_ptr<InputLayout> inputLayout;
		static std::shared_ptr<VertexShader> vs;
		static std::shared_ptr<PixelShader> ps;
	protected:
		static void Activate() {
			blend->Set(true);
			sampler->Set(true);
			rasterizer->Set(true);
			depthStencil->Set(true);
			inputLayout->Set();
			vs->Set();
			ps->Set();
		}
	public:
		RenderableObject() = default;
		virtual ~RenderableObject() = default;
		static void ChangeBlendState(std::shared_ptr<BlendState> blend) { RenderableObject<T>::blend = blend; }
		static void ChangeSamplerState(std::shared_ptr<SamplerState> sampler) { RenderableObject<T>::sampler = sampler; }
		static void ChangeRasterizerState(std::shared_ptr<RasterizerState> rasterizer) { RenderableObject<T>::rasterizer = rasterizer; }
		static void ChangeDepthStencilState(std::shared_ptr<DepthStencilState> depthStencil) { RenderableObject<T>::depthStencil = depthStencil; }
		static void ChangeInputLayout(std::shared_ptr<InputLayout> inputLayout) { RenderableObject<T>::inputLayout = inputLayout; }
		static void ChangeVertexShader(std::shared_ptr<VertexShader> vs) { RenderableObject<T>::vs = vs; }
		static void ChangePixelShader(std::shared_ptr<PixelShader> ps) { RenderableObject<T>::ps = ps; }
		static std::shared_ptr<BlendState> GetBlendState() { return blend; }
		static std::shared_ptr<SamplerState> GetSamplerState() { return sampler; }
		static std::shared_ptr<RasterizerState> GetRasterizerState() { return rasterizer; }
		static std::shared_ptr<DepthStencilState> GetDepthStencilState() { return depthStencil; }
		static std::shared_ptr<InputLayout> GetInputLayout() { return inputLayout; }
		static std::shared_ptr<VertexShader> GetVertexShader() { return vs; }
		static std::shared_ptr<PixelShader> GetPixelShader() { return ps; }
	};
	template<class T> std::shared_ptr<BlendState> RenderableObject<T>::blend;
	template<class T> std::shared_ptr<SamplerState> RenderableObject<T>::sampler;
	template<class T> std::shared_ptr<RasterizerState> RenderableObject<T>::rasterizer;
	template<class T> std::shared_ptr<DepthStencilState> RenderableObject<T>::depthStencil;
	template<class T> std::shared_ptr<InputLayout> RenderableObject<T>::inputLayout;
	template<class T> std::shared_ptr<VertexShader> RenderableObject<T>::vs;
	template<class T> std::shared_ptr<PixelShader> RenderableObject<T>::ps;
}