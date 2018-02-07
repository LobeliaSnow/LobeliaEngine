#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Exception/Exception.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Config/Config.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/View/View.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"
//#include "Movie/Movie.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Pipeline.hpp"

namespace Lobelia::Graphics {
	Pipeline::Pipeline(const std::string& blend_key, const std::string& sampler_key, const std::string& depth_stencil_key, const std::string& rasterizer_key, const std::string& vs_key, int vs_instance_count, InstanceID* vs_instances, const std::string& ps_key, int ps_instance_count, InstanceID* ps_instances)
		:blend(ResourceBank<BlendState>::Get(blend_key)), sampler(ResourceBank<SamplerState>::Get(sampler_key)), depthStencil(ResourceBank<DepthStencilState>::Get(depth_stencil_key)), rasterizer(ResourceBank<RasterizerState>::Get(rasterizer_key)), vs(ResourceBank<VertexShader>::Get(vs_key)), vsInstanceCount(vs_instance_count), ps(ResourceBank<PixelShader>::Get(ps_key)), psInstanceCount(ps_instance_count) {
		vsInstances.resize(vs_instance_count);
		for (int i = 0; i < vs_instance_count; i++) {
			vsInstances[i] = vs_instances[i];
		}
		psInstances.resize(ps_instance_count);
		for (int i = 0; i < ps_instance_count; i++) {
			psInstances[i] = ps_instances[i];
		}
		//vs->Set(vsInstanceCount, vsInstances.data());
		//ps->Set(psInstanceCount, psInstances.data());
	}
	Pipeline::~Pipeline() = default;

	void Pipeline::BlendSet(const std::string& key) { isBlendChange = true; blend = ResourceBank<BlendState>::Get(key); }
	void Pipeline::SamplerSet(const std::string& key) { isSamplerChange = true; sampler = ResourceBank<SamplerState>::Get(key); }
	void Pipeline::DepthStencilSet(const std::string& key) { isDepthStencilChange = true; depthStencil = ResourceBank<DepthStencilState>::Get(key); }
	void Pipeline::RasterizerSet(const std::string& key) { isRasterizerChange = true; rasterizer = ResourceBank<RasterizerState>::Get(key); }
	void Pipeline::VertexShaderSet(const std::string& key, int instance_count, InstanceID* instances) {
		isVSChange = true;
		vs = ResourceBank<VertexShader>::Get(key);
		vsInstanceCount = instance_count;
		vsInstances.resize(instance_count);
		for (int i = 0; i < instance_count; i++) {
			vsInstances[i] = instances[i];
		}
	}
	void Pipeline::VertexShaderSet(int instance_count, InstanceID* instances) {
		isVSChange = true;
		vsInstanceCount = instance_count;
		vsInstances.resize(instance_count);
		for (int i = 0; i < instance_count; i++) {
			vsInstances[i] = instances[i];
		}
	}
	void Pipeline::PixelShaderSet(const std::string& key, int instance_count, InstanceID* instances) {
		isPSChange = true;
		ps = ResourceBank<PixelShader>::Get(key);
		psInstanceCount = instance_count;
		psInstances.resize(instance_count);
		for (int i = 0; i < instance_count; i++) {
			psInstances[i] = instances[i];
		}
	}
	void Pipeline::PixelShaderSet(int instance_count, InstanceID* instances) {
		isPSChange = true;
		psInstanceCount = instance_count;
		psInstances.resize(instance_count);
		for (int i = 0; i < instance_count; i++) {
			psInstances[i] = instances[i];
		}
	}

	VertexShader* Pipeline::GetVertexShader() {
		isVSChange = true;
		return vs;
	}
	PixelShader* Pipeline::GetPixelShader() {
		isPSChange = true;
		return ps;
	}
	void Pipeline::Activate(bool force_set) {
		if (blend&&(force_set || isBlendChange))blend->Set(force_set);
		if (sampler&&(force_set || isSamplerChange))sampler->Set(force_set);
		if (depthStencil&&(force_set || isDepthStencilChange))depthStencil->Set(force_set);
		if (rasterizer&&(force_set || isRasterizerChange))rasterizer->Set(force_set);
		if (vs&&(force_set || isVSChange))vs->Set(vsInstanceCount, vsInstances.data());
		if (ps&&(force_set || isPSChange))ps->Set(psInstanceCount, psInstances.data());
		isBlendChange = isSamplerChange = isDepthStencilChange = isRasterizerChange = isVSChange = isPSChange = false;
		PipelineManager::pipeline = this;
	}
	Pipeline* PipelineManager::pipeline = nullptr;

	void PipelineManager::PipelineRegister(const std::string& key, Pipeline* pipeline) { ResourceBank<Pipeline>::Register(key, std::shared_ptr<Pipeline>(pipeline)); }
	Pipeline* PipelineManager::PipelineGet(const std::string& key) { return ResourceBank<Pipeline>::Get(key); }
	Pipeline* PipelineManager::PipelineGet() { return pipeline; }
	void PipelineManager::PipelineActivate(const std::string& key, bool force_set) { PipelineGet(key)->Activate(force_set); }
	void PipelineManager::PipelineActivate(bool force_set) { pipeline->Activate(force_set); }

}