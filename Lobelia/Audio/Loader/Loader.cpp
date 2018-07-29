#include "Common/Common.hpp"
#include "Audio/Device/Device.hpp"
#include "Audio/Voice/Voice.hpp"
#include "Audio/Loader/Loader.hpp"
#include "Exception/Exception.hpp"

#include "ogg.h"
#include "vorbisfile.h"
#pragma comment(lib,"libogg_static.lib")
#pragma comment(lib,"libvorbis_static.lib")
#pragma comment(lib,"libvorbisfile_static.lib")

namespace Lobelia::Audio {
	void Loader::Wav(const char* file_path, Buffer* buffer) {
		if (!buffer)STRICT_THROW("bufferがnullptrです");
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		char chunkId[5] = {}; char tmp[5] = {};
		fc->Read(chunkId, sizeof(char) * 5, sizeof(char) * 4, 1);
		fc->Read(&buffer->size, sizeof(UINT), sizeof(UINT), 1);
		fc->Read(tmp, sizeof(char) * 5, sizeof(char) * 4, 1);
		if (strcmp(chunkId, "RIFF") || strcmp(tmp, "WAVE"))STRICT_THROW("ファイル形式が違います");
		//子チャンク読み込み
		bool fmtChunk = false, dataChunk = false;
		while (true) {
			fc->Read(chunkId, sizeof(char) * 5, sizeof(char) * 4, 1);
			fc->Read(&buffer->size, sizeof(UINT), sizeof(UINT), 1);
			if (strcmp(chunkId, "fmt ") == 0) {
				if (buffer->size >= sizeof(WAVEFORMATEX)) {
					fc->Read(&buffer->format, sizeof(WAVEFORMATEX), sizeof(WAVEFORMATEX), 1);
					int diff = buffer->size - sizeof(WAVEFORMATEX);
					fc->Seek(diff, Utility::FileController::SeekMode::CURRENT);
				}
				else {
					ZeroMemory(&buffer->format, sizeof(WAVEFORMATEX));
					fc->Read(&buffer->format, sizeof(WAVEFORMATEX), buffer->size, 1);
				}
				fmtChunk = true;
			}
			else if (!strcmp(chunkId, "data")) {
				buffer->source = std::make_unique<BYTE[]>(buffer->size);
				if (fc->Read(buffer->source.get(), sizeof(BYTE)* (buffer->size), sizeof(BYTE), buffer->size) != buffer->size) {
					fc->Close();
					STRICT_THROW("ファイルが壊れている可能性があります");
				}
				dataChunk = true;
			}
			else fc->Seek(buffer->size, Utility::FileController::SeekMode::CURRENT);
			if (fmtChunk&&dataChunk)	break;
		}
		fc->Close();
	}
	void Loader::Ogg(const char* file_path, Buffer* buffer) {
		if (!buffer)STRICT_THROW("bufferがnullptrです");
		OggVorbis_File ovf = {};
		if (ov_fopen(file_path, &ovf) != 0)STRICT_THROW("oggファイルオープンに失敗");
		vorbis_info* info = ov_info(&ovf, -1);
		buffer->format.wFormatTag = WAVE_FORMAT_PCM;
		buffer->format.nChannels = static_cast<WORD>(info->channels);
		buffer->format.nSamplesPerSec = 44100;
		buffer->format.wBitsPerSample = 16;
		buffer->format.nBlockAlign = static_cast<WORD>(info->channels * 16 / 8);
		buffer->format.nAvgBytesPerSec = buffer->format.nSamplesPerSec*buffer->format.nBlockAlign;
		buffer->format.cbSize = 0;
		buffer->size = static_cast<DWORD>(ov_pcm_total(&ovf, -1))* info->channels * 2;
		int bitstream = 0;
		buffer->source = std::make_unique<BYTE[]>(buffer->size);
		DWORD totalReadSize = 0;
		while (totalReadSize < buffer->size) {
			long readSize = ov_read(&ovf, reinterpret_cast<char*>(buffer->source.get()) + totalReadSize, buffer->size - totalReadSize, 0, 2, 1, &bitstream);
			if (readSize < 0) 	STRICT_THROW("oggのでコードに失敗しました");
			totalReadSize += readSize;
		}
		ov_clear(&ovf);
	}
}