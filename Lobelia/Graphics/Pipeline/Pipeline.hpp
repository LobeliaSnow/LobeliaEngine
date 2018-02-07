#pragma once
namespace Lobelia::Graphics {
	//ここはまた後程よく考えて
	//あらかじめステートを作成してこれに当てはめる
	class Pipeline {
	private:
		//後ほどマップ参照はチェンジするときのみにして、キーではなくポインタで持つようにする
		BlendState* blend;
		bool isBlendChange;
		SamplerState* sampler;
		bool isSamplerChange;
		DepthStencilState* depthStencil;
		bool isDepthStencilChange;
		RasterizerState* rasterizer;
		bool isRasterizerChange;
		VertexShader* vs;
		bool isVSChange;
		int vsInstanceCount;
		std::vector<InstanceID> vsInstances;
		PixelShader* ps;
		bool isPSChange;
		int psInstanceCount;
		std::vector<InstanceID> psInstances;
	public:
		Pipeline(const std::string& blend_key, const std::string& sampler_key, const std::string& depth_stencil_key, const std::string& rasterizer_key, const std::string& vs_key, int vs_instance_count, InstanceID* vs_instances, const std::string& ps_key, int ps_instance_count, InstanceID* ps_instances);
		~Pipeline();
		void BlendSet(const std::string& key);
		void SamplerSet(const std::string& key);
		void DepthStencilSet(const std::string& key);
		void RasterizerSet(const std::string& key);
		void VertexShaderSet(const std::string& key, int instance_count, InstanceID* instances);
		void VertexShaderSet(int instance_count, InstanceID* instances);
		void PixelShaderSet(const std::string& key, int instance_count, InstanceID* instances);
		void PixelShaderSet(int instance_count, InstanceID* instances);
		VertexShader* GetVertexShader();
		PixelShader* GetPixelShader();
		void Activate(bool force_set = false);
	};
	//パイプラインを切り替える際はforce_setをtrueにしないと意図した結果が表れない可能性があります
	class PipelineManager {
		friend Pipeline;
	private:
		static Pipeline* pipeline;
	public:
		static void PipelineRegister(const std::string& key, Pipeline* pipeline);
		static Pipeline* PipelineGet(const std::string& key);
		static Pipeline* PipelineGet();
		//static Pipeline* DefaultPipelineSprite2DGet();
		//static Pipeline* DefaultPipelineSprite2DBatchGet();
		//static Pipeline* DefaultPipeline3DStaticGet();
		//static Pipeline* DefaultPipeline3DDynamicGet();
		//static Pipeline* DefaultPipeline3DInstancingStaticGet();
		//static Pipeline* DefaultPipeline3DInstancingDynamicGet();
		static void PipelineActivate(const std::string& key, bool force_set = true);
		static void PipelineActivate(bool force_set = false);
	};
}