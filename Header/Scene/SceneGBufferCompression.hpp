#pragma once
#include "Common/Camera.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/AdaptiveConsole.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"
#include "Common/PostEffectExperimental.hpp"

namespace Lobelia::Game {
	//���k����G-Buffer�i�[�p
	class GBufferManager {
	public:
		//���ׂĐ��x��float16�Ƃ��ĕۑ�����܂�
		ALIGN(16) struct Info {
			//�n�[�t�����o�[�g�ɑ΂��Ċ|������l
			float lightingFactor;
			//�X�؃L�����̋���
			float specularFactor;
			//�G�~�b�V�����̋���
			float emissionFactor;
		};
	public:
		GBufferManager(const Math::Vector2& size);
		~GBufferManager() = default;
		void AddModel(std::shared_ptr<Graphics::Model>& model, Info info);
		void SetSkybox(std::shared_ptr<class SkyBox>& skybox);
		void RenderGBuffer(std::shared_ptr<Graphics::View>& view);
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GetRTs();
	private:
		struct ModelStorage {
			std::weak_ptr<Graphics::Model> model;
			Info info;
		};
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2> rts;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::shared_ptr<Graphics::BlendState> blend;
		std::shared_ptr<SkyBox> skybox;
		std::list<ModelStorage> modelList;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
	};

	//���k����G-Buffer���g�p���ăV�F�[�f�B���O���s���p
	class DeferredShadeManager {
	public:
		DeferredShadeManager(const Math::Vector2& size);
		~DeferredShadeManager() = default;
		void Render(std::shared_ptr<Graphics::View>& view, std::shared_ptr<GBufferManager>& gbuffer);
	private:
		std::unique_ptr<Graphics::View> viewport;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
	};

