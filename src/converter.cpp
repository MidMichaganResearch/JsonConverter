#include "converter.h"

#include <exception>
#include <fstream>

#include "core/file_header.h"
#include "core/line_reader.h"
#include "core/text_utils.h"
#include "handlers/type_registry.h"

namespace plotconv {

    /// <summary>
	/// Main entry point for the conversion process. The file is opened, the header is read, and the appropriate type handler is invoked.
    /// </summary>
    /// <param name="inputPath"> The path to the input file. </param>
    ConversionResult convertFile(const std::filesystem::path& inputPath)
    {
        try
        {
			// Convert the file name to uppercase for case-insensitive matching.
            const std::string upperName = toUpper(inputPath.filename().string());

			// Iterates over the registered type handlers to determine which one matches the input file name.
            std::vector<const TypeHandler*> matches;
            for (const auto& handler : registeredHandlers())
            {
                if (handler->matches(upperName))
                {
                    matches.push_back(handler.get());
                }
            }

			// Verifies if there are no matching type handlers for the input file name,
            if (matches.empty())
            {
                return ConversionResult::unsupported("no file type matches the name '" + inputPath.filename().string() + "'");
            }

			// Verifies if there are multiple matching type handlers for the input file name,
            if (matches.size() > 1)
            {
                std::string names;
                for (const TypeHandler* handler : matches)
                {
                    names += (names.empty() ? "" : ", ") + std::string(handler->name());
                }
                return ConversionResult::error("the file name matches several types (" + names + ")");
            }

			// Verifies if the input file can be opened for reading.
            std::ifstream input(inputPath);
            if (!input.is_open())
            {
                return ConversionResult::error("cannot open the file");
            }

			// Reads the header of the raw file (title, units and parameters), stopping if it is not valid.
            LineReader reader(input);
            FileHeader header;
            std::string headerError;
            if (!readFileHeader(reader, header, headerError))
            {
                return ConversionResult::error(headerError);
            }

			// If all previous checks pass, the matching type handler is invoked to perform the conversion.
            ConversionContext context{inputPath, header, reader};
            return matches.front()->convert(context);
        }
		// Handles any unexpected exceptions that may occur during the conversion process.
        catch (const std::exception& exception)
        {
            return ConversionResult::error(std::string("unexpected failure: ") + exception.what());
        }
    }
}  
