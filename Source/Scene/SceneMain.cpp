#include "Lobelia.hpp"
#include "SceneMain.hpp"

//TODO : �`��X�e�[�g�̃p�C�v���C���N���X�𒲐�
//TODO : �R���g���[���[����
//TODO : �e�N�X�`���u�����h��J��
//TODO : �g�[�����o�͏���(DXGI)�������H
//TODO : deltabase�ł̔�і�����
//Raypick����ۂ�-1����Ă��邱�Ƃ�Y��Ă͂Ȃ�Ȃ��B

namespace Lobelia::Game {
	//�e�N�X�`���ǂݍ��ݗp�ϐ�(�X�}�|�̓_��)
	Graphics::Texture* texture0 = nullptr;
	Graphics::Texture* texture1 = nullptr;
	//�X�v���C�g�N���X
	std::unique_ptr<Graphics::Sprite> sprite;
	//�X�v���C�g�o�b�`�N���X ���̃N���X�͔񐄏�
	std::unique_ptr<Graphics::SpriteBatch> batch;
	//���쎩�@�ʒu
	Math::Vector2 pos(100, 100);

	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
		//�e�N�X�`�����[�h
		Graphics::TextureFileAccessor::Load("test.png", &texture0);
		Graphics::TextureFileAccessor::Load("images.jpg", &texture1);

		//�X���b�g��8�A0~7�܂Ŏg�p�\
		Graphics::SpriteBatchRenderer::GetInstance()->SetTexture(0, texture0);
		Graphics::SpriteBatchRenderer::GetInstance()->SetTexture(1, texture1);

		//�X�v���C�g�N���X�Ƃ��ăe�N�X�`�����[�h
		sprite = std::make_unique<Graphics::Sprite>("test.png");
		//�X�v���C�g�o�b�`�N���X�Ƃ��ăe�N�X�`�����[�h�A��x�ɕ`��ł���ő吔��������
		batch = std::make_unique<Graphics::SpriteBatch>("images.jpg", 10);
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {	}
	void SceneMain::Update() {
		//�L�[���͂ƃf���^�x�[�X�v���O�����̃T���v��
		if (Input::GetKeyboardKey(VK_UP) == 3)pos.y -= 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_DOWN) == 3)pos.y += 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_LEFT) == 3)pos.x -= 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_RIGHT) == 3)pos.x += 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		//�o�b�`�����_���N���X�ւ̓o�^���J�n�B�e�N�X�`���̖���������Ȃ��Ȃ����炱�̃N���X��ʂŃC���X�^���X�����Ďg�p����
		Graphics::SpriteBatchRenderer::GetInstance()->Begin();
		//�����̑O��֌W���`�揇�ɉe��
		Graphics::SpriteBatchRenderer::GetInstance()->Set(1, Math::Vector2(50, 50), texture1->GetSize(), 0.0f, Math::Vector2(), texture1->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteBatchRenderer::GetInstance()->Set(0, pos, texture0->GetSize(), 0.0f, Math::Vector2(), texture0->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteBatchRenderer::GetInstance()->End();
	}
	void SceneMain::Render() {
		view->Activate();
		//�X�v���C�g�����_���N���X�ŕ`��
		Graphics::SpriteRenderer::Render(texture1);
		//�o�b�`�����_���N���X�ŕ`��
		Graphics::SpriteBatchRenderer::GetInstance()->Render();
		//�X�v���C�g�N���X�̋@�\�ŕ`��
		sprite->Render(Math::Vector2(), sprite->GetTexture()->GetSize(), 0.0f, Math::Vector2(), sprite->GetTexture()->GetSize(), 0xFFFFFFFF);
		//�X�v���C�g�o�b�`�N���X�֓o�^ 
		batch->BeginRender();
		//�����߂�ǂ��B�@����
		batch->Render({}, {}, batch->GetMaterial()->GetTexture()->GetSize(), 0.0f, 0.0f, 0xFFFFFFFF);
		batch->EndRender();

	}
}
