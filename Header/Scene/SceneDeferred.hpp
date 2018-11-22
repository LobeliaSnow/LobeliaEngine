#pragma once
#include "../Data/ShaderFile/Define.h"
#include "Common/SkyBox.hpp"
#include "Common/DeferredBuffer.hpp"
#include "Common/DeferredShader.hpp"
#include "Common/ShadowBuffer.hpp"
#include "Common/ComputeBuffer.hpp"
#include "Common/PostEffect.hpp"
#include "Common/Raycaster.hpp"
#include "Common/Camera.hpp"
#include "Common/Character.hpp"

//�p�t�H�[�}���X�\�L�͂����܂ł��ڈ��A�����Ɉڂ�ς��̂�ڎ��Ŋm�F���������Ȃ̂ƁA
//���������̃m�[�gPC�݂̂Ȃ̂ŐM�����͒Ⴂ

namespace Lobelia::Game {
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
		std::shared_ptr<Camera> camera;
		std::unique_ptr<Graphics::RenderTarget> rt;
		//std::unique_ptr<Graphics::View> view;
		std::unique_ptr<DeferredBuffer> deferredBuffer;
#ifdef SIMPLE_SHADER
		std::unique_ptr<DeferredShader> deferredShader;
#endif
#ifdef FULL_EFFECT
		std::unique_ptr<FullEffectDeferred> deferredShader;
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
#ifdef USE_DOF
		std::unique_ptr<DepthOfField> dof;
#endif
		Math::Vector3 lpos;
		float rad;
		//Math::Vector3 pos;
		//Math::Vector3 at;
		//Math::Vector3 up;
		//�`��I�u�W�F�N�g
		std::shared_ptr<Graphics::Model> stage;
		std::shared_ptr<Graphics::Model> box;
		std::shared_ptr<Graphics::Model> stageCollision;
#ifdef USE_CHARACTER
		std::shared_ptr<Character> character;
		//���C����p�I�u�W�F�N�g
		std::shared_ptr<RayMesh> rayMesh;
#endif
#ifdef USE_MOTION_BLUR
		std::unique_ptr<SSMotionBlur> motionBlur;
#endif
		std::shared_ptr<SkyBox> skybox;
		//�`�搧��p�p�����[�^�[
		int useLight;
		int useFog;
		int useMotionBlur;
	};
}