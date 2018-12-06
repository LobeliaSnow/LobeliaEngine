#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//現状遠距離がすごく荒いので、カスケード急ぎたい。
	//Parallel Split Shadow Map (カスケード系)実装予定
	//カスケード
	//カスケードの参考
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
	//描画用のカメラの視錘台を分割し、ライトの位置から各視錘台を完全に入るようにカメラを置いてシャドウマップを作る
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	struct AABB {
		Math::Vector3 min;
		Math::Vector3 max;
	};
	//後に作り直す
	//カスケードの分ける部分で若干影が消える場合がある
	//Exponantial Shadow Mapを合わせたい
	//near,farをきれいになるように調整する必要があるが、
	//表示されるオブジェクトのAABBが必要となるので今回はスルー
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model> model);
		void SetNearPlane(float near_z);
		void SetFarPlane(float far_z);
		void SetLamda(float lamda);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		void SetVariance(bool use_variance);
		void SetEnable(bool use_shadow);
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
		//視錘台のAABB計算
		AABB CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp);
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z, float* split_pos);
		DirectX::XMFLOAT4X4 FrustumAABBClippingMatrix(AABB clip_aabb);
		void CreateCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp);
		void CameraUpdate(Graphics::View* main_camera);
	private:
		//後に代わる
		ALIGN(16) struct Info {
#ifdef CASCADE
			//4前提
			std::array<DirectX::XMFLOAT4X4, 4> cascadeLVP;
			Math::Vector4 pos;
			Math::Vector4 front;
			float splitRange[4];
#else
			DirectX::XMFLOAT4X4 lvp;
#endif
			int useShadowMap;
			int useVariance;
			int nowIndex;
		};
	private:
		std::vector<std::unique_ptr<Graphics::View>> views;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		std::unique_ptr<Graphics::SamplerState> sampler;
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
		float fov;
		float aspect;
		float nearZ;
		float farZ;
		float lamda;
		const int count;
	};

}