#pragma once
#include "../Data/ShaderFile/Define.h"

//�p�t�H�[�}���X�\�L�͂����܂ł��ڈ��A�����Ɉڂ�ς��̂�ڎ��Ŋm�F���������Ȃ̂ƁA
//���������̃m�[�gPC�݂̂Ȃ̂ŐM�����͒Ⴂ
//Compute Shader�łŎ������Ă�����̃p�t�H�[�}���X�v���ׁ̈APixel Shader�ł�����������

namespace Lobelia::Game {
	//GBuffer����
	class DeferredBuffer {
	public:
		enum class BUFFER_TYPE {
			POS,
			NORMAL,
			COLOR,
			VIEW_POS,
		};
	public:
		DeferredBuffer(const Math::Vector2& size);
		~DeferredBuffer() = default;
		void AddModel(std::shared_ptr<Graphics::Model> model, bool use_normal_map);
		void RenderGBuffer();
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget(BUFFER_TYPE type);
		//�t���O����Ăǂ̏���L���ɂ���̂��؂�ւ����Ƃ����Ǝv��
		void Begin();
		void End();
		void DebugRender();
	private:
		ALIGN(16) struct Info {
			int useNormalMap;
			int useSpecularMap;
		};
		struct ModelStorage {
			std::weak_ptr<Graphics::Model> model;
			Info info;
		};
	private:
		//MRT�p �o�b�t�@�̉����̂��߂ɖ@���Ɛ[�x������͕����Ă��� a������depth������ΐ��͌���
		std::shared_ptr<Graphics::RenderTarget> rts[4];
		//��񏑂����ݗp
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::list<ModelStorage> models;
		const Math::Vector2 size;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};
	//���ۂɃV�F�[�f�B���O���镔��
	class DeferredShader {
	public:
		DeferredShader(const char* file_path, const char* entry_vs, const char* entry_ps);
		virtual ~DeferredShader() = default;
		void Render();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
	};
	class SimpleDeferred :public DeferredShader {
	public:
		SimpleDeferred();
	};
	class PointLightDeferred :public DeferredShader {
	public:
		struct PointLight {
			Math::Vector4 pos;
			Utility::Color color;
			float attenuation;
		};
	public:
		PointLightDeferred();
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
	//Parallel Split Shadow Map (�J�X�P�[�h�n)
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count);
	private:
		struct Info {

		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::vector<std::shared_ptr<Graphics::RenderTarget>> rts;
		std::list<std::weak_ptr<Graphics::Model>> models;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//�o���A���X����ۂɎg���\��B
		std::unique_ptr<GaussianFilter> gaussian;
	};
	class PostEffect abstract {
	public:
		PostEffect(const Math::Vector2& size, bool create_rt);
		virtual ~PostEffect() = default;
		std::shared_ptr<Graphics::RenderTarget>& GetRenderTarget();
		//�f�t�H���g�̓���́Art�̕`��
		virtual void Render();
		//�|�X�g�G�t�F�N�g���e�N�X�`���Ƃ��ăZ�b�g����
		virtual void Begin(int slot);
		//��Еt��
		virtual void End();
	protected:
		std::shared_ptr<Graphics::RenderTarget> rt;
		Math::Vector2 size;
		int slot;
	};
	class UnorderedAccessView {
	public:
		UnorderedAccessView(Graphics::Texture* texture);
		void Set(int slot);
		void Clean(int slot);
	private:
		ComPtr<ID3D11UnorderedAccessView> uav;
	};
	//�u���[�p�X�͏Ȃ��Ă��܂�
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x1280 / y 720��
	//AO���� 9.2ms/��108FPS
	//AO�L�� 10.2ms/��98FPS
	//���1ms
	class SSAO :public PostEffect {
	public:
		//TODO : �𑜓x�������悤�ɂ���
		SSAO(const Math::Vector2& size);
		void CreateAO(DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
	private:
		ALIGN(16) struct Info {
			float offsetPerPixel;
			int useAO;
		};
	private:
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::shared_ptr<Graphics::Texture> rwTexture;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//������ƕs����邩�� (?)
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x640 / y 360��
	//�������Ȃ��� 6.0ms/��165FPS
	//�ڂ����L���� 7.0ms/��138FPS
	//���1.0ms
	class GaussianFilter :public PostEffect {
	public:
		GaussianFilter(const Math::Vector2& size);
		~GaussianFilter() = default;
		//���U���̐ݒ�
		void SetDispersion(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XY�u���[���ʂ�`��
		void Render()override;
		void Begin(int slot);
	private:
		ALIGN(16) struct Info {
			float weight[7];
		};
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::ComputeShader> csX;
		std::unique_ptr<Graphics::ComputeShader> csY;
		std::shared_ptr<Graphics::Texture> rwTexturePass1;
		std::shared_ptr<Graphics::Texture> rwTexturePass2;
		std::unique_ptr<UnorderedAccessView> uavPass1;
		std::unique_ptr<UnorderedAccessView> uavPass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//���U��
		float dispersion;
	};
	//SSAO����肽������
	//deferred�ł̖@���}�b�v�����̂���
	class SceneDeferred :public Scene {
	public:
		SceneDeferred() = default;
		~SceneDeferred();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<DeferredBuffer> deferredBuffer;
#ifdef SIMPLE_SHADER
		std::unique_ptr<DeferredShader> deferredShader;
#endif
#ifdef POINT_LIGHT
		std::unique_ptr<PointLightDeferred> deferredShader;
#endif
#ifdef USE_SSAO
		std::unique_ptr<SSAO> ssao;
#endif
		std::unique_ptr<GaussianFilter> gaussian;
		std::unique_ptr<ShadowBuffer> shadow;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
		//�`��I�u�W�F�N�g
		std::shared_ptr<Graphics::Model> model;
		int normalMap;
		int useLight;
	};
}