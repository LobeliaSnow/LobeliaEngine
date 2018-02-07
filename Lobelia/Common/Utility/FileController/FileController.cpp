#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
namespace Lobelia::Utility {
	std::string FileController::ModePurse(OpenMode m) {
		switch (m) {
		default:
		case OpenMode::AppendBinaryPlus:	return "ab+";
		case OpenMode::WriteBinaryPlus:		return "wb+";
		case OpenMode::ReadBinaryPlus:		return "rb+";
		case OpenMode::AppendBinary:		return "ab";
		case OpenMode::WriteBinary:			return "wb";
		case OpenMode::ReadBinary:			return "rb";
		case OpenMode::AppendPlus:			return "a+";
		case OpenMode::WritePlus:				return "w+";
		case OpenMode::ReadPlus:				return "r+";
		case OpenMode::Append:					return "a";
		case OpenMode::Write:					return "w";
		case OpenMode::Read:						return "r";
		}
	}
	FileController::FileController()noexcept : fp(nullptr) {}
	FileController::~FileController()noexcept { if (!IsOpen())Close(); }
	bool FileController::IsAppendMode()const noexcept { return (mode == OpenMode::AppendBinaryPlus || mode == OpenMode::AppendBinary || mode == OpenMode::AppendPlus || mode == OpenMode::Append); }
	bool FileController::IsWriteMode()const noexcept { return (mode == OpenMode::WriteBinaryPlus || mode == OpenMode::WriteBinary || mode == OpenMode::WritePlus || mode == OpenMode::Write); }
	bool FileController::IsReadMode()const noexcept { return (mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary || mode == OpenMode::ReadPlus || mode == OpenMode::Read); }
	bool FileController::IsBinaryMode()const noexcept { return (mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary || mode == OpenMode::WriteBinaryPlus || mode == OpenMode::WriteBinary || mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary); }
	bool FileController::IsTextMode()const noexcept { return (mode == OpenMode::ReadPlus || mode == OpenMode::Read || mode == OpenMode::WritePlus || mode == OpenMode::Write || mode == OpenMode::ReadPlus || mode == OpenMode::Read); }
	bool FileController::IsOpen()const noexcept { return (fp != nullptr&&mode != OpenMode::NoOpen); }
	FileController::OpenMode FileController::GetMode()const noexcept { return mode; }
	void FileController::Open(std::string filename, OpenMode m) {
		if (IsOpen())STRICT_THROW("既に開かれているファイルが存在します");
		mode = m;
		fopen_s(&fp, filename.c_str(), ModePurse(m).c_str());
		if (!fp)	STRICT_THROW("ファイルを開けませんでした");
	}
	void FileController::Close()noexcept {
		if (IsOpen()) {
			fclose(fp);
			fp = nullptr;
			mode = OpenMode::NoOpen;
		}
	}
	bool FileController::Seek(int offset, SeekMode mode) {
		if (!IsOpen())return false;
		int origin = -1;
		switch (mode) {
		case SeekMode::FRONT:		origin = SEEK_SET;	break;
		case SeekMode::CURRENT:	origin = SEEK_CUR;	break;
		case SeekMode::BACK:			origin = SEEK_END;	break;
		default:	STRICT_THROW("範囲外の値です");
		}
		return (fseek(fp, offset, origin) == 0);
	}
	size_t FileController::GetSize() {
		if (!IsOpen())return SIZE_T_MAX;
		Seek(0, SeekMode::BACK);
		return ftell(fp);
	}
	bool FileController::Rewind()noexcept {
		if (!IsOpen())return false;
		rewind(fp);
		return true;
	}
}