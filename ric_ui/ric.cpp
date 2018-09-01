#include "ric_lib/ric.hpp"
#include "ric_lib/version.hpp"
#include <filesystem>
#include <iostream>
void print_help();

int main(int argc, char **argv) {
	std::string source = "", destination = "";
	bool override_specifier = true;
	bool special_call = false;

	for (int i = 1; i < argc; i++){
		if (auto t = std::string(argv[i]); t == "-version") {
			std::cout << ric::get_version() << '\n';
			special_call = true;
		} else if (auto t = std::string(argv[i]); t == "-help") {
			print_help();
			special_call = true;
		} else if (auto t = std::string(argv[i]); t.substr(0, 2) == "-s")
			source = t.substr(2);
		else if (auto t = std::string(argv[i]); t.substr(0, 2) == "-d")
			destination = t.substr(2);
		else if (auto t = std::string(argv[i]); t == "-no-override")
			override_specifier = false;
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

	//Compilation here.
	return 0;
}
void print_help() {
	std::cout << "Usage: ric [-version] [-help] [-s=<source_paths>] [-d=<destination_directory>] [-no-override] \n";
	std::cout << "    <source_paths> - list of path to source files separated with ';'.\n";
	std::cout << "    If a path pointers to a file, the file is compiled.\n";
	std::cout << "        If <destination_directory> was specified, resulting file is placed there.\n";
	std::cout << "        Otherwise <current_source_path> is used.\n";
	std::cout << "    If a path pointers to a directory, all the files it contains are compiled.\n";
	std::cout << "        If <destination_directory> was specified, resulting file is placed there.\n";
	std::cout << "        Otherwise <current_source_path> is used.\n";
	std::cout << "    If '-no-override' is specified and <destination_directory> already contains file with the same name,\n";
	std::cout << "        a suffix is to be added to file name.\n";
	std::cout << "    <destination_directory> is a single directory path. It cannot contain ';'.\n";
}