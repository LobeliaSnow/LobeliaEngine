#pragma once
#include "Common/Camera.hpp"
#include "../Data/ShaderFile/Define.h"
#include "Common/AdaptiveConsole.hpp"

namespace Lobelia::Game {
	//���k����G-Buffer�i�[�p
	class GBufferManager {
	public:
		enum class MATERIAL_TYPE :int {
			LAMBERT = MAT_LAMBERT,
			PHONG = MAT_PHONG,
			COLOR = MAT_COLOR,
		};
	public:
		GBufferManager(const Math::Vector2& size);
		~GBufferManager() = default;
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		void SetSkybox(std::shared_ptr<class SkyBox>& skybox);
		void RenderGBuffer(std::shared_ptr<Graphics::View>& view);
		std::array<std::unique_ptr<Graphics::RenderTarget>, 2>& GetRTs();
	private:
		ALIGN(16) struct Info {
			MATERIAL_TYPE materialType;
			float specularFactor;
			float emissionFactor;
			float transparent;
		};
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
		std::list<std::weak_ptr<Graphics::Model>> modelList;
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

	//�J�X�P�[�h�V���h�E���������邽�߂̃o�b�t�@
	class CascadeShadowBuffers {
	public:
		enum FORMAT {
			BIT_16,
			BIT_32,
		};
	public:
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
		void AddModel(std::shared_ptr<Graphics::Model>& model);
		//�����Ŏ����I�ɌĂ΂�܂��A��̂̏ꍇ�͖����I�ɌĂԕK�v�͂Ȃ�
		void ClearModels();
		//�V���h�E�}�b�v�ւ̐[�x��������
		void RenderShadowMap(Graphics::View* active_view, Graphics::RenderTarget* active_rt);
		//�V���h�E�}�b�v��ʃe�N�X�`���ɏ�������
		void DebugShadowMapRender(int index, const Math::Vector2& pos, const Math::Vector2& size);
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
		std::shared_ptr<Graphics::Model> stage;
		std::unique_ptr<AdaptiveConsole> operationConsole;
	public:
		//�f���p
		//G-Buffer�̕`��t���O
		bool renderGBuffer;
		bool renderColor;
		bool renderDepth;
		bool renderWPos;
		bool renderNormal;
		//ShadowMap�p
		Math::Vector2 shadowMapSize;
		bool useShadow;
		bool useVariance;
		bool renderShadowMap;
		int cascadeCount;
		float shadowNearZ;
		float shadowFarZ;
		float shadowLamda;
		float rad;
		Math::Vector3 lpos;
	};
}