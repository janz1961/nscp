#pragma once

#include <boost/filesystem.hpp>
#include <unicode_char.hpp>

namespace file_helpers {
	class checks {
	public:
#ifdef WIN32
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
		static bool is_directory(unsigned long dwAttr) {
			if (dwAttr == INVALID_FILE_ATTRIBUTES) {
 				return false;
			} else if ((dwAttr&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY) {
 				return true;
 			}
 			return false;
 		}
#endif
		static bool is_directory(std::wstring path) {
			return boost::filesystem::is_directory(path);
		}
		static bool is_file(std::wstring path) {
			return boost::filesystem::is_regular(path);
		}
		static bool exists(std::wstring path) {
			return boost::filesystem::exists(path);
		}
	};

	class meta {
	public:
		static boost::filesystem::path get_path(boost::filesystem::path path) {
			return path.parent_path();
		}
		static std::wstring get_filename(boost::filesystem::path path) {
			return path.filename().wstring();
		}
		static std::wstring get_path(std::wstring file) {
			boost::filesystem::path path(file);
			return path.parent_path().wstring();
		}
		static std::wstring get_filename(std::wstring file) {
			boost::filesystem::path path = file;
			return path.filename().wstring();
		}
	};

	class patterns {
	public:
		typedef std::pair<boost::filesystem::path,boost::filesystem::path> pattern_type;

		static pattern_type split_pattern(boost::filesystem::path path) {
			if (boost::filesystem::is_directory(path))
				return pattern_type(path,boost::filesystem::path());
			return pattern_type(path.branch_path(), path.filename());
		}
		static pattern_type split_path_ex(boost::filesystem::path path) {
			if (boost::filesystem::is_directory(path)) {
				return pattern_type(path, boost::filesystem::path());
			}

			std::string spath = path.string();
			std::string::size_type pos = spath.find_last_of('\\');
			if (pos == std::string::npos) {
				pattern_type(spath, boost::filesystem::path("*.*"));
			}
			return pattern_type(spath.substr(0, pos), spath.substr(pos+1));
		}
		static boost::filesystem::path combine_pattern(pattern_type pattern) {
			return pattern.first / pattern.second;
		}
	}; // END patterns
}
