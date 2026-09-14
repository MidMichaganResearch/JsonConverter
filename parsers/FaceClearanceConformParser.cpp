#include "FaceClearanceConformParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

/// <summary>
/// Class constructor for the Face Clarance Conform Parser.
/// </summary>
/// <param name="path"> Path to the file to parse. </param>
FaceClearanceConformParser::FaceClearanceConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

/// <summary>
/// Parses the Face Clearance Conform file and returns the parsed data.
/// </summary>
PlotData FaceClearanceConformParser::parse()
{
	// Object that holds the parsed data.
    PlotData data;

	// Set the plot type and axis labels for the Face Clearance Conform data.
    data.plotType = PlotType::Surface;
    data.xLabel =
        "Circumferential Node";
    data.yLabel =
        "Output Step";
    data.zLabel =
        "Clearance (in)";

	// Open the file for reading.
    std::ifstream file(filepath);

	// Check if the file is open.
    if (!file.is_open())
    {
        throw std::runtime_error(
            "Cannot open file");
    }

	// Set the variables to hold the number of nodes and the line read from the file.
    std::string line;

    constexpr int HEADER_LINES = 5;

    int NODES = 0;

	// Read the header lines.
    for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);

		// Parse the title from the first line of the header.
        if (i == 0)
        {
            std::string old_word = "THIS FILE CONTAINS THE";
            std::string new_word = "";

            size_t pos = line.find(old_word);

            if (pos != std::string::npos)
            {
                line.replace(pos, old_word.length(), new_word);
            }

            data.title = line;
        }

		// Parse the number of nodes from the fifth line of the header.
        if (i == 4)
        {
            try
            {
                NODES = std::stoi(line);
            }
            catch (const std::invalid_argument&)
            {
                std::cout << "No valid conversion could be performed." << std::endl;
                return data;
            }
            catch (const std::out_of_range&)
            {
                std::cout << "The converted value falls out of the int range." << std::endl;
                return data;
            }
        }
    }

    /*
     * Matrix data
     *
     * NODES gives the expected number of circumferential values per row (columns),
     * read from the header. Each remaining line is one output step (row).
     */
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        for (char& c : line)
        {
            if (c == ',')
            {
                c = ' ';
            }

            if (c == 'D')
            {
                c = 'E';
            }
        }

        std::stringstream ss(line);

        std::vector<double> row;

        double value;

        while (ss >> value)
        {
            row.push_back(value);
        }

        if (!row.empty())
        {
            if (NODES > 0 && static_cast<int>(row.size()) != NODES)
            {
                std::cout
                    << "WARNING: row has "
                    << row.size()
                    << " values, expected "
                    << NODES
                    << std::endl;
            }

            data.matrix.push_back(row);
        }
    }

    return data;
}