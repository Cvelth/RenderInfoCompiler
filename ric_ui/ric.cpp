#include "ric_lib/ric.hpp"
#include "ric_lib/version.hpp"
#include <filesystem>
#include <iostream>
void print_help();
std::list<std::string> split_string(std::string const& string, std::string const& separator = "\n");

int main(int argc, char **argv) {
	std::string source = "", destination = "";
	bool override_specifier = true;
	bool recursive_paths = false;
	bool special_call = false;

	for (int i = 1; i < argc; i++) {
		if (auto t = std::string(argv[i]); t == "-version") {
			std::cout << ric::get_version() << '\n';
			special_call = true;
		} else if (auto t = std::string(argv[i]); t == "-help") {
			print_help();
			special_call = true;
		} else if (auto t = std::string(argv[i]); t.substr(0, 3) == "-s=")
			source = t.substr(3);
		else if (auto t = std::string(argv[i]); t.substr(0, 3) == "-d=")
			destination = t.substr(3);
		else if (auto t = std::string(argv[i]); t == "-no-override")
			override_specifier = false;
		else if (auto t = std::string(argv[i]); t == "-recursive-paths")
			recursive_paths = true;
		else
			std::cout << "Unknown parameter '" << argv[i] << "' was ignored.\n";
	}

	if (source == "")
		if (special_call)
			return 0;
		else {
			std::cout << "Source file was not specified. Nothing to compile.\n";
			std::cout << "Call 'ric -help' for usage details.\n";
			return 1;
		}

	if (split_string(destination, ";").size() != 1) {
		std::cout << "Error: <destination_directory> is a single directory path. It cannot contain ';'.\n";
		return -1;
	}
	
	for (auto path : split_string(source, ";")) {
		auto full_path = std::filesystem::current_path().generic_string() + '/' + path;
		if (std::filesystem::exists(full_path)) {
			if (std::filesystem::is_directory(full_path)) {
				if (recursive_paths) {
					for (auto file : std::filesystem::recursive_directory_iterator(std::filesystem::current_path().generic_string() + path))
						if (!std::filesystem::is_directory(file.path())) {
							//file.
							std::cout << file.path().generic_string() << " is to be compiled.\n";
						}
				} else {
					for (auto file : std::filesystem::directory_iterator(std::filesystem::current_path().generic_string() + path))
						if (!std::filesystem::is_directory(file.path())) {
							//file.
							std::cout << file.path().generic_string() << " is to be compiled.\n";
						}
				}
			} else {
				//file
				std::cout << full_path << " is to be compiled.\n";
			}
		} else {
			std::cout << "Error: file(or directory) '" << full_path << "' does not exist. It is skipped.\n";
		}
	}

	//std::getchar();
	return 0;
}
void print_help() {
	std::cout << "Usage: ric [-version] [-help] [-s=<source_paths>] [-d=<destination_directory>]\n"
		<< "        [-recursive-paths] [-no-override] \n\n"
		<< "    <source_paths> - list of path to source files separated with ''.\n"
		<< "    If a path pointers to a file, the file is compiled.\n"
		<< "        If <destination_directory> was specified, resulting file is placed there.\n"
		<< "        Otherwise <current_source_path> is used.\n"
		<< "    If a path pointers to a directory, all the files it contains are compiled.\n"
		<< "        If <destination_directory> was specified, resulting file is placed there.\n"
		<< "        Otherwise <current_source_path> is used.\n\n"
		<< "    If '-no-override' is specified and <destination_directory> already contains file with the same name,\n"
		<< "        a suffix is to be added to file name.\n"
		<< "    If '-recursive-paths' is specified and <destination_directory> already contains file with the same name,\n"
		<< "        a suffix is to be added to file name.\n"
		<< "    <destination_directory> is a single directory path. It cannot contain ''.\n\n"
		<< "    Please take note, that paths with spaces should be enclosed in \"\".\n";
}
std::list<std::string> split_string(std::string const& string, std::string const& separator) {
	std::list<std::string> ret;
	size_t position, last_position = 0u, length = string.length();
	while (last_position < length + 1) {
		position = string.find_first_of(separator, last_position);
		if (position == std::string::npos) position = length;
		ret.push_back(std::string{string.data() + last_position, position - last_position});
		last_position = position + 1;
	}
	return ret;
}