#include "DesignPattern.hpp"
#include <Windows.h>

//�����ł͎g�p������J
namespace Lobelia::Game {
	//observer�p�^�[����
	class Sender :public Observable<Sender, int> {
	public:
	};
	class Reciver :public Observer<Sender, int> {
	public:
		void Update(Sender* subject, const int& key) override {
			if (key == 0)OutputDebugString("�҂��[��0(�ʒm��������)\n");
			else if (key == 1)OutputDebugString("�҂��[��1(�ʒm��������)\n");
			else OutputDebugString("�����[(�o�O)\n");
		}
	};
	namespace {
		void UseExample() {
			//observer�p�^�[���Љ�
			Sender sender;
			Reciver reciver;
			sender.AddObserver(&reciver);
			sender.Notify(0);
			sender.Notify(1);
			sender.Notify(2);
			sender.DeleteObserver(&reciver);
			//--end--

		}
	}
}