#include "Common/Common.hpp"
#include <shlwapi.h>
namespace Lobelia::Utility {
	std::string FilePathControl::GetExtension(const std::string& file_path) {
		//std::tr2::sys::path path(file_path);
		std::experimental::filesystem::path path(file_path);
		if (!path.has_extension())return "";
		return path.extension().string();
	}
	std::string FilePathControl::GetFilename(const std::string& file_path) {
		std::experimental::filesystem::path path(file_path);
		if (!path.has_filename())return "";
		return path.filename().string();
	}
	std::string FilePathControl::GetRootDirectory(const std::string& file_path) {
		std::experimental::filesystem::path path(file_path);
		if (!path.has_root_directory())return "";
		return path.root_directory().string();
	}
	std::string FilePathControl::GetParentDirectory(const std::string& file_path) {
		std::experimental::filesystem::path path(file_path);
		if (!path.has_parent_path())return "";
		return path.parent_path().string();
	}
	std::string FilePathControl::GetRelativeDirectory(const std::string& file_path) {
		std::experimental::filesystem::path path(file_path);
		if (!path.has_relative_path())return "";
		return path.relative_path().string();
	}
	bool FilePathControl::IsFileOrPath(const std::string& path) {
		return std::experimental::filesystem::exists(path.c_str());
	}
}