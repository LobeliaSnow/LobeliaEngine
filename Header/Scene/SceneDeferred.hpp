#pragma once
#include "../Data/ShaderFile/Define.h"

//�p�t�H�[�}���X�\�L�͂����܂ł��ڈ��A�����Ɉڂ�ς��̂�ڎ��Ŋm�F���������Ȃ̂ƁA
//���������̃m�[�gPC�݂̂Ȃ̂ŐM�����͒Ⴂ
//Compute Shader�łŎ������Ă�����̃p�t�H�[�}���X�v���ׁ̈APixel Shader�ł�����������

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//	Geometry Buffer
	//
	//---------------------------------------------------------------------------------------------
	//GBuffer����
	class DeferredBuffer {
	public:
		enum class BUFFER_TYPE {
			POS,
			NORMAL,
			COLOR,
			VIEW_POS,
			SHADOW,
			MAX,
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
		std::shared_ptr<Graphics::RenderTarget> rts[i_cast(BUFFER_TYPE::MAX)];
		//��񏑂����ݗp
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::list<ModelStorage> models;
		const Math::Vector2 size;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};
	//---------------------------------------------------------------------------------------------
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
	//---------------------------------------------------------------------------------------------
	class SimpleDeferred :public DeferredShader {
	public:
		SimpleDeferred();
	};
	//---------------------------------------------------------------------------------------------
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
	//---------------------------------------------------------------------------------------------
	//���󉓋������������r���̂ŁA�J�X�P�[�h�}�������B
	//Parallel Split Shadow Map (�J�X�P�[�h�n)�����\��
	//�J�X�P�[�h�̎Q�l
	//http://www.project-asura.com/program/d3d11/d3d11_009.html
#ifdef GAUSSIAN_CS
	class GaussianFilterCS;
#endif
#ifdef GAUSSIAN_PS
	class GaussianFilterPS;
#endif
	//��ɍ�蒼��
	class ShadowBuffer {
	public:
		ShadowBuffer(const Math::Vector2& size, int split_count, bool use_variance);
		void AddModel(std::shared_ptr<Graphics::Model> model);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		void CreateShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//�V���h�E�}�b�v���Z�b�g����
		//�e�N�X�`���X���b�g6~6+split_count�Z�b�g����
		//�R���X�^���g�o�b�t�@��
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
		//�̌`���Ńo�C���h�����
		void Begin();
		void End();
		void DebugRender();
	private:
		//void AdjustClipPlanes();
		void ComputeSplit(float lamda, float near_z, float far_z);
		void CameraUpdate();
	private:
		//��ɑ���
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 view;
#ifdef CASCADE
			//4�O��
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
		//near/far�������Ă܂�
		std::vector<float> cascadeValues;
		//�����ʒu�������Ă܂�
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
		const int count;
	};
	//---------------------------------------------------------------------------------------------
	//
	//	Post Effect
	//
	//---------------------------------------------------------------------------------------------
	class PostEffect abstract {
	public:
		PostEffect(const Math::Vector2& size, bool create_rt, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
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
	//---------------------------------------------------------------------------------------------
	class UnorderedAccessView {
	public:
		UnorderedAccessView(Graphics::Texture* texture);
		void Set(int slot);
		void Clean(int slot);
	private:
		ComPtr<ID3D11UnorderedAccessView> uav;
	};
	//---------------------------------------------------------------------------------------------
	//�u���[�p�X�͏Ȃ��Ă��܂�
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x1280 / y 720��
	//AO���� 9.2ms/��108FPS
	//AO�L�� 10.2ms/��98FPS
	//���1ms
	class SSAOCS :public PostEffect {
		friend class SSAOPS;
	public:
		//TODO : �𑜓x�������悤�ɂ���
		SSAOCS(const Math::Vector2& size);
		//�e�N�X�`�����̂܂ܓn���΂��������̕����ɂ���Δėp���͏オ��
		//useAO�I�v�V�����������̎���AO�}�b�v�͍쐬����Ȃ�
		void CreateAO(DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
	private:
		ALIGN(16) struct Info {
			float offsetPerPixel;
			int useAO;
			float offsetPerPixelX;
			float offsetPerPixelY;
		};
	private:
		std::unique_ptr<Graphics::ComputeShader> cs;
		std::shared_ptr<Graphics::Texture> rwTexture;
		std::unique_ptr<UnorderedAccessView> uav;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//�p�t�H�[�}���X��r�\
	class SSAOPS :public PostEffect {
	public:
		SSAOPS(const Math::Vector2& size);
		void CreateAO(Graphics::RenderTarget* active_rt, DeferredBuffer* deferred_buffer);
		//AO�𒼐ڕ`�悷�邱�Ƃ͂Ȃ����߁A�f�o�b�O�`������Ă���
		void Render()override;
		void Begin(int slot);
	private:
		using Info = SSAOCS::Info;
	private:
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//---------------------------------------------------------------------------------------------
	//������ƕs����邩�� (?)
	//Compute Shader�ɂ�����
	//�p�t�H�[�}���X�v������
	//x640 / y 360��
	//�������Ȃ��� 6.0ms/��165FPS
	//�ڂ����L���� 7.0ms/��138FPS
	//���1.0ms
	//����PS�̂ق��������B�Ȃ�ł�B
	class GaussianFilterCS :public PostEffect {
		friend class GaussianFilterPS;
	public:
		GaussianFilterCS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterCS() = default;
		//���U���̐ݒ�
		void SetDispersion(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XY�u���[���ʂ�`��
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		ALIGN(16) struct Info {
			float weight[7];
			float width;
			float height;
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
	class GaussianFilterPS :public PostEffect {
	public:
		GaussianFilterPS(const Math::Vector2& size, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
		~GaussianFilterPS() = default;
		//���U���̐ݒ�
		void SetDispersion(float dispersion);
		//��O���������ۂɂڂ����Ώ�
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, Graphics::Texture* texture);
		void Dispatch(Graphics::View* active_view, Graphics::RenderTarget* active_rt, std::shared_ptr<Graphics::RenderTarget> rt);
		//XY�u���[���ʂ�`��
		void Render()override;
		void Begin(int slot);
		void DebugRender(const Math::Vector2& pos, const Math::Vector2& size);
	private:
		using Info = GaussianFilterCS::Info;
	private:
		std::unique_ptr<Graphics::View> view;
		std::shared_ptr<Graphics::VertexShader> vsX;
		std::shared_ptr<Graphics::VertexShader> vsY;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::RenderTarget> pass2;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
		//���U��
		float dispersion;
	};
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
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
#ifdef FULL_EFFECT
		std::unique_ptr<PointLightDeferred> deferredShader;
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		std::unique_ptr<SSAOPS> ssao;
#endif
#ifdef SSAO_CS
		std::unique_ptr<SSAOCS> ssao;
#endif
#endif
		//std::unique_ptr<GaussianFilterCS> gaussian;
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