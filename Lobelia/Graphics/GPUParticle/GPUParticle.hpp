#pragma once
#include "../Data/ShaderFile/GPUParticle/GPUParticleDefine.hlsli"

namespace Lobelia::Graphics {
	class Pipeline;
	//インスタンス生成少し時間かかる可能性あり
	//生成はCPUで、更新、描画はGPUで
	//TODO : 後に値が不変のものは、staticにしてまとめる
	class GPUParticleSystem {
	public:
		enum class BlendMode { Copy, Add, Sub, Screen };
	private:
		ALIGN(16)struct Info {
		public:
			//そのフレームの追加数
			int appendCount = 0;
			//そのフレームの経過時間
			float elapsedTime = 0.0f;
			//バイトニックソートの比較する要素の間隔
			int compareInterval = 0;
			//昇順ソートか降順ソートかの判別用
			BOOL divideLevel = 0;
			BOOL isBitonicFinal = false;
		public:
			void Update(float elapsed_time);
		};
		struct RWByteAddressBuffer {
			ComPtr<ID3D11Buffer> buffer;
			ComPtr<ID3D11UnorderedAccessView> uav;
			RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
			~RWByteAddressBuffer();
			void ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count);
		private:
			void CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
			void CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer);
		};
	public:
		//パーティクル一つを定義する構造体
		struct Particle {
			Math::Vector3 pos;
			//移動量
			Math::Vector3 move;
			//加速度 (メートル毎秒毎秒)
			Math::Vector3 power;
			//使用するテクスチャの番号
			int textureIndex;
			//0.0f~1.0f
			Math::Vector2 uvPos;
			Math::Vector2 uvSize;
			//生存時間
			float aliveTime;
			//経過時間
			float elapsedTime;
			//フェードインにかかる時間
			float fadeInTime;
			//フェードアウト開始時間
			float fadeOutTime;
			//開始時の拡大率
			float startScale;
			//終了時の拡大率
			float endScale;
			//開始時の回転角
			float startRad;
			//終了時の回転角
			float endRad;
			//補正色
			Math::Vector3 color;
			Particle(const Math::Vector3& pos, const Math::Vector3& move, const Math::Vector3& power, int texture_index, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, float alive_time, float fade_in_time, float fade_out_time, float start_scale, float end_scale, float start_rad, float end_rad, Utility::Color color);
			Particle();
			~Particle();
		};
	private:
		static const int TEXTURE_COUNT = 8;
		static const std::string BLEND_LIST[4];
	private:
		std::unique_ptr<ConstantBuffer<Info>> infoCBuffer;
		Info info;
		std::unique_ptr<RWByteAddressBuffer> appendData;
		std::unique_ptr<RWByteAddressBuffer> indexBuffer;
		std::unique_ptr<RWByteAddressBuffer> dataBuffer;
		std::unique_ptr<RWByteAddressBuffer> indirectArgs;
		std::unique_ptr<ComputeShader> appendCS;
		std::unique_ptr<ComputeShader> sortCS;
		std::unique_ptr<ComputeShader> updateCS;
		std::unique_ptr<GeometryShader> gs;
		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<InputLayout> inputLayout;
		Texture* textureList[TEXTURE_COUNT];
		Particle appendParticles[APPEND_PARTICLE_MAX];
	private:
		void RunAppend();
		void RunSort();
		void RunUpdate();
	public:
		GPUParticleSystem();
		~GPUParticleSystem();
		void SetTexture(int slot, Texture* texture);
		void Append(const Particle& particle);
		//何倍速にするかの決定
		void Update(float time_scale = 1.0f);
		void Render(BlendMode mode = BlendMode::Add);
	};
}