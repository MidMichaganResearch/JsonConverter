#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "converter.h"
#include "core/text_utils.h"

namespace fs = std::filesystem;

    namespace {
        /// <summary>
        /// Verifies if the file has a .txt extension.
        /// </summary>
        /// <param name="path"> The path to the file. </param>
        bool isTextFile(const fs::path& path)
        {
            return plotconv::toUpper(path.extension().string()) == ".TXT";
        }

        /// <summary>
        /// Collects all the files from the given input, which can be a file or a folder.
        /// </summary>
        /// <param name="argument"> The path to the file or folder. </param>
        /// <param name="files"> The vector to store the collected file paths. </param>
        bool collectFiles(const fs::path& argument, std::vector<fs::path>& files)
        {
            std::error_code error;

	        // Verifies if the argument is a directory and collects all the text files from it.
            if (fs::is_directory(argument, error))
            {
                std::vector<fs::path> found;

			    // Iterates through the directory entries and adds the text files to the found vector.
                for (const fs::directory_entry& entry : fs::directory_iterator(argument))
                {
                    if (entry.is_regular_file() && isTextFile(entry.path()))
                    {
                        found.push_back(entry.path());
                    }
                }

			    // Sorts the found files and adds them to the files vector.
                std::sort(found.begin(), found.end());
                files.insert(files.end(), found.begin(), found.end());
            
                return true;
            }

	        // Verifies if the argument is a regular file and adds it to the files vector.
            if (fs::is_regular_file(argument, error))
            {
                files.push_back(argument);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Writes the generated content in the created plot file.
        /// </summary>
        /// <param name="path"> The path to the file. </param>
        /// <param name="content"> The content to write to the file. </param>
        bool writeFile(const fs::path& path, const std::string& content)
        {
            // Opens the output file in binary mode and writes the content.
            std::ofstream out(path, std::ios::binary);
            out << content;
            out.close();

            return !out.fail();
        }

        // Prompts the user to enter a file or folder path.
        std::string promptForPath()
        {
            std::cout << "Enter file or folder path:\n";
            std::string input;

            // Reads the input from the user.
            std::getline(std::cin, input);

	        // Trims the input and removes any surrounding quotes if present.
            std::string_view path = plotconv::trim(input);
            if (path.size() >= 2 && path.front() == '"' && path.back() == '"')
            {
                path = path.substr(1, path.size() - 2);
            }
            return std::string(path);
        }
    }

/// <summary>
/// Main entry point for the converter.
/// </summary>
int main(int argc, char* argv[])
{
    std::vector<std::string> arguments;
    
	// Iterates through the command-line arguments and adds them to the arguments vector.
    for (int i = 1; i < argc; ++i)
    {
        arguments.push_back(argv[i]);
    }
    
	// Verifies if no arguments were provided, and if so, prompts the user for a file or folder path.
    if (arguments.empty())
    {
        arguments.push_back(promptForPath());
    }

    std::vector<fs::path> files;

	// Sets the failed counter to zero.
    int failed = 0;

	// Iterates through the provided arguments and collects the files from each argument.
    for (const std::string& argument : arguments)
    {
        if (!collectFiles(argument, files))
        {
            std::cerr << "FAIL  " << argument << ": not a file or folder\n";
            ++failed;
        }
    }

	// Sets the counters for converted and skipped files to zero.
    int converted = 0;
    int skipped = 0;

	// Iterates through the collected files and converts each file using the plotconv::convertFile function.
    for (const fs::path& input : files)
    {
		// Converts the input file and stores the result in the result variable.
        const plotconv::ConversionResult result = plotconv::convertFile(input);

		// Verifies if the conversion result indicates an unsupported file type, and if so, skips the file and increments the skipped counter.
        if (result.status == plotconv::ConversionStatus::UnsupportedFile)
        {
            std::cout << "SKIP  " << input.string() << "  (" << result.message << ")\n";
            ++skipped;
            continue;
        }

		// Verifies if the conversion result indicates an error, and if so, increments the failed counter, and continues to the next file.
        if (!result.ok())
        {
            std::cerr << "FAIL  " << input.string() << ": " << result.message << "\n";
            ++failed;
            continue;
        }

		// Creates the output path by replacing the input file extension with ".json".
        fs::path output = input;
        output.replace_extension(".json");

		// Writes the plot file, counting the file as failed if it cannot be written.
        if (!writeFile(output, result.json))
        {
            std::cerr << "FAIL  " << input.string() << ": cannot write " << output.string() << "\n";
            ++failed;
            continue;
        }

        std::cout << "OK    " << input.string() << " -> " << output.string() << "\n";
        
		// Iterates through the warnings in the conversion result and prints each warning to the console.
        for (const std::string& warning : result.warnings)
        {
            std::cout << "      warning: " << warning << "\n";
        }
        ++converted;
    }

    std::cout << "\n" << converted << " converted, " << skipped << " skipped, " << failed << " failed\n";

    return (failed == 0 && converted > 0) ? 0 : 1;
}