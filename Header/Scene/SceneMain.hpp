#pragma once
namespace Lobelia::Game {
	//�����X�y�[�X
	//��낤�Ƃ��Ă��邱��
	//�S�����_�����ʂ̃V�F�[�_�[��p���邱�Ƃŋ��ʂ̃}�e���A�����g���܂킹��悤�ɂ��鎖
	//�p�t�H�[�}���X���厖�����A�Ƃ肠�����g���₷���A�ėp�������߂邱�ƂƂ���
	//���ʂ̃p�����[�^��n�������P�[�W�C���X�^���X�����΁A���̑O��̒��_�ϊ��A�V���h�E�L���X�g�ɂ͉e�����Ȃ�
	//�����p���āA�e�����_�����ƂɌŗL�̃V�F�[�_�[�{�}�e���A���ɂ��C���X�^���X�w��ŕ\��
	//DynamicShaderLinkage�ɂ��\��
	//namespace Develop {
	//	class Material {
	//	public:
	//		//�����P�[�W�̃C���X�^���X�p�X�ƁA���O������
	//		Material(const char* file_path, const char* instance_name);

	//	};
	//	//�����_���̒��ۃN���X
	//	class Renderer {
	//	public:
	//		Renderer();
	//		void SetMaterial(std::shared_ptr<Material>& material);
	//		std::shared_ptr<Material>& GetMaterial();
	//		//�e�����_�����Ƃɑ���
	//		virtual void Render() = 0;
	//	protected:
	//		//�V�F�[�_�[����
	//		//�����̓����_�����Ƃɐݒ肳��A�O������ύX�͑z�肳��Ă��Ȃ�
	//		void SetVertexShader(std::shared_ptr<Graphics::VertexShader>& vs);
	//		std::shared_ptr<Graphics::VertexShader>&  GetVertexShader();
	//		void SetPixelShader(std::shared_ptr<Graphics::PixelShader>& ps);
	//		std::shared_ptr<Graphics::PixelShader>&  GetPixelShader();
	//	private:
	//		std::shared_ptr<Material> material;
	//		std::shared_ptr<Graphics::VertexShader> vs;
	//		std::shared_ptr<Graphics::PixelShader> ps;
	//	};
	//	//�S�Ă��X�v���C�g�Ƃ��ĕ`�悷��
	//	class SpriteRenderer {

	//	};
	//	//���_�o�b�t�@������
	//	//������f�[�^��
	//	class Mesh {

	//	};
	//	class Model {

	//	};
	//	//�g�|���W��A���X�^���C�U��������
	//	class MeshFilter {
	//		std::shared_ptr<Mesh> mesh;
	//		D3D11_PRIMITIVE_TOPOLOGY topology;
	//		std::shared_ptr<Graphics::RasterizerState> rs;
	//	};
	//	//�����񂩂�V�F�[�_�[��������\��
	//	//(��)
	//	class ShaderBufferObject {
	//	public:
	//		std::shared_ptr<Graphics::VertexShader> CreateVertexShader(const char* include, const char* vs);
	//		std::shared_ptr<Graphics::PixelShader> CreatePixelShader(const char* include, const char* ps);
	//	};
	//	//���b�V���p�̃����_��
	//	class MeshRenderer :public Renderer {
	//	public:
	//		MeshRenderer();
	//		void SetFilter(std::shared_ptr<MeshFilter> mesh);
	//		void Render()override;
	//	private:
	//	};
	//	//�A�j���[�V�����p�̃����_��
	//	class SkinMeshRenderer :public Renderer {
	//	public:
	//		SkinMeshRenderer();
	//		void Render()override;
	//	};
	//}

#include <dxgi1_6.h>
	class SceneMain :public Lobelia::Scene {
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::Font> font;
		std::shared_ptr<Graphics::RenderTarget> d3d11Texture;
		Microsoft::WRL::ComPtr<IDXGIOutputDuplication> outputDuplication;
		IDXGIOutput1* output;
	public:
		SceneMain();
		~SceneMain();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	};
}