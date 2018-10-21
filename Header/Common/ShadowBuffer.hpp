#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//現状遠距離がすごく荒いので、カスケード急ぎたい。
	//Parallel Split Shadow Map (カスケード系)実装予定
	//カスケードの参考
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	//後に作り直す
	//カスケードの分ける部分で若干影が消える場合がある
	//Exponantial Shadow Mapを合わせたい
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model> model);
		void SetNearPlane(float near_z);
		void SetFarPlane(float far_z);
		void SetLamda(float lamda);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		void CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//シャドウマップをセットする
		//テクスチャスロット6~6+split_countセットする
		//コンスタントバッファは
		//ALIGN(16) struct Info {
		//	DirectX::XMFLOAT4X4 view;
		//	DirectX::XMFLOAT4X4 proj[split_count];
		//#ifdef CASCADE
		//	Math::Vector4 lpos;
		// Math::Vector4 ldir;
		//	Math::Vector4 splitPos;
		//#endif
		//	int useShadowMap;
		//	int useVariance;
		//};
		//の形式でバインドされる
		void Begin();
		void End();
		void DebugRender();
	private:
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z);
		void CameraUpdate();
	private:
		//後に代わる
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 view;
#ifdef CASCADE
			//4前提
			std::array<DirectX::XMFLOAT4X4, 4> proj;
			Math::Vector4 pos;
			Math::Vector4 front;
			float splitPos[4];
#else
			std::array<DirectX::XMFLOAT4X4, 1> proj;
#endif
			int useShadowMap;
			int useVariance;
		};
	private:
		std::vector<std::unique_ptr<Graphics::View>> views;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		//near/farが入ってます
		std::vector<float> cascadeValues;
		//分割位置が入ってます
		std::vector<float> splitPositions;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//std::unique_ptr<Graphics::SamplerState> sampler;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
#ifdef GAUSSIAN_CS
		std::vector<std::unique_ptr<GaussianFilterCS>> gaussian;
#endif
#ifdef GAUSSIAN_PS
		std::vector<std::unique_ptr<GaussianFilterPS>> gaussian;
#endif
		Info info;
		Math::Vector2 size;
		float nearZ;
		float farZ;
		float lamda;
		const int count;
	};

}