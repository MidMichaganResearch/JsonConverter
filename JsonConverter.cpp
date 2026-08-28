#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <string>

#include "factory/parser_factory.h"
#include "builders/json_builder.h"

int main(int argc, char* argv[])
{
    std::string inputFile;

    if (argc > 1)
    {
        inputFile = argv[1];
    }
    else
    {
        std::cout
            << "Enter file path:\n";

        std::getline(
            std::cin,
            inputFile);
    }

    auto parser =
        ParserFactory::create(
            inputFile);

    if (!parser)
    {
        std::cout
            << "\nNo parser available for:\n"
            << inputFile
            << std::endl;

        return 1;
    }

    PlotData data =
        parser->parse();

    std::string json =
        JsonBuilder::build(data);

    namespace fs = std::filesystem;

    std::string outputFile =
        (fs::path(inputFile).parent_path() /
            fs::path(inputFile).stem()).string() + ".json";

    std::ofstream out(
        outputFile);

    if (!out.is_open())
    {
        std::cout
            << "\nFailed creating output file\n";

        return 1;
    }

    out << json;

    out.close();

    std::cout
        << "\nJSON created: "
        << outputFile;

    return 0;
}