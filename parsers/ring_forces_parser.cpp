#include "ring_forces_parser.h"

#include <fstream>
#include <sstream>

RingForcesParser::RingForcesParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData RingForcesParser::parse()
{
    PlotData data;

    std::ifstream file(filepath);
    std::string line;
    constexpr int HEADER_LINES = 5;
    constexpr int COLUMNS = 4;

    if (!file.is_open())  return data;        //Not open, so return


    data.plotType = PlotType::Scatter;

    data.title =        "Cyclic Ring:  ";
    data.xLabel =        "Crank Angle";
    data.yLabel =        "Force";
    data.units =        "N";

    //has 3 values per crank angle, so create a series of data for each plot
    Series gas;
    gas.name = "Gas Pressure Force";
    Series friction;
    friction.name = "Friction Force";
    Series inertial;
    inertial.name = "Inertial Force";


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

            //Set title
            data.title = data.title+line;
            data.subtitle = "";
        }
    }


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

        double angle;
        double gasValue;
        double frictionValue;
        double inertialValue;

        if (!(ss >>
            angle >>
            gasValue >>
            frictionValue >>
            inertialValue))
        {
            continue;
        }

        gas.x.push_back(angle);
        gas.y.push_back(gasValue);

        friction.x.push_back(angle);
        friction.y.push_back(frictionValue);

        inertial.x.push_back(angle);
        inertial.y.push_back(inertialValue);
    }

    data.series.push_back(gas);
    data.series.push_back(friction);
    data.series.push_back(inertial);

    return data;
}