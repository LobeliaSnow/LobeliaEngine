#pragma once
namespace Lobelia::Utility {
	class FileController {
	public:
		enum class OpenMode { NoOpen = -1, AppendBinaryPlus, WriteBinaryPlus, ReadBinaryPlus, AppendBinary, WriteBinary, ReadBinary, AppendPlus, WritePlus, ReadPlus, Append, Write, Read, };
		enum class SeekMode { FRONT, CURRENT, BACK };
	private:
		OpenMode mode;
		FILE* fp;
	private:
		std::string ModePurse(OpenMode m);
	public:
		FileController()noexcept;
		~FileController()noexcept;
		bool IsAppendMode()const noexcept;
		bool IsWriteMode()const noexcept;
		bool IsReadMode()const noexcept;
		bool IsBinaryMode()const noexcept;
		bool IsTextMode()const noexcept;
		bool IsOpen()const noexcept;
		OpenMode GetMode()const noexcept;
		void Open(std::string filename, OpenMode m);
		void Close()noexcept;
		template<class T> size_t Write(T* pointer, size_t count)noexcept;
		template<class T> size_t Read(T* pointer, size_t buffer_size, size_t read_size, size_t count)noexcept;
		template <class... Args> size_t Print(const char *format, Args&& ... args)noexcept;
		template <class... Args> size_t Scan(const char* pointer, Args&& ... args)noexcept;
		bool Seek(int offset, SeekMode mode);
		size_t GetSize();
		bool Rewind()noexcept;
	};
}

#include "FileController.inl"