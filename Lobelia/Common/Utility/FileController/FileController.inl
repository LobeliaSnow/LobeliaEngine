#pragma once
namespace Lobelia::Utility {
	template<class T> inline size_t FileController::Write(T* pointer, size_t count) noexcept {
		if (!IsOpen() || !(IsWriteMode() || IsAppendMode()))return SIZE_T_MAX;
		return fwrite(pointer, sizeof T, count, fp);
	}
	template<class T> inline size_t FileController::Read(T* pointer, size_t buffer_size, size_t read_size, size_t count) noexcept {
		if (!IsOpen() || !IsReadMode())return SIZE_T_MAX;
		return fread_s(pointer, buffer_size, read_size, count, fp);
	}
	template <class... Args> inline size_t FileController::Print(const char *format, Args&& ...args) noexcept {
		if (!IsOpen() || !(IsWriteMode() || IsAppendMode()))return SIZE_T_MAX;
		return fprintf_s(fp, format, std::forward<Args>(args)...);
	}
	template <class... Args> inline size_t FileController::Scan(const char* format, Args&& ...args) noexcept {
		if (!IsOpen() || !IsReadMode())return SIZE_T_MAX;
		return fscanf_s(fp, format, std::forward<Args>(args)...);
	}

}