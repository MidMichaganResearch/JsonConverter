#include "SurfaceAnimParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

SurfaceAnimParser::SurfaceAnimParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData SurfaceAnimParser::parse()
{
    PlotData data;
    std::ifstream file(filepath);

    std::string line;
    constexpr int HEADER_LINES = 5;

    int NODES = 0;           //read the nodes from the header
    int COLUMNS = 0;       //set rows/columns below based on file name
    int ROWS_PER_FRAME = 0;

    if (!file.is_open())  return data;        //Not open, so return

    for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);
        if (i == 0) {
            std::string old_word = "THIS FILE CONTAINS";
            std::string new_word = "";

            // Find the starting position of the target word
            size_t pos = line.find(old_word);

            // Check if the word was actually found
            if (pos != std::string::npos) {
                // Parameters: (starting position, character count to erase, replacement string)
                line.replace(pos, old_word.length(), new_word);
            }
            data.title = line;

            //Some titles have "THE" as the first word (in addition to the above) so remove it too
            old_word = "THE";
            new_word = "";
            pos = data.title.find(old_word);
            if (pos != std::string::npos) {
                data.title.replace(pos, old_word.length(), new_word);
            }
            //Now find the period to shorten string
            pos = data.title.find('.');

            // 2. If found, erase from that position to the end of the string
            if (pos != std::string::npos) {
                data.title.erase(pos);
            }

            data.subtitle = "";
        }
        if (i == 4) {
            //get the number of nodes off the data line
            try {
                // Parse the integer from the beginning of the string
                int number = std::stoi(line);
 //               std::cout << "Successfully parsed: " << number << std::endl;
                NODES = number; //
            }
            catch (const std::invalid_argument& e) {
                std::cout << "No valid conversion could be performed." << std::endl;
                return data;
            }
            catch (const std::out_of_range& e) {
                std::cout << "The converted value falls out of the int range." << std::endl;
                return data;
            }
        }
    }
    data.plotType = PlotType::Surface;

    //UPPER CASE FILE PATH
    // Work on an uppercase copy for case-insensitive matching so we don't
    // attempt to modify the const input parameter.

    std::string upper_filepath = filepath;
    for (char& c : upper_filepath) {
        c = std::toupper(static_cast<unsigned char>(c));
    }



    if (upper_filepath.find("FACEPRES") != std::string::npos)
    {
        data.xLabel = "Circumferential Node";
        data.yLabel = "Axial Node";
        data.zLabel = "Pressure";
        data.units = "PSI";
        COLUMNS = NODES;
        ROWS_PER_FRAME = 101;
    }
    else if (upper_filepath.find("FACE_STRESS") != std::string::npos)
    {
        data.xLabel = "Axial Node";
        data.yLabel = "Circumferential Node";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
        COLUMNS = 101;
        ROWS_PER_FRAME = NODES;
    }
    else if (upper_filepath.find("TOP_STRESS") != std::string::npos)
    {
        data.xLabel = "Axial Node";
        data.yLabel = "Circumferential Node";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
        COLUMNS = 101;
        ROWS_PER_FRAME = NODES;
    }
    else if (upper_filepath.find("BOT_STRESS") != std::string::npos)
    {
        data.xLabel = "Axial Node";
        data.yLabel = "Circumferential Node";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
        COLUMNS = 101;
        ROWS_PER_FRAME = NODES;
    }
    else if (upper_filepath.find("BRA_CYCLE") != std::string::npos)
    {
        data.xLabel = "Circumferential Node";
        data.yLabel = "Axial Node";
        data.zLabel = "Pressure";
        data.units = "PSI";
        COLUMNS = NODES;
        ROWS_PER_FRAME = 101;
    }
    else if (upper_filepath.find("TRA_CYCLE") != std::string::npos)
    {
        data.xLabel = "Circumferential Node";
        data.yLabel = "Axial Node";
        data.zLabel = "Pressure";
        data.units = "PSI";
        COLUMNS = NODES;
        ROWS_PER_FRAME = 101;
    }

    while (true)
    {
        std::vector<std::vector<double>> frame;

        bool frameRead = false;

        for (int row = 0;
            row < ROWS_PER_FRAME;
            row++)
        {
            if (!std::getline(file, line))
            {
                break;
            }

            if (line.empty())
            {
                continue;
            }

            frameRead = true;

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

            std::vector<double> values;

            double value;

            while (ss >> value)
            {
                values.push_back(value);
            }

            if (values.size() == COLUMNS)
            {
                frame.push_back(values);
            }
        }

        if (!frameRead)
        {
            break;
        }

        data.frames.push_back(frame);
    }

    if (!data.frames.empty())
    {
        data.matrix =
            data.frames.front();
    }
    /*
    std::cout
        << "TOTAL FRAMES: "
        << data.frames.size()
        << std::endl;
    */
    return data;
}