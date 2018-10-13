#pragma once
//�f���p�Ȃ̂�1�̃\�[�X�t�@�C���ɂ܂Ƃ߂Ă���
namespace Lobelia::Game {
	ALIGN(16) struct CubeInfo {
		DirectX::XMFLOAT4X4 views[6];
		DirectX::XMFLOAT4X4 projection;
		//���C�e�B���O���邩�ۂ� if���g�p���邪�A�œK���Ńp�t�H�[�}���X�͕ς��Ȃ�
		int isLighting;
	};
	class CubeEnvironmentMap {
	public:
		CubeEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		~CubeEnvironmentMap() = default;
		Math::Vector3 GetPos();
		float GetRadius();
		Math::Vector2 GetTextureSize();
		void SetPos(const Math::Vector3& pos);
		void SetRadius(float radius);
		void UpdateInfo();
		void Clear(Utility::Color color = 0x00000000);
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget();
		CubeInfo& GetCubeInfo();
		void Activate();
	private:
		//�ʒu
		Math::Vector3 pos;
		//�e���͈�
		float radius;
		//�e�N�X�`���T�C�Y
		Math::Vector2 size;
		//������6��
		std::unique_ptr<Graphics::View> views[6];
		std::shared_ptr<Graphics::RenderTarget> rt;
		//�R���X�^���g�o�b�t�@�p���
		CubeInfo info;
	};
	//���̂����e���v���[�g�ɂ���B����͂قڃp���{���C�h�̃R�s�y
	class CubeEnvironmentMapManager {
	public:
		CubeEnvironmentMapManager();
		~CubeEnvironmentMapManager();
		//���t���[���Z�b�g���K�v
		void AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting);
		//���}�b�v�ւ̏�������
		void RenderEnvironmentMap();
		//�߂�l���󂯂Ȃ���Γ����ł��N���[�������
		//���悪�������Ă���Ԃ̂ݓ����Ő�������
		std::shared_ptr<CubeEnvironmentMap> CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		//��ԋ߂��e�����ɂ�����}�b�v���e�N�X�`���X���b�g4�ɃZ�b�g����
		void Activate(const Math::Vector3& pos);
	private:
		void ClearModelList();
	private:
		std::list<std::weak_ptr<CubeEnvironmentMap>> environmentMaps;
		//�V�F�[�_�[
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//�R���X�^���g�o�b�t�@
		std::unique_ptr<Graphics::ConstantBuffer<CubeInfo>> constantBuffer;
		//���f���̃��X�g(���C�e�B���O�t���O�Ƃ̃y�A)
		std::list<std::pair<std::weak_ptr<Graphics::Model>, bool>> models;
	};
	//�C����p
	ALIGN(16) struct SeaInfo {
		float min;
		float max;
		float maxDevide;
		float height;
		float time;
		float reflectiveRatio;
	};
	class WaterShader {
	public:
		WaterShader();
		void LoadDisplacementMap(const char* file_path);
		void LoadNormalMap(const char* file_path);
		void Activate(std::shared_ptr<Graphics::Model> model);
		void Clean();
	private:
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::HullShader> hs;
		std::shared_ptr<Graphics::DomainShader> ds;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		SeaInfo seaInfo;
		std::unique_ptr<Graphics::ConstantBuffer<SeaInfo>> constantBufferSeaInfo;
		//�f�B�X�v���[�X�����g�}�b�v�Ə̂����n�C�g�}�b�v
		Graphics::Texture* displacement;
		Graphics::Texture* normal;
	};

	class SceneSea :public Lobelia::Scene {
	public:
		SceneSea();
		~SceneSea();
		void Initialize()override;
		void AlwaysUpdate()override;
		void AlwaysRender()override;
	private:
#ifdef __PARABOLOID__
		std::unique_ptr<DualParaboloidMapManager> environmentManager;
		std::shared_ptr<DualParaboloidMap> paraboloidMap;
#else
		std::unique_ptr<CubeEnvironmentMapManager> environmentManager;
		std::shared_ptr<CubeEnvironmentMap> cubeMap;
#endif
		std::unique_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
		std::shared_ptr<Graphics::Model> model;
		std::shared_ptr<Graphics::Model> stage;
		std::shared_ptr<Graphics::Model> skyBox;
		std::unique_ptr<WaterShader> waterShader;
		std::shared_ptr<Graphics::RasterizerState> wireState;
		std::shared_ptr<Graphics::RasterizerState> solidState;
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//�t���O
		int wire;
		int sea;
		D3D11_PRIMITIVE_TOPOLOGY topology;

		Math::Vector3 seaPos;
		Graphics::Texture* caustics;
	};
}