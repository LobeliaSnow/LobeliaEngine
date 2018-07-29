#include "Common/Common.hpp"
#include "Audio/Device/Device.hpp"
#include "Audio/Voice/Voice.hpp"
#include "Audio/Loader/Loader.hpp"
#include "Bank.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Audio {
	void Bank::Load(const char* file_path, const char* tag) {
		std::shared_ptr<Buffer> sound = std::make_shared<Buffer>();
		std::string extenxion = Utility::FilePathControl::GetExtension(file_path);
		std::transform(extenxion.cbegin(), extenxion.cend(), extenxion.begin(), tolower);
		//âπê∫ì«Ç›çûÇ›
		(*loaderMap[extenxion])(file_path, sound.get());
		sounds[tag] = std::move(sound);
	}
	void Bank::Clear(const char* tag) {
		if (sounds.find(tag) != sounds.end())sounds.erase(tag);
	}
}