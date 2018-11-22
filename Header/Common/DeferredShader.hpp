#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//���ۂɃV�F�[�f�B���O���镔��
	//�X�؃L�����ɂ͎����I�Ƀu���[����������܂�
	//�G�~�b�V����
	class Camera;
	class DeferredShader {
	public:
		DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps);
		virtual ~DeferredShader() = default;
		//���O�Ńu���[���|����ꍇ
		void Render(class DeferredBuffer* buffer);
		//�u���[���������ł�����ꍇ
		void RenderHDR(Graphics::View* active_view, Graphics::RenderTarget* active_rt, class DeferredBuffer* buffer);
		void DebugRender();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::BlendState> blend;
		std::unique_ptr<Graphics::RenderTarget> hdrTarget;
		std::shared_ptr<class HDRPS> hdr;
	};
	//---------------------------------------------------------------------------------------------
	class SimpleDeferred :public DeferredShader {
	public:
		SimpleDeferred();
	};
	//---------------------------------------------------------------------------------------------
	class FullEffectDeferred :public DeferredShader {
	public:
		struct PointLight {
			Math::Vector4 pos;
			Utility::Color color;
			float attenuation;
		};
	public:
		FullEffectDeferred();
		void SetLightBuffer(int index, const PointLight& p_light);
		void SetUseCount(int use_count);
		void Update();
	private:
		ALIGN(16) struct PointLights {
			Math::Vector4 pos[LIGHT_SUM];
			Math::Vector4 color[LIGHT_SUM];
			Math::Vector4 attenuation[LIGHT_SUM];
			int usedLightCount;
		}lights;
		std::unique_ptr<Graphics::ConstantBuffer<PointLights>> cbuffer;
	};
}