#include "Common/Common.hpp"
#include "Audio/Device/Device.hpp"
#include "Audio/Voice/Voice.hpp"
#include "Audio/Loader/Loader.hpp"
#include "Bank.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Audio {
	void Bank::Load(const char* file_path, const char* tag) {
		if (ResourceBank<Player>::IsExist(tag))return;
		Buffer buffer = {};
		std::string extension = Utility::FilePathControl::GetExtension(file_path);
		if (extension == ".wav")Loader::Wav(file_path, &buffer);
		else if (extension == ".ogg")Loader::Ogg(file_path, &buffer);
		//バッファの管理をDataに任せる
		ResourceBank<Player>::Factory(tag, buffer);
	}
	void Bank::Load3D(const char* file_path, const char* tag) {
		if (ResourceBank<Voice3DPlayer>::IsExist(tag))return;
		Buffer buffer = {};
		std::string extension = Utility::FilePathControl::GetExtension(file_path);
		if (extension == ".wav")Loader::Wav(file_path, &buffer);
		else if (extension == ".ogg")Loader::Ogg(file_path, &buffer);
		//バッファの管理をDataに任せる
		ResourceBank<Voice3DPlayer>::Factory(tag, buffer);
	}
	void Bank::Play(const char* tag, UINT loop) { ResourceBank<Player>::Get(tag)->Play(loop); }
	Voice3DHandle Bank::Play3D(const char* tag, const Math::Vector3& pos, const Math::Vector3& front, bool loop) { return ResourceBank<Voice3DPlayer>::Get(tag)->Play(pos, front, loop); }
	void Bank::Stop(const char* tag) { if(ResourceBank<Player>::IsExist(tag))ResourceBank<Player>::Get(tag)->Stop(); }
	bool Bank::Stop3D(const char* tag, Voice3DHandle handle) { return ResourceBank<Voice3DPlayer>::Get(tag)->Stop(handle); }
	void Bank::Pause(const char* tag) { ResourceBank<Player>::Get(tag)->Pause(); }
	bool Bank::Pause3D(const char* tag, Voice3DHandle handle) { return ResourceBank<Voice3DPlayer>::Get(tag)->Pause(handle); }
	bool Bank::IsPlay(const char* tag) { return ResourceBank<Player>::Get(tag)->IsPlay(); }
	bool Bank::IsPlay3D(const char* tag, Voice3DHandle handle) { return ResourceBank<Voice3DPlayer>::Get(tag)->IsPlay(handle); }
	void Bank::SetVolume(const char* tag, float volume) { ResourceBank<Player>::Get(tag)->SetVolume(volume); }
	bool Bank::SetVolume3D(const char* tag, Voice3DHandle handle, float volume) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetVolume(handle, volume); }
	bool Bank::SetPos3D(const char* tag, Voice3DHandle handle, const Math::Vector3& pos) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetPos(handle, pos); }
	bool Bank::SetFrontVector3D(const char* tag, Voice3DHandle handle, const Math::Vector3& front) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetFrontVector(handle, front); }
	bool Bank::SetDopplerScaler3D(const char* tag, Voice3DHandle handle, float scaler) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetDopplerScaler(handle, scaler); }
	bool Bank::SetImpactDistanceScaler3D(const char* tag, Voice3DHandle handle, float scaler) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetImpactDistanceScaler(handle, scaler); }
	bool Bank::SetSpeed3D(const char* tag, Voice3DHandle handle, const Math::Vector3& speed) { return ResourceBank<Voice3DPlayer>::Get(tag)->SetSpeed(handle, speed); }
	Player* Bank::GetData(const char* tag) { return ResourceBank<Player>::Get(tag); }
	Voice3DPlayer* Bank::GetData3D(const char* tag) { return ResourceBank<Voice3DPlayer>::Get(tag); }
	void Bank::Clear() { ResourceBank<Player>::Clear(); }
	void Bank::Clear3D() { ResourceBank<Voice3DPlayer>::Clear(); }
	void Bank::Clear(const char* tag) { ResourceBank<Player>::Erase(tag); }
	void Bank::Clear3D(const char* tag) { ResourceBank<Voice3DPlayer>::Erase(tag); }
	void Bank::Update() {
		auto& resource = ResourceBank<Player>::Get();
		for each(auto& source in resource) {
			source.second->Update();
		}
		auto& resource3d = ResourceBank<Voice3DPlayer>::Get();
		for each(auto& source3d in resource3d) {
			source3d.second->Update();
		}
	}
}