#pragma once
namespace Lobelia::Graphics {
	//�C���[�W�Ƃ��ẮA���b�V���ɑ΂��ă}�e���A����\��Ƃ����C���[�W�B
	//����̓e�N�X�`���������Ă��邾���̃N���X�����A���͑�����\��B
	//�X�y�L������
	//�e�N�X�`���̃J���[�␳�p�̒萔�o�b�t�@�����\��(Sprite�����̍ۂ���Ɉڍs)
	class Material :private Origin<Material> {
		struct Data {
			Math::Vector4 diffuse = { 1,1,1,1 };//����͎g��Ȃ�
			Math::Vector4 ambient = { 1,1,1,1 };//����͎g��Ȃ�
			Math::Vector4 specular = { 1,1,1,1 };//����͎g��Ȃ�
			Math::Vector4 texColor = { 1,1,1,1 };
		};
	private:
		std::string name;
		Texture* texture;
		Texture* normal;
		Texture* specular;
		Data data;
		bool visible;
		std::unique_ptr<ConstantBuffer<Data>> constantBuffer;
	public:
		Material(const char* name, const char* texture_path);
		~Material();
		const std::string& GetName();
		Texture* GetTexture();

		void SetDiffuseColor(const Math::Vector4& diffuse);
		void SetAmbientColor(const Math::Vector4& ambient);
		void SetSpecularColor(const Math::Vector4& specular);
		void SetTexColor(Utility::Color tex_color);
		void ChangeVisible(bool visible);
		bool IsVisible();
		//���ł��e�N�X�`����؂�ւ����悤�ɁB
		void ChangeTexture(Texture* texture);
		void Set(bool texture = true, bool color = true);
	};
}