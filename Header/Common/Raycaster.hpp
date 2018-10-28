#pragma once
namespace Lobelia::Game {
	//ここではGPGPUを用いたRaycast(Raypick)を実装している
	class StructuredBuffer;
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	//Ray用に作成されるメッシュの形状バッファ
	class RayMesh {
		friend class RayResult;
	public:
		RayMesh(Graphics::Model* model);
		~RayMesh() = default;
		void Set();
		int GetPolygonCount();
	private:
		//GPUへの入力 ポリゴン情報
		struct Input {
			Math::Vector3 pos[3];
		};
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		int polygonCount;
	};
	//---------------------------------------------------------------------------------------------
	class RayResult {
	public:
		//GPUからの出力用バッファ
		struct Output {
			int hit;
			float length;
			Math::Vector3 normal;
		};
	public:
		RayResult(RayMesh* mesh);
		~RayResult() = default;
		void Set();
		void Clean();
		//結果の読み込み
		void Load();
		const Output* Lock();
		void UnLock();
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		std::unique_ptr<class UnorderedAccessView> uav;
		std::unique_ptr<class ReadGPUBuffer> readBuffer;
	};
	//---------------------------------------------------------------------------------------------
	//Rayをうつクラス
	//Singletonでもよいかも？
	class Raycaster {
	public:
		Raycaster() = delete;
		~Raycaster() = delete;
		static void Initialize();
		//第一引数はワールド変換行列
		//現状は裏から入っても当たってしまう
		//すぐ結果を取り出そうとすると著しくパフォーマンスを落とす
		//戻り値はRayが正常に飛ばされたかどうか
		static bool Dispatch(const DirectX::XMMATRIX& world, RayMesh* mesh, RayResult* result, const Math::Vector3& begin, const Math::Vector3& end);
	private:
		//コンスタントバッファ
		struct Info {
			Math::Vector4 rayBegin;
			Math::Vector4 rayEnd;
			DirectX::XMFLOAT4X4 world;
			DirectX::XMFLOAT4X4 worldInverse;
		};
	private:
		static std::unique_ptr<Graphics::ComputeShader> cs;
		static std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		static Info info;
	};

}