#pragma once
namespace Lobelia::Game {
	ALIGN(16) struct ParaboloidInfo {
		DirectX::XMFLOAT4X4 views[2];
		DirectX::XMFLOAT4X4 projection;
		float zNear;
		float zFar;
		int lighting;
	};
	class DualParaboloidMap {
	public:
		DualParaboloidMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		~DualParaboloidMap() = default;
		Math::Vector3 GetPos();
		float GetRadius();
		Math::Vector2 GetTextureSize();
		void SetPos(const Math::Vector3& pos);
		void SetRadius(float radius);
		void UpdateInfo();
		void Clear(Utility::Color color = 0x00000000);
		std::shared_ptr<Graphics::RenderTarget> GetRenderTarget();
		ParaboloidInfo& GetParaboloidInfo();
		void Activate();
	private:
		//�ʒu
		Math::Vector3 pos;
		//�e���͈�
		float radius;
		//�e�N�X�`���T�C�Y
		Math::Vector2 size;
		//�����������r���[
		std::unique_ptr<Graphics::View> views[2];
		//�f���A���p���{�C�h�}�b�v�����_�����O�p(���Ŕz��Ƃ��ėv�f��2�ō쐬)
		std::shared_ptr<Graphics::RenderTarget> rt;
		//�R���X�^���g�o�b�t�@�p���
		ParaboloidInfo info;
	};
	//�o�Ȗʊ��}�b�v�̃}�l�[�W���[
	class DualParaboloidMapManager {
	public:
		DualParaboloidMapManager();
		~DualParaboloidMapManager();
		//���t���[���Z�b�g���K�v
		void AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting);
		//���}�b�v�ւ̏�������
		void RenderEnvironmentMap();
		//�߂�l���󂯂Ȃ���Γ����ł��N���[�������
		//���悪�������Ă���Ԃ̂ݓ����Ő�������
		std::shared_ptr<DualParaboloidMap> CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius);
		//��ԋ߂��e�����ɂ�����}�b�v���e�N�X�`���X���b�g4�ɃZ�b�g����
		void Activate(const Math::Vector3& pos);
	private:
		void ClearModelList();
	private:
		std::list<std::weak_ptr<DualParaboloidMap>> environmentMaps;
		//�V�F�[�_�[
		std::shared_ptr<Graphics::VertexShader> vs;
		std::shared_ptr<Graphics::HullShader> hs;
		std::shared_ptr<Graphics::DomainShader> ds;
		std::shared_ptr<Graphics::GeometryShader> gs;
		std::shared_ptr<Graphics::PixelShader> ps;
		//�R���X�^���g�o�b�t�@
		std::unique_ptr<Graphics::ConstantBuffer<ParaboloidInfo>> constantBuffer;
		//���f�����X�g
		std::list<std::pair<std::weak_ptr<Graphics::Model>, bool>> models;
#ifdef _DEBUG
	public:
		//�f�o�b�O�p�Ɋ��}�b�v��\��
		void DebugRender();
	private:
		ALIGN(16) struct DebugInfo { int index; };
		DebugInfo debugInfo;
		std::unique_ptr<Graphics::ConstantBuffer<DebugInfo>> debugBuffer;
		std::shared_ptr<Graphics::VertexShader> debugVS;
		std::shared_ptr<Graphics::PixelShader> debugPS;
#endif
	};
}