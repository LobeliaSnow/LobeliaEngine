#pragma once
#include <memory>
namespace Hdx {
	/**
	*@file Scene.h
	*@brief �V�[�����N���X(�C���^�[�t�F�[�X �N���X)
	*@author Lobelia_Snow
	*/

	/**
	*@brief �V�[���̊��N���X
	*/
	class Scene Interface {
	public:
		/**@brief �R���X�g���N�^*/
		Scene() = default;
		/**@brief �f�X�g���N�^*/
		virtual ~Scene() = default;
		/**@brief ���������� �������z�֐�*/
		virtual bool Initialize() = 0;
		/**@brief �X�V���� �������z�֐�*/
		virtual void Update() = 0;
		/**@brief �`�揈�� �������z�֐�*/
		virtual void Render() = 0;

	};
}