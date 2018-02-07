#pragma once
#include <memory>
#include <vector>
#include <algorithm>

#ifndef Interface
#define Interface abstract
#endif
//---------------------------------------------------------------------------------------------
//	�����ł͔ėp�����ꂽ�f�U�C���p�^�[������`����Ă���
//---------------------------------------------------------------------------------------------
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//	Observer�p�^�[��
	//---------------------------------------------------------------------------------------------
	//�ʒm�󂯎�� �Ӗ� �ώ@��
	template<class Type, class Key = std::string>class Observer Interface {
		template<class Type, class Key> friend class Observable;
	private:
		//�X�V
		virtual void Update(Type* subject, const Key& key) = 0;
		//�ʒm�\�Ȓl���ۂ��H
		virtual bool ShouldNotify(const Key& key) { return true; };
	};
	//�ʒm�𑗂� �Ӗ� �ϑ��\�� 
	template<class Type, class Key = std::string> class Observable {
	private:
		std::vector<Observer<Type, Key>*> observers;
	public:
		Observable() = default;
		virtual ~Observable() { observers.clear(); }
		//�I�u�U�[�o�[�̒ǉ�
		void AddObserver(Observer<Type, Key>* observer) {
			observers.push_back(observer);
		}
		//�w�肵���I�u�U�[�o�[�̑S���폜
		void DeleteObserver(Observer<Type, Key>* observer) {
			//erase-remove idiom
			//std::remove�ň�v����l��؂�l�߂�B
			//���̍ۂɐ؂�l�߂�ꂽ��̖����̃C�e���[�^�[�Ԃ��Ă���̂ŁA������������č폜
			observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
		}
		//�ʒm
		void Notify(const Key& key) {
			for each(auto& observer in observers) {
				//�ʒm�ł���l���m���߂čX�V����
				if (observer->ShouldNotify(key))observer->Update(static_cast<Type*>(this), key);
			}
		}
	};
	//---------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------

}