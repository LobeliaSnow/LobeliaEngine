#pragma once
namespace Lobelia::Input {
	class Device abstract {
	private:
		HANDLE handle;
	public:
		Device(HANDLE handle);
		~Device();
		HANDLE GetHandle();
	};

}