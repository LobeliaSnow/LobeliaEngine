#pragma once
namespace Lobelia::Audio {
	class Loader {
	public:
		static void Wav(const char* file_path, Buffer* buffer);
		static void Ogg(const char* file_path, Buffer* buffer);
	};
}