	//���k���ꂽG-Buffer���f�R�[�h���A�ʕ\�������s�����肷��
	class GBufferDecodeRenderer {
	public:
		static void Initialize();
		static void Finalize();
		static void ColorRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void DepthRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void WorldPosRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
		static void NormalRender(std::shared_ptr<GBufferManager>& gbuffer, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		static void BufferRender(std::shared_ptr<Graphics::PixelShader>& ps, Graphics::RenderTarget* rt, const Math::Vector2& pos, const Math::Vector2& size);
	private:
		//���󂱂̎O��
		static std::shared_ptr<Graphics::PixelShader> psColor;
		static std::shared_ptr<Graphics::PixelShader> psDepth;
		static std::shared_ptr<Graphics::PixelShader> psWorldPos;
		static std::shared_ptr<Graphics::PixelShader> psNormal;
	};
	//�����J�X�P�[�h�p
	class GaussianTextureArray {
	public:
		GaussianTextureArray(const Math::Vector2& size, int array_count, DXGI_FORMAT format);
		~GaussianTextureArray() = default;
		//�T�C�Y���̕ύX
		void ChangeBuffer(const Math::Vector2& size, int array_count, DXGI_FORMAT format);
		void Update(float dispersion);
		void Dispatch(Graphics::RenderTarget* active_rt, Graphics::View*active_view, Graphics::Texture* tex);
		std::shared_ptr<Graphics::RenderTarget>& GetRTSResult();
	private:
		static constexpr const int DIVISION = 4;
	private:
		ALIGN(16) struct Info {
			float weight[DIVISION];
			int texIndex;
			Math::Vector2 texSize;
		};
	private:
		int arrayCount;
		std::shared_ptr<Graphics::VertexShader> vsX;
		std::shared_ptr<Graphics::VertexShader> vsY;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::RenderTarget> rtsX;
		std::shared_ptr<Graphics::RenderTarget> rtsResult;
		std::unique_ptr<Graphics::View> viewport;
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		Info info;
	};
	//�J�X�P�[�h�V���h�E���������邽�߂̃o�b�t�@
	class CascadeShadowBuffers {
	public:
		enum FORMAT {
			BIT_16,
			BIT_32,
		};
	public:
		//���󕪊���6�������Ή�
		//RenderTarget��6���ō쐬�����ہA�L���[�u�}�b�v�Ƃ��č���Ă���̂Ŕj�]
		//���C�u������蒼���ۂɏC��
		CascadeShadowBuffers(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance);
		~CascadeShadowBuffers() = default;
		//���C�g�J�����p����ݒ�
		void SetNear(float near_z);
		void SetFar(float far_z);
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		//�ΐ������A��l�����̃u�����h��
		void SetLamda(float lamda);
		//�����_�[�^�[�Q�b�g����蒼���܂�
		void ChangeState(int split_count, const Math::Vector2& size, FORMAT format, bool use_variance);
		//�V���h�E�L���A����
		void SetEnable(bool enable);
		/*void SetDispersion(float dispersion);*/
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		//�����Ŏ����I�ɌĂ΂�܂��A��̂̏ꍇ�͖����I�ɌĂԕK�v�͂Ȃ�
		void ClearModels();
		//�V���h�E�}�b�v�ւ̐[�x��������
		void RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//�V���h�E�}�b�v��ʃe�N�X�`���ɏ�������
		void DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size);
		void DebugDataTextureRender(const Math::Vector2& pos, const Math::Vector2& size);
		//�V���h�E�}�b�v���Z�b�g����
		void SetShadowMap(int shadow_map_slot, int data_slot);
	private:
		//����p�p�����[�^
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 lightViewProj;
			Math::Vector4 pos;
			Math::Vector4 front;
			int useShadowMap;
			int useVariance;
			int splitCount;
			int nowIndex;
		};
		struct AABB {
			Math::Vector3 min;
			Math::Vector3 max;
		};
		//�J�X�P�[�h�p�f�[�^
		struct SplitData {
			float splitDist;
			DirectX::XMFLOAT4X4 lvp;
		};
	private:
		//�������AABB�v�Z
		AABB CalcFrustumAABB(Graphics::View* main_camera, float near_z, float far_z, const DirectX::XMFLOAT4X4& lvp);
		//�����v�Z
		void ComputeSplit(float lamda, float near_z, float far_z, float* split_pos);
		//�N���b�v�s��̍쐬
		DirectX::XMFLOAT4X4 FrustumAABBClippingMatrix(AABB clip_aabb);
		//�J�X�P�[�h�Ŏg�p������̌v�Z
		void ComputeCascade(Graphics::View* main_camera, const DirectX::XMFLOAT4X4& lvp);
		//���C�g�̃r���[�v���W�F�N�V�����s����Z�o���A�J�X�P�[�h�̌v�Z
		void Update(Graphics::View* main_camera);
	private:
		//RenderTargetArray
		std::unique_ptr<Graphics::RenderTarget> rts;
		std::unique_ptr<GaussianTextureArray> gaussian;
		//�f�[�^�p�e�N�X�`���̃T�C�Y
		Math::Vector2 dataSize;
		//�f�[�^�������ݗp�e�N�X�`��
		std::unique_ptr<Graphics::Texture> dataTexture;
		//�J�X�P�[�h�Ŏg�p����f�[�^�A�e�N�X�`���ɏ������܂��
		std::vector<SplitData> data;
		//�[�x�o�b�t�@�L�^�p
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		std::unique_ptr<Graphics::View> viewport;
		//std::unique_ptr<GaussianFilterPS> gaussian;
		//�f�o�b�O�`��p
		std::shared_ptr<Graphics::PixelShader> debugPS;
		//�V���h�E�ɂ��Ă̒萔�o�b�t�@
		std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		//����p�����[�^
		Info info;
		//�`�悳��郂�f���̃��X�g
		std::list<std::weak_ptr<Graphics::Model>> models;
		//���C�g�J�����̏��
		float nearZ;
		float farZ;
		float lamda;
		//���C�g�̃g�����X�t�H�[�����
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
	};

	//G-Buffer���k�p�̃V�[���ł�
	class SceneGBufferCompression :public Scene {
	public:
		SceneGBufferCompression() = default;
		~SceneGBufferCompression();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
		std::unique_ptr<Camera> camera;
		std::shared_ptr<GBufferManager> gbuffer;
		std::unique_ptr<DeferredShadeManager> deferredShader;
	public:
		//�f���p�֐�����G���悤��
		std::unique_ptr<CascadeShadowBuffers> shadowMap;
	private:
		std::shared_ptr<Graphics::RenderTarget> offScreen;
		std::shared_ptr<Graphics::RenderTarget> depth;
		std::shared_ptr<Graphics::Model> stage;
		std::shared_ptr<Graphics::Model> box;
		//PostEffect
		std::unique_ptr<Experimental::DepthOfField> dof;
		std::unique_ptr<Experimental::SSAO> ssao;
		std::unique_ptr<AdaptiveConsole> operationConsole;
	public:
		//�f���p
		bool cameraMove;
		//G-Buffer�̕`��t���O
		bool renderGBuffer;
		bool renderColor;
		bool renderDepth;
		bool renderWPos;
		bool renderNormal;
		bool renderSSAO;
		bool renderShadingBuffer;
		bool renderDoFBuffer;
		bool renderShadowMap;
		//ShadowMap�p
		Math::Vector2 shadowMapSize;
		bool useShadow;
		bool useVariance;
		int cascadeCount;
		float shadowNearZ;
		float shadowFarZ;
		float shadowLamda;
		bool useSSAO;
		float ssaoThreshold;
		bool useDoF;
		float focusRange;
		//���f���`��p�p�����[�^�[
		GBufferManager::Info stageInfo;
		GBufferManager::Info boxInfo;
		//�e���C�g�p
		float rad;
		Math::Vector3 lpos;
	};
}